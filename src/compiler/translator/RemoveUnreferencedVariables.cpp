//
// Copyright (c) 2017 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//
// RemoveUnreferencedVariables.cpp:
//  Drop variables that are declared but never referenced in the AST. This avoids adding unnecessary
//  initialization code for them.
//

#include "compiler/translator/RemoveUnreferencedVariables.h"

#include "compiler/translator/IntermTraverse.h"

namespace sh
{

namespace
{

class CollectVariableRefCountsTraverser : public TIntermTraverser
{
  public:
    CollectVariableRefCountsTraverser();

    typedef std::unordered_map<int, unsigned int> RefCountMap;
    RefCountMap &getSymbolIdRefCounts() { return mSymbolIdRefCounts; }

    void visitSymbol(TIntermSymbol *node) override;

  private:
    RefCountMap mSymbolIdRefCounts;
};

CollectVariableRefCountsTraverser::CollectVariableRefCountsTraverser()
    : TIntermTraverser(true, false, false)
{
}

void CollectVariableRefCountsTraverser::visitSymbol(TIntermSymbol *node)
{
    auto iter = mSymbolIdRefCounts.find(node->getId());
    if (iter == mSymbolIdRefCounts.end())
    {
        mSymbolIdRefCounts[node->getId()] = 1u;
        return;
    }
    ++(iter->second);
}

// Traverser that converts all pow operations simultaneously.
class RemoveUnreferencedVariablesTraverser : public TIntermTraverser
{
  public:
    RemoveUnreferencedVariablesTraverser(
        CollectVariableRefCountsTraverser::RefCountMap *symbolIdRefCounts);

    bool visitDeclaration(Visit visit, TIntermDeclaration *node) override;
    void visitSymbol(TIntermSymbol *node) override;

    void traverseBlock(TIntermBlock *block) override;

  private:
    CollectVariableRefCountsTraverser::RefCountMap *mSymbolIdRefCounts;
    bool mRemoveReferences;
};

RemoveUnreferencedVariablesTraverser::RemoveUnreferencedVariablesTraverser(
    CollectVariableRefCountsTraverser::RefCountMap *symbolIdRefCounts)
    : TIntermTraverser(true, false, true),
      mSymbolIdRefCounts(symbolIdRefCounts),
      mRemoveReferences(false)
{
}

bool RemoveUnreferencedVariablesTraverser::visitDeclaration(Visit visit, TIntermDeclaration *node)
{
    if (visit == PreVisit)
    {
        // SeparateDeclarations should have already been run.
        ASSERT(node->getSequence()->size() == 1u);

        TIntermTyped *declarator = node->getSequence()->back()->getAsTyped();
        ASSERT(declarator);

        // We can only remove variables that are not a part of the shader interface.
        TQualifier qualifier = declarator->getQualifier();
        if (qualifier != EvqTemporary && qualifier != EvqGlobal)
        {
            return true;
        }

        // We don't count references to struct types.
        if (declarator->getType().isStructSpecifier())
        {
            return true;
        }

        TIntermSymbol *symbolNode = declarator->getAsSymbolNode();
        bool isUnreferencedSymbol = symbolNode && (*mSymbolIdRefCounts)[symbolNode->getId()] == 1u;
        TIntermBinary *initNode   = declarator->getAsBinaryNode();
        bool isInitNodeWithoutSideEffects = initNode && !initNode->getRight()->hasSideEffects();
        bool isUnreferencedInitNodeWithoutSideEffects = false;
        if (isInitNodeWithoutSideEffects)
        {
            int symbolId = initNode->getLeft()->getAsSymbolNode()->getId();
            isUnreferencedInitNodeWithoutSideEffects = (*mSymbolIdRefCounts)[symbolId] == 1u;
        }
        if (isUnreferencedSymbol || isUnreferencedInitNodeWithoutSideEffects)
        {
            TIntermSequence emptyReplacement;
            mMultiReplacements.push_back(NodeReplaceWithMultipleEntry(getParentNode()->getAsBlock(),
                                                                      node, emptyReplacement));
            mRemoveReferences = true;
        }
        return true;
    }
    ASSERT(visit == PostVisit);
    mRemoveReferences = false;
    return true;
}

void RemoveUnreferencedVariablesTraverser::visitSymbol(TIntermSymbol *node)
{
    if (mRemoveReferences)
    {
        ASSERT(mSymbolIdRefCounts->find(node->getId()) != mSymbolIdRefCounts->end());
        --(*mSymbolIdRefCounts)[node->getId()];
    }
}

// Traverse a block node in reverse order. This way reference counts can be decremented when
// removing initializers, and variables that become unused when initializers are removed can be
// removed on the same traversal.
// TODO: Maybe this could be done in a cleaner way? Put it as an option for TIntermTraverser?
void RemoveUnreferencedVariablesTraverser::traverseBlock(TIntermBlock *node)
{
    ScopedNodeInTraversalPath addToPath(this, node);
    pushParentBlock(node);

    bool visit = true;

    TIntermSequence *sequence = node->getSequence();

    if (preVisit)
        visit = visitBlock(PreVisit, node);

    if (visit)
    {
        for (auto iter = sequence->rbegin(); iter != sequence->rend(); ++iter)
        {
            (*iter)->traverse(this);
            if (visit && inVisit)
            {
                if ((iter + 1) != sequence->rend())
                    visit = visitBlock(InVisit, node);
            }
        }
    }

    if (visit && postVisit)
        visitBlock(PostVisit, node);

    popParentBlock();
}

}  // namespace

void RemoveUnreferencedVariables(TIntermBlock *root)
{
    CollectVariableRefCountsTraverser collector;
    root->traverse(&collector);
    RemoveUnreferencedVariablesTraverser traverser(&collector.getSymbolIdRefCounts());
    root->traverse(&traverser);
    traverser.updateTree();
}

}  // namespace sh
