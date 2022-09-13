//
// Copyright 2022 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//
// EmulateLogicOp.cpp: Emulate logicOp by implicitly reading back from the color attachment (as an
// input attachment) and apply the logic op equation based on a uniform.
//

#include "compiler/translator/tree_ops/vulkan/EmulateLogicOp.h"

#include <map>

#include "GLSLANG/ShaderVars.h"
#include "common/PackedEnums.h"
#include "compiler/translator/Compiler.h"
#include "compiler/translator/StaticType.h"
#include "compiler/translator/SymbolTable.h"
#include "compiler/translator/tree_util/DriverUniform.h"
#include "compiler/translator/tree_util/FindMain.h"
#include "compiler/translator/tree_util/IntermNode_util.h"
#include "compiler/translator/tree_util/IntermTraverse.h"
#include "compiler/translator/tree_util/RunAtTheEndOfShader.h"

namespace sh
{
namespace
{

// All helper functions that may be generated.
class Builder
{
  public:
    Builder(TCompiler *compiler,
            const ShCompileOptions &compileOptions,
            TSymbolTable *symbolTable,
            const DriverUniform *driverUniforms,
            std::vector<ShaderVariable> *uniforms)
        : mCompiler(compiler),
          mCompileOptions(compileOptions),
          mSymbolTable(symbolTable),
          mDriverUniforms(driverUniforms),
          mUniforms(uniforms)
    {}

    bool build(TIntermBlock *root);

  private:
    void findColorOutput(TIntermBlock *root);
    void createSubpassInputVar(TIntermBlock *root);
    TIntermTyped *divideFloatNode(TIntermTyped *dividend, TIntermTyped *divisor);
    TIntermSymbol *premultiplyAlpha(TIntermBlock *opBlock, TIntermTyped *var, const char *name);
    void generatePreamble(TIntermBlock *opBlock);
    void generateLogicOpSwitches(TIntermBlock *opBlock);

    TCompiler *mCompiler;
    const ShCompileOptions &mCompileOptions;
    TSymbolTable *mSymbolTable;
    const DriverUniform *mDriverUniforms;
    std::vector<ShaderVariable> *mUniforms;

    // The color input and output.  Output is the logic op source, and input is the destination.
    const TVariable *mSubpassInputVar = nullptr;
    const TVariable *mOutputVar       = nullptr;

    // The multipliers to change color channels from float to unnormalized integer.
    TIntermSybmol *mChannelMultiplier = nullptr;

    // The value of output, unnormalized
    TIntermSymbol *mSrc = nullptr;
    // The value of input, unnormalized
    TIntermSymbol *mDst = nullptr;
};

bool Builder::build(TIntermBlock *root)
{
    // Find the output variable and declare a corresponding input variable.
    findColorOutput(root);
    createSubpassInputVar(root);

    // Prepare for logic op by:
    //
    // - Loading from subpass input
    // - Turning source and destination to unorm
    TIntermBlock *opBlock = new TIntermBlock;
    generatePreamble(opBlock);

    // Generate the |switch| statements that perform the logic operation based on driver uniforms.
    generateLogicOpSwitches(opBlock);

    // Place the entire block under an if (op != 0)
    TIntermTyped *opUniform = mDriverUniforms->getLogicOpOp();
    TIntermTyped *notZero   = new TIntermBinary(EOpNotEqual, opUniform, CreateUIntNode(0));

    TIntermIfElse *logicOp = new TIntermIfElse(notZero, opBlock, nullptr);
    return RunAtTheEndOfShader(mCompiler, root, logicOp, mSymbolTable);
}

void Builder::findColorOutput(TIntermBlock *root)
{
    for (TIntermNode *node : *root->getSequence())
    {
        TIntermDeclaration *asDecl = node->getAsDeclarationNode();
        if (asDecl == nullptr)
        {
            continue;
        }

        // SeparateDeclarations should have already been run.
        ASSERT(asDecl->getSequence()->size() == 1u);

        TIntermSymbol *symbol = asDecl->getSequence()->front()->getAsSymbolNode();
        if (symbol == nullptr)
        {
            continue;
        }

        const TType &type = symbol->getType();
        if (type.getQualifier() == EvqFragmentOut)
        {
            // There can only be one output in GLES1.
            ASSERT(mOutputVar == nullptr);
            mOutputVar = &symbol->variable();
        }
    }

    // This transformation is only ever called when advanced blend is specified.
    ASSERT(mOutputVar != nullptr);
    ASSERT(mSubpassInputVar == nullptr);
}

TIntermSymbol *MakeVariable(TSymbolTable *symbolTable, const char *name, const TType *type)
{
    const TVariable *var =
        new TVariable(symbolTable, ImmutableString(name), type, SymbolType::AngleInternal);
    return new TIntermSymbol(var);
}

void Builder::createSubpassInputVar(TIntermBlock *root)
{
    const TPrecision precision = mOutputVar->getType().getPrecision();

    // The input attachment index used for this color attachment would be identical to its location
    // (or implicitly 0 if not specified).
    const unsigned int inputAttachmentIndex =
        std::max(0, mOutputVar->getType().getLayoutQualifier().location);

    // Create the subpass input uniform.
    TType *inputAttachmentType = new TType(EbtSubpassInput, precision, EvqUniform, 1);
    TLayoutQualifier inputAttachmentQualifier     = inputAttachmentType->getLayoutQualifier();
    inputAttachmentQualifier.inputAttachmentIndex = inputAttachmentIndex;
    inputAttachmentType->setLayoutQualifier(inputAttachmentQualifier);

    const char *kSubpassInputName = "ANGLEFragmentInput";
    TIntermSymbol *subpassInputSymbol =
        MakeVariable(mSymbolTable, kSubpassInputName, inputAttachmentType);
    mSubpassInputVar = &subpassInputSymbol->variable();

    // Add its declaration to the shader.
    TIntermDeclaration *subpassInputDecl = new TIntermDeclaration;
    subpassInputDecl->appendDeclarator(subpassInputSymbol);
    root->insertStatement(0, subpassInputDecl);

    // Add the new subpass input to the list of uniforms.
    ShaderVariable subpassInputUniform;
    subpassInputUniform.active    = true;
    subpassInputUniform.staticUse = true;
    subpassInputUniform.name.assign(kSubpassInputName);
    subpassInputUniform.mappedName.assign(kSubpassInputName);
    subpassInputUniform.isFragmentInOut = true;
    subpassInputUniform.location        = inputAttachmentIndex;
    mUniforms->push_back(subpassInputUniform);
}

TIntermTyped *Float(float f)
{
    return CreateFloatNode(f, EbpMedium);
}

TFunction *MakeFunction(TSymbolTable *symbolTable,
                        const char *name,
                        const TType *returnType,
                        const TVector<const TVariable *> &args)
{
    TFunction *function = new TFunction(symbolTable, ImmutableString(name),
                                        SymbolType::AngleInternal, returnType, false);
    for (const TVariable *arg : args)
    {
        function->addParameter(arg);
    }
    return function;
}

TIntermFunctionDefinition *MakeFunctionDefinition(const TFunction *function, TIntermBlock *body)
{
    return new TIntermFunctionDefinition(new TIntermFunctionPrototype(function), body);
}

TIntermFunctionDefinition *MakeSimpleFunctionDefinition(TSymbolTable *symbolTable,
                                                        const char *name,
                                                        TIntermTyped *returnExpression,
                                                        const TVector<TIntermSymbol *> &args)
{
    TVector<const TVariable *> argsAsVar;
    for (TIntermSymbol *arg : args)
    {
        argsAsVar.push_back(&arg->variable());
    }

    TIntermBlock *body = new TIntermBlock;
    body->appendStatement(new TIntermBranch(EOpReturn, returnExpression));

    const TFunction *function =
        MakeFunction(symbolTable, name, &returnExpression->getType(), argsAsVar);
    return MakeFunctionDefinition(function, body);
}

void Builder::generateBlendFunctions()
{
    const TPrecision precision = mOutputVar->getType().getPrecision();

    TType *floatParamType = new TType(EbtFloat, precision, EvqParamIn, 1);
    TType *vec3ParamType  = new TType(EbtFloat, precision, EvqParamIn, 3);

    gl::BlendEquationBitSet enabledBlendEquations(mAdvancedBlendEquations.bits());
    for (gl::BlendEquationType equation : enabledBlendEquations)
    {
        switch (equation)
        {
            case gl::BlendEquationType::Multiply:
                // float ANGLE_blend_multiply(float src, float dst)
                // {
                //     return src * dst;
                // }
                {
                    TIntermSymbol *src = MakeVariable(mSymbolTable, "src", floatParamType);
                    TIntermSymbol *dst = MakeVariable(mSymbolTable, "dst", floatParamType);

                    // src * dst
                    TIntermTyped *result = new TIntermBinary(EOpMul, src, dst);

                    mBlendFuncs[equation] = MakeSimpleFunctionDefinition(
                        mSymbolTable, "ANGLE_blend_multiply", result, {src, dst});
                }
                break;
            case gl::BlendEquationType::Screen:
                // float ANGLE_blend_screen(float src, float dst)
                // {
                //     return src + dst - src * dst;
                // }
                {
                    TIntermSymbol *src = MakeVariable(mSymbolTable, "src", floatParamType);
                    TIntermSymbol *dst = MakeVariable(mSymbolTable, "dst", floatParamType);

                    // src + dst
                    TIntermTyped *sum = new TIntermBinary(EOpAdd, src, dst);
                    // src * dst
                    TIntermTyped *mul = new TIntermBinary(EOpMul, src->deepCopy(), dst->deepCopy());
                    // src + dst - src * dst
                    TIntermTyped *result = new TIntermBinary(EOpSub, sum, mul);

                    mBlendFuncs[equation] = MakeSimpleFunctionDefinition(
                        mSymbolTable, "ANGLE_blend_screen", result, {src, dst});
                }
                break;
            case gl::BlendEquationType::Overlay:
            case gl::BlendEquationType::Hardlight:
                // float ANGLE_blend_overlay(float src, float dst)
                // {
                //     if (dst <= 0.5f)
                //     {
                //         return (2.0f * src * dst);
                //     }
                //     else
                //     {
                //         return (1.0f - 2.0f * (1.0f - src) * (1.0f - dst));
                //     }
                //
                //     // Equivalently generated as:
                //     // return dst <= 0.5f ? 2.*src*dst : 2.*(src+dst) - 2.*src*dst - 1.;
                // }
                //
                // float ANGLE_blend_hardlight(float src, float dst)
                // {
                //     // Same as overlay, with the |if| checking |src| instead of |dst|.
                // }
                {
                    TIntermSymbol *src = MakeVariable(mSymbolTable, "src", floatParamType);
                    TIntermSymbol *dst = MakeVariable(mSymbolTable, "dst", floatParamType);

                    // src + dst
                    TIntermTyped *sum = new TIntermBinary(EOpAdd, src, dst);
                    // 2 * (src + dst)
                    TIntermTyped *sum2 = new TIntermBinary(EOpMul, sum, Float(2));
                    // src * dst
                    TIntermTyped *mul = new TIntermBinary(EOpMul, src->deepCopy(), dst->deepCopy());
                    // 2 * src * dst
                    TIntermTyped *mul2 = new TIntermBinary(EOpMul, mul, Float(2));
                    // 2 * (src + dst) - 2 * src * dst
                    TIntermTyped *sum2MinusMul2 = new TIntermBinary(EOpSub, sum2, mul2);
                    // 2 * (src + dst) - 2 * src * dst - 1
                    TIntermTyped *sum2MinusMul2Minus1 =
                        new TIntermBinary(EOpSub, sum2MinusMul2, Float(1));

                    // dst[src] <= 0.5
                    TIntermSymbol *conditionSymbol =
                        equation == gl::BlendEquationType::Overlay ? dst : src;
                    TIntermTyped *lessHalf = new TIntermBinary(
                        EOpLessThanEqual, conditionSymbol->deepCopy(), Float(0.5));
                    // dst[src] <= 0.5f ? ...
                    TIntermTyped *result =
                        new TIntermTernary(lessHalf, mul2->deepCopy(), sum2MinusMul2Minus1);

                    mBlendFuncs[equation] = MakeSimpleFunctionDefinition(
                        mSymbolTable,
                        equation == gl::BlendEquationType::Overlay ? "ANGLE_blend_overlay"
                                                                   : "ANGLE_blend_hardlight",
                        result, {src, dst});
                }
                break;
            case gl::BlendEquationType::Darken:
                // float ANGLE_blend_darken(float src, float dst)
                // {
                //     return min(src, dst);
                // }
                {
                    TIntermSymbol *src = MakeVariable(mSymbolTable, "src", floatParamType);
                    TIntermSymbol *dst = MakeVariable(mSymbolTable, "dst", floatParamType);

                    // src * dst
                    TIntermSequence minArgs = {src, dst};
                    TIntermTyped *result =
                        CreateBuiltInFunctionCallNode("min", &minArgs, *mSymbolTable, 100);

                    mBlendFuncs[equation] = MakeSimpleFunctionDefinition(
                        mSymbolTable, "ANGLE_blend_darken", result, {src, dst});
                }
                break;
            case gl::BlendEquationType::Lighten:
                // float ANGLE_blend_lighten(float src, float dst)
                // {
                //     return max(src, dst);
                // }
                {
                    TIntermSymbol *src = MakeVariable(mSymbolTable, "src", floatParamType);
                    TIntermSymbol *dst = MakeVariable(mSymbolTable, "dst", floatParamType);

                    // src * dst
                    TIntermSequence maxArgs = {src, dst};
                    TIntermTyped *result =
                        CreateBuiltInFunctionCallNode("max", &maxArgs, *mSymbolTable, 100);

                    mBlendFuncs[equation] = MakeSimpleFunctionDefinition(
                        mSymbolTable, "ANGLE_blend_lighten", result, {src, dst});
                }
                break;
            case gl::BlendEquationType::Colordodge:
                // float ANGLE_blend_dodge(float src, float dst)
                // {
                //     if (dst <= 0.0f)
                //     {
                //         return 0.0;
                //     }
                //     else if (src >= 1.0f)   // dst > 0.0
                //     {
                //         return 1.0;
                //     }
                //     else                    // dst > 0.0 && src < 1.0
                //     {
                //         return min(1.0, dst / (1.0 - src));
                //     }
                //
                //     // Equivalently generated as:
                //     // return dst <= 0. ? 0. : src >= 1. ? 1. : min(1., dst / (1. - src));
                // }
                {
                    TIntermSymbol *src = MakeVariable(mSymbolTable, "src", floatParamType);
                    TIntermSymbol *dst = MakeVariable(mSymbolTable, "dst", floatParamType);

                    // 1. - src
                    TIntermTyped *oneMinusSrc = new TIntermBinary(EOpSub, Float(1), src);
                    // dst / (1. - src)
                    TIntermTyped *dstDivOneMinusSrc = new TIntermBinary(EOpDiv, dst, oneMinusSrc);
                    // min(1., dst / (1. - src))
                    TIntermSequence minArgs = {Float(1), dstDivOneMinusSrc};
                    TIntermTyped *result =
                        CreateBuiltInFunctionCallNode("min", &minArgs, *mSymbolTable, 100);

                    // src >= 1
                    TIntermTyped *greaterOne =
                        new TIntermBinary(EOpGreaterThanEqual, src->deepCopy(), Float(1));
                    // src >= 1. ? ...
                    result = new TIntermTernary(greaterOne, Float(1), result);

                    // dst <= 0
                    TIntermTyped *lessZero =
                        new TIntermBinary(EOpLessThanEqual, dst->deepCopy(), Float(0));
                    // dst <= 0. ? ...
                    result = new TIntermTernary(lessZero, Float(0), result);

                    mBlendFuncs[equation] = MakeSimpleFunctionDefinition(
                        mSymbolTable, "ANGLE_blend_dodge", result, {src, dst});
                }
                break;
            case gl::BlendEquationType::Colorburn:
                // float ANGLE_blend_burn(float src, float dst)
                // {
                //     if (dst >= 1.0f)
                //     {
                //         return 1.0;
                //     }
                //     else if (src <= 0.0f)   // dst < 1.0
                //     {
                //         return 0.0;
                //     }
                //     else                    // dst < 1.0 && src > 0.0
                //     {
                //         return 1.0f - min(1.0f, (1.0f - dst) / src);
                //     }
                //
                //     // Equivalently generated as:
                //     // return dst >= 1. ? 1. : src <= 0. ? 0. : 1. - min(1., (1. - dst) / src);
                // }
                {
                    TIntermSymbol *src = MakeVariable(mSymbolTable, "src", floatParamType);
                    TIntermSymbol *dst = MakeVariable(mSymbolTable, "dst", floatParamType);

                    // 1. - dst
                    TIntermTyped *oneMinusDst = new TIntermBinary(EOpSub, Float(1), dst);
                    // (1. - dst) / src
                    TIntermTyped *oneMinusDstDivSrc = new TIntermBinary(EOpDiv, oneMinusDst, src);
                    // min(1., (1. - dst) / src)
                    TIntermSequence minArgs = {Float(1), oneMinusDstDivSrc};
                    TIntermTyped *result =
                        CreateBuiltInFunctionCallNode("min", &minArgs, *mSymbolTable, 100);
                    // 1. - min(1., (1. - dst) / src)
                    result = new TIntermBinary(EOpSub, Float(1), result);

                    // src <= 0
                    TIntermTyped *lessZero =
                        new TIntermBinary(EOpLessThanEqual, src->deepCopy(), Float(0));
                    // src <= 0. ? ...
                    result = new TIntermTernary(lessZero, Float(0), result);

                    // dst >= 1
                    TIntermTyped *greaterOne =
                        new TIntermBinary(EOpGreaterThanEqual, dst->deepCopy(), Float(1));
                    // dst >= 1. ? ...
                    result = new TIntermTernary(greaterOne, Float(1), result);

                    mBlendFuncs[equation] = MakeSimpleFunctionDefinition(
                        mSymbolTable, "ANGLE_blend_burn", result, {src, dst});
                }
                break;
            case gl::BlendEquationType::Softlight:
                // float ANGLE_blend_softlight(float src, float dst)
                // {
                //     if (src <= 0.5f)
                //     {
                //         return (dst - (1.0f - 2.0f * src) * dst * (1.0f - dst));
                //     }
                //     else if (dst <= 0.25f)  // src > 0.5
                //     {
                //         return (dst + (2.0f * src - 1.0f) * dst * ((16.0f * dst - 12.0f) * dst
                //         + 3.0f));
                //     }
                //     else                    // src > 0.5 && dst > 0.25
                //     {
                //         return (dst + (2.0f * src - 1.0f) * (sqrt(dst) - dst));
                //     }
                //
                //     // Equivalently generated as:
                //     // return dst + (2. * src - 1.) * (
                //     //            src <= 0.5  ? dst * (1. - dst) :
                //     //            dst <= 0.25 ? dst * ((16. * dst - 12.) * dst + 3.) :
                //     //                          sqrt(dst) - dst)
                // }
                {
                    TIntermSymbol *src = MakeVariable(mSymbolTable, "src", floatParamType);
                    TIntermSymbol *dst = MakeVariable(mSymbolTable, "dst", floatParamType);

                    // 2. * src
                    TIntermTyped *src2 = new TIntermBinary(EOpMul, Float(2), src);
                    // 2. * src - 1.
                    TIntermTyped *src2Minus1 = new TIntermBinary(EOpSub, src2, Float(1));
                    // 1. - dst
                    TIntermTyped *oneMinusDst = new TIntermBinary(EOpSub, Float(1), dst);
                    // dst * (1. - dst)
                    TIntermTyped *dstTimesOneMinusDst =
                        new TIntermBinary(EOpMul, dst->deepCopy(), oneMinusDst);
                    // 16. * dst
                    TIntermTyped *dst16 = new TIntermBinary(EOpMul, Float(16), dst->deepCopy());
                    // 16. * dst - 12.
                    TIntermTyped *dst16Minus12 = new TIntermBinary(EOpSub, dst16, Float(12));
                    // (16. * dst - 12.) * dst
                    TIntermTyped *dst16Minus12TimesDst =
                        new TIntermBinary(EOpMul, dst16Minus12, dst->deepCopy());
                    // (16. * dst - 12.) * dst + 3.
                    TIntermTyped *dst16Minus12TimesDstPlus3 =
                        new TIntermBinary(EOpAdd, dst16Minus12TimesDst, Float(3));
                    // dst * ((16. * dst - 12.) * dst + 3.)
                    TIntermTyped *dstTimesDst16Minus12TimesDstPlus3 =
                        new TIntermBinary(EOpMul, dst->deepCopy(), dst16Minus12TimesDstPlus3);
                    // sqrt(dst)
                    TIntermSequence sqrtArg = {dst->deepCopy()};
                    TIntermTyped *sqrtDst =
                        CreateBuiltInFunctionCallNode("sqrt", &sqrtArg, *mSymbolTable, 100);
                    // sqrt(dst) - dst
                    TIntermTyped *sqrtDstMinusDst =
                        new TIntermBinary(EOpSub, sqrtDst, dst->deepCopy());

                    // dst <= 0.25
                    TIntermTyped *lessQuarter =
                        new TIntermBinary(EOpLessThanEqual, dst->deepCopy(), Float(0.25));
                    // dst <= 0.25 ? ...
                    TIntermTyped *result = new TIntermTernary(
                        lessQuarter, dstTimesDst16Minus12TimesDstPlus3, sqrtDstMinusDst);

                    // src <= 0.5
                    TIntermTyped *lessHalf =
                        new TIntermBinary(EOpLessThanEqual, src->deepCopy(), Float(0.5));
                    // src <= 0.5 ? ...
                    result = new TIntermTernary(lessHalf, dstTimesOneMinusDst, result);

                    // (2. * src - 1.) * ...
                    result = new TIntermBinary(EOpMul, src2Minus1, result);
                    // dst + (2. * src - 1.) * ...
                    result = new TIntermBinary(EOpAdd, dst->deepCopy(), result);

                    mBlendFuncs[equation] = MakeSimpleFunctionDefinition(
                        mSymbolTable, "ANGLE_blend_softlight", result, {src, dst});
                }
                break;
            case gl::BlendEquationType::Difference:
                // float ANGLE_blend_difference(float src, float dst)
                // {
                //     return abs(dst - src);
                // }
                {
                    TIntermSymbol *src = MakeVariable(mSymbolTable, "src", floatParamType);
                    TIntermSymbol *dst = MakeVariable(mSymbolTable, "dst", floatParamType);

                    // dst - src
                    TIntermTyped *dstMinusSrc = new TIntermBinary(EOpSub, dst, src);
                    // abs(dst - src)
                    TIntermSequence absArgs = {dstMinusSrc};
                    TIntermTyped *result =
                        CreateBuiltInFunctionCallNode("abs", &absArgs, *mSymbolTable, 100);

                    mBlendFuncs[equation] = MakeSimpleFunctionDefinition(
                        mSymbolTable, "ANGLE_blend_difference", result, {src, dst});
                }
                break;
            case gl::BlendEquationType::Exclusion:
                // float ANGLE_blend_exclusion(float src, float dst)
                // {
                //     return src + dst - (2.0f * src * dst);
                // }
                {
                    TIntermSymbol *src = MakeVariable(mSymbolTable, "src", floatParamType);
                    TIntermSymbol *dst = MakeVariable(mSymbolTable, "dst", floatParamType);

                    // src + dst
                    TIntermTyped *sum = new TIntermBinary(EOpAdd, src, dst);
                    // src * dst
                    TIntermTyped *mul = new TIntermBinary(EOpMul, src->deepCopy(), dst->deepCopy());
                    // 2 * src * dst
                    TIntermTyped *mul2 = new TIntermBinary(EOpMul, mul, Float(2));
                    // src + dst - 2 * src * dst
                    TIntermTyped *result = new TIntermBinary(EOpSub, sum, mul2);

                    mBlendFuncs[equation] = MakeSimpleFunctionDefinition(
                        mSymbolTable, "ANGLE_blend_exclusion", result, {src, dst});
                }
                break;
            case gl::BlendEquationType::HslHue:
                // vec3 ANGLE_blend_hsl_hue(vec3 src, vec3 dst)
                // {
                //     return ANGLE_set_lum_sat(src, dst, dst);
                // }
                {
                    TIntermSymbol *src = MakeVariable(mSymbolTable, "src", vec3ParamType);
                    TIntermSymbol *dst = MakeVariable(mSymbolTable, "dst", vec3ParamType);

                    TIntermSequence args = {src, dst, dst->deepCopy()};
                    TIntermTyped *result =
                        TIntermAggregate::CreateFunctionCall(*mSetLumSat->getFunction(), &args);

                    mBlendFuncs[equation] = MakeSimpleFunctionDefinition(
                        mSymbolTable, "ANGLE_blend_hsl_hue", result, {src, dst});
                }
                break;
            case gl::BlendEquationType::HslSaturation:
                // vec3 ANGLE_blend_hsl_saturation(vec3 src, vec3 dst)
                // {
                //     return ANGLE_set_lum_sat(dst, src, dst);
                // }
                {
                    TIntermSymbol *src = MakeVariable(mSymbolTable, "src", vec3ParamType);
                    TIntermSymbol *dst = MakeVariable(mSymbolTable, "dst", vec3ParamType);

                    TIntermSequence args = {dst, src, dst->deepCopy()};
                    TIntermTyped *result =
                        TIntermAggregate::CreateFunctionCall(*mSetLumSat->getFunction(), &args);

                    mBlendFuncs[equation] = MakeSimpleFunctionDefinition(
                        mSymbolTable, "ANGLE_blend_hsl_saturation", result, {src, dst});
                }
                break;
            case gl::BlendEquationType::HslColor:
                // vec3 ANGLE_blend_hsl_color(vec3 src, vec3 dst)
                // {
                //     return ANGLE_set_lum(src, dst);
                // }
                {
                    TIntermSymbol *src = MakeVariable(mSymbolTable, "src", vec3ParamType);
                    TIntermSymbol *dst = MakeVariable(mSymbolTable, "dst", vec3ParamType);

                    TIntermSequence args = {src, dst};
                    TIntermTyped *result =
                        TIntermAggregate::CreateFunctionCall(*mSetLum->getFunction(), &args);

                    mBlendFuncs[equation] = MakeSimpleFunctionDefinition(
                        mSymbolTable, "ANGLE_blend_hsl_color", result, {src, dst});
                }
                break;
            case gl::BlendEquationType::HslLuminosity:
                // vec3 ANGLE_blend_hsl_luminosity(vec3 src, vec3 dst)
                // {
                //     return ANGLE_set_lum(dst, src);
                // }
                {
                    TIntermSymbol *src = MakeVariable(mSymbolTable, "src", vec3ParamType);
                    TIntermSymbol *dst = MakeVariable(mSymbolTable, "dst", vec3ParamType);

                    TIntermSequence args = {dst, src};
                    TIntermTyped *result =
                        TIntermAggregate::CreateFunctionCall(*mSetLum->getFunction(), &args);

                    mBlendFuncs[equation] = MakeSimpleFunctionDefinition(
                        mSymbolTable, "ANGLE_blend_hsl_luminosity", result, {src, dst});
                }
                break;
            default:
                // Only advanced blend equations are possible.
                UNREACHABLE();
        }
    }
}

void Builder::insertGeneratedFunctions(TIntermBlock *root)
{
    // Insert all generated functions in root.  Since they are all inserted at index 0, HSL helpers
    // are inserted last, and in opposite order.
    for (TIntermFunctionDefinition *blendFunc : mBlendFuncs)
    {
        if (blendFunc != nullptr)
        {
            root->insertStatement(0, blendFunc);
        }
    }
    if (mMinv3 != nullptr)
    {
        root->insertStatement(0, mSetLumSat);
        root->insertStatement(0, mSetLum);
        root->insertStatement(0, mClipColor);
        root->insertStatement(0, mSatv3);
        root->insertStatement(0, mLumv3);
        root->insertStatement(0, mMaxv3);
        root->insertStatement(0, mMinv3);
    }
}

// On some platforms 1.0f is not returned even when the dividend and divisor have the same value.
// In such cases emit 1.0f when the dividend and divisor are equal, else return the divide node
TIntermTyped *Builder::divideFloatNode(TIntermTyped *dividend, TIntermTyped *divisor)
{
    TIntermBinary *cond = new TIntermBinary(EOpEqual, dividend->deepCopy(), divisor->deepCopy());
    TIntermBinary *divideExpr =
        new TIntermBinary(EOpDiv, dividend->deepCopy(), divisor->deepCopy());
    return new TIntermTernary(cond, CreateFloatNode(1.0f, EbpHigh), divideExpr->deepCopy());
}

TIntermSymbol *Builder::premultiplyAlpha(TIntermBlock *opBlock, TIntermTyped *var, const char *name)
{
    const TPrecision precision = mOutputVar->getType().getPrecision();
    TType *vec3Type            = new TType(EbtFloat, precision, EvqTemporary, 3);

    // symbol = vec3(0)
    // If alpha != 0 assign symbol based on precisionSafeDivision compile option.
    TIntermTyped *alpha            = new TIntermSwizzle(var, {3});
    TIntermSymbol *symbol          = MakeVariable(mSymbolTable, name, vec3Type);
    TIntermTyped *alphaNotZero     = new TIntermBinary(EOpNotEqual, alpha, Float(0));
    TIntermBlock *rgbDivAlphaBlock = new TIntermBlock;

    constexpr int kColorChannels = 3;
    if (mCompileOptions.precisionSafeDivision)
    {
        // For each component:
        // symbol.x = (var.x == var.w) ? 1.0 : var.x / var.w
        for (int index = 0; index < kColorChannels; index++)
        {
            TIntermTyped *divideNode = divideFloatNode(new TIntermSwizzle(var, {index}), alpha);
            TIntermBinary *assignDivideNode = new TIntermBinary(
                EOpAssign, new TIntermSwizzle(symbol->deepCopy(), {index}), divideNode);
            rgbDivAlphaBlock->appendStatement(assignDivideNode);
        }
    }
    else
    {
        // symbol = rgb/alpha
        TIntermTyped *rgb         = new TIntermSwizzle(var->deepCopy(), {0, 1, 2});
        TIntermTyped *rgbDivAlpha = new TIntermBinary(EOpDiv, rgb, alpha->deepCopy());
        rgbDivAlphaBlock->appendStatement(
            new TIntermBinary(EOpAssign, symbol->deepCopy(), rgbDivAlpha));
    }

    TIntermIfElse *ifBlock = new TIntermIfElse(alphaNotZero, rgbDivAlphaBlock, nullptr);
    opBlock->appendStatement(
        CreateTempInitDeclarationNode(&symbol->variable(), CreateZeroNode(*vec3Type)));
    opBlock->appendStatement(ifBlock);

    return symbol;
}

TIntermTyped *GetFirstElementIfArray(TIntermTyped *var)
{
    TIntermTyped *element = var;
    while (element->getType().isArray())
    {
        element = new TIntermBinary(EOpIndexDirect, element, CreateIndexNode(0));
    }
    return element;
}

void Builder::generatePreamble(TIntermBlock *opBlock)
{
    TIntermTyped *output = new TIntermSymbol(mOutputVar);

    // Use subpassLoad to read from the input attachment
    const TPrecision precision      = mOutputVar->getType().getPrecision();
    TType *vec4Type                 = new TType(EbtFloat, precision, EvqTemporary, 4);
    TIntermSymbol *subpassInputData = MakeVariable(mSymbolTable, "ANGLELastFragData", vec4Type);

    // Initialize it with subpassLoad() result.
    TIntermSequence subpassArguments  = {new TIntermSymbol(mSubpassInputVar)};
    TIntermTyped *subpassLoadFuncCall = CreateBuiltInFunctionCallNode(
        "subpassLoad", &subpassArguments, *mSymbolTable, kESSLInternalBackendBuiltIns);

    opBlock->appendStatement(
        CreateTempInitDeclarationNode(&subpassInputData->variable(), subpassLoadFuncCall));

    // Get the color attachment channel widths.  This is packed in 16 bits, 4 for each channel.
    TIntermTyped *channelWidth = mDriverUniforms->getLogicOpChannelWidth();
    const TVariable *channelWidthVar =
        CreateTempVariable(mSymbolTable, StaticType::GetBasic<EbtUInt, EbpMedium>());
    opBlock->appendStatement(CreateTempInitDeclarationNode(channelWidthVar, channelWidth));

    channelWidth = new TIntermSymbol(channelWidthVar);

    // Unpack into uvec4
    const TType *uvec4Type = StaticType::GetBasic<EbtUInt, EbpMedium, 4>();

    TIntermSequence channelWidthArgs = {
        new TIntermBinary(EOpBitwiseAnd, channelWidth, CreateUIntNode(0xF)),
        new TIntermBinary(
            EOpBitwiseAnd,
            new TIntermBinary(EOpBitShiftRight, channelWidth->deepCopy(), CreateUIntNode(4)),
            CreateUIntNode(0xF)),
        new TIntermBinary(
            EOpBitwiseAnd,
            new TIntermBinary(EOpBitShiftRight, channelWidth->deepCopy(), CreateUIntNode(8)),
            CreateUIntNode(0xF)),
        new TIntermBinary(
            EOpBitwiseAnd,
            new TIntermBinary(EOpBitShiftRight, channelWidth->deepCopy(), CreateUIntNode(12)),
            CreateUIntNode(0xF)),
    };
    channelWidth = TIntermAggregate::CreateConstructor(*uvec4Type, &channelWidthArgs);

    // Generate (1u << width) - 1 to create the multiplier
    TIntermTyped *one = CreateUVecNode({1, 1, 1, 1}, 4, EbpMedium);
    channelMultiplier = new TIntermBinary(EOpBitShiftLeft, one, channelWidthArgs);
    channelMultiplier = new TIntermBinary(EOpSub, channelMultiplier, one->deepCopy());

    const TVariable *channelMultiplierVar = CreateTempVariable(mSymbolTable, uvec4Type);
    opBlock->appendStatement(
        CreateTempInitDeclarationNode(channelMultiplierVar, channelMultiplier));
    mChannelMultiplier = new TIntermSybmol(channelMultiplierVar);

    // Unnormalize source and destination
    TIntermSequence channelMultiplierArgs = {
        channelMultiplier,
    };
    channelMultiplier = TIntermAggregate::CreateConstructor(
        *StaticType::GetBasic<EbtFloat, EbpMedium, 4>(), &channelMultiplierArgs);

    mSrc = MakeVariable(mSymbolTable, "logicOpSrc", uvec4Type);
    mDst = MakeVariable(mSymbolTable, "logicOpDst", uvec4Type);

    // uvec4((src * multiplier) + 0.5);
    output                  = new TIntermBinary(EOpMul, output, channelMultiplier);
    output                  = new TIntermBinary(EOpAdd, output, CreateFloatNode(0.49, EbpMedium));
    TIntermSequence srcArgs = {
        output,
    };
    output = TIntermAggregate::CreateConstructor(uvec4Type, &srcArgs);
    opBlock->appendStatement(CreateTempInitDeclarationNode(mSrc->variable(), output));

    // uvec4((src * multiplier) + 0.5);
    // TODO
}

void Builder::generateLogicOpSwitches(TIntermBlock *opBlock)
{
    const TPrecision precision = mOutputVar->getType().getPrecision();

    TType *vec3Type = new TType(EbtFloat, precision, EvqTemporary, 3);
    TType *vec4Type = new TType(EbtFloat, precision, EvqTemporary, 4);

    // The following code is generated:
    //
    // vec3 f;
    // swtich (equation)
    // {
    //    case A:
    //       f = ANGLE_blend_a(..);
    //       break;
    //    case B:
    //       f = ANGLE_blend_b(..);
    //       break;
    //    ...
    // }
    //
    // vec3 rgb = f * p0 + src * p1 + dst * p2
    // float a = p0 + p1 + p2
    //
    // output = vec4(rgb, a);

    TIntermSymbol *f = MakeVariable(mSymbolTable, "ANGLE_f", vec3Type);
    opBlock->appendStatement(CreateTempDeclarationNode(&f->variable()));

    TIntermBlock *switchBody = new TIntermBlock;

    gl::BlendEquationBitSet enabledBlendEquations(mAdvancedBlendEquations.bits());
    for (gl::BlendEquationType equation : enabledBlendEquations)
    {
        switchBody->appendStatement(
            new TIntermCase(CreateUIntNode(static_cast<uint32_t>(equation))));

        // HSL equations call the blend function with all channels.  Non-HSL equations call it per
        // component.
        if (equation < gl::BlendEquationType::HslHue)
        {
            TIntermSequence constructorArgs;
            for (int channel = 0; channel < 3; ++channel)
            {
                TIntermTyped *srcChannel = new TIntermSwizzle(mSrc->deepCopy(), {channel});
                TIntermTyped *dstChannel = new TIntermSwizzle(mDst->deepCopy(), {channel});

                TIntermSequence args = {srcChannel, dstChannel};
                constructorArgs.push_back(TIntermAggregate::CreateFunctionCall(
                    *mBlendFuncs[equation]->getFunction(), &args));
            }

            TIntermTyped *constructor =
                TIntermAggregate::CreateConstructor(*vec3Type, &constructorArgs);
            switchBody->appendStatement(new TIntermBinary(EOpAssign, f->deepCopy(), constructor));
        }
        else
        {
            TIntermSequence args = {mSrc->deepCopy(), mDst->deepCopy()};
            TIntermTyped *blendCall =
                TIntermAggregate::CreateFunctionCall(*mBlendFuncs[equation]->getFunction(), &args);

            switchBody->appendStatement(new TIntermBinary(EOpAssign, f->deepCopy(), blendCall));
        }

        switchBody->appendStatement(new TIntermBranch(EOpBreak, nullptr));
    }

    // A driver uniform is used to communicate the blend equation to use.
    TIntermTyped *equationUniform = mDriverUniforms->getAdvancedBlendEquation();

    opBlock->appendStatement(new TIntermSwitch(equationUniform, switchBody));

    // Calculate the final blend according to the following formula:
    //
    //     RGB = f(src, dst) * p0 + src * p1 + dst * p2
    //       A = p0 + p1 + p2

    // f * p0
    TIntermTyped *fTimesP0 = new TIntermBinary(EOpVectorTimesScalar, f, mP0);
    // src * p1
    TIntermTyped *srcTimesP1 = new TIntermBinary(EOpVectorTimesScalar, mSrc, mP1);
    // dst * p2
    TIntermTyped *dstTimesP2 = new TIntermBinary(EOpVectorTimesScalar, mDst, mP2);
    // f * p0 + src * p1 + dst * p2
    TIntermTyped *rgb =
        new TIntermBinary(EOpAdd, new TIntermBinary(EOpAdd, fTimesP0, srcTimesP1), dstTimesP2);

    // p0 + p1 + p2
    TIntermTyped *a = new TIntermBinary(
        EOpAdd, new TIntermBinary(EOpAdd, mP0->deepCopy(), mP1->deepCopy()), mP2->deepCopy());

    // Intialize the output with vec4(RGB, A)
    TIntermSequence rgbaArgs  = {rgb, a};
    TIntermTyped *blendResult = TIntermAggregate::CreateConstructor(*vec4Type, &rgbaArgs);

    // If the output has fewer than four channels, swizzle the results
    uint32_t vecSize = mOutputVar->getType().getNominalSize();
    if (vecSize < 4)
    {
        TVector<int> swizzle = {0, 1, 2, 3};
        swizzle.resize(vecSize);
        blendResult = new TIntermSwizzle(blendResult, swizzle);
    }

    TIntermTyped *output = GetFirstElementIfArray(new TIntermSymbol(mOutputVar));

    opBlock->appendStatement(new TIntermBinary(EOpAssign, output, blendResult));
}
}  // anonymous namespace

bool EmulateLogicOp(TCompiler *compiler,
                    const ShCompileOptions &compileOptions,
                    TIntermBlock *root,
                    TSymbolTable *symbolTable,
                    const DriverUniform *driverUniforms,
                    std::vector<ShaderVariable> *uniforms)
{
    Builder builder(compiler, compileOptions, symbolTable, driverUniforms, uniforms);
    return builder.build(root);
}  // namespace

}  // namespace sh
