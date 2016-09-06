//
// Copyright (c) 2016 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//

#include "compiler/translator/AddAndTrueToLoopCondition.h"

#include "compiler/translator/IntermNode.h"

namespace sh
{

namespace
{

// An AST traverser that rewrites for and while loops by replacing "condition" with
// "condition && true" to work around condition bug on Intel Mac.
class LoopConditionRewriter : public TIntermTraverser
{
  public:
    LoopConditionRewriter() : TIntermTraverser(true, false, false) {}

    bool visitLoop(Visit, TIntermLoop *loop) override
    {
        // do-while loop doesn't have this bug.
        if (loop->getType() != ELoopFor && loop->getType() != ELoopWhile)
        {
            return true;
        }

        // For loop may not have a condition.
        if (loop->getCondition() == nullptr)
        {
            return true;
        }

        // Constant true.
        TConstantUnion *trueConstant = new TConstantUnion();
        trueConstant->setBConst(true);
        TIntermTyped *trueValue = new TIntermConstantUnion(trueConstant, TType(EbtBool));

        // CONDITION && true.
        TIntermBinary *andOp = new TIntermBinary(EOpLogicalAnd, loop->getCondition(), trueValue);
        loop->setCondition(andOp);

        return true;
    }
};

}  // anonymous namespace

void RewriteLoopCondition(TIntermNode *root)
{
    LoopConditionRewriter rewriter;
    root->traverse(&rewriter);
}

}  // namespace sh
