//
// Copyright 2022 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//
// EmulateYUVBuiltIns: Adds functions that emulate yuv_2_rgb and rgb_2_yuv built-ins.
//

#include "compiler/translator/tree_ops/spirv/EmulateYUVBuiltIns.h"

#include "compiler/translator/StaticType.h"
#include "compiler/translator/SymbolTable.h"
#include "compiler/translator/tree_util/IntermNode_util.h"
#include "compiler/translator/tree_util/IntermTraverse.h"

namespace sh
{
namespace
{
// A traverser that replaces the yuv built-ins with a function call that emulates it.
class EmulateYUVBuiltInsTraverser : public TIntermTraverser
{
  public:
    EmulateYUVBuiltInsTraverser(TSymbolTable *symbolTable)
        : TIntermTraverser(true, false, false, symbolTable)
    {}

    bool visitAggregate(Visit visit, TIntermAggregate *node) override;

    bool update(TCompiler *compiler, TIntermBlock *root);

  private:
    const TFunction *getYUV2RGBFunc(TPrecision precision);
    const TFunction *getRGB2YUVFunc(TPrecision precision);
    const TFunction *getYUVFunc(TPrecision precision,
                                const char *name,
                                TIntermTyped *itu601Matrix,
                                TIntermTyped *itu709Matrix,
                                TIntermFunctionDefinition **funcDefOut);

    TIntermTyped *replaceYUVFuncCall(TIntermTyped *node);

    // One emulation function for each sampler precision
    std::array<TIntermFunctionDefinition *, EbpLast> mYUV2RGBFuncDefs = {};
    std::array<TIntermFunctionDefinition *, EbpLast> mRGB2YUVFuncDefs = {};
};

bool EmulateYUVBuiltInsTraverser::visitAggregate(Visit visit, TIntermAggregate *node)
{
    TIntermTyped *replacement = replaceYUVFuncCall(node);

    if (replacement != nullptr)
    {
        queueReplacement(replacement, OriginalNode::IS_DROPPED);
        return false;
    }

    return true;
}

TIntermTyped *EmulateYUVBuiltInsTraverser::replaceYUVFuncCall(TIntermTyped *node)
{
    TIntermAggregate *asAggregate = node->getAsAggregate();
    if (asAggregate == nullptr)
    {
        return nullptr;
    }

    TOperator op = asAggregate->getOp();
    if (op != EOpYuv_2_rgb && op != EOpRgb_2_yuv)
    {
        return nullptr;
    }

    ASSERT(asAggregate->getChildCount() == 2);

    TIntermTyped *param0 = asAggregate->getChildNode(0)->getAsTyped();
    TPrecision precision = param0->getPrecision();
    if (precision == EbpUndefined)
    {
        precision = EbpMedium;
    }

    const TFunction *emulatedFunction =
        op == EOpYuv_2_rgb ? getYUV2RGBFunc(precision) : getRGB2YUVFunc(precision);

    // The first parameter of the built-ins (|color|) may itself contain a built-in call.  With
    // TIntermTraverser, if the direct children also needs to be replaced that needs to be done
    // while constructing this node as replacement doesn't work.
    TIntermTyped *param0Replacement = replaceYUVFuncCall(param0);

    if (param0Replacement == nullptr)
    {
        // If param0 is not directly a YUV built-in call, visit it recursively so YIV built-in call
        // sub expressions are replaced.
        param0->traverse(this);
        param0Replacement = param0;
    }

    // Create the function call
    TIntermSequence args = {
        param0Replacement,
        asAggregate->getChildNode(1),
    };
    return TIntermAggregate::CreateFunctionCall(*emulatedFunction, &args);
}

TIntermTyped *MakeMatrix(const std::array<float, 9> &elements)
{
    TIntermSequence matrix;
    for (float element : elements)
    {
        matrix.push_back(CreateFloatNode(element, EbpMedium));
    }

    const TType *matType = StaticType::GetBasic<EbtFloat, EbpMedium, 3, 3>();
    return TIntermAggregate::CreateConstructor(*matType, &matrix);
}

TIntermTyped *MakeVector(float x, float y, float z)
{
    TIntermSequence values;
    values.push_back(CreateFloatNode(x, EbpHigh));
    values.push_back(CreateFloatNode(y, EbpHigh));
    values.push_back(CreateFloatNode(z, EbpHigh));

    const TType *vecType = StaticType::GetBasic<EbtFloat, EbpHigh, 3>();
    return TIntermAggregate::CreateConstructor(*vecType, &values);
}

TIntermConstantUnion *CreateIntConstantNode(int i)
{
    TConstantUnion *constant = new TConstantUnion();
    constant->setIConst(i);
    return new TIntermConstantUnion(constant, TType(EbtInt, EbpHigh));
}

TIntermConstantUnion *CreateFloatConstantNode(float f)
{
    TConstantUnion *constant = new TConstantUnion();
    constant->setFConst(f);
    return new TIntermConstantUnion(constant, TType(EbtFloat, EbpHigh));
}

const TFunction *EmulateYUVBuiltInsTraverser::getYUV2RGBFunc(TPrecision precision)
{
    const char *name = "ANGLE_yuv_2_rgb";
    switch (precision)
    {
        case EbpLow:
            name = "ANGLE_yuv_2_rgb_lowp";
            break;
        case EbpMedium:
            name = "ANGLE_yuv_2_rgb_mediump";
            break;
        case EbpHigh:
            name = "ANGLE_yuv_2_rgb_highp";
            break;
        default:
            UNREACHABLE();
    }

    if (mYUV2RGBFuncDefs[precision] != nullptr)
    {
        return mYUV2RGBFuncDefs[precision]->getFunction();
    }

    // The function prototype is vec3 name(vec3 color, yuvCscStandardEXT conv_standard)
    TType *vec3Type = new TType(*StaticType::GetBasic<EbtFloat, EbpMedium, 3>());
    vec3Type->setPrecision(precision);
    const TType *yuvCscType = StaticType::GetBasic<EbtYuvCscStandardEXT, EbpUndefined>();

    TType *colorType = new TType(*vec3Type);
    TType *convType  = new TType(*yuvCscType);
    colorType->setQualifier(EvqParamIn);
    convType->setQualifier(EvqParamIn);

    TVariable *colorParam =
        new TVariable(mSymbolTable, ImmutableString("color"), colorType, SymbolType::AngleInternal);
    TVariable *convParam = new TVariable(mSymbolTable, ImmutableString("conv_standard"), convType,
                                         SymbolType::AngleInternal);

    TFunction *function = new TFunction(mSymbolTable, ImmutableString(name),
                                        SymbolType::AngleInternal, vec3Type, true);
    function->addParameter(colorParam);
    function->addParameter(convParam);

    TIntermBlock *body = new TIntermBlock;

    TConstantUnion *itu709 = new TConstantUnion;
    itu709->setYuvCscStandardEXTConst(EycsItu709);
    TConstantUnion *itu601FullRange = new TConstantUnion;
    itu601FullRange->setYuvCscStandardEXTConst(EycsItu601FullRange);

    TIntermDeclaration *kRDecl = nullptr;
    TVariable *kR = DeclareTempVariable(mSymbolTable,
            new TIntermTernary(
                new TIntermBinary(EOpEqual,
                    new TIntermSymbol(convParam),
                    new TIntermConstantUnion(itu709, *yuvCscType)),
                CreateFloatConstantNode(0.2126f),
                CreateFloatConstantNode(0.229f)),
            EvqTemporary,
            &kRDecl);
    body->appendStatement(kRDecl);

    TIntermDeclaration *kBDecl = nullptr;
    TVariable *kB = DeclareTempVariable(mSymbolTable,
            new TIntermTernary(
                new TIntermBinary(EOpEqual,
                    new TIntermSymbol(convParam),
                    new TIntermConstantUnion(itu709, *yuvCscType)),
                CreateFloatConstantNode(0.0722f),
                CreateFloatConstantNode(0.114f)),
            EvqTemporary,
            &kBDecl);
    body->appendStatement(kBDecl);

    TIntermDeclaration *expandedDecl = nullptr;
    TVariable *expanded = DeclareTempVariable(mSymbolTable,
            new TIntermTernary(
                new TIntermBinary(EOpEqual,
                    new TIntermSymbol(convParam),
                    new TIntermConstantUnion(itu601FullRange, *yuvCscType)),
                new TIntermBinary(EOpAdd, new TIntermSymbol(colorParam), MakeVector(0.f, -128/255.f, -128/255.f)),
                new TIntermBinary(EOpMul, MakeVector(255.f/219.f, 255.f/224.f, 255.f/224.f),
                        new TIntermBinary(EOpAdd, new TIntermSymbol(colorParam), MakeVector(-16.f/255.f, -128/255.f, -128/255.f)))),
            EvqTemporary,
            &expandedDecl);
    body->appendStatement(expandedDecl);

    TIntermBinary *y = new TIntermBinary(EOpIndexDirect, new TIntermSymbol(expanded), CreateIntConstantNode(0));
    TIntermBinary *cb = new TIntermBinary(EOpIndexDirect, new TIntermSymbol(expanded), CreateIntConstantNode(1));
    TIntermBinary *cr = new TIntermBinary(EOpIndexDirect, new TIntermSymbol(expanded), CreateIntConstantNode(2));

    // r = cr * (1 - kr) * 2 + y
    TIntermDeclaration *outRedDecl = nullptr;
    TVariable *outRed = DeclareTempVariable(mSymbolTable,
            new TIntermBinary(EOpAdd, y->deepCopy(),
                new TIntermBinary(EOpMul, CreateFloatConstantNode(2.f),
                    new TIntermBinary(EOpMul,
                        cr, new TIntermBinary(EOpSub, CreateFloatConstantNode(1.f), new TIntermSymbol(kR))))),
            EvqTemporary,
            &outRedDecl);
    body->appendStatement(outRedDecl);

    // b = cb * (1 - kb) * 2 + y
    TIntermDeclaration *outBlueDecl = nullptr;
    TVariable *outBlue = DeclareTempVariable(mSymbolTable,
            new TIntermBinary(EOpAdd, y->deepCopy(),
                new TIntermBinary(EOpMul, CreateFloatConstantNode(2.f),
                    new TIntermBinary(EOpMul,
                        cb, new TIntermBinary(EOpSub, CreateFloatConstantNode(1.f), new TIntermSymbol(kB))))),
            EvqTemporary,
            &outBlueDecl);
    body->appendStatement(outBlueDecl);

    // g = (y - kr * r - kb * b) / (1 - kr - kb)
    TIntermDeclaration *outGreenDecl = nullptr;
    TVariable *outGreen = DeclareTempVariable(mSymbolTable,
            new TIntermBinary(EOpDiv,
                new TIntermBinary(EOpSub,       // y - kr*r - kb*b
                    new TIntermBinary(EOpSub,
                        y,
                        new TIntermBinary(EOpMul, new TIntermSymbol(kR), new TIntermSymbol(outRed))),
                    new TIntermBinary(EOpMul, new TIntermSymbol(kB), new TIntermSymbol(outBlue))),
                new TIntermBinary(EOpSub,       // 1 - kr - kb
                    new TIntermBinary(EOpSub,
                        CreateFloatConstantNode(1.f),
                        new TIntermSymbol(kR)),
                    new TIntermSymbol(kB))),
            EvqTemporary,
            &outGreenDecl);
    body->appendStatement(outGreenDecl);

    // return vec3(r,g,b)
    TIntermSequence resultValues;
    resultValues.push_back(new TIntermSymbol(outRed));
    resultValues.push_back(new TIntermSymbol(outGreen));
    resultValues.push_back(new TIntermSymbol(outBlue));
    body->appendStatement(new TIntermBranch(EOpReturn, 
                TIntermAggregate::CreateConstructor(*vec3Type, &resultValues)));

    mYUV2RGBFuncDefs[precision] = new TIntermFunctionDefinition(new TIntermFunctionPrototype(function), body);
    return function;
}

const TFunction *EmulateYUVBuiltInsTraverser::getRGB2YUVFunc(TPrecision precision)
{
    const char *name = "ANGLE_rgb_2_yuv";
    switch (precision)
    {
        case EbpLow:
            name = "ANGLE_rgb_2_yuv_lowp";
            break;
        case EbpMedium:
            name = "ANGLE_rgb_2_yuv_mediump";
            break;
        case EbpHigh:
            name = "ANGLE_rgb_2_yuv_highp";
            break;
        default:
            UNREACHABLE();
    }

    constexpr std::array<float, 9> itu601Matrix = {0.299,   -0.1687, 0.5, 0.587,  -0.3313,
                                                   -0.4187, 0.114,   0.5, -0.0813};

    constexpr std::array<float, 9> itu709Matrix = {0.2126,  -0.1146, 0.5, 0.7152, -0.3854,
                                                   -0.4542, 0.0722,  0.5, -0.0458};

    return getYUVFunc(precision, name, MakeMatrix(itu601Matrix), MakeMatrix(itu709Matrix),
                      &mRGB2YUVFuncDefs[precision]);
}

const TFunction *EmulateYUVBuiltInsTraverser::getYUVFunc(TPrecision precision,
                                                         const char *name,
                                                         TIntermTyped *itu601Matrix,
                                                         TIntermTyped *itu709Matrix,
                                                         TIntermFunctionDefinition **funcDefOut)
{
    if (*funcDefOut != nullptr)
    {
        return (*funcDefOut)->getFunction();
    }

    // The function prototype is vec3 name(vec3 color, yuvCscStandardEXT conv_standard)
    TType *vec3Type = new TType(*StaticType::GetBasic<EbtFloat, EbpMedium, 3>());
    vec3Type->setPrecision(precision);
    const TType *yuvCscType = StaticType::GetBasic<EbtYuvCscStandardEXT, EbpUndefined>();

    TType *colorType = new TType(*vec3Type);
    TType *convType  = new TType(*yuvCscType);
    colorType->setQualifier(EvqParamIn);
    convType->setQualifier(EvqParamIn);

    TVariable *colorParam =
        new TVariable(mSymbolTable, ImmutableString("color"), colorType, SymbolType::AngleInternal);
    TVariable *convParam = new TVariable(mSymbolTable, ImmutableString("conv_standard"), convType,
                                         SymbolType::AngleInternal);

    TFunction *function = new TFunction(mSymbolTable, ImmutableString(name),
                                        SymbolType::AngleInternal, vec3Type, true);
    function->addParameter(colorParam);
    function->addParameter(convParam);

    // The function body is as such:
    //
    //     switch (conv_standard)
    //     {
    //       case itu_601:
    //         return itu601Matrix * color;
    //       case itu_601_full_range:
    //         return itu601Matrix * color;
    //       case itu_709:
    //         return itu709Matrix * color;
    //     }
    //
    //     // error
    //     return vec3(0.0);

    // preprocess
    TType *biasedType = new TType(*vec3Type);
    TIntermTyped *biased = new TIntermBinary(EOpAdd, new TIntermSymbol(colorParam), MakeVector(-16.f/255.f, -0.5f, -0.5f));
    TType *scaledType = new TType(*vec3Type);
    TIntermTyped *scaled = new TIntermBinary(EOpMul, biased, MakeVector(255.f/219.f, 255.f/224.f, 255.f/224.f));

    // Matrix * color
    TIntermTyped *itu601Mul =
        new TIntermBinary(EOpMatrixTimesVector, itu601Matrix, scaled);
    TIntermTyped *itu601FullRangeMul = new TIntermBinary(
        EOpMatrixTimesVector, itu601Matrix->deepCopy(), scaled->deepCopy());
    TIntermTyped *itu709Mul =
        new TIntermBinary(EOpMatrixTimesVector, itu709Matrix, scaled->deepCopy());

    // return Matrix * color
    TIntermBranch *returnItu601Mul          = new TIntermBranch(EOpReturn, itu601Mul);
    TIntermBranch *returnItu601FullRangeMul = new TIntermBranch(EOpReturn, itu601FullRangeMul);
    TIntermBranch *returnItu709Mul          = new TIntermBranch(EOpReturn, itu709Mul);

    // itu_* constants
    TConstantUnion *ituConstants = new TConstantUnion[3];
    ituConstants[0].setYuvCscStandardEXTConst(EycsItu601);
    ituConstants[1].setYuvCscStandardEXTConst(EycsItu601FullRange);
    ituConstants[2].setYuvCscStandardEXTConst(EycsItu709);

    TIntermConstantUnion *itu601          = new TIntermConstantUnion(&ituConstants[0], *yuvCscType);
    TIntermConstantUnion *itu601FullRange = new TIntermConstantUnion(&ituConstants[1], *yuvCscType);
    TIntermConstantUnion *itu709          = new TIntermConstantUnion(&ituConstants[2], *yuvCscType);

    // case ...: return ...
    TIntermBlock *switchBody = new TIntermBlock;

    switchBody->appendStatement(new TIntermCase(itu601));
    switchBody->appendStatement(returnItu601Mul);
    switchBody->appendStatement(new TIntermCase(itu601FullRange));
    switchBody->appendStatement(returnItu601FullRangeMul);
    switchBody->appendStatement(new TIntermCase(itu709));
    switchBody->appendStatement(returnItu709Mul);

    // switch (conv_standard) ...
    TIntermSwitch *switchStatement = new TIntermSwitch(new TIntermSymbol(convParam), switchBody);

    TIntermBlock *body = new TIntermBlock;

    body->appendStatement(switchStatement);
    body->appendStatement(new TIntermBranch(EOpReturn, CreateZeroNode(*vec3Type)));

    *funcDefOut = new TIntermFunctionDefinition(new TIntermFunctionPrototype(function), body);

    return function;
}

bool EmulateYUVBuiltInsTraverser::update(TCompiler *compiler, TIntermBlock *root)
{
    // Insert any added function definitions before the first function.
    const size_t firstFunctionIndex = FindFirstFunctionDefinitionIndex(root);
    TIntermSequence funcDefs;

    for (TIntermFunctionDefinition *funcDef : mYUV2RGBFuncDefs)
    {
        if (funcDef != nullptr)
        {
            funcDefs.push_back(funcDef);
        }
    }

    for (TIntermFunctionDefinition *funcDef : mRGB2YUVFuncDefs)
    {
        if (funcDef != nullptr)
        {
            funcDefs.push_back(funcDef);
        }
    }

    root->insertChildNodes(firstFunctionIndex, funcDefs);

    return updateTree(compiler, root);
}
}  // anonymous namespace

bool EmulateYUVBuiltIns(TCompiler *compiler, TIntermBlock *root, TSymbolTable *symbolTable)
{
    EmulateYUVBuiltInsTraverser traverser(symbolTable);
    root->traverse(&traverser);
    return traverser.update(compiler, root);
}
}  // namespace sh
