//
// Copyright 2019 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//
// WindowSurfaceVkMac.mm:
//    Implements methods from WindowSurfaceVkMac.
//

#include "libANGLE/renderer/vulkan/mac/WindowSurfaceVkMac.h"

#include <Metal/Metal.h>
#include <QuartzCore/CAMetalLayer.h>

#include "libANGLE/renderer/vulkan/RendererVk.h"
#include "libANGLE/renderer/vulkan/vk_utils.h"

namespace rx
{

WindowSurfaceVkMac::WindowSurfaceVkMac(const egl::SurfaceState &surfaceState,
                                       EGLNativeWindowType window)
    : WindowSurfaceVk(surfaceState, window), mMetalLayer(NULL)
{}

WindowSurfaceVkMac::~WindowSurfaceVkMac() = default;

angle::Result WindowSurfaceVkMac::createSurfaceVk(vk::Context *context, gl::Extents *extentsOut)
    API_AVAILABLE(macosx(10.11))
{
    mMetalDevice = MTLCreateSystemDefaultDevice();

    CALayer *layer = reinterpret_cast<CALayer *>(mNativeWindowType);

    mMetalLayer                  = [[[CAMetalLayer alloc] init] autorelease];
    mMetalLayer.frame            = layer.frame;
    mMetalLayer.device           = mMetalDevice;
    mMetalLayer.framebufferOnly  = NO;
    mMetalLayer.autoresizingMask = kCALayerWidthSizable | kCALayerHeightSizable;
    mMetalLayer.contentsScale    = layer.contentsScale;

    [layer addSublayer:mMetalLayer];

    VkMetalSurfaceCreateInfoEXT createInfo = {};
    createInfo.sType                       = VK_STRUCTURE_TYPE_METAL_SURFACE_CREATE_INFO_EXT;
    createInfo.flags                       = 0;
    createInfo.pNext                       = NULL;
    createInfo.pLayer                      = mMetalLayer;
    ANGLE_VK_TRY(context, vkCreateMetalSurfaceEXT(context->getRenderer()->getInstance(),
                                                  &createInfo, nullptr, &mSurface));

    return getCurrentWindowSize(context, extentsOut);
}

angle::Result WindowSurfaceVkMac::getCurrentWindowSize(vk::Context *context,
                                                       gl::Extents *extentsOut)
    API_AVAILABLE(macosx(10.11))
{
    ANGLE_VK_CHECK(context, (mMetalLayer != NULL), VK_ERROR_INITIALIZATION_FAILED);

    NSRect viewport = [mMetalLayer bounds];
    *extentsOut     = gl::Extents(static_cast<int>(viewport.size.width),
                              static_cast<int>(viewport.size.height), 1);

    return angle::Result::Continue;
}

}  // namespace rx
