//
// Copyright (c) 2002-2015 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//

#include "compiler/translator/SeparateDeclarations.h"

SeparateDeclarations::SeparateDeclarations()
    : TIntermTraverser(true, false, false)
{
}

bool SeparateDeclarations::visitAggregate(Visit, TIntermAggregate *node)
{
    if (node->getOp() == EOpDeclaration)
    {
        TIntermSequence *sequence = node->getSequence();
        bool sequenceContainsArrays = false;
        for (size_t ii = 0; ii < sequence->size(); ++ii)
        {
            TIntermTyped *typed = sequence->at(ii)->getAsTyped();
            if (typed != nullptr && typed->isArray())
            {
                sequenceContainsArrays = true;
                break;
            }
        }
        if (sequence->size() > 1 && sequenceContainsArrays)
        {
            TIntermAggregate *parentAgg = getParentNode()->getAsAggregate();
            ASSERT(parentAgg != nullptr);

            TIntermSequence replacementDeclarations;
            for (size_t ii = 0; ii < sequence->size(); ++ii)
            {
                TIntermAggregate *replacementDeclaration = new TIntermAggregate;

                replacementDeclaration->setOp(EOpDeclaration);
                replacementDeclaration->getSequence()->push_back(sequence->at(ii));
                replacementDeclaration->setLine(sequence->at(ii)->getLine());
                replacementDeclarations.push_back(replacementDeclaration);
            }

            mMultiReplacements.push_back(NodeReplaceWithMultipleEntry(parentAgg, node, replacementDeclarations));
        }
        return false;
    }
    return true;
}
