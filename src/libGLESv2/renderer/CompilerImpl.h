//
// Copyright (c) 2014 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//

// CompilerImpl.h: Defines the rx::CompilerImpl class, an implementation interface
// for the gl::Compiler object.

#include "libGLESv2/Error.h"

#ifndef LIBGLESV2_RENDERER_COMPILERIMPL_H_
#define LIBGLESV2_RENDERER_COMPILERIMPL_H_

namespace rx
{

class CompilerImpl
{
  public:
    CompilerImpl() {}
    virtual ~CompilerImpl() {}

    virtual gl::Error release() = 0;
};

}

#endif // LIBGLESV2_RENDERER_COMPILERIMPL_H_
