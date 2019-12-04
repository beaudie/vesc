//
// Copyright 2019 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//
// RemoveStaticallyUnusedVariables.h:
//  Drop shader interface variable declarations for those that are statically unused.  This step
//  needs to be done after CollectVariables.  This avoids having to emulate them (e.g. atomic
//  counters for Vulkan) or remove them in glslang wrapper (again, for Vulkan).
//

#ifndef COMPILER_TRANSLATOR_TREEOPS_REMOVESTATICALLYUNUSEDVARIABLES_H_
#define COMPILER_TRANSLATOR_TREEOPS_REMOVESTATICALLYUNUSEDVARIABLES_H_

#include "common/angleutils.h"

namespace sh
{

class TCompiler;
class TIntermBlock;
class TSymbolTable;

ANGLE_NO_DISCARD bool RemoveStaticallyUnusedVariables(TCompiler *compiler,
                                                  TIntermBlock *root,
                                                  TSymbolTable *symbolTable);

}  // namespace sh

#endif  // COMPILER_TRANSLATOR_TREEOPS_REMOVESTATICALLYUNUSEDVARIABLES_H_
