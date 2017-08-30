//
// Copyright (c) 2002-2017 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//
// The SetGeometryShaderInputArraySize function sets the array size of all unsized geometry shader
// inputs for further processing.

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

    TIntermTyped &typedNode = *(sequence.front()->getAsTyped());
    TQualifier qualifier    = typedNode.getQualifier();

    if (qualifier == EvqGeometryIn || qualifier == EvqFlatIn || qualifier == EvqSmoothIn ||
        qualifier == EvqCentroidIn)
    {
        for (TIntermNode *variableNode : sequence)
        {
            TIntermSymbol &variable = *variableNode->getAsSymbolNode();
            variable.getTypePointer()->setArraySize(0, mArraySize);
        }
    }

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