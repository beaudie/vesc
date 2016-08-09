//
// Copyright (c) 2002-2016 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//
// UnfoldShortCircuitAST is an AST traverser to replace short-circuiting
// operations with ternary operations.
//

#ifndef COMPILER_TRANSLATOR_EMULATEGLFRAGCOLORBROADCAST_H_
#define COMPILER_TRANSLATOR_EMULATEGLFRAGCOLORBROADCAST_H_

#include "common/angleutils.h"
#include "compiler/translator/IntermNode.h"

#include <vector>

namespace sh
{
struct OutputVariable;
}

void EmulateGLFragColorBroadcast(TIntermNode *root, int maxDrawBuffers,
                                 std::vector<sh::OutputVariable> *outputVariables);

#endif  // COMPILER_TRANSLATOR_EMULATEGLFRAGCOLORBROADCAST_H_
