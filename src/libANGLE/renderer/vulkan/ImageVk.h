//
// Copyright 2016 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//
// ImageVk.h:
//    Defines the class interface for ImageVk, implementing ImageImpl.
//

#ifndef LIBANGLE_RENDERER_VULKAN_IMAGEVK_H_
#define LIBANGLE_RENDERER_VULKAN_IMAGEVK_H_

#include "libANGLE/renderer/ImageImpl.h"
#include "libANGLE/renderer/vulkan/RenderTargetVk.h"
#include "libANGLE/renderer/vulkan/vk_helpers.h"

namespace rx
{

class ImageVk : public ImageImpl
{
  public:
    ImageVk(const egl::ImageState &state);
    ~ImageVk() override;
    egl::Error initialize(const egl::Display *display) override;

    gl::Error orphan(const gl::Context *context, egl::ImageSibling *sibling) override;

    const vk::ImageHelper &getImage() const;
    angle::Result getRenderTarget(const gl::Context *context, RenderTargetVk **outRT);

  private:
    vk::ImageHelper *mImage;
    RenderTargetVk *mRenderTarget;
};

}  // namespace rx

#endif  // LIBANGLE_RENDERER_VULKAN_IMAGEVK_H_
