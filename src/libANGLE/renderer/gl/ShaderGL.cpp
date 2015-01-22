//
// Copyright 2015 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//

// ShaderGL.cpp: Implements the ShaderGL class.

#include "libANGLE/renderer/gl/ShaderGL.h"

namespace rx
{

ShaderGL::ShaderGL()
{
}

ShaderGL::~ShaderGL()
{
}

bool ShaderGL::compile(gl::Compiler *compiler, const std::string &source)
{
    return true;
}

const std::string &ShaderGL::getInfoLog() const
{
    return mInfoLog;
}

const std::string &ShaderGL::getTranslatedSource() const
{
    return mTranslatedSource;
}

std::string ShaderGL::getDebugInfo() const
{
    return "";
}

}
