//
// Copyright (c) 2002-2014 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//

#ifndef COMPILER_TRANSLATOR_INTERMEDIATE_H_
#define COMPILER_TRANSLATOR_INTERMEDIATE_H_

#include "compiler/translator/IntermNode.h"

namespace sh
{

struct TVectorFields
{
    int offsets[4];
    int num;
};

//
// Set of helper functions to help build the tree.
//
class TIntermediate
{
  public:
    POOL_ALLOCATOR_NEW_DELETE();
    TIntermediate() {}

    TIntermTyped *addIndex(TOperator op,
                           TIntermTyped *base,
                           TIntermTyped *index,
                           const TSourceLoc &line,
                           TDiagnostics *diagnostics);
    static TIntermBlock *EnsureBlock(TIntermNode *node);
    TIntermConstantUnion *addConstantUnion(const TConstantUnion *constantUnion,
                                           const TType &type,
                                           const TSourceLoc &line);

    TIntermTyped *foldAggregateBuiltIn(TIntermAggregate *aggregate, TDiagnostics *diagnostics);

  private:
    void operator=(TIntermediate &);  // prevent assignments
};

}  // namespace sh

#endif  // COMPILER_TRANSLATOR_INTERMEDIATE_H_
