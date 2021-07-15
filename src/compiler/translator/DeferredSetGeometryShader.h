//
// Copyright 2021 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//

// DeferredSetGeometryShader.h: Account for the case where array size is set after the
// declaration of all input variables
#ifndef COMPILER_TRANSLATOR_DEFERREDGSARRAYINPUTSUZE_H_
#define COMPILER_TRANSLATOR_DEFERREDGSARRAYINPUTSUZE_H_

#include "common/angleutils.h"

namespace sh
{
class TSymbolTable;
class TIntermBlock;
class TCompiler;

ANGLE_NO_DISCARD bool DeferredSetGeometryShaderArraySize(TCompiler *compiler,
                                                         TIntermBlock *root,
                                                         TSymbolTable *symbolTable);
}  // namespace sh

#endif  // COMPILER_TRANSLATOR_DEFERREDGSARRAYINPUTSUZE_H_
