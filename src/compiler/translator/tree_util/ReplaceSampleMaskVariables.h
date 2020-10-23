//
// Copyright 2020 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//
// ReplaceSampleMaskVariable.cpp: Find any references to gl_SampleMask and gl_SampleMaskIn, and
// replace it with ANGLESampleMask or ANGLESampleMaskIn.
//

#ifndef COMPILER_TRANSLATOR_TREEUTIL_REPLACESAMPLEMASKVARIABLE_H_
#define COMPILER_TRANSLATOR_TREEUTIL_REPLACESAMPLEMASKVARIABLE_H_

#include "common/angleutils.h"

namespace sh
{

class TCompiler;
class TIntermBlock;
class TSymbolTable;
class TIntermTyped;

// Replace every gl_SampleMask and gl_SampleMaskIn assignment with assignment to "ANGLESampleMask"
// and "ANGLESampleMaskIn", then at the end of shader re-assign the values of this global variable
// to gl_SampleMask and gl_SampleMaskIn. This to solve the problem which the non constant index is
// used for the unsized array problem.
ANGLE_NO_DISCARD bool ReplaceSampleMaskToANGLESampleMask(TCompiler *compiler,
                                                         TIntermBlock *root,
                                                         TSymbolTable *symbolTable,
                                                         const TIntermTyped *numSamplesUniform);

ANGLE_NO_DISCARD bool ReplaceSampleMaskInToANGLESampleMaskIn(TCompiler *compiler,
                                                             TIntermBlock *root,
                                                             TSymbolTable *symbolTable);

}  // namespace sh

#endif  // COMPILER_TRANSLATOR_TREEUTIL_REPLACESAMPLEMASKVARIABLE_H_
