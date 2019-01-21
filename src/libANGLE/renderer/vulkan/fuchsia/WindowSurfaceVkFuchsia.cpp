//
// Copyright 2019 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//

#include "libANGLE/renderer/vulkan/fuchsia/WindowSurfaceVkFuchsia.h"

#include "libANGLE/renderer/vulkan/RendererVk.h"

namespace rx
{

WindowSurfaceVkFuchsia::WindowSurfaceVkFuchsia(const egl::SurfaceState &surfaceState,
                                               EGLNativeWindowType window,
                                               EGLint width,
                                               EGLint height)
    : WindowSurfaceVk(surfaceState, window, width, height)
{}

WindowSurfaceVkFuchsia::~WindowSurfaceVkFuchsia() {}

// static
bool WindowSurfaceVkFuchsia::isValidNativeWindow(EGLNativeWindowType window)
{
    if (!window)
    {
        return false;
    }
    FuchsiaEGLWindow validWindow = {FUCHSIA_EGL_WINDOW_MAGIC, sizeof(validWindow)};
    if (memcmp(window, &validWindow, sizeof(validWindow.magic)) != 0)
    {
        return false;
    }
    if (memcmp(static_cast<char *>(window) + offsetof(FuchsiaEGLWindow, size), &validWindow.size,
               sizeof(validWindow.size)) != 0)
    {
        return false;
    }
    return true;
}

angle::Result WindowSurfaceVkFuchsia::createSurfaceVk(vk::Context *context, gl::Extents *extentsOut)
{
    auto vkCreateImagePipeSurfaceFUCHSIA =
        reinterpret_cast<PFN_vkCreateImagePipeSurfaceFUCHSIA>(vkGetInstanceProcAddr(
            context->getRenderer()->getInstance(), "vkCreateImagePipeSurfaceFUCHSIA"));
    ASSERT(vkCreateImagePipeSurfaceFUCHSIA);

    ASSERT(isValidNativeWindow(mNativeWindowType));
    FuchsiaEGLWindow *fuchsiaEGLWindow = static_cast<FuchsiaEGLWindow *>(mNativeWindowType);

    VkImagePipeSurfaceCreateInfoFUCHSIA createInfo = {};
    createInfo.sType                  = VK_STRUCTURE_TYPE_IMAGEPIPE_SURFACE_CREATE_INFO_FUCHSIA;
    createInfo.imagePipeHandle        = fuchsiaEGLWindow->imagePipeHandle;
    fuchsiaEGLWindow->imagePipeHandle = ZX_HANDLE_INVALID;
    ANGLE_VK_TRY(context, vkCreateImagePipeSurfaceFUCHSIA(context->getRenderer()->getInstance(),
                                                          &createInfo, nullptr, &mSurface));

    *extentsOut = gl::Extents(fuchsiaEGLWindow->width, fuchsiaEGLWindow->height, 0);

    return angle::Result::Continue;
}

}  // namespace rx
