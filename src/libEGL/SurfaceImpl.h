//
// Copyright (c) 2014 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//

// SurfaceImpl.h: Implementation methods of egl::Surface

#ifndef LIBEGL_SURFACEIMPL_H_
#define LIBEGL_SURFACEIMPL_H_

#include "common/angleutils.h"
#include "libEGL/Error.h"

namespace gl
{
class Texture2D;
}

namespace egl
{
class Display;
class Config;

class SurfaceImpl
{
  public:
    SurfaceImpl(Display *display, const Config *config, EGLint width, EGLint height,
                EGLint fixedSize, EGLint postSubBufferSupported, EGLenum textureFormat,
                EGLenum textureType, EGLClientBuffer shareHandle);
    virtual ~SurfaceImpl();

    virtual Error initialize() = 0;
    virtual Error swap() = 0;
    virtual Error postSubBuffer(EGLint x, EGLint y, EGLint width, EGLint height) = 0;
    virtual Error querySurfacePointerANGLE(EGLint attribute, void **value) = 0;
    virtual Error bindTexImage(EGLint buffer) = 0;
    virtual Error releaseTexImage(EGLint buffer) = 0;
    virtual void setSwapInterval(EGLint interval) = 0;

    // width and height can change with client window resizing
    EGLint getWidth() const { return mWidth; }
    EGLint getHeight() const { return mHeight; }

    //TODO(jmadill): Possibly should be redesigned
    virtual EGLNativeWindowType getWindowHandle() const = 0;

    const Config *getConfig() const { return mConfig; }
    void setBoundTexture(gl::Texture2D *texture) { mTexture = texture; }
    gl::Texture2D *getBoundTexture() const { return mTexture; }
    EGLint isFixedSize() const { return mFixedSize; }
    EGLenum getFormat() const;
    EGLint isPostSubBufferSupported() const { return mPostSubBufferSupported; }
    EGLenum getTextureFormat() const { return mTextureFormat; }
    EGLenum getTextureTarget() const { return mTextureTarget; }

  protected:
    Display *const mDisplay;
    const Config *mConfig;    // EGL config surface was created with

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

#endif // LIBEGL_SURFACEIMPL_H_

