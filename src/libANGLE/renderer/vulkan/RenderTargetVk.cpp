//
// Copyright 2016 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//
// RenderTargetVk:
//   Wrapper around a Vulkan renderable resource, using an ImageView.
//

#include "libANGLE/renderer/vulkan/RenderTargetVk.h"

#include "libANGLE/renderer/vulkan/Format.h"

namespace rx
{

RenderTargetVk::RenderTargetVk()
    : mFormat(&vk::Format::Get(GL_NONE)),
      mImage(VK_NULL_HANDLE),
      mImageView(VK_NULL_HANDLE),
      mExtents(),
      mSamples(VK_SAMPLE_COUNT_1_BIT)
{
}

RenderTargetVk::~RenderTargetVk()
{
}

void RenderTargetVk::setFormat(const vk::Format &format)
{
    mFormat = &format;
}

void RenderTargetVk::setImage(VkImage image)
{
    mImage = image;
}

void RenderTargetVk::setImageView(VkImageView imageView)
{
    mImageView = imageView;
}

void RenderTargetVk::setWidth(int width)
{
    mExtents.width = width;
}

void RenderTargetVk::setHeight(int height)
{
    mExtents.height = height;
}

void RenderTargetVk::setDepth(int depth)
{
    mExtents.depth = depth;
}

void RenderTargetVk::setSamples(VkSampleCountFlagBits samples)
{
    mSamples = samples;
}

}  // namespace rx
