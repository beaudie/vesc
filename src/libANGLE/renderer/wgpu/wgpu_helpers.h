//
// Copyright 2024 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//

#ifndef LIBANGLE_RENDERER_WGPU_WGPU_HELPERS_H_
#define LIBANGLE_RENDERER_WGPU_WGPU_HELPERS_H_

#include <dawn/webgpu_cpp.h>
#include <stdint.h>

#include "libANGLE/Error.h"

namespace rx
{

class ImageHelper
{
  public:
    ImageHelper();
    ~ImageHelper();

    angle::Result initImage(wgpu::TextureUsage usage,
                            wgpu::TextureDimension dimension,
                            wgpu::Extent3D size,
                            wgpu::TextureFormat format,
                            std::uint32_t mipLevelCount,
                            std::uint32_t sampleCount,
                            std::size_t ViewFormatCount);

    void flushStagedUpdates(wgpu::Device device);

  private:
    wgpu::Texture mTexture;
    wgpu::TextureUsage mUsage;
    wgpu::TextureDimension mDimension;
    wgpu::Extent3D mSize;
    wgpu::TextureFormat mFormat;
    std::uint32_t mMipLevelCount;
    std::uint32_t mSampleCount;
    std::size_t mViewFormatCount;

    std::vector<wgpu::ImageCopyBuffer> mBufferQueue;
};
}  // namespace rx

#endif  // LIBANGLE_RENDERER_WGPU_WGPU_HELPERS_H_
