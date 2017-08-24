//
// Copyright (c) 2017 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//
// TranslatorGLSLBase.h: Definition of a base class for all GLSL translators

#ifndef COMPILER_TRANSLATOR_TRANSLATORGLSLBASE_H_
#define COMPILER_TRANSLATOR_TRANSLATORGLSLBASE_H_

#include "compiler/translator/Compiler.h"
#include "compiler/translator/OutputGLSLBase.h"

namespace sh
{

class TranslatorGLSLBase : public TCompiler
{
  public:
    TranslatorGLSLBase(sh::GLenum type, ShShaderSpec spec, ShShaderOutput output);

  protected:
    const ReservedNameSet &getReservedNames() const;

  private:
    ReservedNameSet mReservedNames;
};

}  // namespace sh

#endif  // COMPILER_TRANSLATOR_TRANSLATORGLSLBASE_H_
