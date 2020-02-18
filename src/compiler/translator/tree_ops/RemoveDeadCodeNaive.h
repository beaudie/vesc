//
// Copyright 2020 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//
// RemoveDeadCodeNaive.h: Naive DCE implementation that recursively
//   removes dead variables from the bottom up.
//

#ifndef COMPILER_TRANSLATOR_TREEOPS_REMOVEDEADCODENAIVE_H_
#define COMPILER_TRANSLATOR_TREEOPS_REMOVEDEADCODENAIVE_H_

#include "common/angleutils.h"

namespace sh
{

class TCompiler;
class TIntermBlock;
class TSymbolTable;

ANGLE_NO_DISCARD bool RemoveDeadCodeNaive(TCompiler *compiler,
                                          TIntermBlock *root,
                                          TSymbolTable *symbolTable);

}  // namespace sh

#endif  // COMPILER_TRANSLATOR_TREEOPS_REMOVEDEADCODENAIVE_H_
