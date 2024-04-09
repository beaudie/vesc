//
// Copyright 2024 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//

#include "libANGLE/renderer/wgpu/wgpu_helpers.h"

namespace webgpu
{
ImageHelper::ImageHelper() {}

ImageHelper::~ImageHelper() {}

angle::Result ImageHelper::initImage(wgpu::Device &device,
                                     gl::LevelIndex firstAllocatedLevel,
                                     wgpu::TextureDescriptor textureDescriptor)
{
    mTextureDescriptor   = textureDescriptor;
    mFirstAllocatedLevel = firstAllocatedLevel;
    mTexture             = device.CreateTexture(&mTextureDescriptor);

    return angle::Result::Continue;
}

void ImageHelper::flushStagedUpdates(wgpu::Device &device, wgpu::Queue &queue)
{
    wgpu::CommandEncoder encoder = device.CreateCommandEncoder();
    wgpu::ImageCopyTexture dst;
    for (const QueuedDataUpload &src : mBufferQueue)
    {
        if (src.targetLevel < mFirstAllocatedLevel ||
            src.targetLevel >= (mFirstAllocatedLevel + mTextureDescriptor.mipLevelCount))
        {
            continue;
        }
        LevelIndex targetLevelWgpu = toWgpuLevel(src.targetLevel);
        dst.texture                = mTexture;
        dst.mipLevel               = targetLevelWgpu.get();
        encoder.CopyBufferToTexture(&src.copyBuffer, &dst, &mTextureDescriptor.size);
    }
    wgpu::CommandBuffer commandBuffer = encoder.Finish();
    queue.Submit(1, &commandBuffer);
}

wgpu::TextureDescriptor ImageHelper::createTextureDescriptor(wgpu::TextureUsage usage,
                                                             wgpu::TextureDimension dimension,
                                                             wgpu::Extent3D size,
                                                             wgpu::TextureFormat format,
                                                             std::uint32_t mipLevelCount,
                                                             std::uint32_t sampleCount,
                                                             std::size_t viewFormatCount)
{
    wgpu::TextureDescriptor textureDescriptor = {};
    textureDescriptor.usage                   = usage;
    textureDescriptor.dimension               = dimension;
    textureDescriptor.size                    = size;
    textureDescriptor.format                  = format;
    textureDescriptor.mipLevelCount           = mipLevelCount;
    textureDescriptor.sampleCount             = sampleCount;
    textureDescriptor.viewFormatCount         = viewFormatCount;
    return textureDescriptor;
}

angle::Result ImageHelper::stageTextureUpload(wgpu::Device &device,
                                              const gl::Extents &glExtents,
                                              const gl::ImageIndex &index)
{
    gl::LevelIndex levelGL(index.getLevelIndex());
    wgpu::BufferDescriptor bufferDescriptor   = {};
    bufferDescriptor.usage                    = wgpu::BufferUsage::MapWrite;
    bufferDescriptor.size                     = glExtents.width * glExtents.height;
    wgpu::TextureDataLayout textureDataLayout = {};
    textureDataLayout.bytesPerRow             = glExtents.width;
    textureDataLayout.rowsPerImage            = glExtents.height;
    wgpu::Buffer newBuffer                    = device.CreateBuffer(&bufferDescriptor);
    WgpuUserdata userdata;
    newBuffer.MapAsync(
        wgpu::MapMode::Write, toWgpuLevel(mFirstAllocatedLevel).get(), newBuffer.GetSize(),
        [](WGPUBufferMapAsyncStatus status, void *voidUserdata) {
            WgpuUserdata *userdata = static_cast<WgpuUserdata *>(voidUserdata);
            userdata->mapComplete  = true;
            userdata->status       = status;
        },
        &userdata);

    while (!userdata.mapComplete)
    {
        sleep(1);
        device.Tick();
    }
    wgpu::ImageCopyBuffer copyBuffer = {};
    copyBuffer.layout                = textureDataLayout;
    copyBuffer.buffer                = newBuffer;
    QueuedDataUpload dataUpload      = {copyBuffer, levelGL};
    mBufferQueue.push_back(dataUpload);
    return angle::Result::Continue;
}

LevelIndex ImageHelper::toWgpuLevel(gl::LevelIndex levelIndexGl) const
{
    return gl_wgpu::getLevelIndex(levelIndexGl, mFirstAllocatedLevel);
}

gl::LevelIndex ImageHelper::toGlLevel(LevelIndex levelIndexWgpu) const
{
    return wgpu_gl::getLevelIndex(levelIndexWgpu, mFirstAllocatedLevel);
}

TextureInfo ImageHelper::getWgpuTextureInfo(const gl::ImageIndex &index)
{
    TextureInfo textureInfo;
    textureInfo.dimension     = gl_wgpu::getWgpuTextureDimension(index.getType());
    textureInfo.mipLevelCount = index.getLayerCount();
    return textureInfo;
}
}  // namespace webgpu
