//
// Copyright 2019 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//
// RewriteCubeMapSamplersAs2DArray: Change samplerCube samplers to sampler2DArray for seamful cube
// map emulation.
//

#include "compiler/translator/tree_ops/RewriteCubeMapSamplersAs2DArray.h"

#include "compiler/translator/ImmutableStringBuilder.h"
#include "compiler/translator/StaticType.h"
#include "compiler/translator/SymbolTable.h"
#include "compiler/translator/tree_util/IntermNode_util.h"
#include "compiler/translator/tree_util/IntermTraverse.h"
#include "compiler/translator/tree_util/ReplaceVariable.h"

namespace sh
{
namespace
{
class RewriteCubeMapSamplersAs2DArrayTraverser : public TIntermTraverser
{
  public:
    RewriteCubeMapSamplersAs2DArrayTraverser(TSymbolTable *symbolTable)
        : TIntermTraverser(true, false, false, symbolTable)
    {}

    bool visitDeclaration(Visit visit, TIntermDeclaration *node) override
    {
        // TODO: if this works, change RewriteAtomicCountersTraverser to use a similar method of
        // doing the atomic counter declaration right here, instead of keeping variables until
        // visitSymbol.
        const TIntermSequence &sequence = *(node->getSequence());

        TIntermTyped *variable = sequence.front()->getAsTyped();
        const TType &type      = variable->getType();
        bool isSamplerCube =
            type.getQualifier() == EvqUniform && type.getBasicType() == EbtSamplerCube;

        if (isSamplerCube)
        {
            // Samplers cannot have initializers, so the declaration must necessarily be a symbol.
            TIntermSymbol *samplerVariable = variable->getAsSymbolNode();
            ASSERT(samplerVariable != nullptr);

            TIntermDeclaration

                TIntermSequence replacement;

            replacement.push_back(offsetDeclaration);

            mMultiReplacements.emplace_back(mCurrentAtomicCounterDeclParent,
                                            mCurrentAtomicCounterDecl, replacement);
        }

        return false;
    }
    // https://en.wikipedia.org/wiki/Cube_mapping#Memory_addressing

    // TODO: functions to convert: textureCube (with or without bias), textureCubeLod, and from
    // EXT_gpu_shader4, textureCubeGrad.
  private:
    // A helper function to convert xyz coordinates passed to a cube map sampling function into the
    // array layer (cube map face) and uv coordinates.
    TFunction *mCubeXYZToArrayLUV.
};

}  // anonymous namespace

void RewriteCubeMapSamplersAs2DArray(TIntermBlock *root, TSymbolTable *symbolTable)
{
    RewriteCubeMapSamplersAs2DArrayTraverser traverser(symbolTable);
    root->traverse(&traverser);
    traverser.updateTree();
}
