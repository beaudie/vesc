//
// Copyright 2024 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//

#include "libANGLE/renderer/wgpu/wgpu_helpers.h"
#include "libANGLE/formatutils.h"

#include "libANGLE/renderer/wgpu/ContextWgpu.h"
#include "libANGLE/renderer/wgpu/DisplayWgpu.h"

namespace rx
{
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
    dst.texture = mTexture;
    for (const QueuedDataUpload &src : mBufferQueue)
    {
        if (src.targetLevel < mFirstAllocatedLevel ||
            src.targetLevel >= (mFirstAllocatedLevel + mTextureDescriptor.mipLevelCount))
        {
            continue;
        }
        LevelIndex targetLevelWgpu = toWgpuLevel(src.targetLevel);
        dst.mipLevel               = targetLevelWgpu.get();
        encoder.CopyBufferToTexture(&src.buffer, &dst, &mTextureDescriptor.size);
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
    BufferHelper bufferHelper;
    angle::Result result =
        bufferHelper.initBuffer(device, glExtents.width * glExtents.height,
                                wgpu::BufferUsage::MapWrite | wgpu::BufferUsage::CopySrc, true);
    if (result != angle::Result::Continue)
    {
        return result;
    }
    wgpu::TextureDataLayout textureDataLayout = {};
    textureDataLayout.bytesPerRow             = glExtents.width;
    textureDataLayout.rowsPerImage            = glExtents.height;
    wgpu::ImageCopyBuffer imageCopyBuffer;
    imageCopyBuffer.layout      = textureDataLayout;
    imageCopyBuffer.buffer      = bufferHelper.getBuffer();
    QueuedDataUpload dataUpload = {imageCopyBuffer, levelGL};
    mBufferQueue.push_back(dataUpload);
    return angle::Result::Continue;
}

angle::Result ImageHelper::readPackPixelBuffer(const gl::Rectangle &area,
                                               const rx::PackPixelsParams &packPixelsParams,
                                               const angle::Format &aspectFormat,
                                               const uint8_t *readPixelBuffer,
                                               void *pixels)
{
    PackPixels(packPixelsParams, aspectFormat, area.width * aspectFormat.pixelBytes,
               readPixelBuffer, static_cast<uint8_t *>(pixels));
    return angle::Result::Continue;
}

// static
angle::Result ImageHelper::getReadPixelsParams(rx::ContextWgpu *contextWgpu,
                                               const gl::PixelPackState &packState,
                                               gl::Buffer *packBuffer,
                                               GLenum format,
                                               GLenum type,
                                               const gl::Rectangle &area,
                                               const gl::Rectangle &clippedArea,
                                               rx::PackPixelsParams *paramsOut,
                                               GLuint *skipBytesOut)
{
    const gl::InternalFormat &sizedFormatInfo = gl::GetInternalFormatInfo(format, type);

    GLuint outputPitch = 0;
    ANGLE_CHECK_GL_MATH(contextWgpu,
                        sizedFormatInfo.computeRowPitch(type, area.width, packState.alignment,
                                                        packState.rowLength, &outputPitch));
    ANGLE_CHECK_GL_MATH(contextWgpu, sizedFormatInfo.computeSkipBytes(
                                         type, outputPitch, 0, packState, false, skipBytesOut));

    *skipBytesOut += (clippedArea.x - area.x) * sizedFormatInfo.pixelBytes +
                     (clippedArea.y - area.y) * outputPitch;

    angle::FormatID angleFormatID =
        angle::Format::InternalFormatToID(sizedFormatInfo.sizedInternalFormat);
    const angle::Format &angleFormat = angle::Format::Get(angleFormatID);

    *paramsOut = rx::PackPixelsParams(clippedArea, angleFormat, outputPitch,
                                      packState.reverseRowOrder, packBuffer, 0);
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

BufferHelper::BufferHelper() {}

BufferHelper::~BufferHelper() {}

void BufferHelper::reset()
{
    mBuffer = nullptr;
    mMappedState.reset();
}

angle::Result BufferHelper::initBuffer(wgpu::Device device,
                                       size_t size,
                                       wgpu::BufferUsage usage,
                                       bool mappedAtCreation)
{
    wgpu::BufferDescriptor descriptor;
    descriptor.size             = size;
    descriptor.usage            = usage;
    descriptor.mappedAtCreation = mappedAtCreation;

    mBuffer = device.CreateBuffer(&descriptor);

    if (mappedAtCreation)
    {
        mMappedState = {wgpu::MapMode::Read | wgpu::MapMode::Write, 0, size};
    }

    return angle::Result::Continue;
}

angle::Result BufferHelper::mapImmediate(ContextWgpu *context,
                                         wgpu::MapMode mode,
                                         size_t offset,
                                         size_t size)
{
    ASSERT(!mMappedState.has_value());

    WGPUBufferMapAsyncStatus mapResult = WGPUBufferMapAsyncStatus_Unknown;

    wgpu::BufferMapCallbackInfo callbackInfo;
    callbackInfo.mode     = wgpu::CallbackMode::WaitAnyOnly;
    callbackInfo.callback = [](WGPUBufferMapAsyncStatus status, void *userdata) {
        *static_cast<WGPUBufferMapAsyncStatus *>(userdata) = status;
    };
    callbackInfo.userdata = &mapResult;

    wgpu::FutureWaitInfo waitInfo;
    waitInfo.future = mBuffer.MapAsync(mode, offset, size, callbackInfo);

    wgpu::Instance instance = context->getDisplay()->getInstance();
    ANGLE_WGPU_TRY(context, instance.WaitAny(1, &waitInfo, -1));

    ASSERT(waitInfo.completed);

    mMappedState = {mode, offset, size};

    return angle::Result::Continue;
}

angle::Result BufferHelper::unmap()
{
    ASSERT(mMappedState.has_value());
    mBuffer.Unmap();
    mMappedState.reset();
    return angle::Result::Continue;
}

uint8_t *BufferHelper::getMapWritePointer(size_t offset, size_t size) const
{
    ASSERT(mBuffer.GetMapState() == wgpu::BufferMapState::Mapped);
    ASSERT(mMappedState.has_value());
    ASSERT(mMappedState->offset <= offset);
    ASSERT(mMappedState->offset + mMappedState->size >= offset + size);

    void *mapPtr = mBuffer.GetMappedRange(offset, size);
    ASSERT(mapPtr);

    return static_cast<uint8_t *>(mapPtr);
}

const std::optional<BufferMapState> &BufferHelper::getMappedState() const
{
    return mMappedState;
}

bool BufferHelper::canMapForRead() const
{
    return (mMappedState.has_value() && (mMappedState->mode & wgpu::MapMode::Read)) ||
           (mBuffer && (mBuffer.GetUsage() & wgpu::BufferUsage::MapRead));
}

bool BufferHelper::canMapForWrite() const
{
    return (mMappedState.has_value() && (mMappedState->mode & wgpu::MapMode::Write)) ||
           (mBuffer && (mBuffer.GetUsage() & wgpu::BufferUsage::MapWrite));
}

wgpu::Buffer &BufferHelper::getBuffer()
{
    return mBuffer;
}

uint64_t BufferHelper::size() const
{
    return mBuffer ? mBuffer.GetSize() : 0;
}

uint8_t *ImageHelper::getReadPixelBuffer(rx::ContextWgpu *contextWgpu, size_t allocationSize)
{
    wgpu::Device device          = contextWgpu->getDisplay()->getDevice();
    wgpu::CommandEncoder encoder = device.CreateCommandEncoder();
    wgpu::Queue queue            = contextWgpu->getDisplay()->getQueue();
    BufferHelper bufferHelper;

    angle::Result result;
    result = bufferHelper.initBuffer(
        device, allocationSize, wgpu::BufferUsage::MapRead | wgpu::BufferUsage::CopyDst, false);
    if (result != angle::Result::Continue)
    {
        return nullptr;
    }
    wgpu::TextureDataLayout textureDataLayout;
    textureDataLayout.offset       = toWgpuLevel(mFirstAllocatedLevel).get();
    textureDataLayout.bytesPerRow  = mTexture.GetWidth();
    textureDataLayout.rowsPerImage = mTexture.GetHeight();

    wgpu::ImageCopyBuffer copyBuffer;
    copyBuffer.buffer = bufferHelper.getBuffer();
    copyBuffer.layout = textureDataLayout;

    wgpu::ImageCopyTexture copyTexture;
    copyTexture.texture  = mTexture;
    copyTexture.mipLevel = toWgpuLevel(mFirstAllocatedLevel).get();

    encoder.CopyTextureToBuffer(&copyTexture, &copyBuffer, &mTextureDescriptor.size);

    wgpu::CommandBuffer commandBuffer = encoder.Finish();
    queue.Submit(1, &commandBuffer);

    result = bufferHelper.mapImmediate(contextWgpu, wgpu::MapMode::Read, 0, allocationSize);
    if (result != angle::Result::Continue)
    {
        return nullptr;
    }
    return static_cast<uint8_t *>(bufferHelper.getBuffer().GetMappedRange(
        toWgpuLevel(mFirstAllocatedLevel).get(), allocationSize));
}
}  // namespace webgpu
}  // namespace rx
