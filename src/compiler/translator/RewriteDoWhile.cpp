//
// Copyright (c) 2015 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//

// RewriteDoWhile.cpp: rewrites do-while loops using another equivalent
// construct.

#include "compiler/translator/RewriteDoWhile.h"

#include "compiler/translator/IntermNode.h"

namespace
{

class DoWhileRewriter : public TIntermTraverser
{
  public:
    DoWhileRewriter() : TIntermTraverser(true, false, false) {}

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

            if (loop == nullptr || loop->getType() != ELoopDoWhile)
            {
                continue;
            }

            TType boolType = TType(EbtBool);

            TIntermAggregate *declaration = nullptr;
            {
                TConstantUnion *falseConstant = new TConstantUnion();
                falseConstant->setBConst(false);
                TIntermTyped *trueValue = new TIntermConstantUnion(falseConstant, boolType);

                declaration = createTempInitDeclaration(trueValue);
            }

            TIntermBinary *assignFalse = nullptr;
            {
                TConstantUnion *trueConstant = new TConstantUnion();
                trueConstant->setBConst(true);
                TIntermTyped *trueValue = new TIntermConstantUnion(trueConstant, boolType);

                assignFalse = createTempAssignment(trueValue);
            }

            TIntermSelection *breakIf = nullptr;
            {
                TIntermBranch *breakStatement = new TIntermBranch(EOpBreak, nullptr);

                TIntermAggregate *breakBlock = new TIntermAggregate(EOpSequence);
                breakBlock->getSequence()->push_back(breakStatement);

                TIntermUnary *negatedCondition = new TIntermUnary(EOpLogicalNot);
                negatedCondition->setOperand(loop->getCondition());

                TIntermSelection *innerIf =
                    new TIntermSelection(negatedCondition, breakBlock, nullptr);

                TIntermAggregate *innerIfBlock = new TIntermAggregate(EOpSequence);
                innerIfBlock->getSequence()->push_back(innerIf);

                breakIf = new TIntermSelection(createTempSymbol(boolType), innerIfBlock, nullptr);
            }

            TIntermLoop *newLoop = nullptr;
            {
                TConstantUnion *trueConstant = new TConstantUnion();
                trueConstant->setBConst(true);
                TIntermTyped *trueValue = new TIntermConstantUnion(trueConstant, boolType);

                TIntermAggregate *body = nullptr;
                if (loop->getBody() != nullptr)
                {
                    body = loop->getBody()->getAsAggregate();
                }
                else
                {
                    body = new TIntermAggregate(EOpSequence);
                }
                auto sequence = body->getSequence();
                sequence->insert(sequence->begin(), assignFalse);
                sequence->insert(sequence->begin(), breakIf);

                newLoop = new TIntermLoop(ELoopWhile, nullptr, trueValue, nullptr, body);
            }

            TIntermSequence replacement;
            replacement.push_back(declaration);
            replacement.push_back(newLoop);

            node->replaceChildNodeWithMultiple(loop, replacement);
            delete loop;

            nextTemporaryIndex();
        }
        return true;
    }
};

}  // anonymous namespace

void RewriteDoWhile(TIntermNode *root, unsigned int *temporaryIndex)
{
    ASSERT(temporaryIndex != 0);

    DoWhileRewriter rewriter;
    rewriter.useTemporaryIndex(temporaryIndex);

    root->traverse(&rewriter);
}
