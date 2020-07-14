//
// Copyright 2020 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//
// frame_capture_utils.cpp:
//   ANGLE frame capture util implementation.
//

#include "libANGLE/frame_capture_utils.h"

#include "common/MemoryBuffer.h"
#include "common/angleutils.h"
#include "libANGLE/BinaryStream.h"
#include "libANGLE/Buffer.h"
#include "libANGLE/Context.h"
#include "libANGLE/Context.inl.h"
#include "libANGLE/Framebuffer.h"
#include "libANGLE/renderer/FramebufferImpl.h"

namespace angle
{

bool IsValidColorAttachmentBinding(GLenum binding, size_t colorAttachmentsCount)
{
    return binding == GL_BACK || (binding >= GL_COLOR_ATTACHMENT0 &&
                                  (binding - GL_COLOR_ATTACHMENT0) < colorAttachmentsCount);
}

Result ReadPixelsFromAttachment(const gl::Context *context,
                                gl::Framebuffer *framebuffer,
                                const gl::FramebufferAttachment &framebufferAttachment,
                                ScratchBuffer *scratchBuffer,
                                MemoryBuffer **pixels)
{
    gl::Extents extents       = framebufferAttachment.getSize();
    GLenum binding            = framebufferAttachment.getBinding();
    gl::InternalFormat format = *framebufferAttachment.getFormat().info;
    if (IsValidColorAttachmentBinding(binding,
                                      framebuffer->getState().getColorAttachments().size()))
    {
        format = framebuffer->getImplementation()->getImplementationColorReadFormat(context);
    }
    ANGLE_CHECK_GL_ALLOC(const_cast<gl::Context *>(context),
                         scratchBuffer->getInitialized(
                             format.pixelBytes * extents.width * extents.height, pixels, 0));
    ANGLE_TRY(framebuffer->readPixels(context, gl::Rectangle{0, 0, extents.width, extents.height},
                                      format.format, format.type, gl::PixelPackState{}, nullptr,
                                      (*pixels)->data()));
    return Result::Continue;
}

gl::Buffer *CreateTempBuffer(const gl::Context *context, gl::BufferManager &bufferManager)
{
    gl::BufferID tempBufferID = bufferManager.createBuffer();
    gl::Buffer *tempBuffer =
        bufferManager.checkBufferAllocation(context->getImplementation(), tempBufferID);
    if (tempBuffer == nullptr)
    {
        return nullptr;
    }
    GLint64 tempBufferSize = 0;
    for (const auto &buffer : bufferManager)
    {
        gl::Buffer *bufferPtr = buffer.second;
        if (bufferPtr == tempBuffer)
        {
            continue;
        }
        tempBufferSize = std::max(tempBufferSize, bufferPtr->getSize());
    }
    Result result =
        tempBuffer->bufferData(const_cast<gl::Context *>(context), gl::BufferBinding::CopyWrite,
                               nullptr, tempBufferSize, gl::BufferUsage::DynamicRead);
    if (result != Result::Continue)
    {
        return nullptr;
    }
    return tempBuffer;
}

Result SerializeContext(gl::BinaryOutputStream *bos, const gl::Context *context)
{
    const gl::FramebufferManager &framebufferManager =
        context->getState().getFramebufferManagerForCapture();
    for (const auto &framebuffer : framebufferManager)
    {
        gl::Framebuffer *framebufferPtr = framebuffer.second;
        ANGLE_TRY(SerializeFramebuffer(context, bos, framebufferPtr));
    }
    gl::BufferManager &bufferManager =
        const_cast<gl::BufferManager &>(context->getState().getBufferManagerForCapture());
    gl::Buffer *tempBuffer = CreateTempBuffer(context, bufferManager);
    if (tempBuffer == nullptr)
    {
        return Result::Stop;
    }
    for (const auto &buffer : bufferManager)
    {
        gl::Buffer *bufferPtr = buffer.second;
        if (bufferPtr == tempBuffer)
        {
            continue;
        }
        ANGLE_TRY(SerializeBuffer(context, bos, tempBuffer, bufferPtr));
    }
    bufferManager.deleteObject(context, tempBuffer->id());
    return Result::Continue;
}

Result SerializeFramebuffer(const gl::Context *context,
                            gl::BinaryOutputStream *bos,
                            gl::Framebuffer *framebuffer)
{
    return SerializeFramebufferState(context, bos, framebuffer, framebuffer->getState());
}

Result SerializeFramebufferState(const gl::Context *context,
                                 gl::BinaryOutputStream *bos,
                                 gl::Framebuffer *framebuffer,
                                 const gl::FramebufferState &framebufferState)
{
    bos->writeInt(framebufferState.id().value);
    bos->writeString(framebufferState.getLabel());
    bos->writeIntVector(framebufferState.getDrawBufferStates());
    bos->writeInt(framebufferState.getReadBufferState());
    bos->writeInt(framebufferState.getDefaultWidth());
    bos->writeInt(framebufferState.getDefaultHeight());
    bos->writeInt(framebufferState.getDefaultSamples());
    bos->writeInt(framebufferState.getDefaultFixedSampleLocations());
    bos->writeInt(framebufferState.getDefaultLayers());

    const std::vector<gl::FramebufferAttachment> &colorAttachments =
        framebufferState.getColorAttachments();
    ScratchBuffer scratchBuffer(1);
    for (const gl::FramebufferAttachment &colorAttachment : colorAttachments)
    {
        if (colorAttachment.isAttached())
        {
            ANGLE_TRY(SerializeFramebufferAttachment(context, bos, &scratchBuffer, framebuffer,
                                                     colorAttachment));
        }
    }
    if (framebuffer->getDepthStencilAttachment())
    {
        ANGLE_TRY(SerializeFramebufferAttachment(context, bos, &scratchBuffer, framebuffer,
                                                 *framebuffer->getDepthStencilAttachment()));
    }
    else
    {
        if (framebuffer->getDepthAttachment())
        {
            ANGLE_TRY(SerializeFramebufferAttachment(context, bos, &scratchBuffer, framebuffer,
                                                     *framebuffer->getDepthAttachment()));
        }
        if (framebuffer->getStencilAttachment())
        {
            ANGLE_TRY(SerializeFramebufferAttachment(context, bos, &scratchBuffer, framebuffer,
                                                     *framebuffer->getStencilAttachment()));
        }
    }
    scratchBuffer.clear();
    return Result::Continue;
}

Result SerializeFramebufferAttachment(const gl::Context *context,
                                      gl::BinaryOutputStream *bos,
                                      ScratchBuffer *scratchBuffer,
                                      gl::Framebuffer *framebuffer,
                                      const gl::FramebufferAttachment &framebufferAttachment)
{
    bos->writeInt(framebufferAttachment.type());
    // serialize target variable
    bos->writeInt(framebufferAttachment.getBinding());
    if (framebufferAttachment.type() == GL_TEXTURE)
    {
        SerializeImageIndex(bos, framebufferAttachment.getTextureImageIndex());
    }
    bos->writeInt(framebufferAttachment.getNumViews());
    bos->writeInt(framebufferAttachment.isMultiview());
    bos->writeInt(framebufferAttachment.getBaseViewIndex());
    bos->writeInt(framebufferAttachment.getRenderToTextureSamples());

    GLenum prevReadBufferState = framebuffer->getReadBufferState();
    GLenum binding             = framebufferAttachment.getBinding();
    if (IsValidColorAttachmentBinding(binding,
                                      framebuffer->getState().getColorAttachments().size()))
    {
        framebuffer->setReadBuffer(framebufferAttachment.getBinding());
        ANGLE_TRY(framebuffer->syncState(context, GL_FRAMEBUFFER));
    }
    MemoryBuffer *pixelsPtr = nullptr;
    ANGLE_TRY(ReadPixelsFromAttachment(context, framebuffer, framebufferAttachment, scratchBuffer,
                                       &pixelsPtr));
    bos->writeBytes(pixelsPtr->data(), pixelsPtr->size());
    // Reset framebuffer state
    framebuffer->setReadBuffer(prevReadBufferState);
    return Result::Continue;
}

void SerializeImageIndex(gl::BinaryOutputStream *bos, const gl::ImageIndex &imageIndex)
{
    bos->writeEnum(imageIndex.getType());
    bos->writeInt(imageIndex.getLevelIndex());
    bos->writeInt(imageIndex.getLayerIndex());
    bos->writeInt(imageIndex.getLayerCount());
}

Result SerializeBuffer(const gl::Context *context,
                       gl::BinaryOutputStream *bos,
                       gl::Buffer *tempBuffer,
                       gl::Buffer *buffer)
{
    SerializeBufferState(bos, buffer->getState());
    ANGLE_TRY(tempBuffer->copyBufferSubData(context, buffer, 0, 0, buffer->getSize()));
    ANGLE_TRY(tempBuffer->mapRange(context, 0, buffer->getSize(), GL_MAP_READ_BIT));
    GLboolean dontCare;
    bos->writeBytes(reinterpret_cast<const uint8_t *>(tempBuffer->getMapPointer()),
                    tempBuffer->getMapLength());
    ANGLE_TRY(tempBuffer->unmap(context, &dontCare));
    return Result::Continue;
}

void SerializeBufferState(gl::BinaryOutputStream *bos, const gl::BufferState &bufferState)
{
    bos->writeString(bufferState.getLabel());
    bos->writeEnum(bufferState.getUsage());
    bos->writeInt(bufferState.getSize());
    bos->writeInt(bufferState.getAccessFlags());
    bos->writeInt(bufferState.getAccess());
    bos->writeInt(bufferState.isMapped());
    bos->writeInt(bufferState.getMapOffset());
    bos->writeInt(bufferState.getMapLength());
    bos->writeInt(bufferState.getBindingCount());
    bos->writeInt(bufferState.getTransformFeedbackIndexedBindingCount());
    bos->writeInt(bufferState.getTransformFeedbackGenericBindingCount());
}

}  // namespace angle
