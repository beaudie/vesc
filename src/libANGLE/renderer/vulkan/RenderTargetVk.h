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

#include "common/vulkan/vk_headers.h"
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
class Resource;
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
              vk::ImageViewHelper *imageViews,
              vk::ImageHelper *resolveImage,
              vk::ImageViewHelper *resolveImageViews,
              uint32_t levelIndexGL,
              uint32_t layerIndex,
              bool isImageDataEphemeral);
    void reset();
    // This returns the serial from underlying ImageViewHelper, first assigning one if required
    ImageViewSerial getAssignImageViewSerial(ContextVk *contextVk) const;
    ImageViewSerial getAssignResolveImageViewSerial(ContextVk *contextVk) const;

    // Note: RenderTargets should be called in order, with the depth/stencil onRender last.
    angle::Result onColorDraw(ContextVk *contextVk);
    angle::Result onDepthStencilDraw(ContextVk *contextVk);

    vk::ImageHelper &getImageForRenderPass();
    const vk::ImageHelper &getImageForRenderPass() const;

    bool hasResolveAttachment() const { return mResolveImage != nullptr; }
    vk::ImageHelper &getResolveImageForRenderPass();
    const vk::ImageHelper &getResolveImageForRenderPass() const;

    vk::ImageHelper &getImageForCopy() const;
    vk::ImageHelper &getImageForWrite();

    // For cube maps we use single-level single-layer 2D array views.
    angle::Result getImageView(ContextVk *contextVk, const vk::ImageView **imageViewOut) const;
    angle::Result getResolveImageView(ContextVk *contextVk,
                                      const vk::ImageView **imageViewOut) const;

    // For 3D textures, the 2D view created for render target is invalid to read from.  The
    // following will return a view to the whole image (for all types, including 3D and 2DArray).
    angle::Result getAndRetainCopyImageView(ContextVk *contextVk,
                                            const vk::ImageView **imageViewOut) const;

    const vk::Format &getImageFormat() const;
    gl::Extents getExtents() const;
    uint32_t getLevelIndex() const { return mLevelIndexGL; }
    uint32_t getLayerIndex() const { return mLayerIndex; }

    gl::ImageIndex getImageIndex() const;

    // Special mutator for Surface RenderTargets. Allows the Framebuffer to keep a single
    // RenderTargetVk pointer.
    void updateSwapchainImage(vk::ImageHelper *image,
                              vk::ImageViewHelper *imageViews,
                              vk::ImageHelper *resolveImage,
                              vk::ImageViewHelper *resolveImageViews);

    angle::Result flushStagedUpdates(ContextVk *contextVk,
                                     vk::ClearValuesArray *deferredClears,
                                     uint32_t deferredClearIndex);

    void retainImageViews(ContextVk *contextVk) const;

    bool hasDefinedContent() const { return mContentDefined; }
    // mark content as undefined so that certain optimizations are possible such as using DONT_CARE
    // as loadOp of the render target in the next renderpass.
    void invalidateContent() { mContentDefined = false; }
    // TODO: use this everywhere an update is made to the same level/layer as the render target,
    // such as copy target, blit target, storage image write, etc.
    void setContentDefined() { mContentDefined = true; }

    bool isImageDataEphemeral() const { return mIsImageDataEphemeral; }

  private:
    angle::Result getImageViewImpl(ContextVk *contextVk,
                                   const vk::ImageHelper &image,
                                   vk::ImageViewHelper *imageViews,
                                   const vk::ImageView **imageViewOut) const;

    ImageViewSerial getAssignViewSerialImpl(ContextVk *contextVk,
                                            vk::ImageViewHelper *imageViews) const;

    // The color or depth/stencil attachment of the framebuffer and its view.
    vk::ImageHelper *mImage;
    vk::ImageViewHelper *mImageViews;

    // If present, this is the corresponding resolve attachment and its view.  This is used to
    // implement GL_EXT_multisampled_render_to_texture, so while the rendering is done on mImage
    // during the renderpass, the resolved image is the one that actually holds the data.  This
    // means that data uploads and blit are done on this image, copies are done out of this image
    // etc.  This means that if there is no clear, and hasDefinedContent(), the contents of
    // mResolveImage must be copied to mImage since the loadOp of the attachment must be set to
    // LOAD.
    //
    // TODO: There are probably missing setDefinedContent calls all over the texture copy call.
    // Write a test that does:
    //
    // - Invalidate (sets content undefined)
    // - Upload/copy data
    // - Draw with blend (should use LOAD, but would have used DONT_CARE without next line)
    // - Readback, should be correct. With DONT_CARE, it would blend with garbage.
    //
    // TODO: See if there are other places the image can be written to.  Non-deferred clears are
    // an example, but probablye setting mContentDefined at flushStagedUpdates() time would take
    // care of that.  Robust resource init is another example if that doesn't use deferred clears
    // (which, if it doesn't let's make it do it as its directly relevant to WebGL which would be
    // an important user of GL_EXT_multisampled_render_to_texture). Compute shader writes are
    // another example.  Need to set that in setupDraw/Dispatch through the texture the image is
    // bound to.
    //
    // TODO: this method should directly apply to swapchain images, as they are very similar to
    // GL_EXT_multisampled_render_to_texture textures.  Investigate whether deferred clear is
    // being applied to swapchain images (probably yes).  Robust resource init of swapchain images
    // are less critical, but could be good to make sure they use deferred clear too.
    vk::ImageHelper *mResolveImage;
    vk::ImageViewHelper *mResolveImageViews;

    // Which subresource of the image is used as render target.
    uint32_t mLevelIndexGL;
    uint32_t mLayerIndex;

    // TODO: when optimizing blit, level/layer of non-resolve and resolve image don't necessarily
    // match, so should add that data too.  Can potentially make the functions say
    // getLevelIndexForCopy, getLevelIndexForWrite etc.

    // Whether the render target has been invalidated.  If so, DONT_CARE is used instead of LOAD for
    // loadOp of this attachment.
    bool mContentDefined;

    // If resolve attachment exists, this is true if the multisampled results need to be discarded.
    //
    // - GL_EXT_multisampled_render_to_texture: this is true for render targets created for this
    //   extension's usage.  Only color attachments use this optimization at the moment.
    // - GL_EXT_multisampled_render_to_texture2: this is true for depth/stencil textures per this
    //   extension, even though a resolve attachment is not even provided.
    // - Multisampled swapchain: TODO(syoussefi) this is true for the multisampled color attachment.
    //   http://anglebug.com/4836
    // - glBlitFramebuffer optimization: TODO(timvp) this is **false** in this case, as the
    //   multisampled attachment and the resolve attachments belong to independent framebuffers.
    //   http://anglebug.com/4753
    //
    // TODO: rename this to something that indicates whether the image or the resolve image owns the
    // data.  Then where relevant, instead of checking for hasResolveAttachment() or mResolveImage
    // != nullptr, need to check this.  For optimizing blit in particular, there are bugs where
    // updates are made to the resolve attachment, but should be made to the non-resolve image.
    // Maybe getImageForCopy() should also return the non-resolve image in that case?
    bool mIsImageDataEphemeral;
};

// A vector of rendertargets
using RenderTargetVector = std::vector<RenderTargetVk>;
}  // namespace rx

#endif  // LIBANGLE_RENDERER_VULKAN_RENDERTARGETVK_H_
