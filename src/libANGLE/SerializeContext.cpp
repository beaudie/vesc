//
// Copyright 2020 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//
// SerializeContext.cpp:
//   ANGLE serialize context implementation.
//

#include "libANGLE/SerializeContext.h"

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
        SerializeFramebuffer(bos, context, framebufferPtr);
    }
}

void SerializeFramebuffer(gl::BinaryOutputStream *bos,
                          gl::Context *context,
                          gl::Framebuffer *framebuffer)
{
    ANGLE_STATIC_ASSERT_SIZE(gl::Framebuffer, 744)
    SerializeFramebufferState(bos, context,
                              const_cast<gl::FramebufferState *>(&(framebuffer->getState())));
}

void SerializeFramebufferState(gl::BinaryOutputStream *bos,
                               gl::Context *context,
                               gl::FramebufferState *framebufferState)
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

    const std::vector<gl::FramebufferAttachment> &colorAttachments =
        framebufferState->getColorAttachments();
    ScratchBuffer scratchBuffer(1);
    for (size_t i = 0; i < colorAttachments.size(); i++)
    {
        SerializeFramebufferAttachment(bos, context, GL_COLOR_ATTACHMENT0 + (unsigned int)i,
                                       &colorAttachments[i], scratchBuffer);
    }
    scratchBuffer.clear();
}

void SerializeFramebufferAttachment(gl::BinaryOutputStream *bos,
                                    gl::Context *context,
                                    GLenum mode,
                                    const gl::FramebufferAttachment *framebufferAttachment,
                                    ScratchBuffer &scratchBuffer)
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
    if (ReadPixelsFromAttachment(context, mode, framebufferAttachment, scratchBuffer, &pixelsPtr))
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
                              GLenum mode,
                              const gl::FramebufferAttachment *framebufferAttachment,
                              ScratchBuffer &scratchBuffer,
                              MemoryBuffer **pixels)
{
    gl::Extents extents   = framebufferAttachment->getSize();
    GLuint componentCount = framebufferAttachment->getFormat().info->componentCount;
    bool success = scratchBuffer.get(componentCount * extents.width * extents.height, pixels);
    if (!success)
    {
        return false;
    }

    if ((GL_COLOR_ATTACHMENT0 <= mode && mode <= GL_COLOR_ATTACHMENT15) ||
        mode == GL_BACK)  // color attachment
    {
        ASSERT(componentCount == 4 || componentCount == 3 || componentCount == 1);
        context->readBuffer(mode);
        GLenum format = GL_RGBA;
        if (componentCount == 3)
        {
            format = GL_RGB;
        }
        else if (componentCount == 1)
        {
            format = GL_RED;
        }
        context->readPixels(0, 0, extents.width, extents.height, format, GL_UNSIGNED_BYTE,
                            (*pixels)->data());
    }
    else
    {
        context->readPixels(0, 0, extents.width, extents.height, mode, GL_UNSIGNED_BYTE,
                            (*pixels)->data());
    }
    return true;
}

}  // namespace angle
