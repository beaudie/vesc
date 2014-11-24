//
// Copyright (c) 2014 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//

// SurfaceImpl.h: Implementation methods of egl::Surface

#ifndef LIBANGLE_RENDERER_SURFACEIMPL_H_
#define LIBANGLE_RENDERER_SURFACEIMPL_H_

#include "common/angleutils.h"
#include "libANGLE/Error.h"

namespace egl
{
class Display;
class Config;
}

namespace gl
{
class Texture2D;
}

namespace rx
{

class SurfaceImpl
{
  public:
    SurfaceImpl(egl::Display *display, const egl::Config *config, EGLint width, EGLint height,
                EGLint fixedSize, EGLint postSubBufferSupported, EGLenum textureFormat,
                EGLenum textureType, EGLClientBuffer shareHandle);
    virtual ~SurfaceImpl();

    virtual egl::Error initialize() = 0;
    virtual egl::Error swap() = 0;
    virtual egl::Error postSubBuffer(EGLint x, EGLint y, EGLint width, EGLint height) = 0;
    virtual egl::Error querySurfacePointerANGLE(EGLint attribute, void **value) = 0;
    virtual egl::Error bindTexImage(EGLint buffer) = 0;
    virtual egl::Error releaseTexImage(EGLint buffer) = 0;
    virtual void setSwapInterval(EGLint interval) = 0;

    // width and height can change with client window resizing
    EGLint getWidth() const { return mWidth; }
    EGLint getHeight() const { return mHeight; }

    //TODO(jmadill): Possibly should be redesigned
    virtual EGLNativeWindowType getWindowHandle() const = 0;

    const egl::Config *getConfig() const { return mConfig; }
    void setBoundTexture(gl::Texture2D *texture) { mTexture = texture; }
    gl::Texture2D *getBoundTexture() const { return mTexture; }
    EGLint isFixedSize() const { return mFixedSize; }
    EGLenum getFormat() const;
    EGLint isPostSubBufferSupported() const { return mPostSubBufferSupported; }
    EGLenum getTextureFormat() const { return mTextureFormat; }
    EGLenum getTextureTarget() const { return mTextureTarget; }

  protected:
    egl::Display *const mDisplay;
    const egl::Config *mConfig;    // EGL config surface was created with

    EGLint mWidth;
    EGLint mHeight;
    EGLint mFixedSize;
    EGLint mSwapInterval;
    EGLint mPostSubBufferSupported;
    EGLenum mTextureFormat;        // Format of texture: RGB, RGBA, or no texture
    EGLenum mTextureTarget;        // Type of texture: 2D or no texture
    EGLClientBuffer mShareHandle;

    gl::Texture2D *mTexture;

  private:
    DISALLOW_COPY_AND_ASSIGN(SurfaceImpl);
};

}

#endif // LIBANGLE_RENDERER_SURFACEIMPL_H_

