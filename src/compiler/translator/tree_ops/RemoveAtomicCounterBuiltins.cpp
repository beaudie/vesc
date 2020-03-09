//
// Copyright 2020 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//
// RemoveAtomicCounterBuiltins: Remove atomic counter builtins.
//

#include "compiler/translator/tree_ops/RemoveAtomicCounterBuiltins.h"

#include "compiler/translator/Compiler.h"
#include "compiler/translator/tree_util/IntermTraverse.h"

namespace sh
{
namespace
{

// Traverser that removes all GLSL built-ins that use AtomicCounters
// Only called when the builtins are in use, but no atomic counters have been declared
class RemoveAtomicCounterBuiltinsTraverser : public TIntermTraverser
{
  public:
    RemoveAtomicCounterBuiltinsTraverser() : TIntermTraverser(true, false, false) {}

    bool visitDeclaration(Visit visit, TIntermDeclaration *node) override
    {
        if (visit != PreVisit)
        {
            return false;
        }

        if (mFoundAtomicCounterDeclaration)
        {
            return false;
        }

        // TODO (anglebug.com/XXXX): Is there a better/faster way to perform this check?
        // We'd rather not pay the cost for something that just results in an assert.
        const TIntermSequence &sequence = *(node->getSequence());
        TIntermTyped *variable          = sequence.front()->getAsTyped();
        const TType &type               = variable->getType();

        mFoundAtomicCounterDeclaration =
            type.getQualifier() == EvqUniform && type.isAtomicCounter();

        return false;
    }

    bool visitAggregate(Visit visit, TIntermAggregate *node) override
    {
        if (node->getOp() == EOpMemoryBarrierAtomicCounter)
        {
            // Vulkan does not support atomic counters, so if this builtin finds its way here,
            // we need to remove it.
            TIntermSequence emptySequence;
            mMultiReplacements.emplace_back(getParentNode()->getAsBlock(), node, emptySequence);
            return true;
        }

        if (node->getOp() == EOpCallBuiltInFunction &&
            node->getFunction()->isAtomicCounterFunction())
        {
            // TODO (anglebug.com/XXXX): If these sneak through, come back and implement.
            // - atomicCounterIncrement
            // - atomicCounterDecrement
            // - atomicCounter
            UNIMPLEMENTED();
        }

        return false;
    }

    bool foundAtomicCounterDeclaration() const { return mFoundAtomicCounterDeclaration; }

  private:
    bool mFoundAtomicCounterDeclaration;
};

}  // anonymous namespace

bool RemoveAtomicCounterBuiltins(TCompiler *compiler, TIntermBlock *root)
{
    RemoveAtomicCounterBuiltinsTraverser traverser;
    root->traverse(&traverser);
    ASSERT(!traverser.foundAtomicCounterDeclaration());
    if (!traverser.updateTree(compiler, root))
    {
        return false;
    }

    return compiler->validateAST(root);
}
}  // namespace sh
