//
// Copyright (c) 2002-2015 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//
// EmulateGLDrawID is an AST traverser to convert the gl_DrawID builtin
// to uniform int angle_DrawID
//

#ifndef COMPILER_TRANSLATOR_TREEOPS_EMULATEGLDRAWID_H_
#define COMPILER_TRANSLATOR_TREEOPS_EMULATEGLDRAWID_H_

#include <vector>

namespace sh
{
class TIntermBlock;
class TSymbolTable;

void EmulateGLDrawID(TIntermBlock *root, TSymbolTable *symbolTable);
}  // namespace sh

#endif  // COMPILER_TRANSLATOR_TREEOPS_EMULATEGLDRAWID_H_
