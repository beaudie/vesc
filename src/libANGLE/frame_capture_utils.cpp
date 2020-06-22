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
#include "libANGLE/BinaryStream.h"
#include "libANGLE/Context.h"
#include "libANGLE/Framebuffer.h"

#define ANGLE_STRINGIZE_AUX(a) #a
#define ANGLE_STRINGIZE(a) ANGLE_STRINGIZE_AUX(a)

#define ANGLE_STATIC_ASSERT_SIZE(className, classSize) \
    static_assert(                                                       \
        sizeof(className) == classSize,                                  \
        ANGLE_STRINGIZE(className) " class has changed. Please "         \
            "update its serialize method in " __FILE__);

namespace angle
{

void SerializeContext(gl::BinaryOutputStream *bos, gl::Context *context)
{
    const gl::FramebufferManager &framebufferManager =
        context->getState().getFramebufferManagerForCapture();
    for (const auto &framebuffer : framebufferManager)
    {
        gl::Framebuffer *framebufferPtr = framebuffer.second;
        SerializeFramebuffer(context, bos, framebufferPtr);
    }
}

void SerializeFramebuffer(gl::Context *context,
                          gl::BinaryOutputStream *bos,
                          gl::Framebuffer *framebuffer)
{
    ANGLE_STATIC_ASSERT_SIZE(gl::Framebuffer, 744)
    SerializeFramebufferState(context, bos, framebuffer, &(framebuffer->getState()));
}

void SerializeFramebufferState(gl::Context *context,
                               gl::BinaryOutputStream *bos,
                               gl::Framebuffer *framebuffer,
                               const gl::FramebufferState *framebufferState)
{
    ANGLE_STATIC_ASSERT_SIZE(gl::FramebufferState, 488)
    bos->writeInt(framebufferState->id().value);
    bos->writeString(framebufferState->getLabel());
    bos->writeIntVector(framebufferState->getDrawBufferStates());
    bos->writeInt(framebufferState->getReadBufferState());
    bos->writeInt(framebufferState->getDefaultWidth());
    bos->writeInt(framebufferState->getDefaultHeight());
    bos->writeInt(framebufferState->getDefaultSamples());
    bos->writeInt(framebufferState->getDefaultFixedSampleLocations());
    bos->writeInt(framebufferState->getDefaultLayers());

    context->bindFramebuffer(GL_FRAMEBUFFER, framebufferState->id());
    if (framebuffer->syncState(context, GL_FRAMEBUFFER) != Result::Continue)
    {
        return;
    }

    const std::vector<gl::FramebufferAttachment> &colorAttachments =
        framebufferState->getColorAttachments();
    ScratchBuffer scratchBuffer(1);
    for (size_t i = 0; i < colorAttachments.size(); i++)
    {
        if (colorAttachments[i].isAttached())
        {
            SerializeFramebufferAttachment(context, GL_COLOR_ATTACHMENT0 + (unsigned int)i, bos,
                                           scratchBuffer, framebuffer, &colorAttachments[i]);
        }
    }
    scratchBuffer.clear();
}

void SerializeFramebufferAttachment(gl::Context *context,
                                    GLenum mode,
                                    gl::BinaryOutputStream *bos,
                                    ScratchBuffer &scratchBuffer,
                                    gl::Framebuffer *framebuffer,
                                    const gl::FramebufferAttachment *framebufferAttachment)
{
    ANGLE_STATIC_ASSERT_SIZE(gl::FramebufferAttachment, 48)
    bos->writeInt(framebufferAttachment->type());
    // serialize target variable
    bos->writeInt(framebufferAttachment->getBinding());
    if (framebufferAttachment->type() == GL_TEXTURE)
    {
        SerializeImageIndex(bos, framebufferAttachment->getTextureImageIndex());
    }

    bos->writeInt(framebufferAttachment->getNumViews());
    bos->writeInt(framebufferAttachment->isMultiview());
    bos->writeInt(framebufferAttachment->getBaseViewIndex());
    bos->writeInt(framebufferAttachment->getRenderToTextureSamples());
    MemoryBuffer *pixelsPtr = nullptr;
    context->readBuffer(mode);
    if (framebuffer->syncState(context, GL_FRAMEBUFFER) != Result::Continue)
    {
        return;
    }
    if (ReadPixelsFromAttachment(context, framebuffer, framebufferAttachment, scratchBuffer,
                                 &pixelsPtr))
    {
        bos->writeBytes(pixelsPtr->data(), pixelsPtr->size());
    }
}

void SerializeImageIndex(gl::BinaryOutputStream *bos, const gl::ImageIndex &imageIndex)
{
    ANGLE_STATIC_ASSERT_SIZE(gl::ImageIndex, 16)
    bos->writeEnum(imageIndex.getType());
    bos->writeInt(imageIndex.getLevelIndex());
    bos->writeInt(imageIndex.getLayerIndex());
    bos->writeInt(imageIndex.getLayerCount());
}

bool ReadPixelsFromAttachment(gl::Context *context,
                              gl::Framebuffer *framebuffer,
                              const gl::FramebufferAttachment *framebufferAttachment,
                              ScratchBuffer &scratchBuffer,
                              MemoryBuffer **pixels)
{
    gl::Extents extents = framebufferAttachment->getSize();
    gl::Format format   = framebufferAttachment->getFormat();
    bool success =
        scratchBuffer.get(format.info->pixelBytes * extents.width * extents.height, pixels);
    if (!success)
    {
        return false;
    }
    Result result = framebuffer->readPixels(
        context, gl::Rectangle{0, 0, extents.width, extents.height}, format.info->format,
        format.info->type, gl::PixelPackState{}, nullptr, (*pixels)->data());
    return result == Result::Continue;
}

}  // namespace angle
