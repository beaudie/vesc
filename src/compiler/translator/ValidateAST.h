//
// Copyright (c) 2002-2015 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//

#ifndef COMPILER_TRANSLATOR_VALIDATEAST_H_
#define COMPILER_TRANSLATOR_VALIDATEAST_H_

#include "compiler/translator/BaseTypes.h"
#include "compiler/translator/Common.h"

namespace sh
{
class TDiagnostics;
class TIntermBlock;

struct ValidateASTOptions
{
    bool validateSingleParent = true;  // TODO
    bool validateFunctionCall = true;  // TODO
    bool validateNullNodes = true;  // TODO
    bool validateQualifiers = true;  // TODO
    bool validateInitializers = true;  // TODO
    bool validateUniqueFunctions = true;  // TODO
    bool validateStructUsage = true;  // TODO
    bool validateExpressions = true;  // TODO
    bool validateMultiDeclarations = false;  // TODO. Note: only enable if SeparateDeclarations have been called.
    // TODO syoussefi: describe what each option does
    // TODO syoussefi: make an object of these in the Compiler, and use it all the time, adjusting the state (validateMultiDeclarations currently) as passes are done
};

// Check for errors and output error messages on the context.
// Returns true if there are no errors.
bool ValidateAST(TIntermNode *root,
        const ValidateASTOptions &options);

}  // namespace sh

#endif  // COMPILER_TRANSLATOR_VALIDATESWITCH_H_
