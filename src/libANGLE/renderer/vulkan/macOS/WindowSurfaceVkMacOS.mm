//
// Copyright 2019 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//
// WindowSurfaceVkMacOS.mm:
//    Implements methods from WindowSurfaceVkMacOS.
//

#include "libANGLE/renderer/vulkan/macOS/WindowSurfaceVkMacOS.h"

#include <Metal/Metal.h>
#include <QuartzCore/CAMetalLayer.h>

#include "libANGLE/renderer/vulkan/RendererVk.h"
#include "libANGLE/renderer/vulkan/vk_utils.h"

#define METALVIEW_TAG 255

@interface MetalView : NSView {
    NSInteger _tag;
    bool _useHighDPI;
}

- (instancetype)initWithFrame:(NSRect)frame useHighDPI:(bool)useHighDPI;

/* Override superclass tag so this class can set it. */
@property(assign, readonly) NSInteger tag;

@end

@implementation MetalView
/* The synthesized getter should be called by super's viewWithTag. */
@synthesize tag = _tag;

/* Return a Metal-compatible layer. */
+ (Class)layerClass
{
    return NSClassFromString(@"CAMetalLayer");
}

/* Indicate the view wants to draw using a backing layer instead of drawRect. */
- (BOOL)wantsUpdateLayer
{
    return YES;
}

/* When the wantsLayer property is set to YES, this method will be invoked to
 * return a layer instance.
 */
- (CALayer *)makeBackingLayer
{
    return [self.class.layerClass layer];
}

- (instancetype)initWithFrame:(NSRect)frame useHighDPI:(bool)useHighDPI API_AVAILABLE(macosx(10.11))
{
    if ((self = [super initWithFrame:frame]))
    {

        /* Allow resize. */
        self.autoresizingMask = NSViewWidthSizable | NSViewHeightSizable;
        _tag                  = METALVIEW_TAG;

        _useHighDPI = useHighDPI;
        [self updateDrawableSize];
    }

    return self;
}

/* Set the size of the metal drawables when the view is resized. */
- (void)resizeSubviewsWithOldSize:(NSSize)oldSize API_AVAILABLE(macosx(10.11))
{
    [super resizeSubviewsWithOldSize:oldSize];
    [self updateDrawableSize];
}

- (void)updateDrawableSize API_AVAILABLE(macosx(10.11))
{
    NSRect bounds = [self bounds];
    if (_useHighDPI)
    {
        bounds = [self convertRectToBacking:bounds];
    }
    ((CAMetalLayer *)self.layer).drawableSize = NSSizeToCGSize(bounds.size);
}

@end

namespace rx
{

WindowSurfaceVkMacOS::WindowSurfaceVkMacOS(const egl::SurfaceState &surfaceState,
                                           EGLNativeWindowType window,
                                           EGLint width,
                                           EGLint height)
    : WindowSurfaceVk(surfaceState, window, width, height), mMetalView(NULL)
{}

WindowSurfaceVkMacOS::~WindowSurfaceVkMacOS() = default;

angle::Result WindowSurfaceVkMacOS::createSurfaceVk(vk::Context *context, gl::Extents *extentsOut)
    API_AVAILABLE(macosx(10.11))
{
    CALayer *layer = reinterpret_cast<CALayer *>(mNativeWindowType);
    mMetalView     = [[MetalView alloc] initWithFrame:[layer bounds] useHighDPI:false];
    // Instantiate the CAMetalLayer
    mMetalView.wantsLayer = YES;
    [layer addSublayer:[mMetalView layer]];

    VkMacOSSurfaceCreateInfoMVK createInfo = {};

    createInfo.sType = VK_STRUCTURE_TYPE_MACOS_SURFACE_CREATE_INFO_MVK;
    createInfo.flags = 0;
    createInfo.pNext = NULL;
    createInfo.pView = mMetalView;

    ANGLE_VK_TRY(context, vkCreateMacOSSurfaceMVK(context->getRenderer()->getInstance(),
                                                  &createInfo, nullptr, &mSurface));

    return getCurrentWindowSize(context, extentsOut);
}

angle::Result WindowSurfaceVkMacOS::getCurrentWindowSize(vk::Context *context,
                                                         gl::Extents *extentsOut)
    API_AVAILABLE(macosx(10.11))
{
    ANGLE_VK_CHECK(context, (mMetalView != NULL), VK_ERROR_INITIALIZATION_FAILED);

    NSRect viewport = [mMetalView bounds];
    *extentsOut     = gl::Extents(static_cast<int>(viewport.size.width),
                              static_cast<int>(viewport.size.height), 1);

    return angle::Result::Continue;
}

}  // namespace rx
