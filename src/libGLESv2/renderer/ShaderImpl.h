//
// Copyright 2014 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//

// ShaderImpl.h: Defines the abstract rx::ShaderImpl class.

#ifndef LIBGLESV2_RENDERER_SHADERIMPL_H_
#define LIBGLESV2_RENDERER_SHADERIMPL_H_

#include "common/angleutils.h"

namespace rx
{

class ShaderImpl
{
    friend class gl::Shader;

  public:
    virtual ~ShaderImpl() { }

    virtual bool compile(const std::string &source) = 0;
    virtual const std::string &getInfoLog() const = 0;
    virtual const std::string &getTranslatedSource() const = 0;

  protected:
    ShaderImpl() : mShader(NULL) {}

    gl::Shader *mShader;
};

}

#endif // LIBGLESV2_RENDERER_SHADERIMPL_H_
