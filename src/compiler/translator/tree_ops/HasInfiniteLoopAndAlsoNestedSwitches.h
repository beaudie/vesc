//
// Copyright 2024 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//
// HasInfiniteLoopAndAlsoNestedSwitches.h: Attempts to check for a driver bug scenario involving
// infinite loops and nested switches.

#ifndef COMPILER_TRANSLATOR_TREEOPS_HASINFINITELOOPANDALSONESTEDSWITCHES_H_
#define COMPILER_TRANSLATOR_TREEOPS_HASINFINITELOOPANDALSONESTEDSWITCHES_H_

#include "common/angleutils.h"

namespace sh
{
class TCompiler;
class TIntermBlock;
class TSymbolTable;

[[nodiscard]] bool HasInfiniteLoopAndAlsoNestedSwitches(TCompiler *compiler,
                                                        TIntermBlock *root,
                                                        TSymbolTable *symbolTable);
}  // namespace sh

#endif  // COMPILER_TRANSLATOR_TREEOPS_HASINFINITELOOPANDALSONESTEDSWITCHES_H_
