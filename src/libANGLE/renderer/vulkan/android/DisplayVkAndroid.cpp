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

#include "common/android_util.h"
#include "common/version.h"
#include "libANGLE/renderer/vulkan/RendererVk.h"
#include "libANGLE/renderer/vulkan/android/HardwareBufferImageSiblingVkAndroid.h"
#include "libANGLE/renderer/vulkan/android/WindowSurfaceVkAndroid.h"
#include "libANGLE/renderer/vulkan/vk_caps_utils.h"

namespace rx
{

DisplayVkAndroid::DisplayVkAndroid(const egl::DisplayState &state) : DisplayVk(state) {}

egl::Error DisplayVkAndroid::initialize(egl::Display *display)
{
    ANGLE_TRY(DisplayVk::initialize(display));

    std::stringstream strstr;
    strstr << "Version (" << ANGLE_VERSION_STRING << "), ";
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
    // TODO (Issue 4062): Add conditional support for GL_RGB10_A2 and GL_RGBA16F when the
    // Android Vulkan loader adds conditional support for them.
    // constexpr GLenum kColorFormats[] = {GL_RGBA8, GL_RGB8, GL_RGB565};
    struct FormatMap
    {
        uint32_t vkFormat;
        GLenum glFormat;
    };
    std::vector<FormatMap> androidTargetColorFormats(
        {{AHARDWAREBUFFER_FORMAT_R8G8B8A8_UNORM, GL_RGBA8},
         {AHARDWAREBUFFER_FORMAT_R8G8B8_UNORM, GL_RGB8},
         {AHARDWAREBUFFER_FORMAT_R5G6B5_UNORM, GL_RGB565},
         {AHARDWAREBUFFER_FORMAT_R10G10B10A2_UNORM, GL_RGB10_A2},
         {AHARDWAREBUFFER_FORMAT_R16G16B16A16_FLOAT, GL_RGBA16F}});
    std::vector<GLenum> availableTargetColorFormats;
    for (FormatMap formatCombo : androidTargetColorFormats)
    {
        if (angle::android::AHardwareBufferFormatIsSupported(formatCombo.vkFormat))
        {
            availableTargetColorFormats.push_back(formatCombo.glFormat);
        }
    }

    std::vector<FormatMap> androidTargetDepthFormats({
        {AHARDWAREBUFFER_FORMAT_D24_UNORM_S8_UINT, GL_DEPTH24_STENCIL8},
        {AHARDWAREBUFFER_FORMAT_D24_UNORM, GL_DEPTH_COMPONENT24},
        {AHARDWAREBUFFER_FORMAT_D16_UNORM, GL_DEPTH_COMPONENT16},
    });
    std::vector<GLenum> availableTargetDepthFormats({GL_NONE});
    for (FormatMap formatCombo : androidTargetDepthFormats)
    {
        if (angle::android::AHardwareBufferFormatIsSupported(formatCombo.vkFormat))
        {
            availableTargetDepthFormats.push_back(formatCombo.glFormat);
        }
    }

    return egl_vk::GenerateConfigs(
        availableTargetColorFormats.data(), availableTargetColorFormats.size(),
        availableTargetDepthFormats.data(), availableTargetDepthFormats.size(), this);
}

bool DisplayVkAndroid::checkConfigSupport(egl::Config *config)
{
    // TODO(geofflang): Test for native support and modify the config accordingly.
    // anglebug.com/2692
    return true;
}

egl::Error DisplayVkAndroid::validateImageClientBuffer(const gl::Context *context,
                                                       EGLenum target,
                                                       EGLClientBuffer clientBuffer,
                                                       const egl::AttributeMap &attribs) const
{
    switch (target)
    {
        case EGL_NATIVE_BUFFER_ANDROID:
            return HardwareBufferImageSiblingVkAndroid::ValidateHardwareBuffer(mRenderer,
                                                                               clientBuffer);

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
