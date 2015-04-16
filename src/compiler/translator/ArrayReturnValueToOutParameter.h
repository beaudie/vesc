//
// Copyright (c) 2002-2015 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//

#ifndef COMPILER_TRANSLATOR_ARRAYRETURNVALUETOOUTPARAMETER_H_
#define COMPILER_TRANSLATOR_ARRAYRETURNVALUETOOUTPARAMETER_H_

#include "compiler/translator/IntermNode.h"

// This class changes return values of an array type to out parameters in
// function definitions and prototypes. Call sites need to be handled separately.

class ArrayReturnValueToOutParameter : public TIntermTraverser
{
  public:
    ArrayReturnValueToOutParameter();
  private:
    bool visitAggregate(Visit visit, TIntermAggregate *node) override;
    bool visitBranch(Visit visit, TIntermBranch *node) override;

    bool mInFunctionWithArrayReturnValue;
};

#endif  // COMPILER_TRANSLATOR_ARRAYRETURNVALUETOOUTPARAMETER_H_
