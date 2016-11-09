//
// Copyright (c) 2002-2013 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//

#ifndef COMPILER_TRANSLATOR_INITIALIZEVARIABLES_H_
#define COMPILER_TRANSLATOR_INITIALIZEVARIABLES_H_

#include <GLSLANG/ShaderLang.h>

namespace sh
{
class TIntermNode;
class TSymbolTable;

typedef std::vector<sh::ShaderVariable> InitVariableList;

// Currently this function is only capable of initializing variables of basic types,
// array of basic types, or struct of basic types.
void InitializeVariables(TIntermNode *root,
                         const InitVariableList &vars,
                         const TSymbolTable &symbolTable);
}  // namespace sh

#endif  // COMPILER_TRANSLATOR_INITIALIZEVARIABLES_H_
