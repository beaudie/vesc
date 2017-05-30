//
// Copyright (c) 2013 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//

#ifndef COMPILER_TRANSLATOR_VALIDATEOUTPUTS_H_
#define COMPILER_TRANSLATOR_VALIDATEOUTPUTS_H_

#include "compiler/translator/ExtensionBehavior.h"

namespace sh
{

class TIntermBlock;
class TDiagnostics;

// Returns true if the shader has no conflicting or missing fragment outputs.
bool ValidateOutputs(TIntermBlock *root, const TExtensionBehavior &extBehavior, int maxDrawBuffers, TDiagnostics *diagnostics);

}  // namespace sh

#endif  // COMPILER_TRANSLATOR_VALIDATEOUTPUTS_H_
