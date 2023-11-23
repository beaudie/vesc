//
// Copyright 2023 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "compiler/translator/tree_ops/RewriteTextureCubeGrad.h"

#include "compiler/translator/StaticType.h"
#include "compiler/translator/SymbolTable.h"
#include "compiler/translator/tree_util/FindFunction.h"
#include "compiler/translator/tree_util/IntermNode_util.h"
#include "compiler/translator/tree_util/IntermTraverse.h"

namespace sh
{

namespace
{

constexpr ImmutableString kFunctionAGX("ANGLE_textureGradAGX");

class FindTextureCubeGradTraverser : public TIntermTraverser
{
  public:
    FindTextureCubeGradTraverser(TSymbolTable *symbolTable, int shaderVersion)
        : TIntermTraverser(true, false, false, symbolTable), mShaderVersion(shaderVersion)
    {}

    const TFunction *getReplacementFunction(const TType &textureType, const TType &returnType)
    {
        const TBasicType samplerType = textureType.getBasicType();
        ASSERT(IsSamplerCube(samplerType));
        if (mReplacementFunctions[samplerType] != nullptr)
        {
            return mReplacementFunctions[samplerType]->getFunction();
        }

        // Sampler
        TType *texType = new TType(textureType);
        texType->setQualifier(EvqParamIn);
        const TVariable *texture =
            new TVariable(mSymbolTable, kEmptyImmutableString, texType, SymbolType::AngleInternal);

        // Direction vector
        const TType *directionType =
            samplerType == EbtSamplerCubeShadow
                ? StaticType::GetQualified<EbtFloat, EbpMedium, EvqParamIn, 4>()
                : StaticType::GetQualified<EbtFloat, EbpMedium, EvqParamIn, 3>();
        const TVariable *direction = new TVariable(mSymbolTable, kEmptyImmutableString,
                                                   directionType, SymbolType::AngleInternal);

        // Derivatives
        const TType *dType = StaticType::GetQualified<EbtFloat, EbpMedium, EvqParamIn, 3>();
        const TVariable *dPdx =
            new TVariable(mSymbolTable, kEmptyImmutableString, dType, SymbolType::AngleInternal);
        const TVariable *dPdy =
            new TVariable(mSymbolTable, kEmptyImmutableString, dType, SymbolType::AngleInternal);

        TFunction *function =
            new TFunction(mSymbolTable, kFunctionAGX, SymbolType::AngleInternal, &returnType, true);
        function->addParameter(texture);
        function->addParameter(direction);
        function->addParameter(dPdx);
        function->addParameter(dPdy);

        TIntermBlock *body = new TIntermBlock;

        // Select major axis. Apple GPUs have the following rules:
        // * X wins over Y and Z
        // * Y wins over Z

        // vec3 absDirection = abs(direction.xyz);
        const TVariable *absDirection =
            CreateTempVariable(mSymbolTable, StaticType::GetTemporary<EbtFloat, EbpMedium, 3>());
        body->appendStatement(CreateTempInitDeclarationNode(
            absDirection, CreateBuiltInFunctionCallNode(
                              "abs", {new TIntermSwizzle(new TIntermSymbol(direction), {0, 1, 2})},
                              *mSymbolTable, mShaderVersion)));

        TIntermSwizzle *absDirectionX = new TIntermSwizzle(new TIntermSymbol(absDirection), {0});
        TIntermSwizzle *absDirectionY = new TIntermSwizzle(new TIntermSymbol(absDirection), {1});
        TIntermSwizzle *absDirectionZ = new TIntermSwizzle(new TIntermSymbol(absDirection), {2});

        // bool xMajor = absDirection.x >= max(absDirection.y, absDirection.z);
        const TVariable *xMajor =
            CreateTempVariable(mSymbolTable, StaticType::GetTemporary<EbtBool, EbpUndefined>());
        body->appendStatement(CreateTempInitDeclarationNode(
            xMajor,
            new TIntermBinary(EOpGreaterThanEqual, absDirectionX,
                              CreateBuiltInFunctionCallNode("max", {absDirectionY, absDirectionZ},
                                                            *mSymbolTable, mShaderVersion))));

        // bool yMajor = absDirection.y >= absDirection.z;
        const TVariable *yMajor =
            CreateTempVariable(mSymbolTable, StaticType::GetTemporary<EbtBool, EbpUndefined>());
        body->appendStatement(CreateTempInitDeclarationNode(
            yMajor, new TIntermBinary(EOpGreaterThanEqual, absDirectionY->deepCopy(),
                                      absDirectionZ->deepCopy())));

        // Prepare input vectors
        auto addSwizzleStatement = [&](const TVariable *target, const TVariable *source) {
            TIntermSwizzle *sYZX       = new TIntermSwizzle(new TIntermSymbol(source), {1, 2, 0});
            TIntermSwizzle *sXZY       = new TIntermSwizzle(new TIntermSymbol(source), {0, 2, 1});
            TIntermSwizzle *sXYZ       = new TIntermSwizzle(new TIntermSymbol(source), {0, 1, 2});
            TIntermTernary *secondRule = new TIntermTernary(new TIntermSymbol(yMajor), sXZY, sXYZ);
            body->appendStatement(CreateTempInitDeclarationNode(
                target, new TIntermTernary(new TIntermSymbol(xMajor), sYZX, secondRule)));
        };

        // vec3 faceDirection = xMajor ? direction.yzx : (yMajor ? direction.xzy : direction.xyz);
        const TVariable *faceDirection =
            CreateTempVariable(mSymbolTable, StaticType::GetTemporary<EbtFloat, EbpMedium, 3>());
        addSwizzleStatement(faceDirection, direction);

        // vec3 dQdx = xMajor ? dPdx.yzx : (yMajor ? dPdx.xzy : dPdx);
        const TVariable *dQdx =
            CreateTempVariable(mSymbolTable, StaticType::GetTemporary<EbtFloat, EbpMedium, 3>());
        addSwizzleStatement(dQdx, dPdx);

        // vec3 dQdy = xMajor ? dPdy.yzx : (yMajor ? dPdy.xzy : dPdy);
        const TVariable *dQdy =
            CreateTempVariable(mSymbolTable, StaticType::GetTemporary<EbtFloat, EbpMedium, 3>());
        addSwizzleStatement(dQdy, dPdy);

        // Transform all derivatives
        // vec4 d = vec4(dQdx.xy, dQdy.xy) - (Q.xy / Q.z).xyxy * vec4(dQdx.zz, dQdy.zz);
        TIntermAggregate *packXY = TIntermAggregate::CreateConstructor(
            *StaticType::GetTemporary<EbtFloat, EbpMedium, 4>(),
            {new TIntermSwizzle(new TIntermSymbol(dQdx), {0, 1}),
             new TIntermSwizzle(new TIntermSymbol(dQdy), {0, 1})});
        TIntermAggregate *packZZ = TIntermAggregate::CreateConstructor(
            *StaticType::GetTemporary<EbtFloat, EbpMedium, 4>(),
            {new TIntermSwizzle(new TIntermSymbol(dQdx), {2, 2}),
             new TIntermSwizzle(new TIntermSymbol(dQdy), {2, 2})});
        TIntermSwizzle *division = new TIntermSwizzle(
            new TIntermBinary(EOpDiv, new TIntermSwizzle(new TIntermSymbol(faceDirection), {0, 1}),
                              new TIntermSwizzle(new TIntermSymbol(faceDirection), {2})),
            {0, 1, 0, 1});
        const TVariable *d =
            CreateTempVariable(mSymbolTable, StaticType::GetTemporary<EbtFloat, EbpMedium, 4>());
        body->appendStatement(CreateTempInitDeclarationNode(
            d, new TIntermBinary(EOpSub, packXY, new TIntermBinary(EOpMul, division, packZZ))));

        // Final swizzle to put the transformed values into target components
        // X major: X and Z; Y major: X and Y; Z major: Y and Z
        TIntermTernary *transformedX = new TIntermTernary(
            new TIntermSymbol(xMajor), new TIntermSwizzle(new TIntermSymbol(d), {0, 0, 1}),
            new TIntermSwizzle(new TIntermSymbol(d), {0, 1, 0}));
        TIntermTernary *transformedY = new TIntermTernary(
            new TIntermSymbol(xMajor), new TIntermSwizzle(new TIntermSymbol(d), {2, 2, 3}),
            new TIntermSwizzle(new TIntermSymbol(d), {2, 3, 2}));

        TIntermTyped *nativeCall = CreateBuiltInFunctionCallNode(
            mShaderVersion == 100 ? "textureCubeGradEXT" : "textureGrad",
            {new TIntermSymbol(texture), new TIntermSymbol(direction), transformedX, transformedY},
            *mSymbolTable, mShaderVersion);
        body->appendStatement(new TIntermBranch(EOpReturn, nativeCall));

        mReplacementFunctions[samplerType] =
            new TIntermFunctionDefinition(new TIntermFunctionPrototype(function), body);
        mNewFunctionType = samplerType;
        return function;
    }

    bool visitFunctionDefinition(Visit visit, TIntermFunctionDefinition *node) override
    {
        // Do not traverse the wrapper function
        return node->getFunction()->name() != kFunctionAGX;
    }

    bool visitAggregate(Visit visit, TIntermAggregate *node) override
    {
        if (mFound)
        {
            return false;
        }

        switch (node->getOp())
        {
            case EOpTextureCubeGradEXT:
            case EOpTextureGrad:
                break;
            default:
                return true;
        }

        TIntermSequence *parameters = node->getSequence();
        TIntermTyped *tex           = parameters->at(0)->getAsTyped();
        if (!IsSamplerCube(tex->getBasicType()))
        {
            return true;
        }

        queueReplacement(TIntermAggregate::CreateFunctionCall(
                             *getReplacementFunction(tex->getType(), node->getType()), parameters),
                         OriginalNode::IS_DROPPED);
        mFound = true;
        return false;
    }

    void nextIteration()
    {
        mNewFunctionType = EbtVoid;
        mFound           = false;
    }

    TIntermFunctionDefinition *getNewReplacementFunction()
    {
        return mReplacementFunctions[mNewFunctionType];
    }

    bool found() const { return mFound; }

  private:
    const int mShaderVersion;
    std::map<TBasicType, TIntermFunctionDefinition *> mReplacementFunctions;
    TBasicType mNewFunctionType = EbtVoid;
    bool mFound                 = false;
};

}  // anonymous namespace

bool RewriteTextureCubeGradAGX(TCompiler *compiler,
                               TIntermBlock *root,
                               TSymbolTable *symbolTable,
                               int shaderVersion)
{
    FindTextureCubeGradTraverser traverser(symbolTable, shaderVersion);
    do
    {
        traverser.nextIteration();
        root->traverse(&traverser);
        if (traverser.found())
        {
            TIntermFunctionDefinition *newFunction = traverser.getNewReplacementFunction();
            if (newFunction != nullptr)
            {
                root->insertStatement(FindFirstFunctionDefinitionIndex(root), newFunction);
            }

            if (!traverser.updateTree(compiler, root))
            {
                return false;
            }
        }
    } while (traverser.found());

    return true;
}

}  // namespace sh
