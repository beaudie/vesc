//
// Copyright 2019 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//
// EmulateGLBaseVertex is an AST traverser to convert the gl_BaseVertex builtin
// to a uniform int
//

#ifndef COMPILER_TRANSLATOR_TREEOPS_EMULATEGLBASEVERTEX_H_
#define COMPILER_TRANSLATOR_TREEOPS_EMULATEGLBASEVERTEX_H_

#include <GLSLANG/ShaderLang.h>
#include <vector>

#include "compiler/translator/HashNames.h"

namespace sh
{
struct Uniform;
class TIntermBlock;
class TSymbolTable;

void EmulateGLBaseVertex(TIntermBlock *root,
                         TSymbolTable *symbolTable,
                         std::vector<sh::Uniform> *uniforms,
                         bool shouldCollect);

}  // namespace sh

#endif  // COMPILER_TRANSLATOR_TREEOPS_EMULATEGLBASEVERTEX_H_
