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
#include "libANGLE/renderer/vulkan/vk_helpers.h"

namespace rx
{

class ImageVk : public ImageImpl
{
  public:
    ImageVk(const egl::ImageState &state);
    ~ImageVk() override;
    void onDestroy(const egl::Display *display) override;

    egl::Error initialize(const egl::Display *display) override;

    angle::Result orphan(const gl::Context *context, egl::ImageSibling *sibling) override;

    gl::TextureType getImageTextureType() const;
    uint32_t getImageBaseMipLevel() const;
    uint32_t getImageBaseLayer() const;
    vk::ImageHelper *getImage() const;

  private:
    gl::TextureType mBaseTextureType;
    uint32_t mBaseMipLevel;
    uint32_t mBaseLayer;

    bool mOwnsImage;
    vk::ImageHelper *mImage;
};

}  // namespace rx

#endif  // LIBANGLE_RENDERER_VULKAN_IMAGEVK_H_
