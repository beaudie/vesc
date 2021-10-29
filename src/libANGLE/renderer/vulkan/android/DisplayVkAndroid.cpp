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
#    include <android/log.h>
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
#define OLD_CODE
#ifdef OLD_CODE
    // The list of supported swapchain formats is available at:
    // https://cs.android.com/android/platform/superproject/+/master:frameworks/native/vulkan/libvulkan/swapchain.cpp;l=465-486?q=GetNativePixelFormat
    // TODO (Issue 4062): Add conditional support for GL_RGB10_A2 and GL_RGBA16F when the
    // Android Vulkan loader adds conditional support for them.
    const std::array<GLenum, 2> kColorFormats = {GL_RGBA8, GL_RGB565};
#else  // OLD_CODE
    INFO("%s(): GOT TO HERE 1", __FUNCTION__);
    std::vector<GLenum> kColorFormats = {GL_RGBA8, GL_RGB565};
    INFO("%s(): GOT TO HERE 1.1", __FUNCTION__);
    // TODO(ianelliott): Define a Vulkan extension and enable it in RendererVk::initialize().  For
    // now, call the old function with a VK_NULL_HANDLE for the VkSurfaceKHR handle.
    INFO("%s(): GOT TO HERE 1.2", __FUNCTION__);
    VkPhysicalDeviceSurfaceInfo2KHR surfaceInfo2 = {};
    surfaceInfo2.sType   = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_SURFACE_INFO_2_KHR;
    surfaceInfo2.surface = VK_NULL_HANDLE;
    INFO("%s(): GOT TO HERE 1.3", __FUNCTION__);

    uint32_t surfaceFormatCount     = 0;
    VkPhysicalDevice physicalDevice = mRenderer->getPhysicalDevice();
    INFO("%s(): GOT TO HERE 1.4", __FUNCTION__);
    VkResult /*Hack*/ result = vkGetPhysicalDeviceSurfaceFormats2KHR(physicalDevice, &surfaceInfo2,
                                                                     &surfaceFormatCount, nullptr);
    INFO("%s(): GOT TO HERE 2", __FUNCTION__);
    if (result != VK_SUCCESS)
        return egl::ConfigSet();

    std::vector<VkSurfaceFormat2KHR> surfaceFormats2(surfaceFormatCount);
    for (VkSurfaceFormat2KHR &surfaceFormat2 : surfaceFormats2)
    {
        surfaceFormat2.sType = VK_STRUCTURE_TYPE_SURFACE_FORMAT_2_KHR;
    }
    /*Hack*/ result = vkGetPhysicalDeviceSurfaceFormats2KHR(
        physicalDevice, &surfaceInfo2, &surfaceFormatCount, surfaceFormats2.data());
    if (result != VK_SUCCESS)
        return egl::ConfigSet();
    INFO("%s(): GOT TO HERE 3", __FUNCTION__);

    for (VkSurfaceFormat2KHR &surfaceFormat2 : surfaceFormats2)
    {
        INFO("%s(): GOT TO HERE 3.1", __FUNCTION__);
        if (std::find(kColorFormats.begin(), kColorFormats.end(),
                      surfaceFormat2.surfaceFormat.format) == kColorFormats.end())
        {
            INFO("%s(): GOT TO HERE 3.1.1", __FUNCTION__);
            kColorFormats.push_back(surfaceFormat2.surfaceFormat.format);
        }

        // TODO: Look at surfaceFormat2.surfaceFormat.colorSpace too
    }

#endif  // OLD_CODE
    INFO("%s(): GOT TO HERE 4", __FUNCTION__);

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
