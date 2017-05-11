//
// Copyright (c) 2002-2013 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//

#ifndef COMPILER_TRANSLATOR_INITIALIZEVARIABLES_H_
#define COMPILER_TRANSLATOR_INITIALIZEVARIABLES_H_

#include <GLSLANG/ShaderLang.h>

// TODO: Maybe reorg code so that CreateInitCode can live somewhere else than here. Nicer if
// IntermNode.h wouldn't need to be included in this header.
#include "compiler/translator/IntermNode.h"

namespace sh
{
class TSymbolTable;

typedef std::vector<sh::ShaderVariable> InitVariableList;

TIntermSequence *CreateInitCode(TIntermSymbol *initializedSymbol);

// Initialize all uninitialized local variables, so that undefined behavior is avoided.
void InitializeUninitializedLocals(TIntermBlock *root, int shaderVersion);

// Currently this function is only capable of initializing variables of basic types,
// array of basic types, or struct of basic types.
// For now it is used for the following two scenarios:
//   1. initializing gl_Position;
//   2. initializing ESSL 3.00 shaders' output variables (which might be structs, but not arrays of
//   structs).
void InitializeVariables(TIntermBlock *root,
                         const InitVariableList &vars,
                         const TSymbolTable &symbolTable);

}  // namespace sh

#endif  // COMPILER_TRANSLATOR_INITIALIZEVARIABLES_H_
