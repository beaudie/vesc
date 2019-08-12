//
// Copyright 2019 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//
// RewriteRowMajorMatrices: Change row-major matrices to column-major in uniform and storage
// buffers.

#ifndef COMPILER_TRANSLATOR_TREEOPS_REWRITEROWMAJORMATRICES_H_
#define COMPILER_TRANSLATOR_TREEOPS_REWRITEROWMAJORMATRICES_H_

namespace sh
{
class TIntermBlock;
class TSymbolTable;

void RewriteRowMajorMatrices(TIntermBlock *root, TSymbolTable *symbolTable);
}  // namespace sh

#endif  // COMPILER_TRANSLATOR_TREEOPS_REWRITEROWMAJORMATRICES_H_
