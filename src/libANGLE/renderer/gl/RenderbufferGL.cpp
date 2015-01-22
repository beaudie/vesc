//
// Copyright 2015 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//

// RenderbufferGL.cpp: Implements the RenderbufferGL class.

#include "libANGLE/renderer/gl/RenderbufferGL.h"

namespace rx
{

RenderbufferGL::RenderbufferGL()
{
}

RenderbufferGL::~RenderbufferGL()
{
}

gl::Error RenderbufferGL::setStorage(GLsizei width, GLsizei height, GLenum internalformat, GLsizei samples)
{
    return gl::Error(GL_NO_ERROR);
}

}
