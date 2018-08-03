//
// Copyright (c) 2002-2018 The ANGLE Project Authors. All rights reserved.
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
// handle all the calls of atomicExchange and atomicCompSwap without return value.
class RewriteAtomicExchangeAndCompSwapTraverser : public TIntermTraverser
{
  public:
    RewriteAtomicExchangeAndCompSwapTraverser(TSymbolTable *symbolTable);

    bool visitAggregate(Visit visit, TIntermAggregate *node) override;

  protected:
    IntermNodePatternMatcher mPatternToRewriteMatcher;
};

RewriteAtomicExchangeAndCompSwapTraverser::RewriteAtomicExchangeAndCompSwapTraverser(
    TSymbolTable *symbolTable)
    : TIntermTraverser(true, false, false, symbolTable),
      mPatternToRewriteMatcher(
          IntermNodePatternMatcher::kAtomicExchangeAndAtomicCompSwapWithoutReturnValue)
{
}

bool RewriteAtomicExchangeAndCompSwapTraverser::visitAggregate(Visit visit, TIntermAggregate *node)
{
    if (!mPatternToRewriteMatcher.match(node, getParentNode()))
        return true;

    ASSERT(node->getOp() == EOpAtomicExchange || node->getOp() == EOpAtomicCompSwap);

    TIntermSequence insertions;

    // Declare a temporary variable
    TIntermDeclaration *returnVariableDeclaration;
    TVariable *returnVariable = DeclareTempVariable(mSymbolTable, &node->getType(), EvqTemporary,
                                                    &returnVariableDeclaration);
    insertions.push_back(returnVariableDeclaration);

    // Use this variable as the return value of the atomic function call.
    TIntermBinary *atomicFunctionAssignmentNode =
        new TIntermBinary(TOperator::EOpAssign, CreateTempSymbolNode(returnVariable), node);

    insertStatementsInParentBlock(insertions);
    queueReplacement(atomicFunctionAssignmentNode, OriginalNode::IS_DROPPED);

    return false;
}
}  // anonymous namespace

void RewriteAtomicFunctionExpressions(TIntermNode *root, TSymbolTable *symbolTable)
{
    RewriteAtomicExchangeAndCompSwapTraverser traverser(symbolTable);
    traverser.traverse(root);
    traverser.updateTree();
}
}  // namespace sh