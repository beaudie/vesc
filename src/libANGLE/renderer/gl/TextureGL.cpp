//
// Copyright 2015 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//

// TextureGL.cpp: Implements the class methods for TextureGL.

#include "libANGLE/renderer/gl/TextureGL.h"

#include "common/debug.h"
#include "common/utilities.h"
#include "libANGLE/angletypes.h"
#include "libANGLE/formatutils.h"
#include "libANGLE/renderer/gl/BufferGL.h"
#include "libANGLE/renderer/gl/FunctionsGL.h"
#include "libANGLE/renderer/gl/StateManagerGL.h"

namespace rx
{

static void SetUnpackStateForTexImage(StateManagerGL *stateManager, const gl::PixelUnpackState &unpack)
{
    const gl::Buffer *unpackBuffer = unpack.pixelBuffer.get();
    if (unpackBuffer != nullptr)
    {
        UNIMPLEMENTED();
    }
    stateManager->setPixelUnpackState(unpack.alignment, unpack.rowLength);
}

static bool UseTexImage2D(GLenum textureType)
{
    return textureType == GL_TEXTURE_2D || textureType == GL_TEXTURE_CUBE_MAP;
}

static bool UseTexImage3D(GLenum textureType)
{
    return textureType == GL_TEXTURE_2D_ARRAY || textureType == GL_TEXTURE_3D;
}

static bool CompatableTextureTarget(GLenum textureType, GLenum textureTarget)
{
    if (textureType != GL_TEXTURE_CUBE_MAP)
    {
        return textureType == textureTarget;
    }
    else
    {
        return gl::IsCubeMapTextureTarget(textureType);
    }
}

TextureGL::TextureGL(GLenum type, const FunctionsGL *functions, StateManagerGL *stateManager)
    : TextureImpl(),
      mTextureType(type),
      mFunctions(functions),
      mStateManager(stateManager),
      mTextureID(0)
{
    ASSERT(mFunctions);
    ASSERT(mStateManager);

    mFunctions->genTextures(1, &mTextureID);
}

TextureGL::~TextureGL()
{
    if (mTextureID)
    {
        mFunctions->deleteTextures(1, &mTextureID);
        mTextureID = 0;
    }
}

void TextureGL::setUsage(GLenum usage)
{
    // GL_ANGLE_texture_usage not implemented for desktop GL
    UNREACHABLE();
}

template <typename T>
static inline void SyncSamplerStateMember(const FunctionsGL *functions, const gl::SamplerState &newState,
                                          gl::SamplerState &curState, GLenum textureType, GLenum name,
                                          T(gl::SamplerState::*samplerMember))
{
    if (curState.*samplerMember != newState.*samplerMember)
    {
        curState.*samplerMember = newState.*samplerMember;
        functions->texParameterf(textureType, name, curState.*samplerMember);
    }
}

void TextureGL::setSamplerState(const gl::SamplerState &samplerState)
{
    if (mSamplerState != samplerState)
    {
        mStateManager->bindTexture(mTextureType, mTextureID);
        SyncSamplerStateMember(mFunctions, samplerState, mSamplerState, mTextureType, GL_TEXTURE_MIN_FILTER, &gl::SamplerState::minFilter);
        SyncSamplerStateMember(mFunctions, samplerState, mSamplerState, mTextureType, GL_TEXTURE_MAG_FILTER, &gl::SamplerState::magFilter);
        SyncSamplerStateMember(mFunctions, samplerState, mSamplerState, mTextureType, GL_TEXTURE_WRAP_S, &gl::SamplerState::wrapS);
        SyncSamplerStateMember(mFunctions, samplerState, mSamplerState, mTextureType, GL_TEXTURE_WRAP_T, &gl::SamplerState::wrapT);
        SyncSamplerStateMember(mFunctions, samplerState, mSamplerState, mTextureType, GL_TEXTURE_WRAP_R, &gl::SamplerState::wrapR);
        SyncSamplerStateMember(mFunctions, samplerState, mSamplerState, mTextureType, GL_TEXTURE_MAX_ANISOTROPY_EXT, &gl::SamplerState::maxAnisotropy);
        SyncSamplerStateMember(mFunctions, samplerState, mSamplerState, mTextureType, GL_TEXTURE_BASE_LEVEL, &gl::SamplerState::baseLevel);
        SyncSamplerStateMember(mFunctions, samplerState, mSamplerState, mTextureType, GL_TEXTURE_MAX_LEVEL, &gl::SamplerState::maxLevel);
        SyncSamplerStateMember(mFunctions, samplerState, mSamplerState, mTextureType, GL_TEXTURE_MIN_LOD, &gl::SamplerState::minLod);
        SyncSamplerStateMember(mFunctions, samplerState, mSamplerState, mTextureType, GL_TEXTURE_MAX_LOD, &gl::SamplerState::maxLod);
        SyncSamplerStateMember(mFunctions, samplerState, mSamplerState, mTextureType, GL_TEXTURE_COMPARE_MODE, &gl::SamplerState::compareMode);
        SyncSamplerStateMember(mFunctions, samplerState, mSamplerState, mTextureType, GL_TEXTURE_COMPARE_FUNC, &gl::SamplerState::compareFunc);
        SyncSamplerStateMember(mFunctions, samplerState, mSamplerState, mTextureType, GL_TEXTURE_SWIZZLE_R, &gl::SamplerState::swizzleRed);
        SyncSamplerStateMember(mFunctions, samplerState, mSamplerState, mTextureType, GL_TEXTURE_SWIZZLE_G, &gl::SamplerState::swizzleGreen);
        SyncSamplerStateMember(mFunctions, samplerState, mSamplerState, mTextureType, GL_TEXTURE_SWIZZLE_B, &gl::SamplerState::swizzleBlue);
        SyncSamplerStateMember(mFunctions, samplerState, mSamplerState, mTextureType, GL_TEXTURE_SWIZZLE_A, &gl::SamplerState::swizzleAlpha);
    }

}

gl::Error TextureGL::setImage(GLenum target, size_t level, GLenum internalFormat, const gl::Extents &size, GLenum format, GLenum type,
                              const gl::PixelUnpackState &unpack, const uint8_t *pixels)
{
    ASSERT(CompatableTextureTarget(mTextureType, target));

    SetUnpackStateForTexImage(mStateManager, unpack);

    mStateManager->bindTexture(mTextureType, mTextureID);
    if (UseTexImage2D(mTextureType))
    {
        ASSERT(size.depth == 1);
        mFunctions->texImage2D(target, level, internalFormat, size.width, size.height, 0, format, type, pixels);
    }
    else if (UseTexImage3D(mTextureType))
    {
        mFunctions->texImage3D(target, level, internalFormat, size.width, size.height, size.depth, 0, format, type, pixels);
    }
    else
    {
        UNREACHABLE();
    }

    return gl::Error(GL_NO_ERROR);
}

gl::Error TextureGL::setSubImage(GLenum target, size_t level, const gl::Box &area, GLenum format, GLenum type,
                                 const gl::PixelUnpackState &unpack, const uint8_t *pixels)
{
    ASSERT(CompatableTextureTarget(mTextureType, target));

    SetUnpackStateForTexImage(mStateManager, unpack);

    mStateManager->bindTexture(mTextureType, mTextureID);
    if (UseTexImage2D(mTextureType))
    {
        ASSERT(area.z == 0 && area.depth == 1);
        mFunctions->texSubImage2D(target, level, area.x, area.y, area.width, area.height, format, type, pixels);
    }
    else if (UseTexImage3D(mTextureType))
    {
        mFunctions->texSubImage3D(target, level, area.x, area.y, area.z, area.width, area.height, area.depth,
                                  format, type, pixels);
    }
    else
    {
        UNREACHABLE();
    }

    return gl::Error(GL_NO_ERROR);
}

gl::Error TextureGL::setCompressedImage(GLenum target, size_t level, GLenum internalFormat, const gl::Extents &size,
                                        const gl::PixelUnpackState &unpack, const uint8_t *pixels)
{
    ASSERT(CompatableTextureTarget(mTextureType, target));

    SetUnpackStateForTexImage(mStateManager, unpack);

    const gl::InternalFormat &internalFormatInfo = gl::GetInternalFormatInfo(internalFormat);
    size_t depthPitch = internalFormatInfo.computeDepthPitch(GL_UNSIGNED_BYTE, size.width, size.height, unpack.alignment, unpack.rowLength);
    size_t dataSize = internalFormatInfo.computeBlockSize(GL_UNSIGNED_BYTE, size.width, size.height) * depthPitch;

    mStateManager->bindTexture(mTextureType, mTextureID);
    if (UseTexImage2D(mTextureType))
    {
        ASSERT(size.depth == 1);
        mFunctions->compressedTexImage2D(target, level, internalFormat, size.width, size.height, 0, dataSize, pixels);
    }
    else if (UseTexImage3D(mTextureType))
    {
        mFunctions->compressedTexImage3D(target, level, internalFormat, size.width, size.height, size.depth, 0,
                                         dataSize, pixels);
    }
    else
    {
        UNREACHABLE();
    }

    return gl::Error(GL_NO_ERROR);
}

gl::Error TextureGL::setCompressedSubImage(GLenum target, size_t level, const gl::Box &area, GLenum format,
                                           const gl::PixelUnpackState &unpack, const uint8_t *pixels)
{
    UNIMPLEMENTED();
    return gl::Error(GL_INVALID_OPERATION);
}

gl::Error TextureGL::copyImage(GLenum target, size_t level, const gl::Rectangle &sourceArea, GLenum internalFormat,
                               const gl::Framebuffer *source)
{
    UNIMPLEMENTED();
    return gl::Error(GL_INVALID_OPERATION);
}

gl::Error TextureGL::copySubImage(GLenum target, size_t level, const gl::Offset &destOffset, const gl::Rectangle &sourceArea,
                                  const gl::Framebuffer *source)
{
    UNIMPLEMENTED();
    return gl::Error(GL_INVALID_OPERATION);
}

gl::Error TextureGL::setStorage(GLenum target, size_t levels, GLenum internalFormat, const gl::Extents &size)
{
    UNIMPLEMENTED();
    return gl::Error(GL_INVALID_OPERATION);
}

gl::Error TextureGL::generateMipmaps()
{
    UNIMPLEMENTED();
    return gl::Error(GL_INVALID_OPERATION);
}

void TextureGL::bindTexImage(egl::Surface *surface)
{
    UNIMPLEMENTED();
}

void TextureGL::releaseTexImage()
{
    UNIMPLEMENTED();
}

GLuint TextureGL::getTextureID() const
{
    return mTextureID;
}

}
