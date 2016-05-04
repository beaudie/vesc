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

#include "common/angleutils.h"
#include "libANGLE/angletypes.h"
#include "libANGLE/Framebuffer.h"

namespace rx
{

namespace vk
{
struct Format;
}

class RenderTargetVk final : public FramebufferAttachmentRenderTarget
{
  public:
    RenderTargetVk();
    ~RenderTargetVk();

    void setFormat(const vk::Format &format);
    void setImage(VkImage image);
    void setImageView(VkImageView imageView);
    void setWidth(int width);
    void setHeight(int height);
    void setDepth(int depth);
    void setSamples(VkSampleCountFlagBits samples);

    const vk::Format &getFormat() const
    {
        ASSERT(mFormat);
        return *mFormat;
    }
    VkImage getImage() const { return mImage; }
    VkImageView getImageView() const { return mImageView; }
    const gl::Extents &getExtents() const { return mExtents; }
    VkSampleCountFlagBits getSamples() const { return mSamples; }

  private:
    const vk::Format *mFormat;
    // The image (and view) is not owned by the render target.
    VkImage mImage;
    VkImageView mImageView;
    gl::Extents mExtents;
    VkSampleCountFlagBits mSamples;
};

}  // namespace rx

#endif  // LIBANGLE_RENDERER_VULKAN_RENDERTARGETVK_H_
