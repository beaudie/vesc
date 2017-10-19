//
// Copyright (c) 2017 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//
// PrunePureLiteralStatements.cpp: Removes statements that are literals and nothing else.
// Note that this may clear statements from after the last case label of a switch
// statement.

#include "compiler/translator/PrunePureLiteralStatements.h"

#include "compiler/translator/IntermTraverse.h"

namespace sh
{

namespace
{

class PrunePureLiteralStatementsTraverser : public TIntermTraverser
{
  public:
    PrunePureLiteralStatementsTraverser() : TIntermTraverser(true, false, false) {}

    bool visitBlock(Visit visit, TIntermBlock *node) override
    {
        TIntermSequence *statements = node->getSequence();

        for (TIntermNode *statement : *statements)
        {
            if (statement->getAsConstantUnion() != nullptr)
            {
                TIntermSequence emptyReplacement;
                mMultiReplacements.push_back(
                    NodeReplaceWithMultipleEntry(node, statement, emptyReplacement));
            }
        }

        return true;
    }

    bool visitLoop(Visit visit, TIntermLoop *loop) override
    {
        TIntermTyped *expr = loop->getExpression();
        if (expr != nullptr && expr->getAsConstantUnion() != nullptr)
        {
            loop->setExpression(nullptr);
        }

        return true;
    }
};

}  // namespace

void PrunePureLiteralStatements(TIntermNode *root)
{
    PrunePureLiteralStatementsTraverser prune;
    root->traverse(&prune);
    prune.updateTree();
}

}  // namespace sh
