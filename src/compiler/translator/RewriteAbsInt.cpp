//
// Copyright (c) 2016 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//

// RewriteAbsInt.cpp: converts abs(i) built-in calls where i is an integer
// to i * sign(i). This works around an Intel Mac drivers bug.

#include "compiler/translator/RewriteAbsInt.h"

#include "compiler/translator/IntermNode.h"

namespace
{

// An AST traverser that rewrites abs(i) to i * sign(i).
class AbsIntRewriter: public TIntermTraverser
{
  public:
    AbsIntRewriter() : TIntermTraverser(true, false, false) {}

    void nextIteration() { mNeedAnotherIteration = false; }
    bool needAnotherIteration() const { return mNeedAnotherIteration; }

    bool visitUnary(Visit, TIntermUnary *node) override
    {
        if (node->getOp() == EOpAbs && node->getType().getBasicType() == EbtInt) {
            TIntermTyped *iNode = node->getOperand();
            TIntermUnary *sign = new TIntermUnary(EOpSign);
            sign->setOperand(iNode);
            sign->setLine(node->getLine());
            sign->setType(iNode->getType());

            TIntermBinary *mul = new TIntermBinary(EOpMul);
            mul->setLeft(iNode);
            mul->setRight(sign);
            mul->setType(iNode->getType());
            mul->setLine(node->getLine());

            queueReplacement(node, mul, OriginalNode::IS_DROPPED);

            // If the i parameter also needs to be replaced, another traversal is needed.
            if (iNode->getAsUnaryNode() && iNode->getAsUnaryNode()->getOp() == EOpAbs)
            {
                mNeedAnotherIteration = true;
                return false;
            }
        }
        return true;
    }

  protected:
    bool mNeedAnotherIteration;
};

} // anonymous namespace

void RewriteAbsInt(TIntermNode *root)
{
    AbsIntRewriter rewriter;

    do {
        rewriter.nextIteration();
        root->traverse(&rewriter);
        rewriter.updateTree();
    }
    while (rewriter.needAnotherIteration());
}
