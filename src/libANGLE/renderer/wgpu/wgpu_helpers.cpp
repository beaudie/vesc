//
// Copyright 2024 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//

#include "libANGLE/renderer/wgpu/wgpu_helpers.h"
namespace rx
{

ImageHelper::ImageHelper() {}

ImageHelper::~ImageHelper() {}

angle::Result ImageHelper::initImage(wgpu::TextureUsage usage,
                                     wgpu::TextureDimension dimension,
                                     wgpu::Extent3D size,
                                     wgpu::TextureFormat format,
                                     std::uint32_t mipLevelCount,
                                     std::uint32_t sampleCount,
                                     std::size_t viewFormatCount)
{

    mUsage           = usage;
    mDimension       = dimension;
    mSize            = size;
    mFormat          = format;
    mMipLevelCount   = mipLevelCount;
    mSampleCount     = sampleCount;
    mViewFormatCount = viewFormatCount;

    return angle::Result::Continue;
}

void ImageHelper::flushStagedUpdates(wgpu::Device device)
{
    wgpu::CommandEncoder encoder = device.CreateCommandEncoder();
    wgpu::ImageCopyTexture dst;
    for (const auto &src : mBufferQueue)
    {
        dst.texture  = mTexture;
        dst.mipLevel = mMipLevelCount;
        encoder.CopyBufferToTexture(&src, &dst, &mSize);
    }
}
}  // namespace rx
