// Copyright (c) 2017 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//
// VectorizeVectorScalarArithmetic.cpp: Turn some arithmetic operations that operate on a float
// vector-scalar pair into vector-vector operations. This is done recursively. Some scalar binary
// operations inside vector constructors are also turned into vector operations.
//
// This is targeted to work around a bug in NVIDIA OpenGL drivers that was reproducible on NVIDIA
// driver version 387.92.

#include "compiler/translator/VectorizeVectorScalarArithmetic.h"

#include "compiler/translator/IntermNode.h"
#include "compiler/translator/IntermTraverse.h"

namespace sh
{

namespace
{

class VectorizeVectorScalarArithmeticTraverser : public TIntermTraverser
{
  public:
    VectorizeVectorScalarArithmeticTraverser()
        : TIntermTraverser(true, false, false), mReplaced(false)
    {
    }

    bool didReplaceScalarsWithVectors() { return mReplaced; }
    void nextIteration() { mReplaced = false; }

  protected:
    bool visitBinary(Visit visit, TIntermBinary *node) override;
    bool visitAggregate(Visit visit, TIntermAggregate *node) override;

  private:
    TIntermTyped *vectorize(TIntermTyped *node,
                            TType vectorType,
                            TIntermTraverser::OriginalNode *originalNodeFate);

    bool mReplaced;
};

TIntermTyped *VectorizeVectorScalarArithmeticTraverser::vectorize(
    TIntermTyped *node,
    TType vectorType,
    TIntermTraverser::OriginalNode *originalNodeFate)
{
    ASSERT(node->isScalar());
    vectorType.setQualifier(EvqTemporary);
    TIntermSequence vectorizedArgs;
    vectorizedArgs.push_back(node);
    TIntermAggregate *leftVectorized =
        TIntermAggregate::CreateConstructor(vectorType, &vectorizedArgs);
    TIntermTyped *leftVectorizedFolded = leftVectorized->fold(nullptr);
    if (originalNodeFate != nullptr)
    {
        if (leftVectorizedFolded != leftVectorized)
        {
            *originalNodeFate = OriginalNode::IS_DROPPED;
        }
        else
        {
            *originalNodeFate = OriginalNode::BECOMES_CHILD;
        }
    }
    return leftVectorizedFolded;
}

bool VectorizeVectorScalarArithmeticTraverser::visitBinary(Visit /*visit*/, TIntermBinary *node)
{
    TIntermTyped *left  = node->getLeft();
    TIntermTyped *right = node->getRight();
    ASSERT(left);
    ASSERT(right);
    switch (node->getOp())
    {
        case EOpAdd:
        case EOpAddAssign:
            // Only these specific ops are necessary to vectorize.
            break;
        default:
            return true;
    }
    if (node->getBasicType() != EbtFloat)
    {
        // Only float ops have reproduced the bug.
        return true;
    }
    if (left->isScalar() && right->isVector())
    {
        ASSERT(!node->isAssignment());
        ASSERT(!right->isArray());
        OriginalNode originalNodeFate;
        TIntermTyped *leftVectorized = vectorize(left, right->getType(), &originalNodeFate);
        queueReplacementWithParent(node, left, leftVectorized, originalNodeFate);
        mReplaced = true;
        // Don't replace more nodes in the same subtree on this traversal. However, nodes elsewhere
        // in the tree may still be replaced.
        return false;
    }
    else if (left->isVector() && right->isScalar())
    {
        OriginalNode originalNodeFate;
        TIntermTyped *rightVectorized = vectorize(right, left->getType(), &originalNodeFate);
        queueReplacementWithParent(node, right, rightVectorized, originalNodeFate);
        mReplaced = true;
        // Don't replace more nodes in the same subtree on this traversal. However, nodes elsewhere
        // in the tree may still be replaced.
        return false;
    }
    return true;
}

bool VectorizeVectorScalarArithmeticTraverser::visitAggregate(Visit /*visit*/,
                                                              TIntermAggregate *node)
{
    if (node->isConstructor() && node->isVector() && node->getSequence()->size() == 1)
    {
        TIntermTyped *argument = node->getSequence()->back()->getAsTyped();
        ASSERT(argument);
        if (argument->isScalar() && argument->getBasicType() == EbtFloat)
        {
            TIntermBinary *argBinary = argument->getAsBinaryNode();

            // TODO: EOpMulAssign and EOpDivAssign need different handling.

            if (argBinary)
            {
                TIntermTyped *left = argBinary->getLeft();
                TIntermTyped *right = argBinary->getRight();

                // Only specific ops are necessary to vectorize.
                switch (argBinary->getOp())
                {
                    case EOpMul:
                    case EOpDiv:
                    {
                        ASSERT(left->isScalar() && right->isScalar());

                        // Turn:
                        //   a op b
                        // into:
                        //   gvec(a) op gvec(b)
                        TType leftVectorizedType = left->getType();
                        leftVectorizedType.setPrimarySize(
                            static_cast<unsigned char>(node->getType().getNominalSize()));
                        TIntermTyped *leftVectorized = vectorize(left, leftVectorizedType, nullptr);
                        TType rightVectorizedType = right->getType();
                        rightVectorizedType.setPrimarySize(
                            static_cast<unsigned char>(node->getType().getNominalSize()));
                        TIntermTyped *rightVectorized = vectorize(right, rightVectorizedType, nullptr);

                        TIntermBinary *newArg =
                            new TIntermBinary(argBinary->getOp(), leftVectorized, rightVectorized);
                        queueReplacementWithParent(node, argBinary, newArg, OriginalNode::IS_DROPPED);
                        mReplaced = true;
                        // Don't replace more nodes in the same subtree on this traversal. However, nodes
                        // elsewhere in the tree may still be replaced.
                        return false;
                    }
                    case EOpMulAssign:
                    case EOpDivAssign:
                    {
                        ASSERT(left->isScalar() && right->isScalar());

                        if (left->hasSideEffects())
                        {
                            // Bail out, this case might repro the bug but is too complex to be worthwhile to handle.
                            return true;
                        }
                        if (!right->hasSideEffects())
                        {
                            // Turn:
                            //   vec3(a *= b)
                            // into:
                            //   vec3(a *= b, a *= b, a *= b)
                            // etc.
                        }
                        else {
                            // Turn:
                            //   vec3(a *= b)
                            // into:
                            //   float s0 = b;
                            //   vec3(a *= s0, a *= s0, a *= s0)
                            // etc.
                        }
                    }
                    default:
                        return true;
                }
            }
        }
    }
    return true;
}

}  // anonymous namespace

void VectorizeVectorScalarArithmetic(TIntermBlock *root)
{
    VectorizeVectorScalarArithmeticTraverser traverser;
    do
    {
        traverser.nextIteration();
        root->traverse(&traverser);
        traverser.updateTree();
    } while (traverser.didReplaceScalarsWithVectors());
}

}  // namespace sh