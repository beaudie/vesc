//
// Copyright 2016 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//
// RenderTargetVk:
//   Wrapper around a Vulkan renderable resource, using an ImageView.
//

#ifndef LIBANGLE_RENDERER_VULKAN_RENDERTARGETVK_H_
#define LIBANGLE_RENDERER_VULKAN_RENDERTARGETVK_H_

#include <vulkan/vulkan.h>

#include "libANGLE/FramebufferAttachment.h"
#include "libANGLE/renderer/renderer_utils.h"
#include "libANGLE/renderer/vulkan/vk_helpers.h"

namespace rx
{
namespace vk
{
struct Format;
class FramebufferHelper;
class ImageHelper;
class ImageView;
class CommandGraphResource;
class RenderPassDesc;
}  // namespace vk

class ContextVk;
class TextureVk;

// This is a very light-weight class that does not own to the resources it points to.
// It's meant only to copy across some information from a FramebufferAttachment to the
// business rendering logic. It stores Images and ImageViews by pointer for performance.
class RenderTargetVk final : public FramebufferAttachmentRenderTarget
{
  public:
    RenderTargetVk();
    ~RenderTargetVk() override;

    // Used in std::vector initialization.
    RenderTargetVk(RenderTargetVk &&other);

    void init(vk::ImageHelper *image,
              vk::ImageView *imageView,
              vk::ImageView *cubeImageFetchView,
              size_t levelIndex,
              size_t layerIndex);
    void reset();

    void onColorDraw(vk::CommandBuffer *commandBuffer)
    {
        onAccess(vk::ImageLayout::ColorAttachment, commandBuffer);
    }

    void onDepthStencilDraw(vk::CommandBuffer *commandBuffer)
    {
        onAccess(vk::ImageLayout::DepthStencilAttachment, commandBuffer);
    }

    void onAccess(vk::ImageLayout layout, vk::CommandBuffer *commandBuffer)
    {
        ASSERT(mImage && mImage->valid());
        mImage->changeLayoutWithCommand(mImage->getAspectFlags(), layout, commandBuffer);
    }

    vk::ImageHelper &getImage()
    {
        ASSERT(mImage && mImage->valid());
        return *mImage;
    }

    const vk::ImageHelper &getImage() const
    {
        ASSERT(mImage && mImage->valid());
        return *mImage;
    }

    vk::ImageView *getDrawImageView() const
    {
        ASSERT(mImageView && mImageView->valid());
        return mImageView;
    }

    vk::ImageView *getReadImageView() const { return getDrawImageView(); }

    // GLSL's texelFetch() needs a 2D array view to read from cube maps.  This function returns the
    // same view as `getReadImageView()`, except for cubemaps, in which case it returns a 2D array
    // view of it.
    vk::ImageView *getFetchImageView() const;

    const vk::Format &getImageFormat() const
    {
        ASSERT(mImage && mImage->valid());
        return mImage->getFormat();
    }

    gl::Extents getExtents() const
    {
        ASSERT(mImage && mImage->valid());
        return mImage->getLevelExtents2D(mLevelIndex);
    }

    size_t getLevelIndex() const { return mLevelIndex; }
    size_t getLayerIndex() const { return mLayerIndex; }

    // Special mutator for Surface RenderTargets. Allows the Framebuffer to keep a single
    // RenderTargetVk pointer.
    void updateSwapchainImage(vk::ImageHelper *image, vk::ImageView *imageView);

    angle::Result flushStagedUpdates(ContextVk *contextVk);

  private:
    vk::ImageHelper *mImage;
    // Note that the draw and read image views are the same, given the requirements of a render
    // target.
    vk::ImageView *mImageView;
    // For cubemaps, a 2D-array view is also created to be used with shaders that use texelFetch().
    vk::ImageView *mCubeImageFetchView;
    size_t mLevelIndex;
    size_t mLayerIndex;
};

}  // namespace rx

#endif  // LIBANGLE_RENDERER_VULKAN_RENDERTARGETVK_H_
