//
// Copyright (c) 2015 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//

// PBufferSurfaceCGL.cpp: an implementation of PBuffers created from IOSurfaces using
//                        EGL_ANGLE_iosurface_client_buffer

#include "libANGLE/renderer/gl/cgl/IOSurfaceSurfaceCGL.h"

#include <IOSurface/IOSurface.h>
#include <OpenGL/CGLIOSurface.h>

#include "common/debug.h"
#include "libANGLE/renderer/gl/FramebufferGL.h"
#include "libANGLE/renderer/gl/FunctionsGL.h"
#include "libANGLE/renderer/gl/RendererGL.h"
#include "libANGLE/renderer/gl/StateManagerGL.h"
#include "libANGLE/renderer/gl/TextureGL.h"
#include "libANGLE/renderer/gl/cgl/DisplayCGL.h"

#include <iostream>

namespace rx
{

IOSurfaceSurfaceCGL::IOSurfaceSurfaceCGL(const egl::SurfaceState &state,
                                         RendererGL *renderer,
                                         DisplayCGL *display,
                                         EGLClientBuffer buffer)
    : SurfaceGL(state, renderer),
      mDisplay(display),
      mRenderer(renderer),
      mStateManager(renderer->getStateManager()),
      mIOSurface(nullptr),
      mWidth(0),
      mHeight(0)
{
    mIOSurface = reinterpret_cast<IOSurfaceRef>(buffer);
    CFRetain(mIOSurface);

    mWidth  = IOSurfaceGetWidth(mIOSurface);
    mHeight = IOSurfaceGetHeight(mIOSurface);
    std::cout << mWidth << " " << mHeight << std::endl;
}

IOSurfaceSurfaceCGL::~IOSurfaceSurfaceCGL()
{
    if (mIOSurface != nullptr)
    {
        CFRelease(mIOSurface);
        mIOSurface = nullptr;
    }
}

egl::Error IOSurfaceSurfaceCGL::initialize(const egl::Display *display)
{
    return egl::NoError();
}

egl::Error IOSurfaceSurfaceCGL::makeCurrent()
{
    return egl::NoError();
}

egl::Error IOSurfaceSurfaceCGL::swap(const gl::Context *context)
{
    return egl::NoError();
}

egl::Error IOSurfaceSurfaceCGL::postSubBuffer(const gl::Context *context,
                                              EGLint x,
                                              EGLint y,
                                              EGLint width,
                                              EGLint height)
{
    return egl::NoError();
}

egl::Error IOSurfaceSurfaceCGL::querySurfacePointerANGLE(EGLint attribute, void **value)
{
    UNIMPLEMENTED();
    return egl::NoError();
}

egl::Error IOSurfaceSurfaceCGL::bindTexImage(gl::Texture *texture, EGLint buffer)
{
    const TextureGL *textureGL = GetImplAs<TextureGL>(texture);
    GLuint textureID           = textureGL->getTextureID();
    mStateManager->bindTexture(GL_TEXTURE_RECTANGLE, textureID);

    auto error =
        CGLTexImageIOSurface2D(mDisplay->getCGLContext(), GL_TEXTURE_RECTANGLE, GL_RGBA, mWidth,
                               mHeight, GL_BGRA, GL_UNSIGNED_INT_8_8_8_8_REV, mIOSurface, 0);
    ASSERT(error == kCGLNoError);
    std::cout << "Bound tex image" << std::endl;

    return egl::NoError();
}

egl::Error IOSurfaceSurfaceCGL::releaseTexImage(EGLint buffer)
{
    mRenderer->flush();
    return egl::NoError();
}

void IOSurfaceSurfaceCGL::setSwapInterval(EGLint interval)
{
}

EGLint IOSurfaceSurfaceCGL::getWidth() const
{
    return mWidth;
}

EGLint IOSurfaceSurfaceCGL::getHeight() const
{
    return mHeight;
}

EGLint IOSurfaceSurfaceCGL::isPostSubBufferSupported() const
{
    UNIMPLEMENTED();
    return EGL_FALSE;
}

EGLint IOSurfaceSurfaceCGL::getSwapBehavior() const
{
    return EGL_BUFFER_PRESERVED;
}

}  // namespace rx
