//
// Copyright 2015 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//

// CompilerGL.cpp: Implements the CompilerGL class.

#include "libANGLE/renderer/gl/CompilerGL.h"

namespace rx
{

CompilerGL::CompilerGL()
{
}

CompilerGL::~CompilerGL()
{
}

gl::Error CompilerGL::release()
{
    return gl::Error(GL_NO_ERROR);
}

}
