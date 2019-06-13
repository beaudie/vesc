//
// Copyright 2016 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//
// RenderTargetVk:
//   Wrapper around a Vulkan renderable resource, using an ImageView.
//

#include "libANGLE/renderer/vulkan/RenderTargetVk.h"

#include "libANGLE/renderer/vulkan/CommandGraph.h"
#include "libANGLE/renderer/vulkan/ContextVk.h"
#include "libANGLE/renderer/vulkan/TextureVk.h"
#include "libANGLE/renderer/vulkan/vk_format_utils.h"
#include "libANGLE/renderer/vulkan/vk_helpers.h"

namespace rx
{
RenderTargetVk::RenderTargetVk()
    : mImage(nullptr),
      mImageView(nullptr),
      mCubeImageFetchView(nullptr),
      mLevelIndex(0),
      mLayerIndex(0)
{}

RenderTargetVk::~RenderTargetVk() {}

RenderTargetVk::RenderTargetVk(RenderTargetVk &&other)
    : mImage(other.mImage),
      mImageView(other.mImageView),
      mCubeImageFetchView(other.mCubeImageFetchView),
      mLevelIndex(other.mLevelIndex),
      mLayerIndex(other.mLayerIndex)
{}

void RenderTargetVk::init(vk::ImageHelper *image,
                          vk::ImageView *imageView,
                          vk::ImageView *cubeImageFetchView,
                          size_t levelIndex,
                          size_t layerIndex)
{
    mImage              = image;
    mImageView          = imageView;
    mCubeImageFetchView = cubeImageFetchView;
    mLevelIndex         = levelIndex;
    mLayerIndex         = layerIndex;
}

void RenderTargetVk::reset()
{
    mImage              = nullptr;
    mImageView          = nullptr;
    mCubeImageFetchView = nullptr;
    mLevelIndex         = 0;
    mLayerIndex         = 0;
}

vk::ImageView *RenderTargetVk::getFetchImageView() const
{
    return mCubeImageFetchView && mCubeImageFetchView->valid() ? mCubeImageFetchView
                                                               : getReadImageView();
}

void RenderTargetVk::updateSwapchainImage(vk::ImageHelper *image, vk::ImageView *imageView)
{
    ASSERT(image && image->valid() && imageView && imageView->valid());
    mImage              = image;
    mImageView          = imageView;
    mCubeImageFetchView = nullptr;
}

angle::Result RenderTargetVk::flushStagedUpdates(ContextVk *contextVk)
{
    ASSERT(mImage->valid());
    return mImage->flushStagedUpdates(contextVk, mLevelIndex, mLevelIndex + 1, mLayerIndex,
                                      mLayerIndex + 1);
}

}  // namespace rx
