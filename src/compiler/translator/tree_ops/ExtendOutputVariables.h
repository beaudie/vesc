//
// Copyright 2023 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//

#ifndef COMPILER_TRANSLATOR_TREEOPS_EXTENDOUTPUTVARIABLES_H_
#define COMPILER_TRANSLATOR_TREEOPS_EXTENDOUTPUTVARIABLES_H_

#include "common/angleutils.h"
#include "compiler/translator/Compiler.h"

namespace sh
{

// This function extends output variables to vec4.
[[nodiscard]] bool ExtendOutputVariables(TCompiler *compiler, TIntermBlock *root);

}  // namespace sh

#endif  // COMPILER_TRANSLATOR_TREEOPS_EXTENDOUTPUTVARIABLES_H_
