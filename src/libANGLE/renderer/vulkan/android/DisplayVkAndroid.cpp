//
// Copyright 2018 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//
// DisplayVkAndroid.cpp:
//    Implements the class methods for DisplayVkAndroid.
//

#include "libANGLE/renderer/vulkan/android/DisplayVkAndroid.h"

#include <android/log.h>
#include <android/native_window.h>
#include <vulkan/vulkan.h>

#include "common/angle_version_info.h"
#include "libANGLE/renderer/driver_utils.h"
#include "libANGLE/renderer/vulkan/RendererVk.h"
#include "libANGLE/renderer/vulkan/android/HardwareBufferImageSiblingVkAndroid.h"
#include "libANGLE/renderer/vulkan/android/WindowSurfaceVkAndroid.h"
#include "libANGLE/renderer/vulkan/vk_caps_utils.h"

#ifdef OLD_CODE
#else  // OLD_CODE
#    include <unistd.h>
#    undef INFO
#    define INFO(...) __android_log_print(ANDROID_LOG_INFO, "ANGLE", __VA_ARGS__)
#endif  // OLD_CODE

namespace rx
{

DisplayVkAndroid::DisplayVkAndroid(const egl::DisplayState &state) : DisplayVk(state) {}

egl::Error DisplayVkAndroid::initialize(egl::Display *display)
{
    ANGLE_TRY(DisplayVk::initialize(display));

    std::stringstream strstr;
    strstr << "Version (" << angle::GetANGLEVersionString() << "), ";
    strstr << "Renderer (" << mRenderer->getRendererDescription() << ")";
    __android_log_print(ANDROID_LOG_INFO, "ANGLE", "%s", strstr.str().c_str());

    return egl::NoError();
}

bool DisplayVkAndroid::isValidNativeWindow(EGLNativeWindowType window) const
{
    return (ANativeWindow_getFormat(window) >= 0);
}

SurfaceImpl *DisplayVkAndroid::createWindowSurfaceVk(const egl::SurfaceState &state,
                                                     EGLNativeWindowType window)
{
    return new WindowSurfaceVkAndroid(state, window);
}

egl::ConfigSet DisplayVkAndroid::generateConfigs()
{
    // ANGLE's Vulkan back-end on Android traditionally supports EGLConfig's with GL_RGBA8,
    // GL_RGB8, and GL_RGB565.  The Android Vulkan loader used to support all three of these
    // (e.g. Android 7), but this has changed as Android now supports Vulkan devices that do not
    // support all of those formats.  The loader always supports GL_RGBA8.  Other formats are
    // optionally supported, depending on the underlying driver support.  This includes GL_RGB10_A2
    // and GL_RGBA16F, which ANGLE also desires to support EGLConfig's with.
    //
    // The problem for ANGLE is that Vulkan requires a VkSurfaceKHR in order to query available
    // formats from the loader, but ANGLE must determine which EGLConfig's to expose before it has
    // a VkSurfaceKHR.  The VK_GOOGLE_surfaceless_query extension allows ANGLE to query formats
    // without having a VkSurfaceKHR.  The old path is still kept until this extension becomes
    // universally available.

    // Assume GL_RGB8 and GL_RGBA8 is always available.
    std::vector<GLenum> kColorFormats = {GL_RGBA8, GL_RGB8};
    if (!getRenderer()->getFeatures().supportsSurfacelessQueryExtension.enabled)
    {
        // Old path: Assume GL_RGB565 is available, as it is generally available on the devices
        // that support Vulkan.
        kColorFormats.push_back(GL_RGB565);
    }
    else
    {
        // Use the VK_GOOGLE_surfaceless_query extension to query the available formats and
        // colorspaces by using a VK_NULL_HANDLE for the VkSurfaceKHR handle.
        VkPhysicalDevice physicalDevice              = mRenderer->getPhysicalDevice();
        VkPhysicalDeviceSurfaceInfo2KHR surfaceInfo2 = {};
        surfaceInfo2.sType          = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_SURFACE_INFO_2_KHR;
        surfaceInfo2.surface        = VK_NULL_HANDLE;
        uint32_t surfaceFormatCount = 0;

        VkResult result = vkGetPhysicalDeviceSurfaceFormats2KHR(physicalDevice, &surfaceInfo2,
                                                                &surfaceFormatCount, nullptr);
        INFO("%s(): surfaceFormatCount = %u", __FUNCTION__, surfaceFormatCount);
        if (result != VK_SUCCESS)
        {
            return egl::ConfigSet();
        }
        std::vector<VkSurfaceFormat2KHR> surfaceFormats2(surfaceFormatCount);
        for (VkSurfaceFormat2KHR &surfaceFormat2 : surfaceFormats2)
        {
            surfaceFormat2.sType = VK_STRUCTURE_TYPE_SURFACE_FORMAT_2_KHR;
        }
        result = vkGetPhysicalDeviceSurfaceFormats2KHR(physicalDevice, &surfaceInfo2,
                                                       &surfaceFormatCount, surfaceFormats2.data());
        if (result != VK_SUCCESS)
        {
            return egl::ConfigSet();
        }

        for (VkSurfaceFormat2KHR &surfaceFormat2 : surfaceFormats2)
        {
            // Need to convert each Vulkan VkFormat into its GLES equivalent and compare with
            // kColorFormats.
            angle::FormatID angleFormatID =
                vk::GetFormatIDFromVkFormat(surfaceFormat2.surfaceFormat.format);
            const angle::Format &angleFormat = angle::Format::Get(angleFormatID);
            GLenum glFormat                  = angleFormat.glInternalFormat;

            INFO("%s(): GLformat = 0x%04x RGBA(%u, %u, %u, %u); DS(%u, %u)", __FUNCTION__, glFormat,
                 angleFormat.redBits, angleFormat.greenBits, angleFormat.blueBits,
                 angleFormat.alphaBits, angleFormat.depthBits, angleFormat.stencilBits);

            if (glFormat == GL_SRGB8_ALPHA8_EXT ||
                (angleFormat.greenBits == 0 && angleFormat.blueBits == 0))
            {
                // ANGLE won't use GL_SRGB8_ALPHA8_EXT for an EGLConfig (it uses GL_RGBA8 instead).
                // Also, ANGLE is not ready to support the recently-added-to-the-Android-loader
                // GL_RED format.
                INFO("%s(): \t Skipping this format!", __FUNCTION__);
                continue;
            }
            if (std::find(kColorFormats.begin(), kColorFormats.end(), glFormat) ==
                kColorFormats.end())
            {
                INFO("%s(): \t Adding this format!", __FUNCTION__);
                kColorFormats.push_back(glFormat);
                continue;
            }
            INFO("%s(): \t Did not take a path above", __FUNCTION__);
        }
    }

    std::vector<GLenum> depthStencilFormats(
        egl_vk::kConfigDepthStencilFormats,
        egl_vk::kConfigDepthStencilFormats + ArraySize(egl_vk::kConfigDepthStencilFormats));

    if (getCaps().stencil8)
    {
        depthStencilFormats.push_back(GL_STENCIL_INDEX8);
    }
    return egl_vk::GenerateConfigs(kColorFormats.data(), kColorFormats.size(),
                                   depthStencilFormats.data(), depthStencilFormats.size(), this);
}

void DisplayVkAndroid::enableRecordableIfSupported(egl::Config *config)
{
    // TODO(b/181163023): Determine how to properly query for support. This is a hack to unblock
    // launching SwANGLE on Cuttlefish.
    // anglebug.com/6612: This is also required for app compatiblity.
    config->recordable = true;
}

void DisplayVkAndroid::checkConfigSupport(egl::Config *config)
{
    // TODO(geofflang): Test for native support and modify the config accordingly.
    // anglebug.com/2692

    enableRecordableIfSupported(config);
}

egl::Error DisplayVkAndroid::validateImageClientBuffer(const gl::Context *context,
                                                       EGLenum target,
                                                       EGLClientBuffer clientBuffer,
                                                       const egl::AttributeMap &attribs) const
{
    switch (target)
    {
        case EGL_NATIVE_BUFFER_ANDROID:
            return HardwareBufferImageSiblingVkAndroid::ValidateHardwareBuffer(
                mRenderer, clientBuffer, attribs);

        default:
            return DisplayVk::validateImageClientBuffer(context, target, clientBuffer, attribs);
    }
}

ExternalImageSiblingImpl *DisplayVkAndroid::createExternalImageSibling(
    const gl::Context *context,
    EGLenum target,
    EGLClientBuffer buffer,
    const egl::AttributeMap &attribs)
{
    switch (target)
    {
        case EGL_NATIVE_BUFFER_ANDROID:
            return new HardwareBufferImageSiblingVkAndroid(buffer);

        default:
            return DisplayVk::createExternalImageSibling(context, target, buffer, attribs);
    }
}

const char *DisplayVkAndroid::getWSIExtension() const
{
    return VK_KHR_ANDROID_SURFACE_EXTENSION_NAME;
}

bool IsVulkanAndroidDisplayAvailable()
{
    return true;
}

DisplayImpl *CreateVulkanAndroidDisplay(const egl::DisplayState &state)
{
    return new DisplayVkAndroid(state);
}
}  // namespace rx
