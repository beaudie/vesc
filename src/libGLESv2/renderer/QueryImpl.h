//
// Copyright (c) 2013 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//

// QueryImpl.h: Defines the abstract rx::QueryImpl class.

#ifndef LIBGLESV2_RENDERER_QUERYIMPL_H_
#define LIBGLESV2_RENDERER_QUERYIMPL_H_

#include "common/angleutils.h"

namespace rx
{

class QueryImpl
{
  public:
    virtual ~QueryImpl() { }

    virtual bool begin(GLenum type) = 0;
    virtual void end() = 0;
    virtual GLuint getResult(GLenum type) = 0;
    virtual GLboolean isResultAvailable(GLenum type) = 0;
};

}

#endif // LIBGLESV2_RENDERER_QUERYIMPL_H_
