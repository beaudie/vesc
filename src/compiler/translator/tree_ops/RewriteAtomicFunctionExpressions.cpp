//
// Copyright (c) 2018 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//
// Implementation of the function RewriteAtomicFunctionExpressions.
// See the header for more details.

#include "RewriteAtomicFunctionExpressions.h"

#include "compiler/translator/tree_util/IntermNodePatternMatcher.h"
#include "compiler/translator/tree_util/IntermNode_util.h"
#include "compiler/translator/tree_util/IntermTraverse.h"

namespace sh
{
namespace
{
bool IsAtomicExchangeOrCompSwap(TOperator op)
{
    return op == EOpAtomicExchange || op == EOpAtomicCompSwap;
}

// Traverser that simplifies all the atomic function expressions into the ones that can be directly
// translated into HLSL.
class RewriteAtomicFunctionExpressionsTraverser : public TIntermTraverser
{
  public:
    RewriteAtomicFunctionExpressionsTraverser(TSymbolTable *symbolTable);

    bool visitAggregate(Visit visit, TIntermAggregate *node) override;
    bool visitBinary(Visit visit, TIntermBinary *node) override;

    void nextIteration();
    bool foundAtomicFunctionInComplexExpression() const
    {
        return mFoundAtomicfunctionInComplexExpression;
    }

  private:
    void separateAtomicFunctionCallNode(TIntermAggregate *oldAtomicFunctionNode,
                                        bool useTempSymbolNode);

    // We need more iterations to handle atomic function calls in complex expressions.
    bool mFoundAtomicfunctionInComplexExpression;
    IntermNodePatternMatcher mPatternToSingleAtomicFunctionAssignment;
};

RewriteAtomicFunctionExpressionsTraverser::RewriteAtomicFunctionExpressionsTraverser(
    TSymbolTable *symbolTable)
    : TIntermTraverser(true, false, true, symbolTable),
      mFoundAtomicfunctionInComplexExpression(false),
      mPatternToSingleAtomicFunctionAssignment(
          IntermNodePatternMatcher::kSingleAtomicFunctionAssignment)
{
}

void RewriteAtomicFunctionExpressionsTraverser::separateAtomicFunctionCallNode(
    TIntermAggregate *oldAtomicFunctionNode,
    bool useTempSymbolNode)
{
    ASSERT(oldAtomicFunctionNode);

    TIntermSequence insertions;

    // Declare a temporary variable
    TIntermDeclaration *returnVariableDeclaration;
    TVariable *returnVariable = DeclareTempVariable(mSymbolTable, &oldAtomicFunctionNode->getType(),
                                                    EvqTemporary, &returnVariableDeclaration);
    insertions.push_back(returnVariableDeclaration);

    // Use this variable as the return value of the atomic function call.
    TIntermBinary *atomicFunctionAssignment = new TIntermBinary(
        TOperator::EOpAssign, CreateTempSymbolNode(returnVariable), oldAtomicFunctionNode);

    TIntermTyped *newNode = nullptr;
    if (useTempSymbolNode)
    {
        insertions.push_back(atomicFunctionAssignment);
        newNode = CreateTempSymbolNode(returnVariable);
    }
    else
    {
        newNode = atomicFunctionAssignment;
    }

    insertStatementsInParentBlock(insertions);
    queueReplacement(newNode, OriginalNode::IS_DROPPED);
}

bool RewriteAtomicFunctionExpressionsTraverser::visitBinary(Visit visit, TIntermBinary *node)
{
    if (mFoundAtomicfunctionInComplexExpression)
    {
        return false;
    }

    if (visit == Visit::PreVisit &&
        mPatternToSingleAtomicFunctionAssignment.match(node, getParentNode()))
    {
        return false;
    }

    return true;
}

bool RewriteAtomicFunctionExpressionsTraverser::visitAggregate(Visit visit, TIntermAggregate *node)
{
    // We should use post-traversal to ensure the right execution order of the separated atomic
    // function expressions.
    // e.g. value = (atomicAdd(mem, val) - 1) * atomicAnd(mem, val) should be modified into:
    //      int temp1; temp1 = atomicAdd(mem, val);
    //      int temp2; temp2 = atomicAnd(mem, val);
    //      value = (temp1 - 1) * temp2;
    if (visit == Visit::PreVisit)
    {
        return true;
    }

    if (mFoundAtomicfunctionInComplexExpression)
    {
        return false;
    }

    if (!IsAtomicFunction(node->getOp()))
    {
        return true;
    }

    // For a single atomic function call without return value, we only need to handle atomicExchange
    // and atomicCompSwap.
    if (getParentNode() && getParentNode()->getAsBlock())
    {
        if (IsAtomicExchangeOrCompSwap(node->getOp()))
        {
            separateAtomicFunctionCallNode(node, false);
        }
        return true;
    }
    else
    {
        separateAtomicFunctionCallNode(node, true);

        mFoundAtomicfunctionInComplexExpression = true;
        return false;
    }
}

void RewriteAtomicFunctionExpressionsTraverser::nextIteration()
{
    mFoundAtomicfunctionInComplexExpression = false;
}
}  // anonymous namespace

void RewriteAtomicFunctionExpressions(TIntermNode *root, TSymbolTable *symbolTable)
{
    RewriteAtomicFunctionExpressionsTraverser traverser(symbolTable);
    do
    {
        traverser.nextIteration();
        root->traverse(&traverser);
        traverser.updateTree();
    } while (traverser.foundAtomicFunctionInComplexExpression());
}
}  // namespace sh