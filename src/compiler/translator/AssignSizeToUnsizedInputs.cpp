//
// Copyright (c) 2002-2017 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//

#include "compiler/translator/AssignSizeToUnsizedInputs.h"

#include "compiler/translator/BaseTypes.h"
#include "compiler/translator/IntermNode.h"

namespace sh
{

namespace
{

bool IsGeometryInputQualifier(TQualifier qualifier)
{
    return qualifier == EvqGeometryIn || qualifier == EvqSmoothIn || qualifier == EvqCentroidIn ||
           qualifier == EvqFlatIn;
}

bool IsUnsizedGeometryInputDeclaration(TIntermTyped *declaration)
{
    ASSERT(declaration);
    return (declaration->getArraySize() == 0) &&
           (IsGeometryInputQualifier(declaration->getQualifier()));
}

void AssignSizeToUnsizedDeclaration(TIntermDeclaration *declarationBlock, unsigned int arraySize)
{
    ASSERT(declarationBlock);
    for (TIntermNode *declaration : *(declarationBlock->getSequence()))
    {
        TIntermTyped *typedDeclaration = declaration->getAsTyped();
        if (declaration && IsUnsizedGeometryInputDeclaration(typedDeclaration))
        {
            typedDeclaration->getTypePointer()->setArraySize(arraySize);
        }
    }
}

}  // anonymous namespace

void AssignSizeToUnsizedInputs(TIntermNode *root, unsigned int arraySize)
{
    auto *statements = root->getAsBlock();
    for (TIntermNode *statement : *(statements->getSequence()))
    {
        TIntermDeclaration *declarationBlock = statement->getAsDeclarationNode();
        if (declarationBlock)
        {
            AssignSizeToUnsizedDeclaration(declarationBlock, arraySize);
        }
    }
}

}  // namespace sh