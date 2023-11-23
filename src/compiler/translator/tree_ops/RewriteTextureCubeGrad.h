//
// Copyright 2023 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//
// Multiple GPU vendors have issues with transforming explicit cubemap
// derivatives onto the appropriate face. The workarounds are vendor-specific.

#ifndef COMPILER_TRANSLATOR_TREEOPS_REWRITE_TEXTURECUBEGRAD_H_
#define COMPILER_TRANSLATOR_TREEOPS_REWRITE_TEXTURECUBEGRAD_H_

#include "compiler/translator/tree_util/IntermTraverse.h"

namespace sh
{

// Apple GPU specific
[[nodiscard]] bool RewriteTextureCubeGradAGX(TCompiler *compiler,
                                             TIntermBlock *root,
                                             TSymbolTable *symbolTable,
                                             int shaderVersion);

}  // namespace sh

#endif  // COMPILER_TRANSLATOR_TREEOPS_REWRITE_TEXTURECUBEGRAD_H_
