//
// Copyright (c) 2002-2017 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//
// The SetGeometryShaderInputArraySize function sets the array size of all unsized geometry shader
// inputs declared before input primitive declaration for further processing.

#include "SetGeometryShaderInputArraySize.h"

#include "compiler/translator/IntermTraverse.h"

namespace sh
{

namespace
{

class SetGeometryShaderInputArraySizeTraverser : private TIntermTraverser
{
  public:
    static void apply(TIntermNode *root, unsigned int arraySize);

  private:
    SetGeometryShaderInputArraySizeTraverser(unsigned int arraySize);
    bool visitDeclaration(Visit, TIntermDeclaration *node) override;
    bool visitFunctionDefinition(Visit visit, TIntermFunctionDefinition *node) override;
    unsigned int mArraySize;
};

void SetGeometryShaderInputArraySizeTraverser::apply(TIntermNode *root, unsigned int arraySize)
{
    SetGeometryShaderInputArraySizeTraverser setGSInputArraySize(arraySize);
    root->traverse(&setGSInputArraySize);
    setGSInputArraySize.updateTree();
}

bool SetGeometryShaderInputArraySizeTraverser::visitDeclaration(Visit, TIntermDeclaration *node)
{
    TIntermSequence &sequence = *(node->getSequence());
    ASSERT(!sequence.empty());

    TIntermSymbol *symbol = sequence.front()->getAsSymbolNode();
    if (symbol == nullptr)
    {
        return false;
    }

    const TQualifier qualifier = symbol->getQualifier();

    if (qualifier == EvqGeometryIn || qualifier == EvqFlatIn || qualifier == EvqSmoothIn ||
        qualifier == EvqCentroidIn)
    {
        symbol->getTypePointer()->setArraySize(0, mArraySize);
    }

    return false;
}

bool SetGeometryShaderInputArraySizeTraverser::visitFunctionDefinition(
    Visit visit,
    TIntermFunctionDefinition *node)
{
    // We stop traversing function definitions because geometry inputs cannot be defined in a
    // function.
    return false;
}

SetGeometryShaderInputArraySizeTraverser::SetGeometryShaderInputArraySizeTraverser(
    unsigned int arraySize)
    : TIntermTraverser(true, false, false), mArraySize(arraySize)
{
}

}  // namespace

void SetGeometryShaderInputArraySize(TIntermBlock *root, unsigned int arraySize)
{
    ASSERT(arraySize > 0u);
    SetGeometryShaderInputArraySizeTraverser::apply(root, arraySize);
}

}  // namespace sh