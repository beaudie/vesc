//
// Copyright 2022 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//

#include "compiler/translator/tree_ops/RewritePixelLocalStorage.h"

#include "common/angleutils.h"
#include "compiler/translator/StaticType.h"
#include "compiler/translator/SymbolTable.h"
#include "compiler/translator/tree_ops/MonomorphizeUnsupportedFunctions.h"
#include "compiler/translator/tree_util/BuiltIn.h"
#include "compiler/translator/tree_util/FindMain.h"
#include "compiler/translator/tree_util/IntermNode_util.h"
#include "compiler/translator/tree_util/IntermTraverse.h"

#if defined(ANDROID)
// The Android build doesn't generate builtins for desktop GLSL extensions.
//
// TODO(anglebug.com/7279): Tegra X1 has desktop GLSL with ARB_fragment_shader_interlock. Consider
// adding these functions to Android so PLS can be coherent on Tegra X1.
#    define HAS_BUILTIN_FRAGMENT_SYNCHRONIZATION_OPS 0
#else
#    define HAS_BUILTIN_FRAGMENT_SYNCHRONIZATION_OPS 1
#endif

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

// Simple helper for building nodes during PLS rewrite.
class NodeBuilder
{
  public:
    NodeBuilder(TSymbolTable &symbolTable, ShCompileOptions compileOptions, int shaderVersion)
        : mSymbolTable(&symbolTable),
#if HAS_BUILTIN_FRAGMENT_SYNCHRONIZATION_OPS
          mCompileOptions(compileOptions),
#endif
          mShaderVersion(shaderVersion)
    {}

    TSymbolTable *symbolTable() { return mSymbolTable; }

    TIntermTyped *referenceBuiltInVariable(const char *name)
    {
        return ReferenceBuiltInVariable(ImmutableString(name), *mSymbolTable, mShaderVersion);
    }

    static TIntermTyped *CreateConstructor(const TType &type, TIntermSequence args)
    {
        return TIntermAggregate::CreateConstructor(type, &args);
    }

    TIntermTyped *createBuiltInFunctionCall(const char *name, TIntermSequence args = {})
    {
        return CreateBuiltInFunctionCallNode(name, &args, *mSymbolTable, mShaderVersion);
    }

    // Delimits the beginning of a per-pixel critical section. Makes pixel local storage coherent.
    //
    // Either: GL_NV_fragment_shader_interlock,
    //         GL_INTEL_fragment_shader_ordering
    //         GL_ARB_fragment_shader_interlock,
    TIntermNode *createBuiltInInterlockBeginCall()
    {
#if HAS_BUILTIN_FRAGMENT_SYNCHRONIZATION_OPS
        switch (mCompileOptions & SH_FRAGMENT_SYNCHRONIZATION_TYPE_MASK)
        {
            case SH_FRAGMENT_SYNCHRONIZATION_TYPE_NV:
                return CreateBuiltInFunctionCall(EOpBeginInvocationInterlockNV,
                                                 BuiltInId::beginInvocationInterlockNV,
                                                 "beginInvocationInterlockNV");
            case SH_FRAGMENT_SYNCHRONIZATION_TYPE_INTEL:
                return CreateBuiltInFunctionCall(EOpBeginFragmentShaderOrderingINTEL,
                                                 BuiltInId::beginFragmentShaderOrderingINTEL,
                                                 "beginFragmentShaderOrderingINTEL");
            case SH_FRAGMENT_SYNCHRONIZATION_TYPE_ARB:
                return CreateBuiltInFunctionCall(EOpBeginInvocationInterlockARB,
                                                 BuiltInId::beginInvocationInterlockARB,
                                                 "beginInvocationInterlockARB");
        }
#endif
        return nullptr;
    }

    // Delimits the end of a per-pixel critical section. Makes pixel local storage coherent.
    //
    // Either: GL_ARB_fragment_shader_interlock or GL_NV_fragment_shader_interlock.
    // GL_INTEL_fragment_shader_ordering doesn't have an "end()" call.
    TIntermNode *createBuiltInInterlockEndCall()
    {
#if HAS_BUILTIN_FRAGMENT_SYNCHRONIZATION_OPS
        switch (mCompileOptions & SH_FRAGMENT_SYNCHRONIZATION_TYPE_MASK)
        {
            case SH_FRAGMENT_SYNCHRONIZATION_TYPE_NV:
                return CreateBuiltInFunctionCall(EOpEndInvocationInterlockNV,
                                                 BuiltInId::endInvocationInterlockNV,
                                                 "endInvocationInterlockNV");
            case SH_FRAGMENT_SYNCHRONIZATION_TYPE_INTEL:
                return nullptr;  // GL_INTEL_fragment_shader_ordering doesn't have an "end()" call.
            case SH_FRAGMENT_SYNCHRONIZATION_TYPE_ARB:
                return CreateBuiltInFunctionCall(EOpEndInvocationInterlockARB,
                                                 BuiltInId::endInvocationInterlockARB,
                                                 "endInvocationInterlockARB");
        }
#endif
        return nullptr;
    }

  private:
    static TIntermAggregate *CreateBuiltInFunctionCall(TOperator op,
                                                       const TSymbolUniqueId &id,
                                                       const char *name)
    {
        TIntermSequence emptyArgs;
        TFunction *function =
            new TFunction(id, ImmutableString(name), TExtension::UNDEFINED, nullptr, 0,
                          StaticType::GetBasic<EbtVoid, EbpUndefined>(), op, false);
        return TIntermAggregate::CreateBuiltInFunctionCall(*function, &emptyArgs);
    }

    TSymbolTable *const mSymbolTable;
#if HAS_BUILTIN_FRAGMENT_SYNCHRONIZATION_OPS
    const ShCompileOptions mCompileOptions;
#endif
    const int mShaderVersion;
};

// Rewrites high level PLS operations to shader image operations.
class RewriteToImagesTraverser : public TIntermTraverser
{
  public:
    RewriteToImagesTraverser(TCompiler *compiler, NodeBuilder *nodeBuilder)
        : TIntermTraverser(true, false, false, nodeBuilder->symbolTable()),
          mCompiler(compiler),
          mNodeBuilder(nodeBuilder)
    {}

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

        // Insert a global to hold the pixel coordinate as soon as we see PLS declared. This will be
        // initialized at the beginning of main().
        if (!mGlobalPixelCoord)
        {
            TType *coordType  = new TType(EbtInt, EbpHigh, EvqGlobal, 2);
            mGlobalPixelCoord = CreateTempVariable(mSymbolTable, coordType);
            insertStatementInParentBlock(CreateTempDeclarationNode(mGlobalPixelCoord));
        }

        PLSImages &pls = insertNullPLSImages(plsSymbol);
        TLayoutImageInternalFormat format =
            plsSymbol->getType().getLayoutQualifier().imageInternalFormat;
        if (mCompiler->getOutputType() == ShShaderOutput::SH_ESSL_OUTPUT &&
            format != TLayoutImageInternalFormat::EiifR32F &&
            format != TLayoutImageInternalFormat::EiifR32I &&
            format != TLayoutImageInternalFormat::EiifR32UI)
        {
            // ES 3.1 requires image formats other than r32f/r32i/r32ui to be either readonly or
            // writeonly. To work around this, we create two aliases of the same image: one readonly
            // and one writeonly.
            //
            // TODO(anglebug.com/7279): Maybe we could manually pack 4-byte formats into GL_R32UI
            // instead of aliasing them. We could also walk the tree first and see which image is
            // used how. If the image is never loaded, no need to generate the readonly binding for
            // example.
            //
            // First insert a readonly image2D directly before the PLS declaration.
            pls.image2DForLoading = createPLSImage(plsSymbol, ImageAccess::ReadOnly);
            insertStatementInParentBlock(
                new TIntermDeclaration({new TIntermSymbol(pls.image2DForLoading)}));

            // Replace the PLS declaration with a writeonly image2D.
            pls.image2DForStoring = createPLSImage(plsSymbol, ImageAccess::WriteOnly);
            queueReplacement(new TIntermDeclaration({new TIntermSymbol(pls.image2DForStoring)}),
                             OriginalNode::IS_DROPPED);
        }
        else
        {
            // Replace the PLS declaration with a single readwrite image2D.
            TVariable *readWriteImage = createPLSImage(plsSymbol, ImageAccess::ReadWrite);
            queueReplacement(new TIntermDeclaration({new TIntermSymbol(readWriteImage)}),
                             OriginalNode::IS_DROPPED);
            pls.image2DForLoading = pls.image2DForStoring = readWriteImage;
        }

        return false;
    }

    bool visitAggregate(Visit, TIntermAggregate *aggregate) override
    {
        if (!BuiltInGroup::IsPixelLocal(aggregate->getOp()))
        {
            return true;
        }

        const TIntermSequence &args = *aggregate->getSequence();
        ASSERT(args.size() >= 1);
        TIntermSymbol *plsSymbol = args[0]->getAsSymbolNode();
        const PLSImages &pls     = findPLSImages(plsSymbol);
        ASSERT(mGlobalPixelCoord);

        // Rewrite pixelLocalLoadANGLE -> imageLoad.
        if (aggregate->getOp() == EOpPixelLocalLoadANGLE)
        {
            // Replace the pixelLocalLoadANGLE with imageLoad.
            queueReplacement(mNodeBuilder->createBuiltInFunctionCall(
                                 "imageLoad", {new TIntermSymbol(pls.image2DForLoading),
                                               new TIntermSymbol(mGlobalPixelCoord)}),
                             OriginalNode::IS_DROPPED);

            return false;  // No need to recurse since this node is being dropped.
        }

        // Rewrite pixelLocalStoreANGLE -> imageStore.
        if (aggregate->getOp() == EOpPixelLocalStoreANGLE)
        {
            // Surround the store with memoryBarrierImage calls in order to ensure dependent stores
            // and loads in a single shader invocation are coherent. From the ES 3.1 spec:
            //
            //   Using variables declared as "coherent" guarantees only that the results of stores
            //   will be immediately visible to shader invocations using similarly-declared
            //   variables; calling MemoryBarrier is required to ensure that the stores are visible
            //   to other operations.
            //
            // Also hoist the 'value' expression into a temp. In the event of
            // "pixelLocalStoreANGLE(..., pixelLocalLoadANGLE(...))", this ensures the load occurs
            // _before_ the memoryBarrierImage.
            //
            // NOTE: It is generally unsafe to hoist function arguments due to short circuiting,
            // e.g., "if (false && function(...))", but pixelLocalStoreANGLE returns type void, so
            // it is safe in this particular case.
            TType *valueType    = new TType(DataTypeOfPLSType(plsSymbol->getBasicType()),
                                            plsSymbol->getPrecision(), EvqTemporary, 4);
            TVariable *valueVar = CreateTempVariable(mSymbolTable, valueType);
            TIntermDeclaration *valueDecl =
                CreateTempInitDeclarationNode(valueVar, args[1]->getAsTyped());
            valueDecl->traverse(this);  // Rewrite any potential pixelLocalLoadANGLEs in valueDecl.

            insertStatementsInParentBlock(
                {valueDecl,
                 mNodeBuilder->createBuiltInFunctionCall("memoryBarrierImage")},   // Before.
                {mNodeBuilder->createBuiltInFunctionCall("memoryBarrierImage")});  // After.

            // Rewrite the pixelLocalStoreANGLE with imageStore.
            queueReplacement(mNodeBuilder->createBuiltInFunctionCall(
                                 "imageStore", {new TIntermSymbol(pls.image2DForStoring),
                                                new TIntermSymbol(mGlobalPixelCoord),
                                                new TIntermSymbol(valueVar)}),
                             OriginalNode::IS_DROPPED);

            return false;  // No need to recurse since this node is being dropped.
        }

        return true;
    }

    TVariable *globalPixelCoord() const { return mGlobalPixelCoord; }

  private:
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
        int binding = plsSymbol->getType().getLayoutQualifier().binding;
        ASSERT(binding >= 0);
        auto result = mPLSImages.insert({binding, PLSImages()});
        ASSERT(result.second);  // Ensure PLSImages didn't already exist for this symbol.
        return result.first->second;
    }

    // Looks up the PLSImages for the given symbol. An entry must already exist in the map for this
    // symbol.
    PLSImages &findPLSImages(TIntermSymbol *plsSymbol)
    {
        ASSERT(plsSymbol);
        ASSERT(IsPixelLocal(plsSymbol->getBasicType()));
        int binding = plsSymbol->getType().getLayoutQualifier().binding;
        ASSERT(binding >= 0);
        auto iter = mPLSImages.find(binding);
        ASSERT(iter != mPLSImages.end());  // Ensure PLSImages already exist for this symbol.
        return iter->second;
    }

    enum class ImageAccess
    {
        ReadOnly,
        WriteOnly,
        ReadWrite
    };

    // Creates a 'gimage2D' that implements a pixel local storage handle.
    TVariable *createPLSImage(const TIntermSymbol *plsSymbol, ImageAccess access)
    {
        ASSERT(plsSymbol);
        ASSERT(IsPixelLocal(plsSymbol->getBasicType()));

        TMemoryQualifier memoryQualifier;
        memoryQualifier.coherent          = true;
        memoryQualifier.restrictQualifier = access == ImageAccess::ReadWrite;
        memoryQualifier.volatileQualifier = access != ImageAccess::ReadWrite;
        memoryQualifier.readonly          = access == ImageAccess::ReadOnly;
        memoryQualifier.writeonly         = access == ImageAccess::WriteOnly;

        TType *imageType = new TType(plsSymbol->getType());
        imageType->setBasicType(Image2DTypeOfPLSType(plsSymbol->getBasicType()));
        imageType->setMemoryQualifier(memoryQualifier);

        std::string name = "_pls";
        name.append(plsSymbol->getName().data());
        if (access == ImageAccess::ReadOnly)
        {
            name.append("_R");
        }
        else if (access == ImageAccess::WriteOnly)
        {
            name.append("_W");
        }
        return new TVariable(mSymbolTable, ImmutableString(name), imageType, SymbolType::BuiltIn);
    }

    const TCompiler *const mCompiler;
    NodeBuilder *const mNodeBuilder;

    // Stores the shader invocation's pixel coordinate as "ivec2(floor(gl_FragCoord.xy))".
    TVariable *mGlobalPixelCoord = nullptr;

    // Maps PLS variables to their gimage2D aliases.
    angle::HashMap<int, PLSImages> mPLSImages;
};

}  // anonymous namespace

bool RewritePixelLocalStorageToImages(TCompiler *compiler,
                                      TIntermBlock *root,
                                      TSymbolTable &symbolTable,
                                      ShCompileOptions compileOptions,
                                      int shaderVersion)
{
    // If any functions take PLS arguments, monomorphize the functions by removing said parameters
    // and making the PLS calls from main() instead, using the global uniform from the call site
    // instead of the function argument. This is necessary because function arguments don't carry
    // the necessary "binding" or "format" layout qualifiers.
    if (!MonomorphizeUnsupportedFunctions(
            compiler, root, &symbolTable, compileOptions,
            UnsupportedFunctionArgsBitSet{UnsupportedFunctionArgs::PixelLocalStorage}))
    {
        return false;
    }

    NodeBuilder nodeBuilder(symbolTable, compileOptions, shaderVersion);
    TIntermBlock *mainBody = FindMainBody(root);

    // Surround the critical section of PLS operations in fragment synchronization calls, if
    // supported. This makes pixel local storage coherent.
    //
    // TODO(anglebug.com/7279): Inject these functions in a tight critical section, instead of just
    // locking the entire main() function:
    //   - Monomorphize all PLS calls into main().
    //   - Insert begin/end calls around the first/last PLS calls (and outside of flow control).
    if (compileOptions & SH_FRAGMENT_SYNCHRONIZATION_TYPE_MASK)
    {
        if (TIntermNode *interlockBeginCall = nodeBuilder.createBuiltInInterlockBeginCall())
        {
            mainBody->insertStatement(0, interlockBeginCall);
        }
        if (TIntermNode *interlockEndCall = nodeBuilder.createBuiltInInterlockEndCall())
        {
            mainBody->appendStatement(interlockEndCall);
        }
    }

    // Rewrite PLS operations to image operations.
    RewriteToImagesTraverser traverser(compiler, &nodeBuilder);
    root->traverse(&traverser);
    if (!traverser.updateTree(compiler, root))
    {
        return false;
    }

    // Initialize the global pixel coord at the beginning of main():
    //
    //     pixelCoord = ivec2(floor(gl_FragCoord.xy));
    //
    if (traverser.globalPixelCoord())
    {
        TIntermTyped *node;
        node = nodeBuilder.referenceBuiltInVariable("gl_FragCoord");
        node = CreateSwizzle(node, 0, 1);
        node = nodeBuilder.createBuiltInFunctionCall("floor", {node});
        node = NodeBuilder::CreateConstructor(TType(EbtInt, 2), {node});
        node = CreateTempAssignmentNode(traverser.globalPixelCoord(), node);
        mainBody->insertStatement(0, node);
    }

    return compiler->validateAST(root);
}

}  // namespace sh
