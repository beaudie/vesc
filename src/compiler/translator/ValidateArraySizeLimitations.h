//
// Copyright 2010 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//

#ifndef COMPILER_TRANSLATOR_VALIDATEARRAYSIZELIMITATIONS_H_
#define COMPILER_TRANSLATOR_VALIDATEARRAYSIZELIMITATIONS_H_

#include "compiler/translator/IntermNode.h"

namespace sh
{

class TDiagnostics;

// Returns true if the given shader does not violate certain
// implementation-defined limits on array sizes.
bool ValidateArraySizeLimitations(TIntermNode *root,
                                  TSymbolTable *symbolTable,
                                  TDiagnostics *diagnostics);

}  // namespace sh

#endif  // COMPILER_TRANSLATOR_VALIDATEARRAYSIZELIMITATIONS_H_
