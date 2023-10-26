//
// Copyright 2022 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//
// ReswizzleYUVOps: Adjusts swizzles for YUV channel order difference between
//   GLES and Vulkan
//
//

#include "compiler/translator/tree_ops/spirv/EmulateYUVBuiltIns.h"

#include "compiler/translator/StaticType.h"
#include "compiler/translator/SymbolTable.h"
#include "compiler/translator/tree_util/IntermNode_util.h"
#include "compiler/translator/tree_util/IntermTraverse.h"
#include "compiler/translator/tree_util/RunAtTheEndOfShader.h"

namespace sh
{
namespace
{
// A traverser that adjusts channel order for various yuv ops.
class ReswizzleYUVOpsTraverser : public TIntermTraverser
{
  public:
    ReswizzleYUVOpsTraverser(TSymbolTable *symbolTable)
        : TIntermTraverser(true, false, false, symbolTable)
    {}

    bool visitAggregate(Visit visit, TIntermAggregate *node) override;
    bool visitDeclaration(Visit visit, TIntermDeclaration *node) override;

    bool adjustOutput(TCompiler *compiler, TIntermBlock *root);

  private:
    TIntermSymbol *mYuvOutput = nullptr;
};

bool ReswizzleYUVOpsTraverser::visitAggregate(Visit visit, TIntermAggregate *node)
{
    if (visit != Visit::PreVisit)
    {
        return true;
    }

    if (!BuiltInGroup::IsBuiltIn(node->getOp()))
    {
        return true;
    }

    TOperator op = node->getFunction()->getBuiltInOp();
    if (op != EOpTexture && op != EOpTextureProj && op != EOpTexelFetch)
    {
        return true;
    }

    TIntermSequence *arguments = node->getSequence();
    TType const &samplerType   = (*arguments)[0]->getAsTyped()->getType();
    if (samplerType.getBasicType() != EbtSamplerExternal2DY2YEXT)
    {
        return true;
    }

    // texture(...).gbra
    TIntermTyped *replacement = new TIntermSwizzle(node, {1, 2, 0, 3});

    if (replacement != nullptr)
    {
        queueReplacement(replacement, OriginalNode::BECOMES_CHILD);
        return false;
    }

    return true;
}

bool ReswizzleYUVOpsTraverser::visitDeclaration(Visit visit, TIntermDeclaration *node)
{
    const TIntermSequence &sequence = *node->getSequence();
    ASSERT(sequence.size() == 1);

    TIntermSymbol *symbol = sequence.front()->getAsSymbolNode();
    if (symbol == nullptr)
    {
        return true;
    }

    if (symbol->getQualifier() != EvqFragmentOut)
    {
        return true;
    }

    if (!symbol->getType().getLayoutQualifier().yuv)
    {
        return true;
    }

    mYuvOutput = symbol;
    return true;
}

bool ReswizzleYUVOpsTraverser::adjustOutput(TCompiler *compiler, TIntermBlock *root)
{
    if (mYuvOutput == nullptr)
    {
        // we never saw an output with layout(yuv) so there's nothing to do.
        return true;
    }

    // TODO: consider rewriting the existing usages rather than just emitting
    // an assignment, to be cleaner.

    TIntermBlock *block = new TIntermBlock;

    // output = output.brga
    TVector<int> swizzle = { 2, 0, 1, 3 };
    const int size = mYuvOutput->getType().getNominalSize();
    if (size < 4)
    {
        swizzle.resize(size);
    }

    TIntermTyped *assignment = new TIntermBinary(EOpAssign, mYuvOutput->deepCopy(),
            new TIntermSwizzle(mYuvOutput->deepCopy(), swizzle));
    block->appendStatement(assignment);

    return RunAtTheEndOfShader(compiler, root, block, mSymbolTable);
}

}  // anonymous namespace

bool ReswizzleYUVOps(TCompiler *compiler, TIntermBlock *root, TSymbolTable *symbolTable)
{
    ReswizzleYUVOpsTraverser traverser(symbolTable);
    root->traverse(&traverser);
    if (!traverser.updateTree(compiler, root))
    {
        return false;
    }
    return traverser.adjustOutput(compiler, root);
}
}  // namespace sh
