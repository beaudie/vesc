//
// Copyright 2019 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//

// OpenGLTextureSurfaceGL.cpp: an implementation of PBuffers created from OpenGL textures using
//                             EGL_ANGLE_opengl_texture_client_buffer

#include "libANGLE/renderer/gl/OpenGLTextureSurfaceGL.h"

#include "common/debug.h"
#include "libANGLE/AttributeMap.h"
#include "libANGLE/renderer/gl/TextureGL.h"
#include "libANGLE/renderer/gl/renderergl_utils.h"

namespace rx
{

OpenGLTextureSurfaceGL::OpenGLTextureSurfaceGL(const egl::SurfaceState &state,
                                               EGLClientBuffer buffer,
                                               const egl::AttributeMap &attribs)
    : SurfaceGL(state),
      mTexture(0),
      mWidth(0),
      mHeight(0),
      mTextureTarget(0),
      mInternalFormat(0),
      mFormat(0),
      mType(0),
      mBoundTexture(nullptr)
{
    mTexture        = static_cast<GLuint>(attribs.get(EGL_OPENGL_TEXTURE_ANGLE));
    mWidth          = static_cast<int>(attribs.get(EGL_WIDTH));
    mHeight         = static_cast<int>(attribs.get(EGL_HEIGHT));
    mTextureTarget  = static_cast<int>(attribs.get(EGL_TEXTURE_TARGET));
    mInternalFormat = static_cast<int>(attribs.get(EGL_TEXTURE_INTERNAL_FORMAT_ANGLE));
    mFormat         = static_cast<int>(attribs.get(EGL_TEXTURE_FORMAT));
    mType           = static_cast<int>(attribs.get(EGL_TEXTURE_TYPE_ANGLE));
}

OpenGLTextureSurfaceGL::~OpenGLTextureSurfaceGL()
{
}

egl::Error OpenGLTextureSurfaceGL::initialize(const egl::Display *display)
{
    return egl::NoError();
}

egl::Error OpenGLTextureSurfaceGL::makeCurrent(const gl::Context *context)
{
    return egl::NoError();
}

egl::Error OpenGLTextureSurfaceGL::unMakeCurrent(const gl::Context *context)
{
    GetFunctionsGL(context)->flush();
    return egl::NoError();
}

egl::Error OpenGLTextureSurfaceGL::swap(const gl::Context *context)
{
    return egl::NoError();
}

egl::Error OpenGLTextureSurfaceGL::postSubBuffer(const gl::Context *context,
                                              EGLint x,
                                              EGLint y,
                                              EGLint width,
                                              EGLint height)
{
    UNREACHABLE();
    return egl::NoError();
}

egl::Error OpenGLTextureSurfaceGL::querySurfacePointerANGLE(EGLint attribute, void **value)
{
    UNREACHABLE();
    return egl::NoError();
}

egl::Error OpenGLTextureSurfaceGL::bindTexImage(const gl::Context *context,
                                                gl::Texture *texture,
                                                EGLint buffer)
{
    TextureGL *textureGL = GetImplAs<TextureGL>(texture);
    textureGL->saveTextureIDForOpenGLTextureClientBuffer(mTexture);
    mBoundTexture = textureGL;
    return egl::NoError();
}

egl::Error OpenGLTextureSurfaceGL::releaseTexImage(const gl::Context *context, EGLint buffer)
{
    mBoundTexture->restoreTextureIDForOpenGLTextureClientBuffer();
    mBoundTexture = nullptr;
    return egl::NoError();
}

void OpenGLTextureSurfaceGL::setSwapInterval(EGLint interval)
{
    UNREACHABLE();
}

EGLint OpenGLTextureSurfaceGL::getWidth() const
{
    return mWidth;
}

EGLint OpenGLTextureSurfaceGL::getHeight() const
{
    return mHeight;
}

EGLint OpenGLTextureSurfaceGL::isPostSubBufferSupported() const
{
    UNREACHABLE();
    return EGL_FALSE;
}

EGLint OpenGLTextureSurfaceGL::getSwapBehavior() const
{
    // N/A because you can't MakeCurrent a texture; return any valid value.
    return EGL_BUFFER_PRESERVED;
}

// static
bool OpenGLTextureSurfaceGL::validateAttributes(EGLClientBuffer buffer,
                                                const egl::AttributeMap &attribs)
{
    // buffer must be a valid OpenGL texture object.
    if (buffer == 0)
    {
        return false;
    }

    // The width and height specified must be at least (1, 1).
    EGLAttrib width  = attribs.get(EGL_WIDTH);
    EGLAttrib height = attribs.get(EGL_HEIGHT);
    if (width <= 0 || height <= 0)
    {
        return false;
    }

    // TODO(kbr): consider adding more validation.

    return true;
}

FramebufferImpl *OpenGLTextureSurfaceGL::createDefaultFramebuffer(const gl::Context *context,
                                                               const gl::FramebufferState &state)
{
    ERR() << "Not supported for OpenGL texture surfaces.";
    return nullptr;
}

}  // namespace rx
