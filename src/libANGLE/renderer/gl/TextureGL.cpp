//
// Copyright 2015 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//

// TextureGL.cpp: Implements the class methods for TextureGL.

#include "libANGLE/renderer/gl/TextureGL.h"

#include "common/debug.h"
#include "libANGLE/angletypes.h"
#include "libANGLE/renderer/gl/BufferGL.h"
#include "libANGLE/renderer/gl/FunctionsGL.h"
#include "libANGLE/renderer/gl/StateManagerGL.h"

namespace rx
{

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

gl::Error TextureGL::setImage(GLenum target, size_t level, GLenum internalFormat, const gl::Extents &size, GLenum format, GLenum type,
                              const gl::PixelUnpackState &unpack, const uint8_t *pixels)
{
    const gl::Buffer *unpackBuffer = unpack.pixelBuffer.get();
    if (unpackBuffer != nullptr)
    {
        UNIMPLEMENTED();
    }
    mStateManager->setPixelUnpackState(unpack.alignment, unpack.rowLength);

    mStateManager->setTexture(mTextureType, mTextureID);
    if (mTextureType == GL_TEXTURE_2D || mTextureType == GL_TEXTURE_CUBE_MAP)
    {
        mFunctions->texImage2D(target, level, internalFormat, size.width, size.height, 0, format, type, pixels);
    }
    else if (mTextureType == GL_TEXTURE_2D_ARRAY || mTextureType == GL_TEXTURE_3D)
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
    UNIMPLEMENTED();
    return gl::Error(GL_INVALID_OPERATION);
}

gl::Error TextureGL::setCompressedImage(GLenum target, size_t level, GLenum internalFormat, const gl::Extents &size,
                                        const gl::PixelUnpackState &unpack, const uint8_t *pixels)
{
    UNIMPLEMENTED();
    return gl::Error(GL_INVALID_OPERATION);
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
