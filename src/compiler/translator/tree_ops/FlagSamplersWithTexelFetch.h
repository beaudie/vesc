//
// Copyright 2020 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//
// FlagSamplersForTexelFetch.h: finds all instances of texelFetch used with a static reference to a
// sampler uniform, and flag that uniform as having been used with texelFetch
//

#ifndef COMPILER_TRANSLATOR_TREEOPS_FLAGSAMPLERSWITHTEXELFETCH_H_
#define COMPILER_TRANSLATOR_TREEOPS_FLAGSAMPLERSWITHTEXELFETCH_H_

#include "GLSLANG/ShaderVars.h"
#include "common/angleutils.h"

namespace sh
{
class TCompiler;
class TIntermBlock;
class TSymbolTable;

ANGLE_NO_DISCARD bool FlagSamplersForTexelFetch(TCompiler *compiler,
                                                TIntermBlock *root,
                                                TSymbolTable *symbolTable,
                                                std::vector<sh::ShaderVariable> *uniforms);
}  // namespace sh

#endif  // COMPILER_TRANSLATOR_TREEOPS_FLAGSAMPLERSWITHTEXELFETCH_H_
