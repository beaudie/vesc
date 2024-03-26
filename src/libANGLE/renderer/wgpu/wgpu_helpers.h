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
#include "libANGLE/ImageIndex.h"
#include "libANGLE/angletypes.h"
#include "libANGLE/renderer/wgpu/wgpu_utils.h"

namespace webgpu
{

struct QueuedDataUpload
{
    wgpu::ImageCopyTexture copyTexture;
    gl::LevelIndex targetLevel;
};

// Stores subset of information required to create a wgpu::Texture
struct TextureInfo
{
    wgpu::TextureUsage usage = wgpu::TextureUsage::None;
    wgpu::TextureDimension dimension;
    uint32_t mipLevelCount;
};

wgpu::TextureDimension toWgpuTextureDimension(gl::TextureType glTextureType);

class ImageHelper
{
  public:
    ImageHelper();
    ~ImageHelper();

    angle::Result initImage(wgpu::Device &device,
                            gl::LevelIndex firstAllocatedLevel,
                            wgpu::TextureDescriptor textureDescriptor);

    void flushStagedUpdates(wgpu::Device &device);

    wgpu::TextureDescriptor createTextureDescriptor(wgpu::TextureUsage usage,
                                                    wgpu::TextureDimension dimension,
                                                    wgpu::Extent3D size,
                                                    wgpu::TextureFormat format,
                                                    std::uint32_t mipLevelCount,
                                                    std::uint32_t sampleCount,
                                                    std::size_t viewFormatCount);

    angle::Result stageTextureUpload(wgpu::Device &device,
                                     const gl::ImageIndex &index,
                                     const gl::Offset &offset,
                                     wgpu::TextureDescriptor textureDescriptor);

    LevelIndex toWgpuLevel(gl::LevelIndex levelIndexGl) const;
    gl::LevelIndex toGlLevel(LevelIndex levelIndexWgpu) const;
    wgpu::Texture &getTexture() { return mTexture; }
    TextureInfo getWgpuTextureInfo(const gl::ImageIndex &index);
    wgpu::TextureFormat toWgpuTextureFormat() const { return mTextureDescriptor.format; }
    const wgpu::TextureDescriptor &getTextureDescriptor() const { return mTextureDescriptor; }
    gl::LevelIndex getFirstAllocatedLevel() { return mFirstAllocatedLevel; }

  private:
    wgpu::Texture mTexture;
    wgpu::TextureDescriptor mTextureDescriptor = {};

    gl::LevelIndex mFirstAllocatedLevel;

    std::vector<QueuedDataUpload> mBufferQueue;
};
}  // namespace webgpu

#endif  // LIBANGLE_RENDERER_WGPU_WGPU_HELPERS_H_
