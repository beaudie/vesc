//
// Copyright (c) 2015 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//

// SurfaceGL.cpp: OpenGL implementation of egl::Surface

#include "libANGLE/renderer/gl/SurfaceGL.h"

#include "libANGLE/renderer/gl/DefaultAttachmentGL.h"
#include "libANGLE/renderer/gl/FramebufferGL.h"

namespace rx
{

SurfaceGL::SurfaceGL(egl::Display *display, const egl::Config *config,
                     EGLint fixedSize, EGLint postSubBufferSupported, EGLenum textureFormat,
                     EGLenum textureType)
    : SurfaceImpl(display, config, fixedSize, postSubBufferSupported, textureFormat, textureType)
{
}

SurfaceGL::~SurfaceGL()
{
}

FramebufferImpl *SurfaceGL::createDefaultFramebuffer()
{
    return new FramebufferGL();
}

DefaultAttachmentImpl *SurfaceGL::createDefaultAttachment(GLenum type)
{
    return new DefaultAttachmentGL();
}

}
