//
// Copyright (c) 2002-2015 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//

#ifndef COMPILER_TRANSLATOR_SEPARATEDECLARATIONS_H_
#define COMPILER_TRANSLATOR_SEPARATEDECLARATIONS_H_

#include "compiler/translator/IntermNode.h"

// This class separates declarations so that each declarator gets its own declaration.
// This is useful as an intermediate step when initialization needs to be separated from declaration.
// Example:
//     int a = 1, b = 2;
// gets transformed when run through this class into the AST equivalent of:
//     int a = 1;
//     int b = 2;

class SeparateDeclarations : public TIntermTraverser
{
  public:
    SeparateDeclarations();
  private:
    bool visitAggregate(Visit, TIntermAggregate *node) override;
};

#endif  // COMPILER_TRANSLATOR_SEPARATEDECLARATIONS_H_
