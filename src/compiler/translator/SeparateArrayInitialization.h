//
// Copyright (c) 2002-2015 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//

#ifndef COMPILER_TRANSLATOR_SEPARATEARRAYINITIALIZATION_H_
#define COMPILER_TRANSLATOR_SEPARATEARRAYINITIALIZATION_H_

#include "compiler/translator/IntermNode.h"

// This class splits each array initialization into a declaration and an assignment.
// Example:
//     type[n] a = initializer;
// will effectively become
//     type[n] a;
//     a = initializer;

class SeparateArrayInitialization : public TIntermTraverser
{
  public:
    SeparateArrayInitialization();
  private:
    bool visitAggregate(Visit, TIntermAggregate *node) override;
};

#endif  // COMPILER_TRANSLATOR_SEPARATEARRAYINITIALIZATION_H_
