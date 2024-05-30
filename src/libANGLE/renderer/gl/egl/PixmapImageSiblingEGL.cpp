//
// Copyright The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//

// PixmapImageSiblingEGL.cpp: Defines the PixmapImageSiblingEGL to wrap EGL images
// created from pixmaps

#include "libANGLE/renderer/gl/egl/PixmapImageSiblingEGL.h"

namespace rx
{
PixmapImageSiblingEGL::PixmapImageSiblingEGL(EGLClientBuffer buffer,
                                             const egl::AttributeMap &attribs)
    : mBuffer(buffer), mAttribs(attribs), mFormat(GL_NONE)
{
    ASSERT(mAttribs.contains(EGL_WIDTH));
    mSize.width = mAttribs.getAsInt(EGL_WIDTH);
    ASSERT(mAttribs.contains(EGL_HEIGHT));
    mSize.height = mAttribs.getAsInt(EGL_HEIGHT);

    mFormat = gl::Format(GL_RGBA8);
}

PixmapImageSiblingEGL::~PixmapImageSiblingEGL() {}

egl::Error PixmapImageSiblingEGL::initialize(const egl::Display *display)
{
    return egl::NoError();
}

gl::Format PixmapImageSiblingEGL::getFormat() const
{
    return mFormat;
}

bool PixmapImageSiblingEGL::isRenderable(const gl::Context *context) const
{
    return true;
}

bool PixmapImageSiblingEGL::isTexturable(const gl::Context *context) const
{
    return true;
}

bool PixmapImageSiblingEGL::isYUV() const
{
    return false;
}

bool PixmapImageSiblingEGL::hasProtectedContent() const
{
    return false;
}

gl::Extents PixmapImageSiblingEGL::getSize() const
{
    return mSize;
}

size_t PixmapImageSiblingEGL::getSamples() const
{
    return 0;
}

EGLClientBuffer PixmapImageSiblingEGL::getBuffer() const
{
    return mBuffer;
}

void PixmapImageSiblingEGL::getImageCreationAttributes(std::vector<EGLint> *outAttributes) const {}

}  // namespace rx
