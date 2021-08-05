//
// Copyright 2021 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//
// PropagatePrecision.cpp: Propagates precision to AST nodes.
//

#include "compiler/translator/tree_util/PropagatePrecision.h"

#include "common/hash_utils.h"
#include "common/utilities.h"
#include "compiler/translator/Compiler.h"
#include "compiler/translator/IntermNode.h"
#include "compiler/translator/Symbol.h"
#include "compiler/translator/tree_util/IntermTraverse.h"

namespace sh
{

namespace
{

// A traverser that determines the precision of every node based on its operands following GLSL
// rules.  Every node is always post-visited, so the precision of the children would always be
// known.
class PropagatePrecisionTraverser : public TIntermTraverser
{
  public:
    PropagatePrecisionTraverser() : TIntermTraverser(false, false, true) {}

    bool visitSwizzle(Visit visit, TIntermSwizzle *node) override
    {
        visitTyped(node);
        return true;
    }

    bool visitUnary(Visit visit, TIntermUnary *node) override
    {
        visitTyped(node);
        return true;
    }

    bool visitBinary(Visit visit, TIntermBinary *node) override
    {
        visitTyped(node);
        return true;
    }

    bool visitTernary(Visit visit, TIntermTernary *node) override
    {
        visitTyped(node);
        return true;
    }

    bool visitAggregate(Visit visit, TIntermAggregate *node) override
    {
        visitTyped(node);
        return true;
    }

  private:
    void visitTyped(TIntermTyped *node) { node->propagatePrecision(node->derivePrecision()); }
};
}  // anonymous namespace

void PropagatePrecision(TIntermNode *root)
{
    PropagatePrecisionTraverser propagator;
    root->traverse(&propagator);
}

}  // namespace sh
