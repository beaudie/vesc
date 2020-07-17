//
// Copyright 2016 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//
// RenderTargetVk:
//   Wrapper around a Vulkan renderable resource, using an ImageView.
//

#include "libANGLE/renderer/vulkan/RenderTargetVk.h"

#include "libANGLE/renderer/vulkan/ContextVk.h"
#include "libANGLE/renderer/vulkan/ResourceVk.h"
#include "libANGLE/renderer/vulkan/TextureVk.h"
#include "libANGLE/renderer/vulkan/vk_format_utils.h"
#include "libANGLE/renderer/vulkan/vk_helpers.h"

namespace rx
{

RenderTargetVk::RenderTargetVk()
{
    reset();
}

RenderTargetVk::~RenderTargetVk() {}

RenderTargetVk::RenderTargetVk(RenderTargetVk &&other)
    : mImage(other.mImage),
      mImageViews(other.mImageViews),
      mResolveImage(other.mResolveImage),
      mResolveImageViews(other.mResolveImageViews),
      mLevelIndex(other.mLevelIndex),
      mLayerIndex(other.mLayerIndex),
      mContentDefined(other.mContentDefined)
{
    other.reset();
}

void RenderTargetVk::init(vk::ImageHelper *image,
                          vk::ImageViewHelper *imageViews,
                          vk::ImageHelper *resolveImage,
                          vk::ImageViewHelper *resolveImageViews,
                          uint32_t levelIndex,
                          uint32_t layerIndex,
                          bool isImageDataEphemeral)
{
    mImage             = image;
    mImageViews        = imageViews;
    mResolveImage      = resolveImage;
    mResolveImageViews = resolveImageViews;
    mLevelIndex        = levelIndex;
    mLayerIndex        = layerIndex;

    // Conservatively assume the content is defined.
    mContentDefined = true;

    mIsImageDataEphemeral = isImageDataEphemeral;
}

void RenderTargetVk::reset()
{
    mImage             = nullptr;
    mImageViews        = nullptr;
    mResolveImage      = nullptr;
    mResolveImageViews = nullptr;
    mLevelIndex        = 0;
    mLayerIndex        = 0;
    mContentDefined    = false;
}

ImageViewSerial RenderTargetVk::getAssignImageViewSerial(ContextVk *contextVk)
{
    ASSERT(mImageViews);
    ASSERT(mLayerIndex < std::numeric_limits<uint16_t>::max());
    ASSERT(mLevelIndex < std::numeric_limits<uint16_t>::max());

    ImageViewSerial imageViewSerial =
        mImageViews->getAssignSerial(contextVk, mLevelIndex, mLayerIndex);
    ASSERT(imageViewSerial.getValue() < std::numeric_limits<uint32_t>::max());
    return imageViewSerial;
}

angle::Result RenderTargetVk::onColorDraw(ContextVk *contextVk)
{
    ASSERT(!mImage->getFormat().actualImageFormat().hasDepthOrStencilBits());

    contextVk->onRenderPassImageWrite(VK_IMAGE_ASPECT_COLOR_BIT, vk::ImageLayout::ColorAttachment,
                                      mImage);
    if (mResolveImage)
    {
        contextVk->onRenderPassImageWrite(VK_IMAGE_ASPECT_COLOR_BIT,
                                          vk::ImageLayout::ColorAttachment, mResolveImage);
    }
    retainImageViews(contextVk);

    mContentDefined = true;

    return angle::Result::Continue;
}

angle::Result RenderTargetVk::onDepthStencilDraw(ContextVk *contextVk)
{
    ASSERT(mImage->getFormat().actualImageFormat().hasDepthOrStencilBits());

    const angle::Format &format    = mImage->getFormat().actualImageFormat();
    VkImageAspectFlags aspectFlags = vk::GetDepthStencilAspectFlags(format);

    contextVk->onRenderPassImageWrite(aspectFlags, vk::ImageLayout::DepthStencilAttachment, mImage);
    if (mResolveImage)
    {
        contextVk->onRenderPassImageWrite(aspectFlags, vk::ImageLayout::DepthStencilAttachment,
                                          mResolveImage);
    }
    retainImageViews(contextVk);

    mContentDefined = true;

    return angle::Result::Continue;
}

vk::ImageHelper &RenderTargetVk::getImageForRenderPass()
{
    ASSERT(mImage && mImage->valid());
    return *mImage;
}

const vk::ImageHelper &RenderTargetVk::getImageForRenderPass() const
{
    ASSERT(mImage && mImage->valid());
    return *mImage;
}

vk::ImageHelper &RenderTargetVk::getResolveImageForRenderPass()
{
    ASSERT(mResolveImage && mResolveImage->valid());
    return *mResolveImage;
}

const vk::ImageHelper &RenderTargetVk::getResolveImageForRenderPass() const
{
    ASSERT(mResolveImage && mResolveImage->valid());
    return *mResolveImage;
}

angle::Result RenderTargetVk::getImageViewImpl(ContextVk *contextVk,
                                               const vk::ImageHelper &image,
                                               vk::ImageViewHelper *imageViews,
                                               const vk::ImageView **imageViewOut) const
{
    ASSERT(image.valid() && imageViews);
    return imageViews->getLevelLayerDrawImageView(contextVk, image, mLevelIndex, mLayerIndex,
                                                  imageViewOut);
}

angle::Result RenderTargetVk::getImageView(ContextVk *contextVk,
                                           const vk::ImageView **imageViewOut) const
{
    // TODO: retain the views here?
    ASSERT(mImage);
    return getImageViewImpl(contextVk, *mImage, mImageViews, imageViewOut);
}

angle::Result RenderTargetVk::getResolveImageView(ContextVk *contextVk,
                                                  const vk::ImageView **imageViewOut) const
{
    // TODO: retain the views here?
    ASSERT(mResolveImage);
    return getImageViewImpl(contextVk, *mResolveImage, mResolveImageViews, imageViewOut);
}

angle::Result RenderTargetVk::getAndRetainCopyImageView(ContextVk *contextVk,
                                                        const vk::ImageView **imageViewOut) const
{
    // TODO: write a test that copies out of the multisampled_rendered_to_texture through
    // glTex[Sub]Image2D.  It should use the single-sampled image.
    retainImageViews(contextVk);

    const vk::ImageViewHelper *imageViews = mResolveImageViews ? mResolveImageViews : mImageViews;
    const vk::ImageView &copyView         = imageViews->getCopyImageView();

    // If the source of render target is a texture or renderbuffer, this will always be valid.  This
    // is also where 3D or 2DArray images could be the source of the render target.
    if (copyView.valid())
    {
        *imageViewOut = &copyView;
        return angle::Result::Continue;
    }

    // TODO: looks like swapchain also gives the view helper, so why did I add the above check?
    // See what fails:
    ASSERT(false);

    // Otherwise, this must come from the surface, in which case the image is 2D, so the image view
    // used to draw is just as good for fetching.  If resolve attachment is present, fetching is
    // done from that.
    return hasResolveAttachment() ? getResolveImageView(contextVk, imageViewOut)
                                  : getImageView(contextVk, imageViewOut);
}

const vk::Format &RenderTargetVk::getImageFormat() const
{
    ASSERT(mImage && mImage->valid());
    return mImage->getFormat();
}

gl::Extents RenderTargetVk::getExtents() const
{
    ASSERT(mImage && mImage->valid());
    return mImage->getLevelExtents2D(static_cast<uint32_t>(mLevelIndex));
}

void RenderTargetVk::updateSwapchainImage(vk::ImageHelper *image,
                                          vk::ImageViewHelper *imageViews,
                                          vk::ImageHelper *resolveImage,
                                          vk::ImageViewHelper *resolveImageViews)
{
    ASSERT(image && image->valid() && imageViews);
    mImage             = image;
    mImageViews        = imageViews;
    mResolveImage      = resolveImage;
    mResolveImageViews = resolveImageViews;
}

vk::ImageHelper &RenderTargetVk::getImageForCopy() const
{
    ASSERT(mImage && mImage->valid());
    return mResolveImage ? *mResolveImage : *mImage;
}

vk::ImageHelper &RenderTargetVk::getImageForWrite()
{
    // TODO: write a test that copies into the multisampled_rendered_to_texture through blit.  It
    // should use the single-sampled image.
    //
    // TODO: write a similar test for depth/stencil to make sure depth/stencil resolve back to
    // multisampled is correct too.
    ASSERT(mImage && mImage->valid());

    // TODO: there was a bug here with this line missing.  Write a test that does this:
    // - Invalidate (sets content undefined)
    // - Blit into this framebuffer
    // - Draw with blend (should use LOAD, but would have used DONT_CARE without next line)
    // - Readback, should be correct. With DONT_CARE, it would blend with garbage.
    mContentDefined = true;
    return mResolveImage ? *mResolveImage : *mImage;
}

angle::Result RenderTargetVk::flushStagedUpdates(ContextVk *contextVk,
                                                 vk::ClearValuesArray *deferredClears,
                                                 uint32_t deferredClearIndex)
{
    ASSERT(mImage->valid());

    // Note that the layer index for 3D textures is always zero according to Vulkan.
    uint32_t layerIndex = mLayerIndex;
    if (mImage->getType() == VK_IMAGE_TYPE_3D)
    {
        layerIndex = 0;
    }

    vk::ImageHelper *image = mResolveImage ? mResolveImage : mImage;

    // All updates should be staged on the resolved image if present as the source of truth.  Even
    // clears.  With deferred clears, the |deferredClears| array will be filled going over the
    // resolved image, but will eventually be applied to the multisampled image when starting the
    // render pass.
    ASSERT(mResolveImage == nullptr || !mImage->isUpdateStaged(mLevelIndex, layerIndex));

    if (!image->isUpdateStaged(mLevelIndex, layerIndex))
    {
        return angle::Result::Continue;
    }

    // TODO: this line was missing. See who calls flushStagedUpdates and based on that write a test
    mContentDefined = true;

    vk::CommandBuffer *commandBuffer;
    ANGLE_TRY(contextVk->endRenderPassAndGetCommandBuffer(&commandBuffer));
    return image->flushSingleSubresourceStagedUpdates(
        contextVk, mLevelIndex, layerIndex, commandBuffer, deferredClears, deferredClearIndex);
}

void RenderTargetVk::retainImageViews(ContextVk *contextVk) const
{
    mImageViews->retain(&contextVk->getResourceUseList());
    mResolveImageViews->retain(&contextVk->getResourceUseList());
}

gl::ImageIndex RenderTargetVk::getImageIndex() const
{
    // Determine the GL type from the Vk Image properties.
    if (mImage->getType() == VK_IMAGE_TYPE_3D)
    {
        return gl::ImageIndex::Make3D(mLevelIndex, mLayerIndex);
    }

    // We don't need to distinguish 2D array and cube.
    if (mImage->getLayerCount() > 1)
    {
        return gl::ImageIndex::Make2DArray(mLevelIndex, mLayerIndex);
    }

    ASSERT(mLayerIndex == 0);
    return gl::ImageIndex::Make2D(mLevelIndex);
}
}  // namespace rx
