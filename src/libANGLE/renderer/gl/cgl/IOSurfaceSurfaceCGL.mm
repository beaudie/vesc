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
#include "libANGLE/AttributeMap.h"
#include "libANGLE/renderer/gl/FramebufferGL.h"
#include "libANGLE/renderer/gl/FunctionsGL.h"
#include "libANGLE/renderer/gl/RendererGL.h"
#include "libANGLE/renderer/gl/StateManagerGL.h"
#include "libANGLE/renderer/gl/TextureGL.h"
#include "libANGLE/renderer/gl/cgl/DisplayCGL.h"

namespace rx
{

namespace
{

struct IOSurfaceFormatInfo
{
    GLenum internalFormat;
    GLenum format;
    GLenum type;

    size_t componentBytes;
};

static const IOSurfaceFormatInfo kIOSurfaceFormats[] = {
    {GL_RED, GL_RED, GL_UNSIGNED_BYTE, 1}, {GL_R16UI, GL_RED, GL_UNSIGNED_SHORT, 2},
    {GL_RG, GL_RG, GL_UNSIGNED_BYTE, 2},   {GL_RGBA, GL_BGRA, GL_UNSIGNED_INT_8_8_8_8_REV, 4},
    {GL_RGBA, GL_RGBA, GL_HALF_FLOAT, 8},
};

}  // anonymous namespace

IOSurfaceSurfaceCGL::IOSurfaceSurfaceCGL(const egl::SurfaceState &state,
                                         RendererGL *renderer,
                                         DisplayCGL *display,
                                         EGLClientBuffer buffer,
                                         const egl::AttributeMap &attribs)
    : SurfaceGL(state, renderer),
      mDisplay(display),
      mRenderer(renderer),
      mStateManager(renderer->getStateManager()),
      mIOSurface(nullptr),
      mWidth(0),
      mHeight(0),
      mPlane(0),
      mFormatIndex(-1)
{
    // Keep reference to the IOSurface so it doesn't get deleted while the pbuffer exists.
    mIOSurface = reinterpret_cast<IOSurfaceRef>(buffer);
    CFRetain(mIOSurface);

    // Extract attribs useful for the call to CGLTexImageIOSurface2D
    mWidth  = attribs.get(EGL_WIDTH);
    mHeight = attribs.get(EGL_HEIGHT);
    mPlane  = attribs.get(EGL_IOSURFACE_PLANE_ANGLE);

    EGLAttrib internalFormat = attribs.get(EGL_TEXTURE_INTERNAL_FORMAT_ANGLE);
    EGLAttrib type           = attribs.get(EGL_TEXTURE_TYPE_ANGLE);
    EGLAttrib format         = attribs.get(EGL_TEXTURE_FORMAT);
    for (int i = 0; i < static_cast<int>(ArraySize(kIOSurfaceFormats)); ++i)
    {
        const auto &formatInfo = kIOSurfaceFormats[i];
        if (formatInfo.internalFormat == internalFormat && formatInfo.format == format &&
            formatInfo.type == type)
        {
            mFormatIndex = i;
            break;
        }
    }

    ASSERT(mFormatIndex >= 0);
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
    // Make current is not supported on IOSurface pbuffers.
    return egl::EglBadSurface();
}

egl::Error IOSurfaceSurfaceCGL::swap(const gl::Context *context)
{
    UNREACHABLE();
    return egl::NoError();
}

egl::Error IOSurfaceSurfaceCGL::postSubBuffer(const gl::Context *context,
                                              EGLint x,
                                              EGLint y,
                                              EGLint width,
                                              EGLint height)
{
    UNREACHABLE();
    return egl::NoError();
}

egl::Error IOSurfaceSurfaceCGL::querySurfacePointerANGLE(EGLint attribute, void **value)
{
    UNREACHABLE();
    return egl::NoError();
}

egl::Error IOSurfaceSurfaceCGL::bindTexImage(gl::Texture *texture, EGLint buffer)
{
    const TextureGL *textureGL = GetImplAs<TextureGL>(texture);
    GLuint textureID           = textureGL->getTextureID();
    mStateManager->bindTexture(GL_TEXTURE_RECTANGLE, textureID);

    const auto &format = kIOSurfaceFormats[mFormatIndex];
    auto error         = CGLTexImageIOSurface2D(mDisplay->getCGLContext(), GL_TEXTURE_RECTANGLE,
                                        format.format, mWidth, mHeight, format.type,
                                        format.internalFormat, mIOSurface, mPlane);
    ASSERT(error == kCGLNoError);  // XXX what to do on error?

    return egl::NoError();
}

egl::Error IOSurfaceSurfaceCGL::releaseTexImage(EGLint buffer)
{
    gl::Error error = mRenderer->flush();
    if (error.isError())
    {
        return egl::EglContextLost();
    }
    return egl::NoError();
}

void IOSurfaceSurfaceCGL::setSwapInterval(EGLint interval)
{
    UNREACHABLE();
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
    UNREACHABLE();
    return EGL_FALSE;
}

EGLint IOSurfaceSurfaceCGL::getSwapBehavior() const
{
    // N/A because you can't MakeCurrent an IOSurface, return any valid value.
    return EGL_BUFFER_PRESERVED;
}

// static
bool IOSurfaceSurfaceCGL::validateAttributes(EGLClientBuffer buffer,
                                             const egl::AttributeMap &attribs)
{
    IOSurfaceRef ioSurface = reinterpret_cast<IOSurfaceRef>(buffer);

    // The plane must exist for this IOSurface
    EGLAttrib plane = attribs.get(EGL_IOSURFACE_PLANE_ANGLE);
    if (plane < 0 || static_cast<size_t>(plane) >= IOSurfaceGetPlaneCount(ioSurface))
    {
        return false;
    }

    // The width height specified must be at least (1, 1) and at most the plane size
    EGLAttrib width  = attribs.get(EGL_WIDTH);
    EGLAttrib height = attribs.get(EGL_HEIGHT);
    if (width <= 0 || static_cast<size_t>(width) > IOSurfaceGetWidthOfPlane(ioSurface, plane) ||
        height < 0 || static_cast<size_t>(height) > IOSurfaceGetHeightOfPlane(ioSurface, plane))
    {
        return false;
    }

    // Find this IOSurface format
    EGLAttrib internalFormat = attribs.get(EGL_TEXTURE_INTERNAL_FORMAT_ANGLE);
    EGLAttrib type           = attribs.get(EGL_TEXTURE_TYPE_ANGLE);
    EGLAttrib format         = attribs.get(EGL_TEXTURE_FORMAT);

    int formatIndex = -1;
    for (int i = 0; i < static_cast<int>(ArraySize(kIOSurfaceFormats)); ++i)
    {
        const auto &formatInfo = kIOSurfaceFormats[i];
        if (formatInfo.internalFormat == internalFormat && formatInfo.format == format &&
            formatInfo.type == type)
        {
            formatIndex = i;
            break;
        }
    }

    if (formatIndex < 0)
    {
        return false;
    }

    // Check that the format matches this IOSurface plane
    if (IOSurfaceGetBytesPerElementOfPlane(ioSurface, plane) !=
        kIOSurfaceFormats[formatIndex].componentBytes)
    {
        return false;
    }

    return true;
}

}  // namespace rx
