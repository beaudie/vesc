//
// Copyright (c) 2016 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//
// AddDefaultReturnStatements.cpp: Add default return statements to functions that do not end in a
//                                 return.
//

#include "compiler/translator/AddDefaultReturnStatements.h"

#include "compiler/translator/IntermNode.h"

namespace
{

class AddDefaultReturnStatementsTraverser : private TIntermTraverser
{
  public:
    static void apply(TIntermNode *root)
    {
        AddDefaultReturnStatementsTraverser separateInit;
        root->traverse(&separateInit);
        separateInit.updateTree();
    }

  private:
    AddDefaultReturnStatementsTraverser() : TIntermTraverser(true, false, false) {}

    static bool isFunctionWithoutReturnStatement(TIntermAggregate *node, TType *returnType)
    {
        *returnType = node->getType();
        if (node->getOp() != EOpFunction || node->getType().getBasicType() == EbtVoid)
        {
            return false;
        }

        TIntermAggregate *lastNode = node->getSequence()->back()->getAsAggregate();
        if (lastNode == nullptr)
        {
            return true;
        }

        TIntermBranch *returnNode = lastNode->getSequence()->front()->getAsBranchNode();
        if (returnNode != nullptr && returnNode->getFlowOp() == EOpReturn)
        {
            return false;
        }

        return true;
    }

    bool visitAggregate(Visit, TIntermAggregate *node) override
    {
        TType returnType;
        if (isFunctionWithoutReturnStatement(node, &returnType))
        {
            size_t objectSize             = returnType.getObjectSize();
            TConstantUnion *constantUnion = new TConstantUnion[objectSize];
            for (size_t constantIdx = 0; constantIdx < objectSize; constantIdx++)
            {
                constantUnion[constantIdx].setFConst(0.0f);
            }

            TIntermConstantUnion *intermConstantUnion =
                new TIntermConstantUnion(constantUnion, returnType);

            TIntermBranch *branch = new TIntermBranch(EOpReturn, intermConstantUnion);

            TIntermAggregate *lastNode = node->getSequence()->back()->getAsAggregate();
            lastNode->getSequence()->push_back(branch);

            return false;
        }

        return true;
    }
};
}

namespace sh
{

void AddDefaultReturnStatements(TIntermNode *node)
{
    AddDefaultReturnStatementsTraverser::apply(node);
}

}  // namespace sh
