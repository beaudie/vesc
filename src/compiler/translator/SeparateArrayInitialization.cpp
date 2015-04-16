//
// Copyright (c) 2002-2015 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//

#include "compiler/translator/SeparateArrayInitialization.h"

SeparateArrayInitialization::SeparateArrayInitialization()
    : TIntermTraverser(true, false, false)
{
}

bool SeparateArrayInitialization::visitAggregate(Visit, TIntermAggregate *node)
{
    if (node->getOp() == EOpDeclaration)
    {
        // We rely on that array declarations have been isolated to single declarations here.
        TIntermSequence *sequence = node->getSequence();
        TIntermBinary *initNode = sequence->back()->getAsBinaryNode();
        if (initNode != nullptr && initNode->getOp() == EOpInitialize)
        {
            TIntermTyped *initializer = initNode->getRight();
            if (initializer->isArray())
            {
                ASSERT(sequence->size() == 1);
                TIntermTyped *symbol = initNode->getLeft();
                TIntermAggregate *parentAgg = getParentNode()->getAsAggregate();
                ASSERT(parentAgg != nullptr);

                TIntermSequence replacements;

                TIntermAggregate *replacementDeclaration = new TIntermAggregate;
                replacementDeclaration->setOp(EOpDeclaration);
                replacementDeclaration->getSequence()->push_back(symbol);
                replacementDeclaration->setLine(symbol->getLine());
                replacements.push_back(replacementDeclaration);

                TIntermBinary *replacementAssignment = new TIntermBinary(EOpAssign);
                replacementAssignment->setLeft(symbol);
                replacementAssignment->setRight(initializer);
                replacementAssignment->setType(initializer->getType());
                replacementAssignment->setLine(symbol->getLine());
                replacements.push_back(replacementAssignment);

                mMultiReplacements.push_back(NodeReplaceWithMultipleEntry(parentAgg, node, replacements));
            }
        }
        return false;
    }
    return true;
}
