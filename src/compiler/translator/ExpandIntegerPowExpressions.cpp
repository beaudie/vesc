//
// Copyright (c) 2016 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//
// Implementation of the integer pow expressions HLSL bug workaround.
// See header for more info.

#include "compiler/translator/ExpandIntegerPowExpressions.h"

#include <cmath>

#include "compiler/translator/IntermNode.h"

namespace sh
{

namespace
{

class Traverser : public TIntermTraverser
{
  public:
    static void Apply(TIntermNode *root);

  private:
    Traverser();
    bool visitAggregate(Visit visit, TIntermAggregate *node) override;

    unsigned int mTempIndex = 0;
};

// static
void Traverser::Apply(TIntermNode *root)
{
    Traverser traverser;
    root->traverse(&traverser);
    traverser.updateTree();
}

Traverser::Traverser() : TIntermTraverser(true, false, false)
{
    useTemporaryIndex(&mTempIndex);
}

bool Traverser::visitAggregate(Visit visit, TIntermAggregate *node)
{
    // Test 0: skip non-pow operators.
    if (node->getOp() != EOpPow)
    {
        return true;
    }

    const TIntermSequence *sequence = node->getSequence();
    ASSERT(sequence->size() == 2u);
    const TIntermConstantUnion *constantNode = sequence->at(1)->getAsConstantUnion();

    // Test 1: check for a constant.
    if (!constantNode)
    {
        return true;
    }

    ASSERT(constantNode->getNominalSize() == 1);
    const TConstantUnion *constant = constantNode->getUnionArrayPointer();

    TConstantUnion asFloat;
    asFloat.cast(EbtFloat, *constant);

    float value = asFloat.getFConst();

    // Test 2: value is in the problematic range.
    if (value < -5.0f || value > 9.0f)
    {
        return true;
    }

    // Test 3: value is integer or pretty close to an integer.
    float frac = abs(value) - std::floor(abs(value));
    if (frac > 0.0001f)
    {
        return true;
    }

    // Test 4: skip 1 and -1
    int exponent = static_cast<int>(value);
    int n        = abs(exponent);
    if (n == 1)
    {
        return true;
    }

    // Potential problem case detected, apply workaround.
    TIntermTyped *lhs = sequence->at(0)->getAsTyped();
    ASSERT(lhs);

    TIntermAggregate *init = createTempInitDeclaration(lhs);
    TIntermTyped *current  = createTempSymbol(lhs->getType());

    insertStatementInParentBlock(init);

    // Create a chain of n-1 multiples.
    for (int i = 1; i < n; ++i)
    {
        TIntermBinary *mul = new TIntermBinary(EOpMul);
        mul->setLeft(current);
        mul->setRight(createTempSymbol(lhs->getType()));
        mul->setType(node->getType());
        mul->setLine(node->getLine());
        current = mul;
    }

    // For negative pow, compute the reciprocal of the positive pow.
    if (exponent < 0)
    {
        TConstantUnion *oneVal = new TConstantUnion();
        oneVal->setFConst(1.0f);
        TIntermConstantUnion *oneNode = new TIntermConstantUnion(oneVal, node->getType());
        TIntermBinary *div            = new TIntermBinary(EOpDiv);
        div->setLeft(oneNode);
        div->setRight(current);
        current = div;
    }

    replace(node, current);
    return true;
}

}  // anonymous namespace

void ExpandIntegerPowExpressions(TIntermNode *root)
{
    Traverser::Apply(root);
}

}  // namespace sh
