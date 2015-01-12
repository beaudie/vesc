//
// Copyright (c) 2015 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//

// SurfaceGL.cpp: OpenGL implementation of egl::Surface

#include "libANGLE/renderer/gl/SurfaceGL.h"

namespace rx
{

SurfaceGL::SurfaceGL(egl::Display *display, const egl::Config *config, EGLint width, EGLint height,
                     EGLint fixedSize, EGLint postSubBufferSupported, EGLenum textureFormat,
                     EGLenum textureType, EGLClientBuffer shareHandle)
    : SurfaceImpl(display, config, width, height, fixedSize, postSubBufferSupported, textureFormat,
                  textureType, shareHandle)
{
}

SurfaceGL::~SurfaceGL()
{
}

}
