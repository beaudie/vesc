//
// Copyright (c) 2002-2017 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//
// The SetGeometryShaderInputArraySize function sets the array size of all unsized geometry shader
// inputs declared before input primitive declaration for further processing.

#ifndef COMPILER_TRANSLATOR_SETGEOMETRYSHADERINPUTARRAYSIZE_H_
#define COMPILER_TRANSLATOR_SETGEOMETRYSHADERINPUTARRAYSIZE_H_

namespace sh
{

class TIntermBlock;

void SetGeometryShaderInputArraySize(TIntermBlock *root, unsigned int arraySize);

}  // namespace sh

#endif