//
// Copyright 2021 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//
// ReplaceForAdvancedBlendEquation.h: Find any blend equation, replace it with the corresponding
// equation function and add the operation for the blending job.
//

#include "compiler/translator/tree_ops/vulkan/ReplaceForAdvancedBlendEquation.h"

#include <map>

#include "GLSLANG/ShaderVars.h"
#include "common/PackedEnums.h"
#include "compiler/translator/InfoSink.h"
#include "compiler/translator/SymbolTable.h"
#include "compiler/translator/tree_util/DriverUniform.h"
#include "compiler/translator/tree_util/FindMain.h"
#include "compiler/translator/tree_util/IntermNode_util.h"
#include "compiler/translator/tree_util/IntermTraverse.h"
#include "compiler/translator/tree_util/RunAtTheBeginningOfShader.h"
#include "compiler/translator/tree_util/RunAtTheEndOfShader.h"
#include "compiler/translator/tree_util/SpecializationConstant.h"

namespace sh
{
namespace
{

struct MatchedTIntermTyped
{
    const TType &type;
    TIntermTyped *output;
    TIntermTyped *source;
};

using MatchedTypeWithLocMap = std::map<int, MatchedTIntermTyped>;

static constexpr const int kRedChannelIndex   = 0;
static constexpr const int kGreenChannelIndex = 1;
static constexpr const int kBlueChannelIndex  = 2;
static constexpr const int kAlphaChannelIndex = 3;

static constexpr const int kNormalBlendFuncChannel = 1;
static constexpr const int kHslBlendFuncChannel    = 3;

static const char *kColorCoeffVarName       = "coeff_color";
static const char *kAlphaCoeffVarName       = "coeff_alpha";
static const char *kSourceVarName           = "src";
static const char *kDestinationVarName      = "dst";
static const char *kOutputVarName           = "blend_result";
static const char *kPremultipliedSrcVarName = "premul_src";
static const char *kPremultipliedDstVarName = "premul_dst";
static const char *kBlendMainFuncName       = "main";

struct BlendEquationMap
{
    gl::BlendEquationType blendEquationType;
    const char *funcName;
    const char *dataType;
    const char *funcBody;
};

// clang-format off
static constexpr struct BlendEquationMap kBlendEquationMap[] = {
    {gl::BlendEquationType::Multiply, "multiply", "float",
     "    return (src * dst);"},
    {gl::BlendEquationType::Screen, "screen", "float",
     "    return (src + dst - (src * dst));"},
    {gl::BlendEquationType::Overlay, "overlay", "float",
     R"(if (dst <= 0.5f)
    {
        return (2.0f * src * dst);
    }
    else
    {
        return (1.0f - 2.0f * (1.0f - src) * (1.0f - dst));
    })"},
    {gl::BlendEquationType::Darken, "darken", "float",
     "return min(src, dst);"},
    {gl::BlendEquationType::Lighten, "lighten", "float",
     "return max(src, dst);"},
    {gl::BlendEquationType::Colordodge, "colordodge", "float",
     R"(if (dst <= 0.0f)
    {
        return 0.0;
    }
    else if (src >= 1.0f)   // dst > 0.0
    {
        return 1.0;
    }
    else                    // dst > 0.0 && src < 1.0
    {
        return min(1.0, dst / (1.0 - src));
    })"},
    {gl::BlendEquationType::Colorburn, "colorburn", "float",
     R"(if (dst >= 1.0f)
    {
        return 1.0f;
    }
    else if (src <= 0.0f)   // dst < 1.0f
    {
        return 0.0f;
    }
    else                    // dst < 1.0f && src > 0.0f
    {
        return (1.0f - min(1.0f, (1.0f - dst) / src));
    })"},
    {gl::BlendEquationType::Hardlight, "hardlight", "float",
    R"(if (src <= 0.5f)
    {
        return (2.0f * src * dst);
    }
    else
    {
        return (1.0f - 2.0f * (1.0f - src) * (1.0f - dst));
    })"},
    {gl::BlendEquationType::Softlight, "softlight", "float",
    R"(if (src <= 0.5f)
    {
        return (dst - (1.0f - 2.0f * src) * dst * (1.0f - dst));
    }
    else if (dst <= 0.25f)  // src > 0.5
    {
        return (dst + (2.0f * src - 1.0f) * dst * ((16.0f * dst - 12.0f) * dst + 3.0f));
    }
    else                    // src > 0.5 && dst > 0.25
    {
        return (dst + (2.0f * src - 1.0f) * (sqrt(dst) - dst));
    })"},
    {gl::BlendEquationType::Difference, "difference", "float",
     "    return abs(dst - src);"},
    {gl::BlendEquationType::Exclusion, "exclusion", "float",
     "    return (src + dst - (2.0f * src * dst));"},
    {gl::BlendEquationType::HslHue, "hsl_hue", "vec3",
     "    return angle_adv_blend_eqn_set_lum_sat(src, dst, dst);"},
    {gl::BlendEquationType::HslSaturation, "hsl_saturation", "vec3",
     "    return angle_adv_blend_eqn_set_lum_sat(dst, src, dst);"},
    {gl::BlendEquationType::HslColor, "hsl_color", "vec3",
     "    return angle_adv_blend_eqn_set_lum(src, dst);"},
    {gl::BlendEquationType::HslLuminosity, "hsl_luminosity", "vec3",
     "    return angle_adv_blend_eqn_set_lum(dst, src);"}
};
// clang-format on

TVector<int> CreateSwizzleIndexList(int swizzleSize)
{
    TVector<int> result(swizzleSize);
    for (int index = 0; index < swizzleSize; index++)
    {
        result[index] = index;
    }
    return result;
}

std::string GetBlendFunctionName(const std::string &equationName)
{
    TInfoSinkBase nameSink;
    nameSink << "angle_adv_blend_eqn_" << equationName;
    return nameSink.str().c_str();
}

std::string GetBlendVariableName(const std::string &variableName)
{
    TInfoSinkBase nameSink;
    nameSink << "angle_adv_blend_var_" << variableName;
    return nameSink.str();
}

std::string GetInputAttachmentName(unsigned int index)
{
    TInfoSinkBase nameSink;
    nameSink << sh::vk::kInputAttachmentName << index;
    return nameSink.str();
}

std::string GetBlendFuncString(const std::string &funcName,
                               const std::string &dataType,
                               const std::string &funcBody)
{
    // mediump <dataType> <funcName>(mediump <dataType> src, mediump <dataType> dst)
    // {
    //     <funcBody>
    // }

    TInfoSinkBase function;

    function << "mediump " << dataType << " " << GetBlendFunctionName(funcName) << "(mediump "
             << dataType << " src, mediump " << dataType << " dst)\n"
             << "{\n"
             << funcBody << "\n"
             << "}\n";

    return function.c_str();
}

std::string GetHslParamsFuncString(const std::string &funcName,
                                   const std::string &returnType,
                                   const std::string &params,
                                   const std::string &funcBody)
{
    // mediump <returnType> <funcName>(<params>)
    // {
    //     <funcBody>
    // }

    TInfoSinkBase function;

    function << "mediump " << returnType << " " << GetBlendFunctionName(funcName) << "(" << params
             << ")\n"
             << "{\n"
             << funcBody << "\n"
             << "}\n";

    return function.c_str();
}

std::string GetHslRelatedFuncString()
{
    // clang-format off
    // mediump float angle_adv_blend_eqn_minv3(mediump vec3 c)
    // {
    //     return min(min(c.r, c.g), c.b);
    // }
    // mediump float angle_adv_blend_eqn_maxv3(mediump vec3 c)
    // {
    //     return max(max(c.r, c.g), c.b);
    // }
    // mediump float angle_adv_blend_eqn_lumv3(mediump vec3 c)
    // {
    //     return dot(c, vec3(0.30f, 0.59f, 0.11f));
    // }
    // mediump float angle_adv_blend_eqn_satv3(mediump vec3 c)
    // {
    //     return angle_adv_blend_eqn_maxv3(c) - angle_adv_blend_eqn_minv3(c);
    // }
    // mediump vec3 angle_adv_blend_eqn_clip_color(mediump vec3 color)
    // {
    //     mediump float lum = angle_adv_blend_eqn_lumv3(color);
    //     mediump float mincol = angle_adv_blend_eqn_minv3(color);
    //     mediump float maxcol = angle_adv_blend_eqn_maxv3(color);
    //     if (mincol < 0.0f)
    //     {
    //         color = lum + ((color - lum) * lum) / (lum - mincol);
    //     }
    //     if (maxcol > 1.0f + kEpsilon)
    //     {
    //         color = lum + ((color - lum) * (1.0f - lum)) / (maxcol - lum);
    //     }
    //     return color;
    // }
    // mediump vec3 angle_adv_blend_eqn_set_lum(mediump vec3 cbase, mediump vec3 clum)
    // {
    //     mediump float lbase = angle_adv_blend_eqn_lumv3(cbase);
    //     mediump float llum = angle_adv_blend_eqn_lumv3(clum);
    //     mediump float ldiff = llum - lbase;
    //     mediump vec3 color = cbase + vec3(ldiff);
    //     return angle_adv_blend_eqn_clip_color(color);
    // }
    // mediump vec3 angle_adv_blend_eqn_set_lum_sat(mediump vec3 cbase, mediump vec3 csat, mediump vec3 clum)
    // {
    //     mediump float minbase = angle_adv_blend_eqn_minv3(cbase);
    //     mediump float sbase = angle_adv_blend_eqn_satv3(cbase);
    //     mediump float ssat = angle_adv_blend_eqn_satv3(csat);
    //     mediump vec3 color;
    //     if (sbase > 0.0f)
    //     {
    //         color = (cbase - minbase) * ssat / sbase;
    //     }
    //     else
    //     {
    //         color = vec3(0.0f);
    //     }
    //     return angle_adv_blend_eqn_set_lum(color, clum);
    // }
    // clang-format on
    TInfoSinkBase functions;

    const char *vec3ColorParam         = "mediump vec3 color";
    const char *vec3CbaseClumParam     = "mediump vec3 cbase, mediump vec3 clum";
    const char *vec3CbaseCsatClumParam = "mediump vec3 cbase, mediump vec3 csat, mediump vec3 clum";

    // minv3
    const char *minv3Char = "    return min(min(color.r, color.g), color.b);";
    functions << GetHslParamsFuncString("minv3", "float", vec3ColorParam, minv3Char);

    // maxv3
    const char *maxv3Char = "    return max(max(color.r, color.g), color.b);";
    functions << GetHslParamsFuncString("maxv3", "float", vec3ColorParam, maxv3Char);

    // lumv3
    const char *lumv3Char = "    return dot(color, vec3(0.30f, 0.59f, 0.11f));";
    functions << GetHslParamsFuncString("lumv3", "float", vec3ColorParam, lumv3Char);

    // satv3
    const char *satv3Char =
        "    return angle_adv_blend_eqn_maxv3(color) - angle_adv_blend_eqn_minv3(color);";
    functions << GetHslParamsFuncString("satv3", "float", vec3ColorParam, satv3Char);

    // clip_color
    const char *clipColorChar =
        R"(mediump float lum = angle_adv_blend_eqn_lumv3(color);
           mediump float mincol = angle_adv_blend_eqn_minv3(color);
           mediump float maxcol = angle_adv_blend_eqn_maxv3(color);
           if (mincol < 0.0f)
           {
               color = lum + ((color - lum) * lum) / (lum - mincol);
           }
           if (maxcol > 1.0f)
           {
               color = lum + ((color - lum) * (1.0f - lum)) / (maxcol - lum);
           }
           return color;)";
    functions << GetHslParamsFuncString("clip_color", "vec3", vec3ColorParam, clipColorChar);

    // set_lum
    const char *setLumChar =
        R"(mediump float lbase = angle_adv_blend_eqn_lumv3(cbase);
           mediump float llum = angle_adv_blend_eqn_lumv3(clum);
           mediump float ldiff = llum - lbase;
           mediump vec3 color = cbase + vec3(ldiff);
           return angle_adv_blend_eqn_clip_color(color);)";
    functions << GetHslParamsFuncString("set_lum", "vec3", vec3CbaseClumParam, setLumChar);

    // set_lum_sat
    const char *setLumSatChar =
        R"(mediump float minbase = angle_adv_blend_eqn_minv3(cbase);
           mediump float sbase = angle_adv_blend_eqn_satv3(cbase);
           mediump float ssat = angle_adv_blend_eqn_satv3(csat);
           mediump vec3 color;
           if (sbase > 0.0f)
           {
               color = (cbase - minbase) * ssat / sbase;
           }
           else
           {
               color = vec3(0.0f);
           }
           return angle_adv_blend_eqn_set_lum(color, clum);)";
    functions << GetHslParamsFuncString("set_lum_sat", "vec3", vec3CbaseCsatClumParam,
                                        setLumSatChar);

    return functions.c_str();
}

TFunction *CreateBlendFunction(TSymbolTable *symbolTable,
                               const ImmutableString &functionName,
                               const TType *returnType,
                               const std::vector<const TVariable *> &args)
{
    TFunction *newFunction =
        new TFunction(symbolTable, functionName, SymbolType::AngleInternal, returnType, false);
    for (const TVariable *arg : args)
    {
        newFunction->addParameter(arg);
    }
    return newFunction;
}

class OutputVariableTraverser : public TIntermTraverser
{
  public:
    OutputVariableTraverser(MatchedTypeWithLocMap *outputSymbolList)
        : TIntermTraverser(true, false, false), mOutputSymbolList(outputSymbolList)
    {
        mOutputSymbolList->clear();
    }

    bool visitDeclaration(Visit visit, TIntermDeclaration *node) override;
    bool visitAggregate(Visit visit, TIntermAggregate *node) override;

  private:
    MatchedTypeWithLocMap *mOutputSymbolList;
};

bool OutputVariableTraverser::visitDeclaration(Visit visit, TIntermDeclaration *node)
{
    const TIntermSequence &sequence = *(node->getSequence());

    if (sequence.size() != 1)
    {
        return true;
    }

    TIntermTyped *variable = sequence.front()->getAsTyped();
    TIntermSymbol *symbol  = variable->getAsSymbolNode();
    if (symbol == nullptr)
    {
        return true;
    }

    if (symbol->getType().getQualifier() == EvqFragmentOut ||
        symbol->getType().getQualifier() == EvqFragmentInOut)
    {
        const TType &symbolType = symbol->getType();

        // OpenGL ES 3.2 spec:
        //
        // > 15.1.5 Blending
        // > ...
        // > Blending applies only if the color buffer has a fixed-point or floating-point format.
        // > ...
        //
        if (symbolType.getBasicType() == EbtFloat)
        {
            int outputLocation = symbol->getType().getLayoutQualifier().location;
            if (mOutputSymbolList->find(outputLocation) == mOutputSymbolList->end())
            {
                if (!symbol->isArray())
                {
                    MatchedTIntermTyped outSrcMatched = {symbolType, symbol, nullptr};
                    mOutputSymbolList->emplace(outputLocation, outSrcMatched);
                }
                else
                {
                    size_t arraySize = symbol->getOutermostArraySize();
                    for (size_t arrayIndex = 0; arrayIndex < arraySize; arrayIndex++)
                    {
                        TIntermTyped *indexedSymbol = new TIntermBinary(
                            EOpIndexDirect, symbol->deepCopy(), CreateIndexNode(arrayIndex));
                        MatchedTIntermTyped outSrcMatched = {symbolType, indexedSymbol, nullptr};
                        mOutputSymbolList->emplace(outputLocation + arrayIndex, outSrcMatched);
                    }
                }
            }
        }
    }

    return true;
}

bool OutputVariableTraverser::visitAggregate(Visit visit, TIntermAggregate *node)
{
    // Find out the subpassLoad call which is created by the gl_LastFragData or the inout variable
    const TFunction *subpassLoadFunc = node->getFunction();
    if (!subpassLoadFunc || (subpassLoadFunc->symbolType() == SymbolType::BuiltIn &&
                             subpassLoadFunc->name().contains("subpassLoad") == false))
    {
        return true;
    }

    TIntermSymbol *inputAttachment = node->getSequence()->back()->getAsSymbolNode();
    ASSERT(inputAttachment);

    int inputAttachmentLocation =
        inputAttachment->getType().getLayoutQualifier().inputAttachmentIndex;
    ASSERT(mOutputSymbolList->find(inputAttachmentLocation) != mOutputSymbolList->end());

    MatchedTIntermTyped &outTypePair = mOutputSymbolList->at(inputAttachmentLocation);
    outTypePair.source               = node;

    return true;
}

TIntermSymbol *CreateInputAttachmentVar(TIntermBlock *root,
                                        TSymbolTable *symbolTable,
                                        const int inputAttachmentIndex)
{
    TType *inputAttachmentType = new TType(TBasicType::EbtSubpassInput, EbpUndefined, EvqUniform);
    TLayoutQualifier inputAttachmentQualifier     = inputAttachmentType->getLayoutQualifier();
    inputAttachmentQualifier.inputAttachmentIndex = inputAttachmentIndex;
    inputAttachmentType->setLayoutQualifier(inputAttachmentQualifier);

    TVariable *newInputAttachmentVar =
        new TVariable(symbolTable, ImmutableString(GetInputAttachmentName(inputAttachmentIndex)),
                      inputAttachmentType, SymbolType::AngleInternal);
    TIntermSymbol *inputAttachmentSymbol = new TIntermSymbol(newInputAttachmentVar);

    TIntermDeclaration *inputAttachmentDecl = new TIntermDeclaration;
    inputAttachmentDecl->appendDeclarator(inputAttachmentSymbol->deepCopy());
    root->insertStatement(0, inputAttachmentDecl);

    return inputAttachmentSymbol;
}

void AddShaderVariableForInputAttachment(std::vector<ShaderVariable> *uniforms,
                                         const ImmutableString &name,
                                         const uint32_t inputAttachmentIndex)
{
    ShaderVariable inputAttachmentUniform;
    inputAttachmentUniform.active    = true;
    inputAttachmentUniform.staticUse = true;
    inputAttachmentUniform.name.assign(name.data(), name.length());
    inputAttachmentUniform.mappedName.assign(inputAttachmentUniform.name);
    inputAttachmentUniform.isFragmentInOut = true;
    inputAttachmentUniform.location        = inputAttachmentIndex;
    uniforms->push_back(inputAttachmentUniform);
}

TIntermTyped *CallSubpassLoadFunc(TSymbolTable *symbolTable, TIntermSymbol *inputAttachmentSymbol)
{
    TIntermSequence arguments = TIntermSequence();
    arguments.push_back(inputAttachmentSymbol->deepCopy());
    return CreateBuiltInFunctionCallNode("subpassLoad", &arguments, *symbolTable, kESSLVulkanOnly);
}

TIntermSymbol *GetSourceColorForBlend(TCompiler *compiler,
                                      TIntermBlock *root,
                                      TSymbolTable *symbolTable,
                                      std::vector<ShaderVariable> *uniforms,
                                      const TIntermTyped *subpassLoadCall,
                                      const TType &outputType,
                                      const int inputAttachmentIndex)
{
    //
    // layout(location=<output_location>) uniform subpassInput ANGLEInputAttachment<output_number>;
    // ..
    // <output_type> angle_adv_blend_var_src = subpassLoad(subpassInput).<output_nominal_size>
    //
    TInfoSinkBase srcString;
    srcString << GetBlendVariableName("src") << "_" << inputAttachmentIndex;

    TType *srcColorType =
        new TType(outputType.getBasicType(), outputType.getPrecision(), EvqTemporary,
                  static_cast<unsigned char>(outputType.getNominalSize()));
    TVariable *srcColorVar        = new TVariable(symbolTable, ImmutableString(srcString.str()),
                                           srcColorType, SymbolType::AngleInternal);
    TIntermSymbol *srcColorSymbol = new TIntermSymbol(srcColorVar);

    TIntermTyped *inputAttachmentData;
    if (subpassLoadCall)
    {
        // subpassLoadCall : subpassLoad(subpassInput).xyzw
        inputAttachmentData = subpassLoadCall->deepCopy();
    }
    else
    {
        TIntermSymbol *inputAttachmentSymbol =
            CreateInputAttachmentVar(root, symbolTable, inputAttachmentIndex);
        AddShaderVariableForInputAttachment(uniforms, inputAttachmentSymbol->getName(),
                                            inputAttachmentIndex);
        inputAttachmentData = CallSubpassLoadFunc(symbolTable, inputAttachmentSymbol);
    }

    TIntermSwizzle *initializeValue = new TIntermSwizzle(
        inputAttachmentData, CreateSwizzleIndexList(srcColorType->getNominalSize()));
    TIntermBinary *assignInputAttachmentColor =
        new TIntermBinary(EOpInitialize, srcColorSymbol->deepCopy(), initializeValue);
    TIntermDeclaration *srcColorDecl = new TIntermDeclaration;
    srcColorDecl->appendDeclarator(assignInputAttachmentColor);
    if (!RunAtTheBeginningOfShader(compiler, root, srcColorDecl))
    {
        return nullptr;
    }
    return srcColorSymbol;
}

TIntermTyped *DivideFloatNode(TIntermTyped *dividend, TIntermTyped *divisor)
{
    // If dividend and divisor is same, the return node is 1.0f, else the divide node is returned.
    // In some cases, 1.0f is not generated even though the dividend and divisor is same.
    TIntermBinary *cond = new TIntermBinary(EOpEqual, dividend->deepCopy(), divisor->deepCopy());
    TIntermBinary *divideExpr =
        new TIntermBinary(EOpDiv, dividend->deepCopy(), divisor->deepCopy());
    return new TIntermTernary(cond, CreateFloatNode(1.0f, EbpHigh), divideExpr->deepCopy());
}

TIntermSymbol *GetPremultipliedValue(TSymbolTable *symbolTable,
                                     TIntermBlock *mainBlock,
                                     TIntermTyped *origVar,
                                     const ImmutableString &premulName)
{
    //
    // vec3 premultipledVar = origVar.rgb;
    // if (origVar.a  == 0.0f)
    // {
    //     premultipledVar = vec3(0.0f);
    // }
    // else
    // {
    //     premultipledVar.r = (origVar.r == origVar.a ? 1.0f : origVar.r / origVar.a);
    //     premultipledVar.g = (origVar.g == origVar.a ? 1.0f : origVar.g / origVar.a);
    //     premultipledVar.b = (origVar.b == origVar.a ? 1.0f : origVar.b / origVar.a);
    // }
    //
    TIntermSwizzle *origVarRGB =
        CreateSwizzle(origVar->deepCopy(), kRedChannelIndex, kGreenChannelIndex, kBlueChannelIndex);
    TIntermSwizzle *origVarA = CreateSwizzle(origVar->deepCopy(), kAlphaChannelIndex);

    TType *threeComponentVectorType = new TType(TBasicType::EbtFloat, EbpMedium, EvqTemporary, 3);
    TVariable *premulVar =
        new TVariable(symbolTable, premulName, threeComponentVectorType, SymbolType::AngleInternal);
    TIntermSymbol *premulSymbol = new TIntermSymbol(premulVar);

    TIntermBinary *premulInitializer =
        new TIntermBinary(EOpInitialize, premulSymbol->deepCopy(), origVarRGB->deepCopy());
    TIntermDeclaration *premulDecl = new TIntermDeclaration();
    premulDecl->appendDeclarator(premulInitializer);
    mainBlock->appendStatement(premulDecl);

    TIntermBinary *isBlendAlphaZero =
        new TIntermBinary(EOpEqual, origVarA->deepCopy(), CreateFloatNode(0.0f, EbpHigh));
    TIntermBlock *ifBlock               = new TIntermBlock();
    TIntermBinary *assignZeroToBlendSrc = new TIntermBinary(
        EOpAssign, premulSymbol->deepCopy(), CreateZeroNode(*threeComponentVectorType));
    ifBlock->appendStatement(assignZeroToBlendSrc);

    TIntermBlock *elseBlock = new TIntermBlock();
    for (size_t swizzleIndex = kRedChannelIndex; swizzleIndex <= kBlueChannelIndex; swizzleIndex++)
    {
        TIntermTyped *divideNode =
            DivideFloatNode(CreateSwizzle(origVar->deepCopy(), swizzleIndex), origVarA->deepCopy());
        TIntermBinary *assignDivideNode =
            new TIntermBinary(EOpAssign, CreateSwizzle(premulSymbol->deepCopy(), swizzleIndex),
                              divideNode->deepCopy());
        elseBlock->appendStatement(assignDivideNode);
    }

    TIntermIfElse *calcPremultiplied = new TIntermIfElse(isBlendAlphaZero, ifBlock, elseBlock);
    mainBlock->appendStatement(calcPremultiplied);

    return premulSymbol;
}

TIntermSymbol *CreateCoeffVar(TSymbolTable *symbolTable,
                              TIntermBlock *mainBody,
                              const ImmutableString &coeffName)
{
    TType *coeffType = new TType(TBasicType::EbtFloat, EbpMedium, EvqTemporary, 3);
    TVariable *coeffVar =
        new TVariable(symbolTable, coeffName, coeffType, SymbolType::AngleInternal);
    TIntermSymbol *coeffSymbol    = new TIntermSymbol(coeffVar);
    TIntermDeclaration *coeffDecl = new TIntermDeclaration();
    coeffDecl->appendDeclarator(coeffSymbol->deepCopy());
    mainBody->appendStatement(coeffDecl);

    return coeffSymbol;
}

TIntermBinary *CalcBlendedColorComponent(TIntermTyped *blendedResult,
                                         TIntermTyped *premulSrc,
                                         TIntermTyped *premulDst,
                                         TIntermTyped *colorCoeff,
                                         TIntermTyped *alphaCoeff)
{

    TIntermBinary *firstTerm = new TIntermBinary(
        EOpMul, colorCoeff->deepCopy(), CreateSwizzle(alphaCoeff->deepCopy(), kRedChannelIndex));
    TIntermBinary *secondTerm = new TIntermBinary(
        EOpMul, premulSrc->deepCopy(), CreateSwizzle(alphaCoeff->deepCopy(), kGreenChannelIndex));
    TIntermBinary *thirdTerm = new TIntermBinary(
        EOpMul, premulDst->deepCopy(), CreateSwizzle(alphaCoeff->deepCopy(), kBlueChannelIndex));

    TIntermBinary *assignBlendedColorComponent = new TIntermBinary(
        EOpAssign, blendedResult->deepCopy(),
        new TIntermBinary(EOpAdd, new TIntermBinary(EOpAdd, firstTerm, secondTerm), thirdTerm));

    return assignBlendedColorComponent;
}

class BlendMainFunctionBuilder
{
  public:
    BlendMainFunctionBuilder(TIntermBlock *root,
                             TSymbolTable *symbolTable,
                             SpecConst *specConst,
                             const DriverUniform *driverUniforms)
        : mRoot(root),
          mSymbolTable(symbolTable),
          mSpecConst(specConst),
          mDriverUniforms(driverUniforms)
    {}

    sh::TFunction *createBlendMainFunction();
    void addCaseStatement(const TIntermSequence &sequence)
    {
        for (auto node = sequence.begin(); node != sequence.end(); node++)
        {
            mCaseBlock->appendStatement(*node);
        }
    }

    const TIntermSymbol *getSrcVarable() { return mFirstVar; }
    TIntermSymbol *getColorCoeff() { return mColorCoeff; }
    TIntermSymbol *getPremulSrc() { return mPremulSrc; }
    TIntermSymbol *getPremulDst() { return mPremulDst; }

  private:
    void createFunction();
    void calcAlphaCoeff();
    void getSwtichCaseBlock();
    void calcBlendedColor();

    TIntermSymbol *mFirstVar;
    TIntermSymbol *mSecondVar;
    TFunction *mFunction;
    TIntermBlock *mMainBlock;

    TIntermSymbol *mPremulSrc;
    TIntermSymbol *mPremulDst;

    TIntermSymbol *mColorCoeff;

    TIntermSymbol *mAlphaCoeff;
    TIntermSwizzle *mAlphaCoeffX;
    TIntermSwizzle *mAlphaCoeffY;
    TIntermSwizzle *mAlphaCoeffZ;

    TIntermBlock *mCaseBlock;

    TIntermSymbol *mBlendedColor;

    TIntermBlock *mRoot;
    TSymbolTable *mSymbolTable;
    SpecConst *mSpecConst;
    const DriverUniform *mDriverUniforms;
};

TFunction *BlendMainFunctionBuilder::createBlendMainFunction()
{
    // Create the prototype and definition of the blend main function
    createFunction();
    if (mFunction == nullptr)
    {
        return nullptr;
    }

    // Get premultiplied source and destination color value
    mPremulSrc =
        GetPremultipliedValue(mSymbolTable, mMainBlock, mFirstVar->deepCopy(),
                              ImmutableString(GetBlendVariableName(kPremultipliedSrcVarName)));
    mPremulDst =
        GetPremultipliedValue(mSymbolTable, mMainBlock, mSecondVar->deepCopy(),
                              ImmutableString(GetBlendVariableName(kPremultipliedDstVarName)));

    // Get coefficient variables
    mColorCoeff = CreateCoeffVar(mSymbolTable, mMainBlock,
                                 ImmutableString(GetBlendVariableName(kColorCoeffVarName)));

    calcAlphaCoeff();

    // Calc alpha coefficient value
    getSwtichCaseBlock();

    // Calc blended color value
    TIntermDeclaration *blendedColorDecl = new TIntermDeclaration();
    blendedColorDecl->appendDeclarator(mBlendedColor);
    mMainBlock->appendStatement(blendedColorDecl);

    calcBlendedColor();

    // Return blended color value
    TIntermBranch *returnBlendMain = new TIntermBranch(EOpReturn, mBlendedColor->deepCopy());
    mMainBlock->appendStatement(returnBlendMain);

    return mFunction;
}

void BlendMainFunctionBuilder::createFunction()
{
    size_t positionOfMainFunction = FindMainIndex(mRoot);
    if (positionOfMainFunction == std::numeric_limits<size_t>::max())
    {
        return;
    }

    TType *funcSrcType = new TType(TBasicType::EbtFloat, EbpMedium, EvqParamIn, 4);
    TVariable *firstVar =
        new TVariable(mSymbolTable, ImmutableString(GetBlendVariableName(kSourceVarName)),
                      funcSrcType, SymbolType::AngleInternal);
    mFirstVar = new TIntermSymbol(firstVar);

    TVariable *secondVar =
        new TVariable(mSymbolTable, ImmutableString(GetBlendVariableName(kDestinationVarName)),
                      funcSrcType, SymbolType::AngleInternal);
    mSecondVar = new TIntermSymbol(secondVar);

    TType *funcOutType = new TType(TBasicType::EbtFloat, EbpMedium, EvqTemporary, 4);
    TVariable *outVar =
        new TVariable(mSymbolTable, ImmutableString(GetBlendVariableName(kOutputVarName)),
                      funcOutType, SymbolType::AngleInternal);
    mBlendedColor = new TIntermSymbol(outVar);

    std::vector<const TVariable *> blendMainArgs;
    blendMainArgs.push_back(firstVar);
    blendMainArgs.push_back(secondVar);

    mFunction =
        CreateBlendFunction(mSymbolTable, ImmutableString(GetBlendFunctionName(kBlendMainFuncName)),
                            funcOutType, blendMainArgs);

    TIntermFunctionPrototype *prototype   = new TIntermFunctionPrototype(mFunction);
    mMainBlock                            = new TIntermBlock();
    TIntermFunctionDefinition *definition = new TIntermFunctionDefinition(prototype, mMainBlock);

    mRoot->insertStatement(positionOfMainFunction, definition);
}

void BlendMainFunctionBuilder::calcAlphaCoeff()
{
    mAlphaCoeff = CreateCoeffVar(mSymbolTable, mMainBlock,
                                 ImmutableString(GetBlendVariableName(kAlphaCoeffVarName)));

    mAlphaCoeffX = CreateSwizzle(mAlphaCoeff->deepCopy(), kRedChannelIndex);
    mAlphaCoeffY = CreateSwizzle(mAlphaCoeff->deepCopy(), kGreenChannelIndex);
    mAlphaCoeffZ = CreateSwizzle(mAlphaCoeff->deepCopy(), kBlueChannelIndex);

    // mAlphaCoeff.x = srcAlpha.a * dstAlpha.a;
    TIntermSwizzle *srcAlpha = CreateSwizzle(mFirstVar->deepCopy(), kAlphaChannelIndex);
    TIntermSwizzle *dstAlpha = CreateSwizzle(mSecondVar->deepCopy(), kAlphaChannelIndex);
    TIntermBinary *setAlphaCoeffX =
        new TIntermBinary(EOpAssign, mAlphaCoeffX->deepCopy(),
                          new TIntermBinary(EOpMul, srcAlpha->deepCopy(), dstAlpha->deepCopy()));
    mMainBlock->appendStatement(setAlphaCoeffX);

    // mAlphaCoeff.y = srcAlpha.a * (1 - dstAlpha.a);
    TIntermBinary *oneMinusDstAlpha =
        new TIntermBinary(EOpSub, CreateFloatNode(1.0f, EbpHigh), dstAlpha->deepCopy());
    TIntermBinary *setAlphaCoeffY =
        new TIntermBinary(EOpAssign, mAlphaCoeffY->deepCopy(),
                          new TIntermBinary(EOpMul, srcAlpha->deepCopy(), oneMinusDstAlpha));
    mMainBlock->appendStatement(setAlphaCoeffY);

    // mAlphaCoeff.z = (1 - srcAlpha.a) * dstAlpha.a;
    TIntermBinary *oneMinusSrcAlpha =
        new TIntermBinary(EOpSub, CreateFloatNode(1.0, EbpHigh), srcAlpha->deepCopy());
    TIntermBinary *setAlphaCoeffZ =
        new TIntermBinary(EOpAssign, mAlphaCoeffZ->deepCopy(),
                          new TIntermBinary(EOpMul, oneMinusSrcAlpha, dstAlpha->deepCopy()));
    mMainBlock->appendStatement(setAlphaCoeffZ);
}

void BlendMainFunctionBuilder::getSwtichCaseBlock()
{
    // switch (ANGLEBlendEquation) / switch (ANGLEUniforms.blendEquation)
    // {
    //     < mCaseBlock >
    // }
    TIntermTyped *switchVar = mSpecConst->getBlendEquation();
    if (switchVar == nullptr)
    {
        switchVar = mDriverUniforms->getBlendEquationRef();
    }
    mCaseBlock                         = new TIntermBlock();
    TIntermSwitch *selectBlendFunction = new TIntermSwitch(switchVar, mCaseBlock);
    mMainBlock->appendStatement(selectBlendFunction);
}

void BlendMainFunctionBuilder::calcBlendedColor()
{
    // Calculate the blended red color
    // > blendedColor.r = colorCoeff.r * mAlphaCoeff.x + premulSrc.r * mAlphaCoeff.y + premulDst.r *
    // mAlphaCoeff.z;
    TIntermSwizzle *premulSrcR    = CreateSwizzle(mPremulSrc, kRedChannelIndex);
    TIntermSwizzle *premulDstR    = CreateSwizzle(mPremulDst, kRedChannelIndex);
    TIntermSwizzle *blendedColorR = CreateSwizzle(mBlendedColor->deepCopy(), kRedChannelIndex);
    TIntermSwizzle *colorCoeffR   = CreateSwizzle(mColorCoeff->deepCopy(), kRedChannelIndex);
    TIntermBinary *assignBlendedColorR =
        CalcBlendedColorComponent(blendedColorR, premulSrcR, premulDstR, colorCoeffR, mAlphaCoeff);
    mMainBlock->appendStatement(assignBlendedColorR);

    // Calculate the blended green color
    // > blendedColor.g = colorCoeff.g * mAlphaCoeff.x + premulSrc.g * mAlphaCoeff.y + premulDst.g *
    // mAlphaCoeff.z;
    TIntermSwizzle *premulSrcG    = CreateSwizzle(mPremulSrc, kGreenChannelIndex);
    TIntermSwizzle *premulDstG    = CreateSwizzle(mPremulDst, kGreenChannelIndex);
    TIntermSwizzle *blendedColorG = CreateSwizzle(mBlendedColor->deepCopy(), kGreenChannelIndex);
    TIntermSwizzle *colorCoeffG   = CreateSwizzle(mColorCoeff->deepCopy(), kGreenChannelIndex);
    TIntermBinary *assignBlendedColorG =
        CalcBlendedColorComponent(blendedColorG, premulSrcG, premulDstG, colorCoeffG, mAlphaCoeff);
    mMainBlock->appendStatement(assignBlendedColorG);

    // Calculate the blended blue color
    // > blendedColor.b = colorCoeff.b * mAlphaCoeff.x + premulSrc.b * mAlphaCoeff.y + premulDst.b *
    // mAlphaCoeff.z;
    TIntermSwizzle *premulSrcB    = CreateSwizzle(mPremulSrc, kBlueChannelIndex);
    TIntermSwizzle *premulDstB    = CreateSwizzle(mPremulDst, kBlueChannelIndex);
    TIntermSwizzle *blendedColorB = CreateSwizzle(mBlendedColor->deepCopy(), kBlueChannelIndex);
    TIntermSwizzle *colorCoeffB   = CreateSwizzle(mColorCoeff->deepCopy(), kBlueChannelIndex);
    TIntermBinary *assignBlendedColorB =
        CalcBlendedColorComponent(blendedColorB, premulSrcB, premulDstB, colorCoeffB, mAlphaCoeff);
    mMainBlock->appendStatement(assignBlendedColorB);

    // Calculate the blended alpha
    // > blendedColor.a =
    TIntermBinary *addAlphaCoeffGB =
        new TIntermBinary(EOpAdd, mAlphaCoeffY->deepCopy(), mAlphaCoeffZ->deepCopy());
    TIntermBinary *addAlphaCoeffRGB =
        new TIntermBinary(EOpAdd, mAlphaCoeffX->deepCopy(), addAlphaCoeffGB);
    TIntermSwizzle *blendedAlpha = CreateSwizzle(mBlendedColor->deepCopy(), kAlphaChannelIndex);
    TIntermBinary *assignBlendedAlpha =
        new TIntermBinary(EOpAssign, blendedAlpha, addAlphaCoeffRGB);
    mMainBlock->appendStatement(assignBlendedAlpha);
}

TFunction *CreateRawBlendFunction(TSymbolTable *symbolTable,
                                  const ImmutableString &funcName,
                                  const uint32_t vectorSize)
{
    TType *paramType = new TType(TBasicType::EbtFloat, EbpMedium, EvqTemporary,
                                 static_cast<unsigned char>(vectorSize));
    TVariable *srcParamVar =
        new TVariable(symbolTable, ImmutableString("src"), paramType, SymbolType::AngleInternal);
    TVariable *dstParamVar =
        new TVariable(symbolTable, ImmutableString("dst"), paramType, SymbolType::AngleInternal);

    std::vector<const TVariable *> args;
    args.push_back(srcParamVar);
    args.push_back(dstParamVar);

    return CreateBlendFunction(symbolTable, funcName, paramType, args);
}

TIntermAggregate *CallBlendFunc(const TFunction *function,
                                const TIntermTyped *srcVar,
                                const TIntermTyped *dstVar)
{
    TIntermSequence *params = new TIntermSequence();
    params->push_back(srcVar->deepCopy());
    params->push_back(dstVar->deepCopy());

    return TIntermAggregate::CreateRawFunctionCall(*function, params);
}

TIntermSequence *CreateCaseStatement(TSymbolTable *symbolTable,
                                     const ImmutableString &funcName,
                                     const uint32_t blendEquationNum,
                                     const sh::TLayoutBlendEquation blendEquation,
                                     TIntermSymbol *colorCoeff,
                                     TIntermSymbol *premulSrc,
                                     TIntermSymbol *premulDst)
{
    // 1. Case - Normal blend equation
    // > case <BlendEquationType_number>:
    // >     colorCoeff.r = angle_adv_blend_eqn_<func>(premulSrc.r, premulDst.r);
    // >     colorCoeff.g = angle_adv_blend_eqn_<func>(premulSrc.g, premulDst.g);
    // >     colorCoeff.b = angle_adv_blend_eqn_<func>(premulSrc.b, premulDst.b);
    // >     break;
    //
    // 2. Case - HSL-related blend equation
    // > case <BlendEquationType_number>:
    // >     colorCoeff = angle_adv_blend_eqn_<func>(premulSrc, premulDst);
    // >     break;
    TIntermSequence *caseSequence = new TIntermSequence();

    TIntermCase *caseNode = new TIntermCase(CreateUIntNode(blendEquationNum));
    caseSequence->push_back(caseNode);

    if (blendEquation < sh::TLayoutBlendEquation::HslHue)
    {
        // The HSL-not related function is operated as a per-component operation.
        TFunction *rawBlendFunction =
            CreateRawBlendFunction(symbolTable, funcName, kNormalBlendFuncChannel);

        for (size_t swizzleIndex = kRedChannelIndex; swizzleIndex < kAlphaChannelIndex;
             swizzleIndex++)
        {
            TIntermSwizzle *coeffComponent = CreateSwizzle(colorCoeff->deepCopy(), swizzleIndex);
            TIntermSwizzle *premulSrcComponent = CreateSwizzle(premulSrc->deepCopy(), swizzleIndex);
            TIntermSwizzle *premulDstComponent = CreateSwizzle(premulDst->deepCopy(), swizzleIndex);

            TIntermAggregate *callblendFunc =
                CallBlendFunc(rawBlendFunction, premulSrcComponent, premulDstComponent);
            TIntermBinary *assignResult =
                new TIntermBinary(EOpAssign, coeffComponent, callblendFunc);

            caseSequence->push_back(assignResult);
        }
    }
    else
    {
        // The HSL related function is operated as a vec3 operation
        TFunction *rawBlendFunction =
            CreateRawBlendFunction(symbolTable, funcName, kHslBlendFuncChannel);
        TIntermAggregate *callblendFunc = CallBlendFunc(rawBlendFunction, premulSrc, premulDst);

        TIntermBinary *assignResult =
            new TIntermBinary(EOpAssign, colorCoeff->deepCopy(), callblendFunc);
        caseSequence->push_back(assignResult);
    }

    TIntermBranch *breakNode = new TIntermBranch(EOpBreak, nullptr);
    caseSequence->push_back(breakNode);

    return caseSequence;
}

TIntermTyped *ExpandVarToVec4(TSymbolTable *symbolTable,
                              TIntermTyped *origSrc,
                              const size_t vectorSize)
{
    // The default value of alpha component is 1.0f. That of color component is 0.0f.
    constexpr const size_t kVec4NominalSize = 4;
    const size_t remainColorComponentCount  = kVec4NominalSize - vectorSize - 1;
    TIntermSequence constructorArgs;
    constructorArgs.push_back(
        new TIntermSwizzle(origSrc->deepCopy(), CreateSwizzleIndexList(vectorSize)));
    for (size_t remainComponent = 0; remainComponent < remainColorComponentCount; remainComponent++)
    {
        constructorArgs.push_back(CreateFloatNode(0.0f, EbpHigh));
    }
    constructorArgs.push_back(CreateFloatNode(1.0f, EbpHigh));

    TType *vec4Type = new TType(EbtFloat, EbpMedium, EvqTemporary, 4);
    return TIntermAggregate::CreateConstructor(*vec4Type, &constructorArgs);
}

}  // anonymous namespace

ANGLE_NO_DISCARD bool AddBlendMainCaller(TCompiler *compiler,
                                         TInfoSinkBase &sink,
                                         TIntermBlock *root,
                                         TSymbolTable *symbolTable,
                                         SpecConst *specConst,
                                         const DriverUniform *driverUniforms,
                                         std::vector<ShaderVariable> *uniforms,
                                         const AdvancedBlendEquation &blendEquation)
{
    // Find out the output variable, and the predefined subpassLoad function related to the output
    // variable. The predefined subpassLoad function can be created by the gl_LastFragData or the
    // inout type variable
    MatchedTypeWithLocMap outputVarList;
    OutputVariableTraverser outputTraverser(&outputVarList);
    root->traverse(&outputTraverser);
    ASSERT(outputVarList.size() > 0);

    // Get source colors by calling the subpassLoad function. If there is the predefined subpassLoad
    // function, that will be reused. Unless, a new input attachment variable and subpassLoad
    // function will be created.
    for (auto iter = outputVarList.begin(); iter != outputVarList.end(); iter++)
    {
        const int inputAttachmentIndex = iter->first < 0 ? 0 : iter->first;

        MatchedTIntermTyped &matchedOutSrc = iter->second;
        const TType &outputType            = matchedOutSrc.type;
        TIntermTyped **srcVar              = &matchedOutSrc.source;

        TIntermTyped *srcColor = GetSourceColorForBlend(compiler, root, symbolTable, uniforms,
                                                        *srcVar, outputType, inputAttachmentIndex);
        if (srcColor == nullptr)
        {
            return false;
        }

        // If the predefined subpassLoad function exists, that will be replaced to the source color
        // variable. If not, the previous value of the srcVar is nullptr.
        *srcVar = srcColor;
    }

    // Create and add the blend main function which is inserted at the bottom of the shader main
    // function.
    BlendMainFunctionBuilder mainFunctionBuilder(root, symbolTable, specConst, driverUniforms);
    TFunction *blendMainFunc = mainFunctionBuilder.createBlendMainFunction();
    if (blendMainFunc == nullptr)
    {
        return false;
    }

    // Add each blend function and add the related case-statement
    size_t blendEquationIndex                  = 0;
    AdvancedBlendEquation enabledAdvancedBlend = blendEquation;
    if (enabledAdvancedBlend.isAnyHslBlendEquation())
    {
        // If any HSL-related blend function is used, the utility function should be added.
        sink << GetHslRelatedFuncString() << "\n";
    }

    while (enabledAdvancedBlend.isAnyBlendEquation())
    {
        sh::TLayoutBlendEquation currentEquation =
            static_cast<sh::TLayoutBlendEquation>(blendEquationIndex);
        if (enabledAdvancedBlend.isEnabled(currentEquation))
        {
            // Add raw blend function
            const struct BlendEquationMap &currentIndexData = kBlendEquationMap[blendEquationIndex];
            sink << GetBlendFuncString(currentIndexData.funcName, currentIndexData.dataType,
                                       currentIndexData.funcBody);

            // Add case statement for calling the supported blend function
            const ImmutableString funcName(GetBlendFunctionName(currentIndexData.funcName));
            const uint32_t blendEquationNum =
                static_cast<uint32_t>(currentIndexData.blendEquationType);
            TIntermSequence *caseStatement = CreateCaseStatement(
                symbolTable, funcName, blendEquationNum, currentEquation,
                mainFunctionBuilder.getColorCoeff(), mainFunctionBuilder.getPremulSrc(),
                mainFunctionBuilder.getPremulDst());
            mainFunctionBuilder.addCaseStatement(*caseStatement);

            enabledAdvancedBlend.unset(currentEquation);
        }

        blendEquationIndex++;
    }

    // Add the default statementof the switch-case statement. The default statement will returns the
    // source color. Because the default statement is used if user doesn't use the advanced blending
    // equation even though the layout qualifier for the advanced blending is specified at the
    // fragment shader.
    TIntermSequence defaultCase;
    TIntermCase *defaultCaseNode = new TIntermCase(nullptr);
    defaultCase.push_back(defaultCaseNode);
    TIntermBranch *returnNode =
        new TIntermBranch(EOpReturn, mainFunctionBuilder.getSrcVarable()->deepCopy());
    defaultCase.push_back(returnNode);
    mainFunctionBuilder.addCaseStatement(defaultCase);

    // Assign each result of adv_blend_eqn_main to each color output. If the output variable is not
    // the four-component vector type, it will be expanded to the four-component vector type.
    for (auto iter : outputVarList)
    {
        MatchedTIntermTyped &matchedOutSrc = iter.second;
        const TType &outputType            = matchedOutSrc.type;
        TIntermTyped *srcVar               = matchedOutSrc.source;
        TIntermTyped *outVar               = matchedOutSrc.output;

        uint32_t vectorSize = outputType.getNominalSize();
        ASSERT(vectorSize <= 4);

        TIntermTyped *vec4Src = srcVar;
        TIntermTyped *vec4Dst = outVar;
        if (vectorSize < 4)
        {
            vec4Src = ExpandVarToVec4(symbolTable, srcVar, vectorSize);
            vec4Dst = ExpandVarToVec4(symbolTable, outVar, vectorSize);
        }

        TIntermTyped *blendResult = CallBlendFunc(blendMainFunc, vec4Dst, vec4Src);
        if (vectorSize < 4)
        {
            blendResult =
                new TIntermSwizzle(blendResult->deepCopy(), CreateSwizzleIndexList(vectorSize));
        }

        TIntermBinary *assignOutColor =
            new TIntermBinary(EOpAssign, outVar->deepCopy(), blendResult);
        if (!RunAtTheEndOfShader(compiler, root, assignOutColor, symbolTable))
        {
            return false;
        }
    }

    return compiler->validateAST(root);
}  // namespace

}  // namespace sh
