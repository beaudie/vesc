//
// Copyright 2022 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//
// Copyright 2022 Rive
//

#include "compiler/translator/tree_ops/RewritePixelLocalStorage.h"

#include "common/angleutils.h"
#include "compiler/translator/SymbolTable.h"
#include "compiler/translator/tree_util/IntermNode_util.h"
#include "compiler/translator/tree_util/IntermTraverse.h"

namespace sh
{

namespace
{

constexpr static TBasicType DataTypeOfPLSType(TBasicType plsType)
{
    switch (plsType)
    {
        case EbtPixelLocalANGLE:
            return EbtFloat;
        case EbtIPixelLocalANGLE:
            return EbtInt;
        case EbtUPixelLocalANGLE:
            return EbtUInt;
        default:
            UNREACHABLE();
            return EbtVoid;
    }
}

constexpr static TBasicType Image2DTypeOfPLSType(TBasicType plsType)
{
    switch (plsType)
    {
        case EbtPixelLocalANGLE:
            return EbtImage2D;
        case EbtIPixelLocalANGLE:
            return EbtIImage2D;
        case EbtUPixelLocalANGLE:
            return EbtUImage2D;
        default:
            UNREACHABLE();
            return EbtVoid;
    }
}

// Rewrites high level PLS operations to shader image operations.
class RewriteToImagesTraverser : public TIntermTraverser
{
  public:
    static bool RewriteShader(TCompiler *compiler,
                              TIntermBlock *root,
                              TSymbolTable &symbolTable,
                              int shaderVersion)
    {
        RewriteToImagesTraverser traverser(symbolTable, shaderVersion);
        for (auto pass : {Pass::RewriteLoads, Pass::RewriteStores})
        {
            traverser.mCurrentPass = pass;
            root->traverse(&traverser);
            if (!traverser.updateTree(compiler, root))
            {
                return false;
            }
        }
        return true;
    }

    bool visitDeclaration(Visit, TIntermDeclaration *decl) override
    {
        TIntermTyped *declVariable = (decl->getSequence())->front()->getAsTyped();
        ASSERT(declVariable);

        if (!IsPixelLocal(declVariable->getBasicType()))
        {
            return true;
        }

        // PLS is not allowed in arrays.
        ASSERT(!declVariable->isArray());

        // This visitDeclaration doesn't get called for function arguments, and opaque types can
        // otherwise only be uniforms.
        ASSERT(declVariable->getQualifier() == EvqUniform);

        TIntermSymbol *plsSymbol = declVariable->getAsSymbolNode();
        ASSERT(plsSymbol);

        // Insert a global to hold the pixel coordinate as soon as we see PLS declared.
        if (!mGlobalPixelCoord)
        {
            TType *coordType  = new TType(EbtInt, EbpHigh, EvqGlobal, 2);
            mGlobalPixelCoord = CreateTempVariable(mSymbolTable, coordType);

            // ivec2(floor(gl_FragCoord.xy))
            TIntermTyped *expr;
            expr = ReferenceBuiltInVariable(ImmutableString("gl_FragCoord"), *mSymbolTable,
                                            mShaderVersion);
            expr = CreateSwizzle(expr, 0, 1);
            expr = CreateBuiltInUnaryFunctionCallNode("floor", expr, *mSymbolTable, mShaderVersion);
            TIntermSequence typeConversionArgs = {expr};
            expr = TIntermAggregate::CreateConstructor(TType(EbtInt, 2), &typeConversionArgs);

            insertStatementInParentBlock(CreateTempInitDeclarationNode(mGlobalPixelCoord, expr));
        }

        // Insert a readonly image2D directly before the PLS declaration.
        if (mCurrentPass == Pass::RewriteLoads)
        {
            PLSImages &pls        = insertNullPLSImages(plsSymbol);
            pls.image2DForLoading = createPLSImage(plsSymbol, ImageAccess::Readonly);
            insertStatementInParentBlock(
                new TIntermDeclaration({new TIntermSymbol(pls.image2DForLoading)}));
        }

        // Replace the PLS declaration with a writeonly image2D.
        if (mCurrentPass == Pass::RewriteStores)
        {
            PLSImages &pls        = findPLSImages(plsSymbol);
            pls.image2DForStoring = createPLSImage(plsSymbol, ImageAccess::Writeonly);
            queueReplacement(new TIntermDeclaration({new TIntermSymbol(pls.image2DForStoring)}),
                             OriginalNode::IS_DROPPED);
        }

        return false;
    }

    bool visitAggregate(Visit, TIntermAggregate *aggregate) override
    {
        const TIntermSequence &args = *aggregate->getSequence();

        // Rewrite pixelLocalLoadANGLE -> imageLoad.
        if (mCurrentPass == Pass::RewriteLoads && aggregate->getOp() == EOpPixelLocalLoadANGLE)
        {
            ASSERT(args.size() == 1);
            TIntermSymbol *plsSymbol = args[0]->getAsSymbolNode();
            const PLSImages &pls     = findPLSImages(plsSymbol);
            ASSERT(mGlobalPixelCoord);

            // Create a node that calls imageLoad().
            TIntermSequence imageLoadArgs = {new TIntermSymbol(pls.image2DForLoading),
                                             new TIntermSymbol(mGlobalPixelCoord)};
            TIntermTyped *imageLoadNode   = CreateBuiltInFunctionCallNode(
                  "imageLoad", &imageLoadArgs, *mSymbolTable, mShaderVersion);

            // Hoist the imageLoad invocation and store it in a temp. In the event of
            // "pixelLocalStoreANGLE(..., pixelLocalLoadANGLE(...))", this ensures the load occurs
            // outside the pixelLocalStoreANGLE expression, which is necessary because the store
            // needs to be tightly surrounded with memoryBarriers.
            TType *type     = new TType(DataTypeOfPLSType(plsSymbol->getBasicType()),
                                        plsSymbol->getPrecision(), EvqTemporary, 4);
            TVariable *temp = CreateTempVariable(mSymbolTable, type);
            insertStatementInParentBlock(CreateTempInitDeclarationNode(temp, imageLoadNode));

            // Replace the pixelLocalStoreANGLE with our new temp variable.
            queueReplacement(new TIntermSymbol(temp), OriginalNode::IS_DROPPED);

            return false;  // pixelLocalLoad/Store can't recurse on themselves or each other.
        }

        // Rewrite pixelLocalStoreANGLE -> imageStore.
        if (mCurrentPass == Pass::RewriteStores && aggregate->getOp() == EOpPixelLocalStoreANGLE)
        {
            ASSERT(args.size() == 2);
            const PLSImages &pls = findPLSImages(args[0]->getAsSymbolNode());
            ASSERT(mGlobalPixelCoord);

            // Since ES 3.1 makes us have readonly and writeonly aliases of the same image, we have
            // to surround every pixelLocalStoreANGLE with memoryBarrier calls.
            insertStatementsInParentBlock({createMemoryBarrierNode()},   // Before.
                                          {createMemoryBarrierNode()});  // After.

            // Rewrite the pixelLocalStoreANGLE with imageStore.
            TIntermSequence imageStoreArgs = {new TIntermSymbol(pls.image2DForStoring),
                                              new TIntermSymbol(mGlobalPixelCoord),
                                              args[1]->deepCopy()};
            TIntermTyped *imageStoreNode   = CreateBuiltInFunctionCallNode(
                  "imageStore", &imageStoreArgs, *mSymbolTable, mShaderVersion);
            queueReplacement(imageStoreNode, OriginalNode::IS_DROPPED);

            return false;  // pixelLocalLoad/Store can't recurse on themselves or each other.
        }

        return true;
    }

  private:
    RewriteToImagesTraverser(TSymbolTable &symbolTable, int shaderVersion)
        : TIntermTraverser(true, false, false, &symbolTable), mShaderVersion(shaderVersion)
    {}

    // Internal implementation of an opaque 'gpixelLocalANGLE' handle. Since ES 3.1 requires most
    // image formats to be either readonly or writeonly, we have to make two separate images that
    // alias the same binding.
    struct PLSImages
    {
        TVariable *image2DForLoading = nullptr;
        TVariable *image2DForStoring = nullptr;
    };

    // Adds a null 'PLSImages' entry to the map for the given symbol. An entry must not already
    // exist in the map for this symbol.
    PLSImages &insertNullPLSImages(TIntermSymbol *plsSymbol)
    {
        ASSERT(plsSymbol);
        ASSERT(IsPixelLocal(plsSymbol->getBasicType()));
        auto result = mPLSImages.insert({&plsSymbol->variable(), PLSImages()});
        ASSERT(result.second);  // Ensure PLSImages didn't already exist for this symbol.
        return result.first->second;
    }

    // Looks up the PLSImages for the given symbol. An entry must already exist in the map for this
    // symbol.
    PLSImages &findPLSImages(TIntermSymbol *plsSymbol)
    {
        ASSERT(plsSymbol);
        ASSERT(IsPixelLocal(plsSymbol->getBasicType()));
        auto iter = mPLSImages.find(&plsSymbol->variable());
        ASSERT(iter != mPLSImages.end());  // Ensure PLSImages already exist for this symbol.
        return iter->second;
    }

    enum class ImageAccess
    {
        Readonly,
        Writeonly
    };

    // Creates a 'gimage2D' that implements a pixel local storage handle.
    TVariable *createPLSImage(const TIntermSymbol *plsSymbol, ImageAccess access)
    {
        ASSERT(plsSymbol);
        ASSERT(IsPixelLocal(plsSymbol->getBasicType()));

        TMemoryQualifier memoryQualifier;
        memoryQualifier.coherent          = true;
        memoryQualifier.restrictQualifier = false;
        memoryQualifier.volatileQualifier = true;
        memoryQualifier.readonly          = access == ImageAccess::Readonly;
        memoryQualifier.writeonly         = access == ImageAccess::Writeonly;

        TType *imageType = new TType(plsSymbol->getType());
        imageType->setBasicType(Image2DTypeOfPLSType(plsSymbol->getBasicType()));
        imageType->setMemoryQualifier(memoryQualifier);

        std::string name = "_pls";
        name.append(plsSymbol->getName().data());
        name.append(access == ImageAccess::Readonly ? "_R" : "_W");
        return new TVariable(mSymbolTable, ImmutableString(name), imageType, SymbolType::BuiltIn);
    }

    // Creates a function call to memoryBarrier().
    TIntermNode *createMemoryBarrierNode()
    {
        TIntermSequence emptyArgs;
        return CreateBuiltInFunctionCallNode("memoryBarrier", &emptyArgs, *mSymbolTable,
                                             mShaderVersion);
    }

    const int mShaderVersion;

    // Stores the shader invocation's pixel coordinate as "ivec2(floor(gl_FragCoord.xy))".
    TVariable *mGlobalPixelCoord = nullptr;

    // Maps PLS variables to their gimage2D aliases.
    angle::HashMap<const TVariable *, PLSImages> mPLSImages;

    // We rewrite pixel local storage in two passes, in order to properly handle
    // "pixelLocalStoreANGLE(..., pixelLocalLoadANGLE(...))".
    enum class Pass
    {
        RewriteLoads,
        RewriteStores
    };

    Pass mCurrentPass;
};

}  // anonymous namespace

bool RewritePixelLocalStorageToImages(TCompiler *compiler,
                                      TIntermBlock *root,
                                      TSymbolTable &symbolTable,
                                      int shaderVersion)
{
    return RewriteToImagesTraverser::RewriteShader(compiler, root, symbolTable, shaderVersion);
}

}  // namespace sh
