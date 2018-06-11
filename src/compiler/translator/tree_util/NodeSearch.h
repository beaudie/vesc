//
// Copyright (c) 2002-2013 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//
// NodeSearch.h: Utilities for searching translator node graphs
//

#ifndef COMPILER_TRANSLATOR_TREEUTIL_NODESEARCH_H_
#define COMPILER_TRANSLATOR_TREEUTIL_NODESEARCH_H_

#include "compiler/translator/tree_util/IntermTraverse.h"

namespace sh
{

template <class Parent>
class NodeSearchTraverser : public TIntermTraverser2
{
  public:
    NodeSearchTraverser() : TIntermTraverser2(true, false, false), mFound(false) {}

    bool found() const { return mFound; }

    static bool search(TIntermNode *node)
    {
        Parent searchTraverser;
        searchTraverser.traverse(node);
        return searchTraverser.found();
    }

  protected:
    bool mFound;
};

class FindDiscard : public NodeSearchTraverser<FindDiscard>
{
  public:
    TAction *visitBranch(Visit visit, TIntermBranch *node) override
    {
        switch (node->getFlowOp())
        {
            case EOpKill:
                mFound = true;
                return new TAction(Continue::Stop);

            default:
                break;
        }

        return nullptr;
    }
};
}

#endif  // COMPILER_TRANSLATOR_TREEUTIL_NODESEARCH_H_
