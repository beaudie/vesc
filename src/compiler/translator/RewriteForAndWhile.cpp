//
// Copyright (c) 2016 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//

// RewriteForAndWhile.cpp: rewrites condition of for and while loops to work around
// condition bug on Intel Mac.

#include "compiler/translator/RewriteForAndWhile.h"

#include "compiler/translator/IntermNode.h"

namespace sh
{

namespace
{

// An AST traverser that rewrites for and while loops by replacing "condition" with
// "condition && true" to work around condition bug on Intel Mac.
class ForAndWhileRewriter : public TIntermTraverser
{
  public:
    ForAndWhileRewriter() : TIntermTraverser(true, false, false) {}

    bool visitAggregate(Visit, TIntermAggregate *node) override
    {
        if (node->getOp() != EOpSequence)
        {
            return true;
        }

        TIntermSequence *statements = node->getSequence();

        for (size_t i = 0; i < statements->size(); i++)
        {
            TIntermNode *statement = (*statements)[i];
            TIntermLoop *loop      = statement->getAsLoopNode();

            if (loop == nullptr || (loop->getType() != ELoopFor && loop->getType() != ELoopWhile))
            {
                continue;
            }

            // For loop may not have a condition.
            if (loop->getCondition() == nullptr)
            {
                continue;
            }

            TType boolType = TType(EbtBool);

            // Constant true.
            TConstantUnion *trueConstant = new TConstantUnion();
            trueConstant->setBConst(true);
            TIntermTyped *trueValue = new TIntermConstantUnion(trueConstant, boolType);

            // CONDITION && true.
            TIntermBinary *andOp =
                new TIntermBinary(EOpLogicalAnd, loop->getCondition(), trueValue);
            andOp->setType(boolType);
            loop->setCondition(andOp);

            nextTemporaryIndex();
        }
        return true;
    }
};

}  // anonymous namespace

void RewriteForAndWhile(TIntermNode *root, unsigned int *temporaryIndex)
{
    ASSERT(temporaryIndex != 0);

    ForAndWhileRewriter rewriter;
    rewriter.useTemporaryIndex(temporaryIndex);

    root->traverse(&rewriter);
}

}  // namespace sh
