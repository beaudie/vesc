//
// Copyright (c) 2002-2013 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//
// UnfoldShortCircuitToIf is an AST traverser to convert short-circuiting operators to if-else
// statements.
// The results are assigned to s# temporaries, which are used by the main translator instead of
// the original expression.
//

#include "compiler/translator/tree_ops/UnfoldShortCircuitToIf.h"

#include "compiler/translator/StaticType.h"
#include "compiler/translator/tree_util/IntermNodePatternMatcher.h"
#include "compiler/translator/tree_util/IntermNode_util.h"
#include "compiler/translator/tree_util/IntermTraverse.h"

namespace sh
{

namespace
{

class UnfoldShortCircuitTraverser : public TIntermTraverser2
{
  public:
    UnfoldShortCircuitTraverser(TSymbolTable *symbolTable);

    TAction *visitBinary(Visit visit, TIntermBinary *node) override;
    TAction *visitTernary(Visit visit, TIntermTernary *node) override;

  protected:

    IntermNodePatternMatcher mPatternToUnfoldMatcher;
};

UnfoldShortCircuitTraverser::UnfoldShortCircuitTraverser(TSymbolTable *symbolTable)
    : TIntermTraverser2(true, false, false, symbolTable),
      mPatternToUnfoldMatcher(IntermNodePatternMatcher::kUnfoldedShortCircuitExpression)
{
}

TAction *UnfoldShortCircuitTraverser::visitBinary(Visit visit, TIntermBinary *node)
{
    if (!mPatternToUnfoldMatcher.match(node, getParentNode()))
        return nullptr;

    // If our right node doesn't have side effects, we know we don't need to unfold this
    // expression: there will be no short-circuiting side effects to avoid
    // (note: unfolding doesn't depend on the left node -- it will always be evaluated)
    ASSERT(node->getRight()->hasSideEffects());

    TAction *action = new TAction(Continue::ThroughThisSubtree);

    switch (node->getOp())
    {
        case EOpLogicalOr:
        {
            // "x || y" is equivalent to "x ? true : y", which unfolds to "bool s; if(x) s = true;
            // else s = y;",
            // and then further simplifies down to "bool s = x; if(!s) s = y;".

            TIntermSequence insertions;
            const TType *boolType = StaticType::Get<EbtBool, EbpUndefined, EvqTemporary, 1, 1>();
            TVariable *resultVariable = CreateTempVariable(mSymbolTable, boolType);

            ASSERT(node->getLeft()->getType() == *boolType);
            auto insertion =
                new TMutation(Mutate::InsertBeforeThisNodeInParentBlock,
                              CreateTempInitDeclarationNode(resultVariable, node->getLeft()));
            action->mMutations.push_back(insertion);

            TIntermBlock *assignRightBlock = new TIntermBlock();
            ASSERT(node->getRight()->getType() == *boolType);
            assignRightBlock->getSequence()->push_back(
                CreateTempAssignmentNode(resultVariable, node->getRight()));

            TIntermUnary *notTempSymbol =
                new TIntermUnary(EOpLogicalNot, CreateTempSymbolNode(resultVariable), nullptr);
            TIntermIfElse *ifNode = new TIntermIfElse(notTempSymbol, assignRightBlock, nullptr);
            insertion = new TMutation(Mutate::InsertBeforeThisNodeInParentBlock, ifNode);
            action->mMutations.push_back(insertion);

            auto replacement =
                new TMutation(Mutate::ReplaceThisNode, CreateTempSymbolNode(resultVariable));
            action->mMutations.push_back(replacement);
            return action;
        }
        case EOpLogicalAnd:
        {
            // "x && y" is equivalent to "x ? y : false", which unfolds to "bool s; if(x) s = y;
            // else s = false;",
            // and then further simplifies down to "bool s = x; if(s) s = y;".
            const TType *boolType = StaticType::Get<EbtBool, EbpUndefined, EvqTemporary, 1, 1>();
            TVariable *resultVariable = CreateTempVariable(mSymbolTable, boolType);

            ASSERT(node->getLeft()->getType() == *boolType);
            auto insertion =
                new TMutation(Mutate::InsertBeforeThisNodeInParentBlock,
                              CreateTempInitDeclarationNode(resultVariable, node->getLeft()));
            action->mMutations.push_back(insertion);

            TIntermBlock *assignRightBlock = new TIntermBlock();
            ASSERT(node->getRight()->getType() == *boolType);
            assignRightBlock->getSequence()->push_back(
                CreateTempAssignmentNode(resultVariable, node->getRight()));

            TIntermIfElse *ifNode =
                new TIntermIfElse(CreateTempSymbolNode(resultVariable), assignRightBlock, nullptr);
            insertion = new TMutation(Mutate::InsertBeforeThisNodeInParentBlock, ifNode);
            action->mMutations.push_back(insertion);

            auto replacement =
                new TMutation(Mutate::ReplaceThisNode, CreateTempSymbolNode(resultVariable));
            action->mMutations.push_back(replacement);
            return action;
        }
        default:
            UNREACHABLE();
            return nullptr;
    }
}

TAction *UnfoldShortCircuitTraverser::visitTernary(Visit visit, TIntermTernary *node)
{
    if (!mPatternToUnfoldMatcher.match(node))
        return nullptr;

    TAction *action = new TAction(Continue::ThroughThisSubtree);

    // Unfold "b ? x : y" into "type s; if(b) s = x; else s = y;"
    TIntermSequence insertions;
    TIntermDeclaration *tempDeclaration = nullptr;
    TVariable *resultVariable = DeclareTempVariable(mSymbolTable, new TType(node->getType()),
                                                    EvqTemporary, &tempDeclaration);
    auto insertion = new TMutation(Mutate::InsertBeforeThisNodeInParentBlock, tempDeclaration);
    action->mMutations.push_back(insertion);

    TIntermBlock *trueBlock = new TIntermBlock();
    TIntermBinary *trueAssignment =
        CreateTempAssignmentNode(resultVariable, node->getTrueExpression());
    trueBlock->getSequence()->push_back(trueAssignment);

    TIntermBlock *falseBlock = new TIntermBlock();
    TIntermBinary *falseAssignment =
        CreateTempAssignmentNode(resultVariable, node->getFalseExpression());
    falseBlock->getSequence()->push_back(falseAssignment);

    TIntermIfElse *ifNode =
        new TIntermIfElse(node->getCondition()->getAsTyped(), trueBlock, falseBlock);
    insertion = new TMutation(Mutate::InsertBeforeThisNodeInParentBlock, ifNode);
    action->mMutations.push_back(insertion);

    TIntermSymbol *ternaryResult = CreateTempSymbolNode(resultVariable);
    auto replacement             = new TMutation(Mutate::ReplaceThisNode, ternaryResult);
    action->mMutations.push_back(replacement);
    return action;
}

}  // namespace

void UnfoldShortCircuitToIf(TIntermNode *root, TSymbolTable *symbolTable)
{
    UnfoldShortCircuitTraverser traverser(symbolTable);
    traverser.traverse(root);
}

}  // namespace sh
