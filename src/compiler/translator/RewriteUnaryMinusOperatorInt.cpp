//
// Copyright (c) 2016 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//
// Implementation of evaluating unary integer variable bug workaround.
// See header for more info.

#include "compiler/translator/RewriteUnaryMinusOperatorInt.h"

#include "compiler/translator/IntermNode.h"

namespace sh
{

namespace
{

class Traverser : public TIntermTraverser
{
  public:
    static void Apply(TIntermNode *root, unsigned int *tempIndex);

  private:
    Traverser();
    bool visitUnary(Visit visit, TIntermUnary *node) override;
    void nextIteration();

    bool mFound = false;
};

// static
void Traverser::Apply(TIntermNode *root, unsigned int *tempIndex)
{
    Traverser traverser;
    traverser.useTemporaryIndex(tempIndex);
    do
    {
        traverser.nextIteration();
        root->traverse(&traverser);
        if (traverser.mFound)
        {
            traverser.updateTree();
        }
    } while (traverser.mFound);
}

Traverser::Traverser() : TIntermTraverser(true, false, false)
{
}

void Traverser::nextIteration()
{
    mFound = false;
    nextTemporaryIndex();
}

bool Traverser::visitUnary(Visit visit, TIntermUnary *node)
{
    // Decide if the current unary operator is unary minus.
    if (node->getOp() != EOpNegative)
    {
        return true;
    }

    // Decide if the current operand is an integer variable.
    TIntermTyped *opr = node->getOperand();
    if (!opr->getType().isScalarSignedInt())
    {
        return true;
    }

    // Potential problem case detected, apply workaround.
    // Replace -(int) by ~(int) + 1.
    TIntermBinary *add = new TIntermBinary(EOpAdd);
    add->setType(opr->getType());

    // ~(int).
    TIntermUnary *bitwiseNot = new TIntermUnary(EOpBitwiseNot);
    bitwiseNot->setType(opr->getType());
    bitwiseNot->setOperand(opr);
    bitwiseNot->setLine(opr->getLine());
    add->setLeft(bitwiseNot);

    // Constant 1.
    TConstantUnion *one = new TConstantUnion();
    one->setIConst(1);
    TType *intType                = new TType(EbtInt);
    TIntermConstantUnion *oneNode = new TIntermConstantUnion(one, *intType);
    add->setRight(oneNode);

    queueReplacement(node, add, OriginalNode::IS_DROPPED);

    mFound = true;
    return true;
}

}  // anonymous namespace

void RewriteUnaryMinusOperatorInt(TIntermNode *root, unsigned int *tempIndex)
{
    Traverser::Apply(root, tempIndex);
}

}  // namespace sh