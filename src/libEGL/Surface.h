//
// Copyright (c) 2002-2014 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//

// Surface.h: Defines the egl::Surface class, representing a drawing surface
// such as the client area of a window, including any back buffers.
// Implements EGLSurface and related functionality. [EGL 1.4] section 2.2 page 3.

#ifndef LIBEGL_SURFACE_H_
#define LIBEGL_SURFACE_H_

#include "common/angleutils.h"
#include "libEGL/Error.h"

#include <EGL/egl.h>

namespace gl
{
class Texture2D;
}

namespace egl
{
class Display;
class Config;
class Surface;
class SurfaceImpl;

class Surface
{
  public:
    Surface(SurfaceImpl *impl);
    virtual ~Surface();

    SurfaceImpl *getImplementation() const { return mImplementation; }

    Error initialize();
    Error swap();
    Error postSubBuffer(EGLint x, EGLint y, EGLint width, EGLint height);
    Error querySurfacePointerANGLE(EGLint attribute, void **value);
    Error bindTexImage(EGLint buffer);
    Error releaseTexImage(EGLint buffer);

    EGLNativeWindowType getWindowHandle() const;

    virtual EGLint isPostSubBufferSupported() const;

    void setSwapInterval(EGLint interval);

    virtual EGLint getConfigID() const;

    // width and height can change with client window resizing
    virtual EGLint getWidth() const;
    virtual EGLint getHeight() const;
    virtual EGLint getPixelAspectRatio() const;
    virtual EGLenum getRenderBuffer() const;
    virtual EGLenum getSwapBehavior() const;
    virtual EGLenum getTextureFormat() const;
    virtual EGLenum getTextureTarget() const;
    virtual EGLenum getFormat() const;

    virtual void setBoundTexture(gl::Texture2D *texture);
    virtual gl::Texture2D *getBoundTexture() const;

    EGLint isFixedSize() const;

  private:
    DISALLOW_COPY_AND_ASSIGN(Surface);

    SurfaceImpl *mImplementation;

//  EGLint horizontalResolution;   // Horizontal dot pitch
//  EGLint verticalResolution;     // Vertical dot pitch
//  EGLBoolean largestPBuffer;     // If true, create largest pbuffer possible
//  EGLBoolean mipmapTexture;      // True if texture has mipmaps
//  EGLint mipmapLevel;            // Mipmap level to render to
//  EGLenum multisampleResolve;    // Multisample resolve behavior
    EGLint mPixelAspectRatio;      // Display aspect ratio
    EGLenum mRenderBuffer;         // Render buffer
    EGLenum mSwapBehavior;         // Buffer swap behavior
//  EGLenum vgAlphaFormat;         // Alpha format for OpenVG
//  EGLenum vgColorSpace;          // Color space for OpenVG
};

}

#endif   // LIBEGL_SURFACE_H_
