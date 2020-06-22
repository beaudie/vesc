// Copyright 2020 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//
// SerializeContext.cpp:
//   ANGLE serialize context implementation.
//

#include "libANGLE/SerializeContext.h"

#include <iostream>

#define ANGLE_MACRO_STRINGIZE_AUX(a) #a
#define ANGLE_MACRO_STRINGIZE(a) ANGLE_MACRO_STRINGIZE_AUX(a)

#define ANGLE_MACRO_STATIC_ASSERT_SIZE(className, classSize) \
    static_assert(                                                       \
        sizeof(className) == classSize,                                  \
        ANGLE_MACRO_STRINGIZE(className) " class has changed. Please "   \
            "update Serialize" ANGLE_MACRO_STRINGIZE(className) " method");

namespace gl
{

void Serialize(BinaryOutputStream *bos, Context *context)
{
    const FramebufferManager &framebufferManager =
        context->getState().getFramebufferManagerForCapture();
    for (auto framebuffer : framebufferManager)
    {
        SerializeFramebuffer(bos, context, framebuffer.second);
    }
}

void SerializeFramebuffer(BinaryOutputStream *bos, Context *context, Framebuffer *framebuffer)
{
    ANGLE_MACRO_STATIC_ASSERT_SIZE(Framebuffer, 744)
    SerializeFramebufferState(bos, context, &framebuffer->mState);
    if (framebuffer->mCachedStatus.valid())
    {
        bos->writeInt<GLuint>(framebuffer->mCachedStatus.value());
    }
    bos->writeInt<unsigned long>(framebuffer->mFloat32ColorAttachmentBits.to_ulong());
}

void SerializeFramebufferState(BinaryOutputStream *bos,
                               Context *context,
                               FramebufferState *framebufferState)
{
    ANGLE_MACRO_STATIC_ASSERT_SIZE(FramebufferState, 488)
    bos->writeInt<GLuint>(framebufferState->mId.value);
    bos->writeInt<ContextID>(framebufferState->mOwningContextID);
    bos->writeString(framebufferState->mLabel);
    bos->writeIntVector<unsigned int>(framebufferState->mDrawBufferStates);
    bos->writeInt<unsigned int>(framebufferState->mReadBufferState);
    bos->writeInt<unsigned long>(framebufferState->mEnabledDrawBuffers.to_ulong());
    bos->writeInt<unsigned long>(framebufferState->mDrawBufferTypeMask.to_ulong());
    bos->writeInt<GLint>(framebufferState->mDefaultWidth);
    bos->writeInt<GLint>(framebufferState->mDefaultHeight);
    bos->writeInt<GLint>(framebufferState->mDefaultSamples);
    bos->writeInt<bool>(framebufferState->mDefaultFixedSampleLocations);
    bos->writeInt<GLint>(framebufferState->mDefaultLayers);
    bos->writeInt<bool>(framebufferState->mWebGLDepthStencilConsistent);
    bos->writeInt<unsigned long>(framebufferState->mDrawBufferFeedbackLoops.to_ulong());
    bos->writeInt<bool>(framebufferState->mDepthBufferFeedbackLoop);
    bos->writeInt<bool>(framebufferState->mStencilBufferFeedbackLoop);
    bos->writeInt<bool>(framebufferState->mHasRenderingFeedbackLoop);
    SerializeOffset(bos, &framebufferState->mSurfaceTextureOffset);
    context->bindFramebuffer(GL_FRAMEBUFFER, framebufferState->mId);

    if (framebufferState->mId.value == 0)  // default framebuffer
    {
        const FramebufferAttachment *backBufferAttachment =
            framebufferState->getAttachment(context, GL_BACK);
        std::vector<uint8_t> backBufferPixels =
            ReadPixelsFromAttachment(context, GL_BACK, backBufferAttachment);
        bos->writeBytes(backBufferPixels.data(), backBufferPixels.size());
        // TODO(nguyenmh) : http://anglebug.com/4777: Serialize depth and stencil attachments
    }
    else
    {
        for (size_t i = 0; i < framebufferState->mColorAttachments.size(); i++)
        {
            SerializeFramebufferAttachment(bos, context, GL_COLOR_ATTACHMENT0 + (unsigned int)i,
                                           &framebufferState->mColorAttachments[i]);
        }

        // TODO(nguyenmh) : http://anglebug.com/4777: Serialize depth and stencil attachments
    }
}

void SerializeOffset(BinaryOutputStream *bos, Offset *offset)
{
    ANGLE_MACRO_STATIC_ASSERT_SIZE(Offset, 12)
    bos->writeInt<int>(offset->x);
    bos->writeInt<int>(offset->y);
    bos->writeInt<int>(offset->z);
}

void SerializeFramebufferAttachment(BinaryOutputStream *bos,
                                    Context *context,
                                    GLenum mode,
                                    FramebufferAttachment *framebufferAttachment)
{
    ANGLE_MACRO_STATIC_ASSERT_SIZE(FramebufferAttachment, 48)
    bos->writeInt<unsigned int>(framebufferAttachment->mType);
    // serialize target variable
    bos->writeInt<unsigned int>(framebufferAttachment->mTarget.mBinding);
    SerializeImageIndex(bos, &framebufferAttachment->mTarget.mTextureIndex);

    bos->writeInt<GLsizei>(framebufferAttachment->mNumViews);
    bos->writeInt<bool>(framebufferAttachment->mIsMultiview);
    bos->writeInt<GLint>(framebufferAttachment->mBaseViewIndex);
    bos->writeInt<GLint>(framebufferAttachment->mRenderToTextureSamples);
    std::vector<uint8_t> pixels = ReadPixelsFromAttachment(context, mode, framebufferAttachment);
    bos->writeBytes(pixels.data(), pixels.size());
}

void SerializeImageIndex(BinaryOutputStream *bos, ImageIndex *imageIndex)
{
    ANGLE_MACRO_STATIC_ASSERT_SIZE(ImageIndex, 16)
    bos->writeEnum<TextureType>(imageIndex->mType);
    bos->writeInt<GLint>(imageIndex->mLevelIndex);
    bos->writeInt<GLint>(imageIndex->mLayerIndex);
    bos->writeInt<GLint>(imageIndex->mLayerCount);
}

std::vector<uint8_t> ReadPixelsFromAttachment(Context *context,
                                              GLenum mode,
                                              const FramebufferAttachment *framebufferAttachment)
{
    Extents extents       = framebufferAttachment->getSize();
    GLuint componentCount = framebufferAttachment->getFormat().info->componentCount;
    std::vector<uint8_t> pixels(extents.width * extents.height * componentCount);
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
                            pixels.data());
    }
    else
    {
        context->readPixels(0, 0, extents.width, extents.height, mode, GL_UNSIGNED_BYTE,
                            pixels.data());
    }
    return pixels;
}

}  // namespace gl
