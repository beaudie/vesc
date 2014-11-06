//
// Copyright (c) 2002-2013 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//
// UnfoldShortCircuitAST is an AST traverser to replace short-circuiting
// operations with ternary operations.
//

#ifndef COMPILER_UNFOLD_SHORT_CIRCUIT_AST_H_
#define COMPILER_UNFOLD_SHORT_CIRCUIT_AST_H_

#include "compiler/translator/ReplaceNodesAST.h"

// This traverser identifies all the short circuit binary nodes that need to
// be replaced, and creates the corresponding replacement nodes. However,
// the actual replacements happen after the traverse through updateTree().

class UnfoldShortCircuitAST : public ReplaceNodesAST
{
  public:
    UnfoldShortCircuitAST() { }

    virtual bool visitBinary(Visit visit, TIntermBinary *);

  private:
    DISALLOW_COPY_AND_ASSIGN(UnfoldShortCircuitAST);
};

#endif  // COMPILER_UNFOLD_SHORT_CIRCUIT_AST_H_
