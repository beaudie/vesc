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
class RewriteSamplerVideoWEBGLAs2DTraverser : public TIntermTraverser
{
  public:
    RewriteSamplerVideoWEBGLAs2DTraverser(TSymbolTable *symbolTable)
        : TIntermTraverser(true, true, true, symbolTable)
    {}

    bool visitDeclaration(Visit visit, TIntermDeclaration *node) override
    {
        if (visit != PreVisit)
        {
            return true;
        }

        const TIntermSequence &sequence = *(node->getSequence());

        TIntermTyped *variable   = sequence.front()->getAsTyped();
        const TType &type        = variable->getType();
        bool isSamplerVideoWEBGL = type.getQualifier() == EvqUniform && type.isSamplerVideoWEBGL();

        if (isSamplerVideoWEBGL)
        {
            // Samplers cannot have initializers, so the declaration must necessarily be a symbol.
            TIntermSymbol *samplerVariable = variable->getAsSymbolNode();
            ASSERT(samplerVariable != nullptr);

            declareSampler2D(&samplerVariable->variable(), node);
            return false;
        }

        return true;
    }

    void visitFunctionPrototype(TIntermFunctionPrototype *node) override
    {
        const TFunction *function = node->getFunction();
        // Go over the parameters and replace the samplerCube arguments with a sampler2DArray.
        mRetyper.visitFunctionPrototype();
        for (size_t paramIndex = 0; paramIndex < function->getParamCount(); ++paramIndex)
        {
            const TVariable *param = function->getParam(paramIndex);
            TVariable *replacement = convertFunctionParameter(node, param);
            if (replacement)
            {
                mRetyper.replaceFunctionParam(param, replacement);
            }
        }

        TIntermFunctionPrototype *replacementPrototype =
            mRetyper.convertFunctionPrototype(mSymbolTable, function);
        if (replacementPrototype)
        {
            queueReplacement(replacementPrototype, OriginalNode::IS_DROPPED);
        }
    }

  private:
    void declareSampler2D(const TVariable *samplerVideoWEBGLVar, TIntermDeclaration *node)
    {
        TType *newType = new TType(samplerVideoWEBGLVar->getType());
        newType->setBasicType(EbtSampler2D);

        TVariable *sampler2DVar = new TVariable(mSymbolTable, samplerVideoWEBGLVar->name(), newType,
                                                SymbolType::UserDefined);

        TIntermDeclaration *sampler2DDecl = new TIntermDeclaration();
        sampler2DDecl->appendDeclarator(new TIntermSymbol(sampler2DVar));

        TIntermSequence replacement;
        replacement.push_back(sampler2DDecl);
        mMultiReplacements.emplace_back(getParentNode()->getAsBlock(), node, replacement);

        // Remember the sampler2DArray variable.
        mRetyper.replaceGlobalVariable(samplerVideoWEBGLVar, sampler2DVar);
    }

    TVariable *convertFunctionParameter(TIntermNode *parent, const TVariable *param)
    {
        if (!param->getType().isSamplerVideoWEBGL())
        {
            return nullptr;
        }

        TType *newType = new TType(param->getType());
        newType->setBasicType(EbtSampler2D);

        TVariable *replacementVar =
            new TVariable(mSymbolTable, param->name(), newType, SymbolType::UserDefined);

        return replacementVar;
    }

    RetypeOpaqueVariablesHelper mRetyper;
};

}  // anonymous namespace

bool RewriteSamplerVideoWEBGLAs2D(TCompiler *compiler,
                                  TIntermBlock *root,
                                  TSymbolTable *symbolTable)
{
    RewriteSamplerVideoWEBGLAs2DTraverser traverser(symbolTable);
    root->traverse(&traverser);
    return traverser.updateTree(compiler, root);
}

}  // namespace sh
