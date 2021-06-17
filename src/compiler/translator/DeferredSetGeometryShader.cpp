//
// Copyright 2021 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//

// DeferredSetGeometryShader.cpp: Account for the case where array size is set after the
// declaration of all input variables

#include "compiler/translator/DeferredSetGeometryShader.h"

#include "compiler/translator/SymbolTable.h"
#include "compiler/translator/tree_util/IntermTraverse.h"

namespace sh
{
class TIntermTraverser;

namespace
{
class DeferredGSTraverser : public TIntermTraverser
{
  public:
    DeferredGSTraverser(TSymbolTable *symbolTable)
        : TIntermTraverser(true, false, false, symbolTable)
    {}

  private:
    bool visitDeclaration(Visit visit, TIntermDeclaration *node) override;
};

bool DeferredGSTraverser::visitDeclaration(Visit visit, TIntermDeclaration *node)
{
    TIntermSequence *sequence = node->getSequence();

    if (sequence->size() != 1)
    {
        return true;
    }

    TIntermSymbol *symbol = sequence->front()->getAsSymbolNode();
    if (symbol == nullptr)
    {
        return true;
    }

    if (symbol->getQualifier() == EvqGeometryIn && symbol->isArray())
    {
        TType *sizedType = new TType(symbol->getType());
        sizedType->sizeOutermostUnsizedArray(
            mSymbolTable->getGlInVariableWithArraySize()->getType().getOutermostArraySize());

        TVariable *sizedVariable   = new TVariable(mSymbolTable, symbol->getName(), sizedType,
                                                 symbol->variable().symbolType());
        TIntermSymbol *sizedSymbol = new TIntermSymbol(sizedVariable);

        TIntermDeclaration *newDeclaration = new TIntermDeclaration();
        newDeclaration->appendDeclarator(sizedSymbol);
        queueReplacement(newDeclaration, OriginalNode::IS_DROPPED);
    }

    return true;
}
}  // anonymous namespace

ANGLE_NO_DISCARD bool DeferredSetGeometryShaderArraySize(TCompiler *compiler,
                                                         TIntermBlock *root,
                                                         TSymbolTable *symbolTable)
{
    DeferredGSTraverser traverser(symbolTable);
    root->traverse(&traverser);
    if (!traverser.updateTree(compiler, root))
    {
        return false;
    }
    return true;
}
}  // namespace sh
