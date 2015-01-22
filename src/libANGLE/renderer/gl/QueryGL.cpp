//
// Copyright 2015 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//

// QueryGL.cpp: Implements the QueryGL class.

#include "libANGLE/renderer/gl/QueryGL.h"

namespace rx
{

QueryGL::QueryGL(GLenum type)
    : QueryImpl(type)
{
}

QueryGL::~QueryGL()
{
}

gl::Error QueryGL::begin()
{
    return gl::Error(GL_NO_ERROR);
}

gl::Error QueryGL::end()
{
    return gl::Error(GL_NO_ERROR);
}

gl::Error QueryGL::getResult(GLuint *params)
{
    return gl::Error(GL_NO_ERROR);
}

gl::Error QueryGL::isResultAvailable(GLuint *available)
{
    return gl::Error(GL_NO_ERROR);
}

}
