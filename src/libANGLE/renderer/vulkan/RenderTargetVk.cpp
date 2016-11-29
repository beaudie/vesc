//
// Copyright 2016 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//
// RenderTargetVk:
//   Wrapper around a Vulkan renderable resource, using an ImageView.
//

#include "libANGLE/renderer/vulkan/RenderTargetVk.h"

#include "libANGLE/renderer/vulkan/formatutilsvk.h"

namespace rx
{

RenderTargetVk::RenderTargetVk(const vk::Format &format, const gl::Extents &extents)
    : mFormat(&format),
      mImage(VK_NULL_HANDLE),
      mImageView(VK_NULL_HANDLE),
      mExtents(extents),
      mSamples(VK_SAMPLE_COUNT_1_BIT)
{
}

RenderTargetVk::~RenderTargetVk()
{
}

void RenderTargetVk::setImage(vk::Image *image)
{
    mImage = image;
}

void RenderTargetVk::setImageView(vk::ImageView *imageView)
{
    mImageView = imageView;
}

void RenderTargetVk::updateExtents(const gl::Extents &extents)
{
    mExtents = extents;
}

void RenderTargetVk::updateImage(vk::Image *image, vk::ImageView *imageView)
{
    mImage     = image;
    mImageView = imageView;
}

void RenderTargetVk::setSamples(VkSampleCountFlagBits samples)
{
    mSamples = samples;
}

}  // namespace rx
