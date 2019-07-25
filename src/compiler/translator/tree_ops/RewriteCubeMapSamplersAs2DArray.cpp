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
        : TIntermTraverser(true, false, true, symbolTable), mCubeXYZToArrayLUV(nullptr)
    {}

    bool visitDeclaration(Visit visit, TIntermDeclaration *node) override
    {
        ASSERT(visit == PreVisit);

        // TODO: if this works, change RewriteAtomicCountersTraverser to use a similar method of
        // doing the atomic counter declaration right here, instead of keeping variables until
        // visitSymbol.
        const TIntermSequence &sequence = *(node->getSequence());

        TIntermTyped *variable = sequence.front()->getAsTyped();
        const TType &type      = variable->getType();
        bool isSamplerCube     = type.getQualifier() == EvqUniform && type.isSamplerCube();

        if (isSamplerCube)
        {
            // Samplers cannot have initializers, so the declaration must necessarily be a symbol.
            TIntermSymbol *samplerVariable = variable->getAsSymbolNode();
            ASSERT(samplerVariable != nullptr);

            declareSampler2DArray(samplerVariable, node);
        }

        return false;
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
            mRetyper.convertFunctionPrototype(function);
        if (replacementPrototype)
        {
            queueReplacement(replacementPrototype, OriginalNode::IS_DROPPED);
        }
    }

    bool visitAggregate(Visit visit, TIntermAggregate *node) override
    {
        if (visit == PreVisit)
        {
            mRetyper.preVisitAggregate();
            return true;
        }

        ASSERT(visit == PostVisit);

        if (node->getOp() == EOpCallBuiltInFunction)
        {
            convertBuiltinFunction(node);
        }
        else if (node->getOp() == EOpCallFunctionInAST)
        {
            TIntermAggregate *substituteCall = mRetyper.convertASTFunction(node);
            if (replacement)
            {
                queueReplacement(substituteCall, OriginalNode::IS_DROPPED);
            }
        }

        return true;
    }

    void visitSymbol(TIntermSymbol *symbol) override
    {
        if (!symbol->getType().isSamplerCube())
        {
            return;
        }

        const TVariable *samplerCubeVar = &symbol->variable();

        TIntermTyped *sampler2DArrayVar =
            new TIntermSymbol(mRetyper.getVariableReplacement(samplerCubeVar));
        ASSERT(sampler2DArrayVar != nullptr);

        TIntermNode *argument = symbol;

        // We need to replace the whole function call argument with the symbol replaced.  The
        // argument can either be the sampler (array) itself, or a subscript into a sampler array.
        TIntermBinary *arrayExpression = getParentNode()->getAsBinaryNode();
        if (arrayExpression)
        {
            ASSERT(arrayExpression->getOp() == EOpIndexDirect ||
                   arrayExpression->getOp() == EOpIndexIndirect);

            argument = arrayExpression;

            sampler2DArrayVar = new TIntermBinary(arrayExpression->getOp(), sampler2DArrayVar,
                                                  arrayExpression->getRight()->deepCopy())
        }

        mRetyper.replaceFunctionCallArg(argument, sampler2DArrayVar);
    }

    // https://en.wikipedia.org/wiki/Cube_mapping#Memory_addressing

    // TODO: functions to convert: textureCube (with or without bias), textureCubeLod, and from
    // EXT_gpu_shader4, textureCubeGrad.
  private:
    void declareSampler2DArray(const TVariable *samplerCubeVar, TIntermDeclaration *node)
    {
        TIntermDeclaration *coordTranslationFunctionDecl = nullptr;
        if (mCubeXYZToArrayLUV == nullptr)
        {
            // If not done yet, declare the function that transforms cube map texture sampling
            // coordinates to face index and uv coordinates.
            coordTranslationFunctionDecl = declareCoordTranslationFunction();
            // TODO: create dPdx/dPdy transformation functions as well. Possibly one function that
            // calculates both.
        }

        TType *newType = new TType(samplerCubeVar->getType());
        newType->setBasicType(EbtSampler2DArray);

        TVariable *sampler2DArrayVar =
            new TVariable(mSymbolTable, samplerCubeVar->name(), newType, SymbolType::UserDefined);

        TIntermDeclaration *sampler2DArrayDecl = new TIntermDeclaration();
        sampler2DArrayDecl->appendDeclarator(new TIntermSymbol(sampler2DArrayVar));

        TIntermSequence replacement;
        if (coordTranslationFunctionDecl)
        {
            replacement.push_back(coordTranslationFunctionDecl);
        }
        replacement.push_back(sampler2DArrayDecl);
        mMultiReplacements.emplace_back(getParentNode(), node, replacement);

        // Remember the sampler2DArray variable.
        mRetyper.replaceGlobalVariable(samplerCubeVar, sampler2DArrayVar);
    }

    TIntermDeclaration *declareCoordTranslationFunction()
    {
        // TODO: create mCubeXYZToArrayLUV.
    }

    TVariable *convertFunctionParameter(TIntermNode *parent, const TVariable *param)
    {
        if (!param->getType().isSamplerCube())
        {
            return nullptr;
        }

        TType *newType = new TType(param->getType());
        newType->setBasicType(EbtSampler2DArray);

        TVariable *replacementVar =
            new TVariable(mSymbolTable, param->name(), newType, SymbolType::UserDefined);

        return replacementVar;
    }

    void convertBuiltinFunction(TIntermAggregate *node)
    {
        const TFunction *function = node->getFunction();
        if (!function->name().beginsWith("textureCube"))
        {
            return;
        }

        // All textureCube* functions are in the form:
        //
        //     textureCube??(samplerCube, vec3, ??)
        //
        // They should be converted to:
        //
        //     texture??(sampler2DArray, convertCoords(vec3), ??)
        //
        // We assume the target platform supports texture() functions (currently only used in
        // Vulkan).
        //
        // The intrinsics map as follows:
        //
        //     textureCube -> texture
        //     textureCubeLod -> textureLod
        //     textureCubeLodEXT -> textureLod
        //     textureCubeGrad -> textureGrad
        //     textureCubeGradEXT -> textureGrad
        //
        // Note that dPdx and dPdy in textureCubeGrad* are vec3, while the textureGrad equivalent
        // for sampler2DArray is vec2.  The EXT_shader_texture_lod that introduces thid function
        // says:
        //
        // > For the "Grad" functions, dPdx is the explicit derivative of P with respect
        // > to window x, and similarly dPdy with respect to window y. ...  For a cube map texture,
        // > dPdx and dPdy are vec3.
        // >
        // > Let
        // >
        // >     dSdx = dPdx.s;
        // >     dSdy = dPdy.s;
        // >     dTdx = dPdx.t;
        // >     dTdy = dPdy.t;
        // >
        // > and
        // >
        // >             / 0.0;    for two-dimensional texture
        // >     dRdx = (
        // >             \ dPdx.p; for cube map texture
        // >
        // >             / 0.0;    for two-dimensional texture
        // >     dRdy = (
        // >             \ dPdy.p; for cube map texture
        // >
        // > (See equation 3.12a in The OpenGL ES 2.0 Specification.)
        //
        // It's unclear to me what dRdx and dRdy are.  EXT_gpu_shader4 that promotes this function
        // has the following additional information:
        //
        // > For the "Cube" versions, the partial
        // > derivatives ddx and ddy are assumed to be in the coordinate system used
        // > before texture coordinates are projected onto the appropriate cube
        // > face. The partial derivatives of the post-projection texture coordinates,
        // > which are used for level-of-detail and anisotropic filtering
        // > calculations, are derived from coord, ddx and ddy in an
        // > implementation-dependent manner.
        //
        // The calculation of dPdx and dPdy is declared as implementation-dependent, so could come
        // up with any formula. TODO: come up with something, can use face index and drop the
        // irrelevant axis, need to take care of negative growth directions per face as well as the
        // fact that coords are in [-1, 1] but uv is in [0, 1], so derivatives should divide by 2.

        const char *substituteFunctionName = "texture";
        bool isGrad                        = false;
        if (function->name().beginsWith("textureCubeLod"))
        {
            substituteFunctionName = "textureLod";
        }
        else if (function->name().beginsWith("textureCubeGrad"))
        {
            substituteFunctionName = "textureGrad";
            isGrad                 = true;
        }

        TIntermSequence *arguments = node->getSequence();
        ASSERT(arguments->size() >= 2);

        TIntermSequence *substituteArguments = new TIntermSequence;
        // Replace the first argument (samplerCube) with the sampler2DArray.
        substituteArguments->push_back(mRetyper.getFunctionCallArgReplacement((*arguments)[0]));
        // Replace the second argument with the coordination transformation.
        substituteArguments->push_back(createCoordTransformationCall((*arguments)[1]));
        if (isGrad)
        {
            ASSERT(arguments->size() == 4);
            // If grad, convert the last two arguments as well.
            substituteArguments->push_back(createDPdxTransformationCall((*arguments)[2]));
            substituteArguments->push_back(createDPdyTransformationCall((*arguments)[3]));
        }
        else
        {
            // Pass the rest of the parameters as is.
            for (size_t argIndex = 2; argIndex < arguments->size())
            {
                substituteArguments->push_back((*arguments)[argIndex]->deepCopy());
            }
        }

        TIntermTyped *substituteCall = CreateBuiltInFunctionCallNode(
            substituteFunctionName, substituteArguments, *mSymbolTable, 300);

        queueReplacement(substituteCall, OriginalNode::IS_DROPPED);
    }

    RetypeOpaqueVariablesHelper mRetyper;

    // A helper function to convert xyz coordinates passed to a cube map sampling function into the
    // array layer (cube map face) and uv coordinates.
    TFunction *mCubeXYZToArrayUVL;
};

}  // anonymous namespace

void RewriteCubeMapSamplersAs2DArray(TIntermBlock *root, TSymbolTable *symbolTable)
{
    RewriteCubeMapSamplersAs2DArrayTraverser traverser(symbolTable);
    root->traverse(&traverser);
    traverser.updateTree();
}
