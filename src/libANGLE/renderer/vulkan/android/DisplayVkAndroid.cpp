//
// Copyright 2018 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//
// DisplayVkAndroid.cpp:
//    Implements the class methods for DisplayVkAndroid.
//

#include "libANGLE/renderer/vulkan/android/DisplayVkAndroid.h"

#include <android/native_window.h>
#include <vulkan/vulkan.h>

#include "libANGLE/renderer/vulkan/android/WindowSurfaceVkAndroid.h"

namespace rx
{

DisplayVkAndroid::DisplayVkAndroid(const egl::DisplayState &state) : DisplayVk(state)
{
}

bool DisplayVkAndroid::isValidNativeWindow(EGLNativeWindowType window) const
{
    return (ANativeWindow_getFormat(window) >= 0);
}

SurfaceImpl *DisplayVkAndroid::createWindowSurfaceVk(const egl::SurfaceState &state,
                                                     EGLNativeWindowType window,
                                                     EGLint width,
                                                     EGLint height)
{
    return new WindowSurfaceVkAndroid(state, window, width, height);
}

egl::ConfigSet DisplayVkAndroid::generateConfigs()
{
    const GLenum colorFormats[]        = {GL_RGBA8, GL_RGB8};
    const GLenum depthStencilFormats[] = {GL_NONE, GL_DEPTH24_STENCIL8};
    const EGLint sampleCounts[]        = {0};
    return egl_vk::GenerateConfigs(colorFormats, depthStencilFormats, sampleCounts, this);
}

bool DisplayVkAndroid::checkConfigSupport(egl::Config *config)
{
    // TODO(geofflang): Test for native support and modify the config accordingly.
    // anglebug.com/2692
    return true;
}

const char *DisplayVkAndroid::getWSIName() const
{
    return VK_KHR_ANDROID_SURFACE_EXTENSION_NAME;
}

}  // namespace rx
