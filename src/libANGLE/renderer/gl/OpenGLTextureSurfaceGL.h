//
// Copyright 2019 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//

// OpenGLTextureSurfaceGL.h: an implementation of PBuffers created from OpenGL textures using
//                           EGL_ANGLE_opengl_texture_client_buffer

#ifndef LIBANGLE_RENDERER_GL_OPENGLTEXTURESURFACEGL_H_
#define LIBANGLE_RENDERER_GL_OPENGLTEXTURESURFACEGL_H_

#include "libANGLE/renderer/gl/SurfaceGL.h"
#include "libANGLE/renderer/gl/cgl/DisplayCGL.h"

namespace egl
{
class AttributeMap;
}  // namespace egl

namespace rx
{

class DisplayCGL;
class FunctionsGL;
class StateManagerGL;
class TextureGL;

class OpenGLTextureSurfaceGL : public SurfaceGL
{
  public:
    OpenGLTextureSurfaceGL(const egl::SurfaceState &state,
                           // TODO(kbr): what should the context be, if anything?
                           // CGLContextObj cglContext,
                           EGLClientBuffer buffer,
                           const egl::AttributeMap &attribs);
    ~OpenGLTextureSurfaceGL() override;

    egl::Error initialize(const egl::Display *display) override;
    egl::Error makeCurrent(const gl::Context *context) override;
    egl::Error unMakeCurrent(const gl::Context *context) override;

    egl::Error swap(const gl::Context *context) override;
    egl::Error postSubBuffer(const gl::Context *context,
                             EGLint x,
                             EGLint y,
                             EGLint width,
                             EGLint height) override;
    egl::Error querySurfacePointerANGLE(EGLint attribute, void **value) override;
    egl::Error bindTexImage(const gl::Context *context,
                            gl::Texture *texture,
                            EGLint buffer) override;
    egl::Error releaseTexImage(const gl::Context *context, EGLint buffer) override;
    void setSwapInterval(EGLint interval) override;

    EGLint getWidth() const override;
    EGLint getHeight() const override;

    EGLint isPostSubBufferSupported() const override;
    EGLint getSwapBehavior() const override;

    static bool validateAttributes(EGLClientBuffer buffer, const egl::AttributeMap &attribs);
    FramebufferImpl *createDefaultFramebuffer(const gl::Context *context,
                                              const gl::FramebufferState &state) override;

  private:
    GLuint mTexture;
    int mWidth;
    int mHeight;
    GLenum mTextureTarget;
    GLenum mInternalFormat;
    GLenum mFormat;
    GLenum mType;

    rx::TextureGL *mBoundTexture;
};

}  // namespace rx

#endif  // LIBANGLE_RENDERER_GL_OPENGLTEXTURESURFACEGL_H_
