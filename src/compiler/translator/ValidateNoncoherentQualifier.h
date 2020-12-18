//
// Copyright 2020 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//
// ValidateNoncoherentQualifier validates noncoherent qualifier. It checks whether noncoherent
// qualifier are missed at gl_LastFragData. 'inout' variable is already checked at ParseContext.cpp
//

#ifndef COMPILER_TRANSLATOR_VALIDATENONCOHERENTQUALIFIER_H_
#define COMPILER_TRANSLATOR_VALIDATENONCOHERENTQUALIFIER_H_

#include "compiler/translator/ExtensionBehavior.h"

namespace sh
{

class TIntermBlock;
class TDiagnostics;

// Returns true if the shader has no conflicting or otherwise erroneous fragment outputs.
bool ValidateNoncoherentQualifier(TIntermBlock *root,
                                  const TExtensionBehavior &extBehavior,
                                  TDiagnostics *diagnostics);

}  // namespace sh

#endif  // COMPILER_TRANSLATOR_VALIDATENONCOHERENTQUALIFIER_H_
