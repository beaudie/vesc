//
// Copyright (c) 2017 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//
// ReplaceSwitchLastStatementNoOp.h: In case the last statement of a switch statement is a certain
// type of a no-op statement, replace it with another statement that won't be pruned from the AST
// and that platform compilers will accept.

#include "compiler/translator/ReplaceSwitchLastCaseNoOp.h"

#include "compiler/translator/IntermNode.h"
#include "compiler/translator/IntermNode_util.h"
#include "compiler/translator/SymbolTable.h"

namespace sh
{

namespace
{

bool AreEmptyBlocksOrEmptyDeclarations(TIntermSequence *statements, size_t i);

bool IsEmptyBlockOrEmptyDeclaration(TIntermNode *node)
{
    TIntermBlock *asBlock = node->getAsBlock();
    if (asBlock)
    {
        if (asBlock->getSequence()->empty())
        {
            return true;
        }
        return AreEmptyBlocksOrEmptyDeclarations(asBlock->getSequence(), 0u);
    }
    TIntermDeclaration *asDeclaration = node->getAsDeclarationNode();
    if (asDeclaration)
    {
        if (asDeclaration->getSequence()->empty())
        {
            return true;
        }
    }
    return false;
}

// Return true if all statements in "statements" starting from index i consist only of empty blocks
// and empty declarations.
bool AreEmptyBlocksOrEmptyDeclarations(TIntermSequence *statements, size_t i)
{
    for (; i < statements->size(); ++i)
    {
        if (!IsEmptyBlockOrEmptyDeclaration(statements->at(i)))
        {
            return false;
        }
    }
    return true;
}

}  // anonymous namespace

void ReplaceSwitchLastCaseNoOp(TIntermBlock *statementList, TSymbolTable *symbolTable)
{
    TIntermSequence *statements = statementList->getSequence();

    if (statements->empty())
    {
        return;
    }

    size_t i = statements->size();
    while (i > 0u && !(*statements)[i - 1]->getAsCaseNode())
    {
        --i;
    }
    // Now i is the index of the first statement following the last label inside the switch
    // statement.
    ASSERT(i > 0u);

    if (AreEmptyBlocksOrEmptyDeclarations(statements, i))
    {
        statements->erase(statements->begin() + i, statements->end());
        TSymbolUniqueId tempId(symbolTable);
        statements->push_back(
            CreateTempInitDeclarationNode(tempId, CreateZeroNode(TType(EbtBool)), EvqTemporary));
    }
}

}  // namespace sh
