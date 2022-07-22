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

constexpr static TBasicType DataTypeOfImageType(TBasicType imageType)
{
    switch (imageType)
    {
        case EbtImage2D:
            return EbtFloat;
        case EbtIImage2D:
            return EbtInt;
        case EbtUImage2D:
            return EbtUInt;
        default:
            UNREACHABLE();
            return EbtVoid;
    }
}

// GLES and D3D only allow r32f/r32i/r32ui images to be both read and written in a single shader.
// When outputting to these languages, we have to pack all PLS formats into r32* images.
constexpr static bool ShaderLanguageNeedsR32Packing(ShShaderOutput shaderOutput)
{
    switch (shaderOutput)
    {
        case ShShaderOutput::SH_ESSL_OUTPUT:
        case ShShaderOutput::SH_HLSL_3_0_OUTPUT:
        case ShShaderOutput::SH_HLSL_4_1_OUTPUT:
        case ShShaderOutput::SH_HLSL_4_0_FL9_3_OUTPUT:
            return true;
        default:
            return false;
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

        // Replace the PLS declaration with an image2D.
        TVariable *image2D = createPLSImage(plsSymbol);
        insertNewPLSImage(plsSymbol, image2D);
        queueReplacement(new TIntermDeclaration({new TIntermSymbol(image2D)}),
                         OriginalNode::IS_DROPPED);

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
        TVariable *image2D       = findPLSImage(plsSymbol);
        ASSERT(mGlobalPixelCoord);

        // Rewrite pixelLocalLoadANGLE -> imageLoad.
        if (aggregate->getOp() == EOpPixelLocalLoadANGLE)
        {
            // Replace the pixelLocalLoadANGLE with imageLoad.
            TIntermTyped *pls;
            pls = mNodeBuilder->createBuiltInFunctionCall(
                "imageLoad", {new TIntermSymbol(image2D), new TIntermSymbol(mGlobalPixelCoord)});
            pls = unpackImageDataIfNecessary(pls, plsSymbol, image2D);
            queueReplacement(pls, OriginalNode::IS_DROPPED);
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
            queueReplacement(
                mNodeBuilder->createBuiltInFunctionCall(
                    "imageStore", {new TIntermSymbol(image2D), new TIntermSymbol(mGlobalPixelCoord),
                                   packPLSDataIfNecessary(valueVar, plsSymbol, image2D)}),
                OriginalNode::IS_DROPPED);

            return false;  // No need to recurse since this node is being dropped.
        }

        return true;
    }

    TVariable *globalPixelCoord() const { return mGlobalPixelCoord; }

  private:
    // Sets the given image2D as the backing storage for the plsSymbol's binding point. An entry
    // must not already exist in the map for this binding point.
    void insertNewPLSImage(TIntermSymbol *plsSymbol, TVariable *image2D)
    {
        ASSERT(plsSymbol);
        ASSERT(IsPixelLocal(plsSymbol->getBasicType()));
        int binding = plsSymbol->getType().getLayoutQualifier().binding;
        ASSERT(binding >= 0);
        auto result = mPLSImages.insert({binding, image2D});
        ASSERT(result.second);  // Ensure an image didn't already exist for this symbol.
    }

    // Looks up the image2D backing storage for the given plsSymbol's binding point. An entry must
    // already exist in the map for this binding point.
    TVariable *findPLSImage(TIntermSymbol *plsSymbol)
    {
        ASSERT(plsSymbol);
        ASSERT(IsPixelLocal(plsSymbol->getBasicType()));
        int binding = plsSymbol->getType().getLayoutQualifier().binding;
        ASSERT(binding >= 0);
        auto iter = mPLSImages.find(binding);
        ASSERT(iter != mPLSImages.end());  // Ensure PLSImages already exist for this symbol.
        return iter->second;
    }

    // Creates an image2D that implements a pixel local storage handle.
    TVariable *createPLSImage(const TIntermSymbol *plsSymbol)
    {
        ASSERT(plsSymbol);
        ASSERT(IsPixelLocal(plsSymbol->getBasicType()));

        TType *imageType = new TType(plsSymbol->getType());

        TLayoutQualifier layoutQualifier = imageType->getLayoutQualifier();
        switch (layoutQualifier.imageInternalFormat)
        {
            case TLayoutImageInternalFormat::EiifRGBA8:
                if (ShaderLanguageNeedsR32Packing(mCompiler->getOutputType()))
                {
                    layoutQualifier.imageInternalFormat = EiifR32UI;
                    imageType->setPrecision(EbpHigh);
                    imageType->setBasicType(EbtUImage2D);
                }
                else
                {
                    imageType->setBasicType(EbtImage2D);
                }
                break;
            case TLayoutImageInternalFormat::EiifRGBA8I:
                if (ShaderLanguageNeedsR32Packing(mCompiler->getOutputType()))
                {
                    layoutQualifier.imageInternalFormat = EiifR32I;
                    imageType->setPrecision(EbpHigh);
                }
                imageType->setBasicType(EbtIImage2D);
                break;
            case TLayoutImageInternalFormat::EiifRGBA8UI:
                if (ShaderLanguageNeedsR32Packing(mCompiler->getOutputType()))
                {
                    layoutQualifier.imageInternalFormat = EiifR32UI;
                    imageType->setPrecision(EbpHigh);
                }
                imageType->setBasicType(EbtUImage2D);
                break;
            case TLayoutImageInternalFormat::EiifR32F:
                imageType->setBasicType(EbtImage2D);
                break;
            case TLayoutImageInternalFormat::EiifR32UI:
                imageType->setBasicType(EbtUImage2D);
                break;
            default:
                UNREACHABLE();
        }
        imageType->setLayoutQualifier(layoutQualifier);

        TMemoryQualifier memoryQualifier{};
        memoryQualifier.coherent          = true;
        memoryQualifier.restrictQualifier = true;
        memoryQualifier.volatileQualifier = false;
        // TODO(anglebug.com/7279): Maybe we could walk the tree first and see which image is used
        // how. If the image is never loaded, no need to generate the readonly binding for example.
        memoryQualifier.readonly  = false;
        memoryQualifier.writeonly = false;
        imageType->setMemoryQualifier(memoryQualifier);

        std::string name = "_pls";
        name.append(plsSymbol->getName().data());
        return new TVariable(mSymbolTable, ImmutableString(name), imageType, SymbolType::BuiltIn);
    }

    // Unpacks the PLS data from a raw r32* image if the storage is packed (for GLES and D3D).
    TIntermTyped *unpackImageDataIfNecessary(TIntermTyped *data,
                                             TIntermSymbol *plsSymbol,
                                             TVariable *image2D)
    {
        TLayoutImageInternalFormat plsFormat =
            plsSymbol->getType().getLayoutQualifier().imageInternalFormat;
        TLayoutImageInternalFormat imageFormat =
            image2D->getType().getLayoutQualifier().imageInternalFormat;
        if (plsFormat == imageFormat)
        {
            return data;  // This PLS storage isn't packed.
        }
        switch (plsFormat)
        {
            case EiifRGBA8:
                // unpackUnorm4x8(data)
                data = CreateSwizzle(data, 0);
                data = mNodeBuilder->createBuiltInFunctionCall("unpackUnorm4x8", {data});
                break;
            case EiifRGBA8I:
            case EiifRGBA8UI:
            {
                constexpr unsigned shifts[] = {24, 16, 8, 0};
                // Shift left, then right, to preserve the sign for ints. ("highp r32i" is exactly
                // 32-bit two's compliment.)
                //
                //     data.rrrr << uvec4(24, 16, 8, 0) >> 24u
                //
                data = CreateSwizzle(data, 0, 0, 0, 0);
                data = new TIntermBinary(EOpBitShiftLeft, data, CreateUVecNode(shifts, 4, EbpHigh));
                data = new TIntermBinary(EOpBitShiftRight, data, CreateUIntNode(24));
                break;
            }
            default:
                UNREACHABLE();
        }
        return data;
    }

    // Packs the PLS data for a raw r32* image if the storage is packed (for GLES and D3D).
    TIntermTyped *packPLSDataIfNecessary(TVariable *plsVar,
                                         TIntermSymbol *plsSymbol,
                                         TVariable *image2D)
    {
        TLayoutImageInternalFormat plsFormat =
            plsSymbol->getType().getLayoutQualifier().imageInternalFormat;
        TLayoutImageInternalFormat imageFormat =
            image2D->getType().getLayoutQualifier().imageInternalFormat;
        TIntermTyped *packedExpr = new TIntermSymbol(plsVar);
        if (plsFormat == imageFormat)
        {
            return packedExpr;  // This PLS storage isn't packed.
        }
        switch (plsFormat)
        {
            case EiifRGBA8:
            {
                // anglebug.com/7523: unpackUnorm4x8 doesn't work on mediump vec4 on Pixel 4.
                // Use an intermediate highp vec4.
                TType *highpType              = new TType(EbtFloat, EbpHigh, EvqTemporary, 4);
                TVariable *workaroundHighpVar = CreateTempVariable(mSymbolTable, highpType);
                insertStatementInParentBlock(
                    CreateTempInitDeclarationNode(workaroundHighpVar, packedExpr));
                packedExpr = new TIntermSymbol(workaroundHighpVar);

                // Denormalize and pack r,g,b,a into a single uint:
                //
                //     packUnorm4x8(workaroundHighpVar)
                //
                packedExpr = mNodeBuilder->createBuiltInFunctionCall("packUnorm4x8", {packedExpr});
                break;
            }
            case EiifRGBA8I:
            case EiifRGBA8UI:
            {
                // Pack r,g,b,a into a single uint:
                //
                //     (r & 0xff) | ((g & 0xff) << 8) | ((b & 0xff) << 16) | (a << 24)
                //
                TIntermTyped *accessPLSVar = packedExpr;
                ASSERT(accessPLSVar->getAsSymbolNode());
                TIntermTyped *mask =
                    plsFormat == EiifRGBA8I ? CreateIntNode(0xff) : CreateUIntNode(0xff);
                auto maskAndShiftComponent = [=](int componentIdx) {
                    TIntermTyped *expr = CreateSwizzle(accessPLSVar, componentIdx);
                    if (componentIdx < 3)
                    {
                        expr = new TIntermBinary(EOpBitwiseAnd, expr, mask);
                    }
                    if (componentIdx > 0)
                    {
                        expr = new TIntermBinary(EOpBitShiftLeft, expr,
                                                 CreateUIntNode(componentIdx * 8));
                    }
                    return expr;
                };
                packedExpr = maskAndShiftComponent(0);
                packedExpr = new TIntermBinary(EOpBitwiseOr, packedExpr, maskAndShiftComponent(1));
                packedExpr = new TIntermBinary(EOpBitwiseOr, packedExpr, maskAndShiftComponent(2));
                packedExpr = new TIntermBinary(EOpBitwiseOr, packedExpr, maskAndShiftComponent(3));
                break;
            }
            default:
                UNREACHABLE();
        }
        // Convert the packed data to a {u,i}vec4 for imageStore.
        TType imageStoreType(DataTypeOfImageType(image2D->getType().getBasicType()), 4);
        packedExpr = NodeBuilder::CreateConstructor(imageStoreType, {packedExpr});
        return packedExpr;
    }

    const TCompiler *const mCompiler;
    NodeBuilder *const mNodeBuilder;

    // Stores the shader invocation's pixel coordinate as "ivec2(floor(gl_FragCoord.xy))".
    TVariable *mGlobalPixelCoord = nullptr;

    // Maps PLS variables to their image2D backing storage.
    angle::HashMap<int, TVariable *> mPLSImages;
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
