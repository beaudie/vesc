// Copyright (c) 2017 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//
// VectorizeVectorScalarArithmetic.h: Turn some arithmetic operations that operate on a float
// vector-scalar pair into vector-vector operations. This is done recursively. Some scalar binary
// operations inside vector constructors are also turned into vector operations.
//
// This is targeted to work around a bug in NVIDIA OpenGL drivers that was reproducible on NVIDIA
// driver version 387.92.

#ifndef COMPILER_TRANSLATOR_VECTORIZEVECTORSCALARARITHMETIC_H_
#define COMPILER_TRANSLATOR_VECTORIZEVECTORSCALARARITHMETIC_H_

namespace sh
{
class TIntermBlock;

void VectorizeVectorScalarArithmetic(TIntermBlock *root);

}  // namespace sh

#endif  // COMPILER_TRANSLATOR_VECTORIZEVECTORSCALARARITHMETIC_H_