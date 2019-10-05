//
// Copyright 2016 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//
// SimplifyLoopConditions is an AST traverser that converts loop conditions and loop expressions
// to regular statements inside the loop. This way further transformations that generate statements
// from loop conditions and loop expressions work correctly.
//

#include "compiler/translator/tree_ops/SimplifyLoopConditions.h"

#include "compiler/translator/StaticType.h"
#include "compiler/translator/tree_util/IntermNodePatternMatcher.h"
#include "compiler/translator/tree_util/IntermNode_util.h"
#include "compiler/translator/tree_util/IntermTraverse.h"

namespace sh
{

namespace
{

class SimplifyLoopConditionsTraverser : public TLValueTrackingTraverser
{
  public:
    SimplifyLoopConditionsTraverser(unsigned int conditionsToSimplifyMask,
                                    TSymbolTable *symbolTable);

    void traverseLoop(TIntermLoop *node) override;

    bool visitUnary(Visit visit, TIntermUnary *node) override;
    bool visitBinary(Visit visit, TIntermBinary *node) override;
    bool visitAggregate(Visit visit, TIntermAggregate *node) override;
    bool visitTernary(Visit visit, TIntermTernary *node) override;
    bool visitDeclaration(Visit visit, TIntermDeclaration *node) override;
    bool visitBranch(Visit visit, TIntermBranch *node) override;

    bool foundLoopToChange() const { return mFoundLoopToChange; }

  protected:
    void fixupLoopBranches(TIntermBlock *body,
                           TVariable *condVar,
                           TIntermTyped *cond,
                           TIntermTyped *expr);

    // Marked to true once an operation that needs to be hoisted out of a loop expression has been
    // found.
    bool mFoundLoopToChange;
    bool mInsideLoopInitConditionOrExpression;
    IntermNodePatternMatcher mConditionsToSimplify;

    int mCurrentLoopDepth = 0;
    struct BranchFixupState
    {
        int loopDepth = -1;
        TVariable *condVar;
        TIntermTyped *cond;
        TIntermTyped *expr;
    };
    BranchFixupState mBranchFixupState = {};
};

SimplifyLoopConditionsTraverser::SimplifyLoopConditionsTraverser(
    unsigned int conditionsToSimplifyMask,
    TSymbolTable *symbolTable)
    : TLValueTrackingTraverser(true, false, false, symbolTable),
      mFoundLoopToChange(false),
      mInsideLoopInitConditionOrExpression(false),
      mConditionsToSimplify(conditionsToSimplifyMask)
{}

// If we're inside a loop initialization, condition, or expression, we check for expressions that
// should be moved out of the loop condition or expression. If one is found, the loop is
// transformed.
// If we're not inside loop initialization, condition, or expression, we only need to traverse nodes
// that may contain loops.

bool SimplifyLoopConditionsTraverser::visitUnary(Visit visit, TIntermUnary *node)
{
    if (!mInsideLoopInitConditionOrExpression)
        return false;

    if (mFoundLoopToChange)
        return false;  // Already decided to change this loop.

    mFoundLoopToChange = mConditionsToSimplify.match(node);
    return !mFoundLoopToChange;
}

bool SimplifyLoopConditionsTraverser::visitBinary(Visit visit, TIntermBinary *node)
{
    if (!mInsideLoopInitConditionOrExpression)
        return false;

    if (mFoundLoopToChange)
        return false;  // Already decided to change this loop.

    mFoundLoopToChange = mConditionsToSimplify.match(node, getParentNode(), isLValueRequiredHere());
    return !mFoundLoopToChange;
}

bool SimplifyLoopConditionsTraverser::visitAggregate(Visit visit, TIntermAggregate *node)
{
    if (!mInsideLoopInitConditionOrExpression)
        return false;

    if (mFoundLoopToChange)
        return false;  // Already decided to change this loop.

    mFoundLoopToChange = mConditionsToSimplify.match(node, getParentNode());
    return !mFoundLoopToChange;
}

bool SimplifyLoopConditionsTraverser::visitTernary(Visit visit, TIntermTernary *node)
{
    if (!mInsideLoopInitConditionOrExpression)
        return false;

    if (mFoundLoopToChange)
        return false;  // Already decided to change this loop.

    mFoundLoopToChange = mConditionsToSimplify.match(node);
    return !mFoundLoopToChange;
}

bool SimplifyLoopConditionsTraverser::visitDeclaration(Visit visit, TIntermDeclaration *node)
{
    if (!mInsideLoopInitConditionOrExpression)
        return false;

    if (mFoundLoopToChange)
        return false;  // Already decided to change this loop.

    mFoundLoopToChange = mConditionsToSimplify.match(node);
    return !mFoundLoopToChange;
}

void SimplifyLoopConditionsTraverser::traverseLoop(TIntermLoop *node)
{
    mCurrentLoopDepth++;

    // Mark that we're inside a loop condition or expression, and determine if the loop needs to be
    // transformed.

    ScopedNodeInTraversalPath addToPath(this, node);

    mInsideLoopInitConditionOrExpression = true;
    mFoundLoopToChange                   = false;

    if (!mFoundLoopToChange && node->getInit())
    {
        node->getInit()->traverse(this);
    }

    if (!mFoundLoopToChange && node->getCondition())
    {
        node->getCondition()->traverse(this);
    }

    if (!mFoundLoopToChange && node->getExpression())
    {
        node->getExpression()->traverse(this);
    }

    mInsideLoopInitConditionOrExpression = false;

    if (mFoundLoopToChange)
    {
        const TType *boolType        = StaticType::Get<EbtBool, EbpUndefined, EvqTemporary, 1, 1>();
        TVariable *conditionVariable = CreateTempVariable(mSymbolTable, boolType);

        // Replace the loop condition with a boolean variable that's updated on each iteration.
        TLoopType loopType = node->getType();
        if (loopType == ELoopWhile)
        {
            // Transform:
            //   while (expr) { body; }
            // into
            //   bool s0 = expr;
            //   while (s0) { { body; } s0 = expr; }
            TIntermDeclaration *tempInitDeclaration =
                CreateTempInitDeclarationNode(conditionVariable, node->getCondition()->deepCopy());
            insertStatementInParentBlock(tempInitDeclaration);

            TIntermBlock *newBody = new TIntermBlock();
            if (node->getBody())
            {
                fixupLoopBranches(node->getBody(), conditionVariable, node->getCondition(),
                                  nullptr);
                newBody->getSequence()->push_back(node->getBody());
            }
            newBody->getSequence()->push_back(
                CreateTempAssignmentNode(conditionVariable, node->getCondition()->deepCopy()));

            // Can't use queueReplacement to replace old body, since it may have been nullptr.
            // It's safe to do the replacements in place here - the new body will still be
            // traversed, but that won't create any problems.
            node->setBody(newBody);
            node->setCondition(CreateTempSymbolNode(conditionVariable));
        }
        else if (loopType == ELoopDoWhile)
        {
            // Transform:
            //   do {
            //     body;
            //   } while (expr);
            // into
            //   bool s0 = true;
            //   do {
            //     { body; }
            //     s0 = expr;
            //   } while (s0);
            TIntermDeclaration *tempInitDeclaration =
                CreateTempInitDeclarationNode(conditionVariable, CreateBoolNode(true));
            insertStatementInParentBlock(tempInitDeclaration);

            TIntermBlock *newBody = new TIntermBlock();
            if (node->getBody())
            {
                fixupLoopBranches(node->getBody(), conditionVariable, node->getCondition(),
                                  nullptr);
                newBody->getSequence()->push_back(node->getBody());
            }
            newBody->getSequence()->push_back(
                CreateTempAssignmentNode(conditionVariable, node->getCondition()->deepCopy()));

            // Can't use queueReplacement to replace old body, since it may have been nullptr.
            // It's safe to do the replacements in place here - the new body will still be
            // traversed, but that won't create any problems.
            node->setBody(newBody);
            node->setCondition(CreateTempSymbolNode(conditionVariable));
        }
        else if (loopType == ELoopFor)
        {
            // Move the loop condition inside the loop.
            // Transform:
            //   for (init; expr; exprB) { body; }
            // into
            //   {
            //     init;
            //     bool s0 = expr;
            //     while (s0) {
            //       { body; }
            //       exprB;
            //       s0 = expr;
            //     }
            //   }
            TIntermBlock *loopScope            = new TIntermBlock();
            TIntermSequence *loopScopeSequence = loopScope->getSequence();

            // Insert "init;"
            if (node->getInit())
            {
                loopScopeSequence->push_back(node->getInit());
            }

            // Insert "bool s0 = expr;" if applicable, "bool s0 = true;" otherwise
            TIntermTyped *conditionInitializer = nullptr;
            if (node->getCondition())
            {
                conditionInitializer = node->getCondition()->deepCopy();
            }
            else
            {
                conditionInitializer = CreateBoolNode(true);
            }
            loopScopeSequence->push_back(
                CreateTempInitDeclarationNode(conditionVariable, conditionInitializer));

            // Insert "{ body; }" in the while loop
            TIntermBlock *whileLoopBody = new TIntermBlock();
            if (node->getBody())
            {
                fixupLoopBranches(node->getBody(), conditionVariable, node->getCondition(),
                                  node->getExpression());
                whileLoopBody->getSequence()->push_back(node->getBody());
            }
            // Insert "exprB;" in the while loop
            if (node->getExpression())
            {
                whileLoopBody->getSequence()->push_back(node->getExpression());
            }
            // Insert "s0 = expr;" in the while loop
            if (node->getCondition())
            {
                whileLoopBody->getSequence()->push_back(
                    CreateTempAssignmentNode(conditionVariable, node->getCondition()->deepCopy()));
            }

            // Create "while(s0) { whileLoopBody }"
            TIntermLoop *whileLoop =
                new TIntermLoop(ELoopWhile, nullptr, CreateTempSymbolNode(conditionVariable),
                                nullptr, whileLoopBody);
            loopScopeSequence->push_back(whileLoop);
            queueReplacement(loopScope, OriginalNode::IS_DROPPED);

            // After this the old body node will be traversed and loops inside it may be
            // transformed. This is fine, since the old body node will still be in the AST after the
            // transformation that's queued here, and transforming loops inside it doesn't need to
            // know the exact post-transform path to it.
        }
    }

    mFoundLoopToChange = false;

    // We traverse the body of the loop even if the loop is transformed.
    if (node->getBody())
        node->getBody()->traverse(this);

    mCurrentLoopDepth--;
}

bool SimplifyLoopConditionsTraverser::visitBranch(Visit visit, TIntermBranch *node)
{
    if (node->getFlowOp() == EOpContinue || node->getFlowOp() == EOpBreak)
    {
        if (mBranchFixupState.loopDepth != mCurrentLoopDepth)
        {
            return true;
        }
        if (!mBranchFixupState.expr && node->getFlowOp() == EOpBreak)
        {
            return true;
        }

        TIntermBlock *block            = new TIntermBlock();
        TIntermSequence *blockSequence = block->getSequence();

        if (mBranchFixupState.expr)
        {
            blockSequence->push_back(mBranchFixupState.expr->deepCopy());
        }
        if (node->getFlowOp() == EOpContinue)
        {
            blockSequence->push_back(CreateTempAssignmentNode(mBranchFixupState.condVar,
                                                              mBranchFixupState.cond->deepCopy()));
        }
        blockSequence->push_back(node);

        queueReplacement(block, OriginalNode::IS_DROPPED);
    }

    return true;
}

void SimplifyLoopConditionsTraverser::fixupLoopBranches(TIntermBlock *body,
                                                        TVariable *condVar,
                                                        TIntermTyped *cond,
                                                        TIntermTyped *expr)
{
    BranchFixupState prevState = mBranchFixupState;
    {
        mBranchFixupState = {mCurrentLoopDepth, condVar, cond, expr};
        body->traverse(this);
    }
    mBranchFixupState = prevState;
}

}  // namespace

bool SimplifyLoopConditions(TCompiler *compiler,
                            TIntermNode *root,
                            unsigned int conditionsToSimplifyMask,
                            TSymbolTable *symbolTable)
{
    SimplifyLoopConditionsTraverser traverser(conditionsToSimplifyMask, symbolTable);
    root->traverse(&traverser);
    return traverser.updateTree(compiler, root);
}

}  // namespace sh
