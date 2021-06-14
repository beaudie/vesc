//
// Copyright 2021 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//

#ifndef COMPILER_TRANSLATOR_DEFERREDGSARRAYINPUTSUZE_H_
#define COMPILER_TRANSLATOR_DEFERREDGSARRAYINPUTSUZE_H_

#include "common/angleutils.h"

namespace sh
{
class TSymbolTable;
class TIntermBlock;
class TCompiler;

ANGLE_NO_DISCARD bool DeferredSetGeometryShaderArrayInputSize(TCompiler *compiler,
                                                              TIntermBlock *root,
                                                              TSymbolTable *symbolTable);
}  // namespace sh

#endif  // COMPILER_TRANSLATOR_DEFERREDGSARRAYINPUTSUZE_H_
