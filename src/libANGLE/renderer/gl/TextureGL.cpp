//
// Copyright 2015 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//

// TextureGL.cpp: Implements the TextureGL class.

#include "libANGLE/renderer/gl/TextureGL.h"

namespace rx
{

TextureGL::TextureGL()
{
}

TextureGL::~TextureGL()
{
}

void TextureGL::setUsage(GLenum usage)
{
}

gl::Error TextureGL::setImage(GLenum target, size_t level, GLenum internalFormat, const gl::Extents &size, GLenum format, GLenum type, const gl::PixelUnpackState &unpack, const uint8_t *pixels)
{
    return gl::Error(GL_NO_ERROR);
}

gl::Error TextureGL::setSubImage(GLenum target, size_t level, const gl::Box &area, GLenum format, GLenum type, const gl::PixelUnpackState &unpack, const uint8_t *pixels)
{
    return gl::Error(GL_NO_ERROR);
}

gl::Error TextureGL::setCompressedImage(GLenum target, size_t level, GLenum internalFormat, const gl::Extents &size, const gl::PixelUnpackState &unpack, const uint8_t *pixels)
{
    return gl::Error(GL_NO_ERROR);
}

gl::Error TextureGL::setCompressedSubImage(GLenum target, size_t level, const gl::Box &area, GLenum format, const gl::PixelUnpackState &unpack, const uint8_t *pixels)
{
    return gl::Error(GL_NO_ERROR);
}

gl::Error TextureGL::copyImage(GLenum target, size_t level, const gl::Rectangle &sourceArea, GLenum internalFormat, const gl::Framebuffer *source)
{
    return gl::Error(GL_NO_ERROR);
}

gl::Error TextureGL::copySubImage(GLenum target, size_t level, const gl::Offset &destOffset, const gl::Rectangle &sourceArea, const gl::Framebuffer *source)
{
    return gl::Error(GL_NO_ERROR);
}

gl::Error TextureGL::setStorage(GLenum target, size_t levels, GLenum internalFormat, const gl::Extents &size)
{
    return gl::Error(GL_NO_ERROR);
}

gl::Error TextureGL::generateMipmaps()
{
    return gl::Error(GL_NO_ERROR);
}

void TextureGL::bindTexImage(egl::Surface *surface)
{
}

void TextureGL::releaseTexImage()
{
}

}
