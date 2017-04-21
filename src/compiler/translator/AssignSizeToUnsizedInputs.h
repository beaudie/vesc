//
// Copyright (c) 2002-2017 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//
// Assign size to all unsized geometry shader input variables when the
// input primitive is declared.
//

#ifndef COMPILER_TRANSLATOR_ASSIGNSIZETOUNSIZEDINPUTS_H_
#define COMPILER_TRANSLATOR_ASSIGNSIZETOUNSIZEDINPUTS_H_

#include "compiler/translator/IntermNode.h"

namespace sh
{
class TIntermNode;

void AssignSizeToUnsizedInputs(TIntermNode *root, unsigned int arraySize);
}  // namespace sh

#endif  // COMPILER_TRANSLATOR_ASSIGNSIZETOUNSIZEDINPUTS_H_