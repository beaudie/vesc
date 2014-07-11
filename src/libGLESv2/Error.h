//
// Copyright (c) 2014 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//

#ifndef LIBGLESV2_ERROR_H_
#define LIBGLESV2_ERROR_H_

#include "angle_gl.h"

#include <string>

namespace gl
{

class Error
{
  public:
    explicit Error(GLenum errorCode);
    Error(GLenum errorCode, const std::string &msg, ...);

    GLenum getCode() const { return mCode; }
    bool isError() const { return (mCode != GL_NO_ERROR); }

    const std::string &getMessage() const { return mMessage; }

  private:
    GLenum mCode;
    std::string mMessage;
};

}

#endif // LIBGLESV2_ERROR_H_
