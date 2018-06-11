//
// Copyright (c) 2015 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//
// FindSymbol.cpp:
//     Utility for finding a symbol node inside an AST tree.

#include "compiler/translator/tree_util/FindSymbolNode.h"

#include "compiler/translator/ImmutableString.h"
#include "compiler/translator/Symbol.h"
#include "compiler/translator/tree_util/IntermTraverse.h"

namespace sh
{

namespace
{

class SymbolFinder : public TIntermTraverser2
{
  public:
    SymbolFinder(const ImmutableString &symbolName)
        : TIntermTraverser2(true, false, false), mSymbolName(symbolName), mNodeFound(nullptr)
    {
    }

    TAction *visitSymbol(Visit visit, TIntermSymbol *node) override
    {
        if (node->variable().symbolType() != SymbolType::Empty && node->getName() == mSymbolName)
        {
            mNodeFound = node;
            return new TAction(Continue::Stop);
        }
        return nullptr;
    }

    bool isFound() const { return mNodeFound != nullptr; }
    const TIntermSymbol *getNode() const { return mNodeFound; }

  private:
    ImmutableString mSymbolName;
    TIntermSymbol *mNodeFound;
};

}  // anonymous namespace

const TIntermSymbol *FindSymbolNode(TIntermNode *root, const ImmutableString &symbolName)
{
    SymbolFinder finder(symbolName);
    finder.traverse(root);
    return finder.getNode();
}

}  // namespace sh
