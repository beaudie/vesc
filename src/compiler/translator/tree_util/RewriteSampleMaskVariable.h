//
// Copyright 2020 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//
// RewriteSampleMaskVariable.cpp: Find any references to gl_SampleMask and gl_SampleMaskIn, and
// rewrite its index with a constant.
//

#ifndef COMPILER_TRANSLATOR_TREEUTIL_REWRITESAMPLEMASKVARIABLE_H_
#define COMPILER_TRANSLATOR_TREEUTIL_REWRITESAMPLEMASKVARIABLE_H_

#include "common/angleutils.h"

namespace sh
{

class TCompiler;
class TIntermBlock;
class TSymbolTable;
class TIntermTyped;

// Rewrite indices to gl_SampleMask and gl_SampleMaskIn with a constant value.  This to solve the
// problem where a non-constant index is used for the unsized array.
ANGLE_NO_DISCARD bool RewriteSampleMask(TCompiler *compiler,
                                        TIntermBlock *root,
                                        TSymbolTable *symbolTable,
                                        const TIntermTyped *numSamplesUniform);

ANGLE_NO_DISCARD bool RewriteSampleMaskIn(TCompiler *compiler,
                                          TIntermBlock *root,
                                          TSymbolTable *symbolTable);

}  // namespace sh

#endif  // COMPILER_TRANSLATOR_TREEUTIL_REWRITESAMPLEMASKVARIABLE_H_
