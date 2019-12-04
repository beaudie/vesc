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
#include "compiler/translator/util.h"

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

RemoveStaticallyUnusedVariablesTraverser::RemoveStaticallyUnusedVariablesTraverser(
    TSymbolTable *symbolTable)
    : TIntermTraverser(true, false, false, symbolTable)
{}

bool RemoveStaticallyUnusedVariablesTraverser::visitDeclaration(Visit visit,
                                                                TIntermDeclaration *node)
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
    if (mSymbolTable->isStaticallyUsed(variable))
    {
        return false;
    }

    // Don't remove struct definitions.
    const TType &type = declarator->getType();
    if (type.isStructSpecifier())
    {
        return false;
    }

    // Don't remove varyings.  Imagine a situation where the VS doesn't write to a varying
    // but the FS reads from it.  This is allowed, though the value of the varying is undefined.
    // If the varying is removed here, the situation is changed to VS not declaring the varying,
    // but the FS reading from it, which is not allowed.
    if (IsVarying(type.getQualifier()))
    {
        return false;
    }

    // If a uniform or storage buffer, the declaration may be without a variable name, in which
    // case we need to look at the fields of buffer to determine if any of them are statically
    // used.
    bool isStaticallyUsed = false;
    if (type.isInterfaceBlock() && variable.symbolType() == SymbolType::Empty)
    {
        const TInterfaceBlock *block = type.getInterfaceBlock();
        for (const TField *field : block->fields())
        {
            // Individual fields are present in the symbol table as variables and Static use of
            // them is recorded.
            const TSymbol *fieldSymbol = mSymbolTable->findGlobal(field->name());
            ASSERT(fieldSymbol && fieldSymbol->isVariable());
            if (mSymbolTable->isStaticallyUsed(*static_cast<const TVariable *>(fieldSymbol)))
            {
                isStaticallyUsed = true;
                break;
            }
        }
    }

    if (!isStaticallyUsed)
    {
        TIntermSequence emptySequence;
        mMultiReplacements.emplace_back(getParentNode()->getAsBlock(), node, emptySequence);
    }

    return false;
}

}  // namespace

bool RemoveStaticallyUnusedVariables(TCompiler *compiler,
                                     TIntermBlock *root,
                                     TSymbolTable *symbolTable)
{
    RemoveStaticallyUnusedVariablesTraverser traverser(symbolTable);
    root->traverse(&traverser);
    return traverser.updateTree(compiler, root);
}

}  // namespace sh
