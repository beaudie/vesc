//
// Copyright 2019 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//
// RemoveStaticallyUnusedVariables.h:
//  Drop shader interface variable declarations for those that are statically unused.
//

#include "compiler/translator/tree_ops/RemoveStaticallyUnusedVariables.h"

#include "compiler/translator/SymbolTable.h"
#include "compiler/translator/tree_util/IntermTraverse.h"

namespace sh
{

namespace
{

// Traverser that removes all declarations that correspond to statically unused variables.
class RemoveStaticallyUnusedVariablesTraverser : public TIntermTraverser
{
  public:
    RemoveStaticallyUnusedVariablesTraverser(TSymbolTable *symbolTable);

    bool visitDeclaration(Visit visit, TIntermDeclaration *node) override;
};

RemoveStaticallyUnusedVariablesTraverser::RemoveStaticallyUnusedVariablesTraverser(TSymbolTable *symbolTable)
    : TIntermTraverser(true, false, false, symbolTable)
{}

bool RemoveStaticallyUnusedVariablesTraverser::visitDeclaration(Visit visit, TIntermDeclaration *node)
{
    // SeparateDeclarations should have already been run.
    ASSERT(node->getSequence()->size() == 1u);

    TIntermTyped *declarator = node->getSequence()->front()->getAsTyped();
    ASSERT(declarator);

    TIntermSymbol *asSymbol = declarator->getAsSymbolNode();
    if (!asSymbol)
    {
        return false;
    }

    const TVariable &variable = asSymbol->variable();
    if (!mSymbolTable->isStaticallyUsed(variable))
    {
        TIntermSequence emptySequence;
        mMultiReplacements.emplace_back(getParentNode()->getAsBlock(), node, emptySequence);
    }

    return false;
}

}  // namespace

bool RemoveStaticallyUnusedVariables(TCompiler *compiler, TIntermBlock *root, TSymbolTable *symbolTable)
{
    RemoveStaticallyUnusedVariablesTraverser traverser(symbolTable);
    root->traverse(&traverser);
    return traverser.updateTree(compiler, root);
}

}  // namespace sh
