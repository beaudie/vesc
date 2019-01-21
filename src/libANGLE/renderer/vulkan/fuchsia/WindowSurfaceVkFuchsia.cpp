//
// Copyright 2019 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//
// WindowSurfaceVkFuchsia.cpp:
//    Implements methods from WindowSurfaceVkFuchsia.
//

#include "libANGLE/renderer/vulkan/fuchsia/WindowSurfaceVkFuchsia.h"

#include <zircon/syscalls.h>
#include <zircon/syscalls/object.h>

#include "libANGLE/renderer/vulkan/RendererVk.h"
#include "libANGLE/renderer/vulkan/vk_utils.h"

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

    zx_handle_t *handle_ptr = reinterpret_cast<zx_handle_t *>(window);

    zx_info_handle_basic_t info;
    zx_status_t status = zx_object_get_info(*handle_ptr, ZX_INFO_HANDLE_BASIC, &info, sizeof(info),
                                            nullptr, nullptr);
    if (status != ZX_OK)
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
    zx_handle_t *handle_ptr = reinterpret_cast<zx_handle_t *>(mNativeWindowType);

    VkImagePipeSurfaceCreateInfoFUCHSIA createInfo = {};
    createInfo.sType = VK_STRUCTURE_TYPE_IMAGEPIPE_SURFACE_CREATE_INFO_FUCHSIA;
    std::swap(createInfo.imagePipeHandle, *handle_ptr);
    ANGLE_VK_TRY(context, vkCreateImagePipeSurfaceFUCHSIA(context->getRenderer()->getInstance(),
                                                          &createInfo, nullptr, &mSurface));

    // Use EGL_ANGLE_window_fixed_size to choose the surface size.
    *extentsOut = gl::Extents(0, 0, 0);

    return angle::Result::Continue;
}

}  // namespace rx
