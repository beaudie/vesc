//
// Copyright 2021 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//
// EmulateDithering: Adds dithering code to fragment shader outputs based on a specialization
// constant control value.
//

#include "compiler/translator/tree_ops/vulkan/EmulateDithering.h"

#include "compiler/translator/Compiler.h"
#include "compiler/translator/StaticType.h"
#include "compiler/translator/SymbolTable.h"
#include "compiler/translator/tree_util/DriverUniform.h"
#include "compiler/translator/tree_util/IntermNode_util.h"
#include "compiler/translator/tree_util/IntermTraverse.h"
#include "compiler/translator/tree_util/RunAtTheEndOfShader.h"
#include "compiler/translator/tree_util/SpecializationConstant.h"

namespace sh
{
namespace
{
using FragmentOutputVariableList = TVector<const TVariable *>;

void GatherFragmentOutputs(TIntermBlock *root,
                           FragmentOutputVariableList *fragmentOutputVariablesOut)
{
    TIntermSequence &sequence = *root->getSequence();

    for (TIntermNode *node : sequence)
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
            fragmentOutputVariablesOut->push_back(&symbol->variable());
        }
    }
}

void EmitFragmentOutputDither(TCompiler *compiler,
                              TSymbolTable *symbolTable,
                              TIntermBlock *ditherBlock,
                              TIntermTyped *ditherControl,
                              TIntermTyped *ditherParam,
                              TIntermTyped *fragmentOutput,
                              uint32_t location)
{
    // dither >> 2*location
    TIntermBinary *ditherControlShifted = new TIntermBinary(
        EOpBitShiftRight, ditherControl->deepCopy(), CreateUIntNode(location * 2));

    // (dither >> 2*location) & 3
    TIntermBinary *thisDitherControlValue =
        new TIntermBinary(EOpBitwiseAnd, ditherControlShifted, CreateUIntNode(3));

    // const uint dither_i = (dither >> 2*location) & 3;
    TIntermSymbol *thisDitherControl = new TIntermSymbol(
        CreateTempVariable(symbolTable, StaticType::GetBasic<EbtUInt, EbpHigh>()));
    TIntermDeclaration *thisDitherControlDecl =
        CreateTempInitDeclarationNode(&thisDitherControl->variable(), thisDitherControlValue);
    ditherBlock->appendStatement(thisDitherControlDecl);

    TIntermBlock *switchBody = new TIntermBlock;

    // case kDitherControlDither4444:
    //     fragmentOutput += vec4(ditherParam * 2)
    {
        TIntermSequence ditherValueElements = {
            new TIntermBinary(EOpMul, ditherParam->deepCopy(), CreateFloatNode(2.0f, EbpMedium)),
        };
        TIntermAggregate *ditherValue = TIntermAggregate::CreateConstructor(
            *StaticType::GetBasic<EbtFloat, EbpMedium, 4>(), &ditherValueElements);

        TIntermTyped *addDitherNode = new TIntermBinary(EOpAddAssign, fragmentOutput, ditherValue);

        switchBody->appendStatement(new TIntermCase(CreateUIntNode(vk::kDitherControlDither4444)));
        switchBody->appendStatement(addDitherNode);
        switchBody->appendStatement(new TIntermBranch(EOpBreak, nullptr));
    }

    // case kDitherControlDither5551:
    //     fragmentOutput += vec4(ditherParam, ditherParam, ditherParam, ditherParam * 32)
    {
        TIntermSequence ditherValueElements = {
            ditherParam->deepCopy(),
            ditherParam->deepCopy(),
            ditherParam->deepCopy(),
            new TIntermBinary(EOpMul, ditherParam->deepCopy(), CreateFloatNode(32.0f, EbpMedium)),
        };
        TIntermAggregate *ditherValue = TIntermAggregate::CreateConstructor(
            *StaticType::GetBasic<EbtFloat, EbpMedium, 4>(), &ditherValueElements);

        TIntermTyped *addDitherNode =
            new TIntermBinary(EOpAddAssign, fragmentOutput->deepCopy(), ditherValue);

        switchBody->appendStatement(new TIntermCase(CreateUIntNode(vk::kDitherControlDither5551)));
        switchBody->appendStatement(addDitherNode);
        switchBody->appendStatement(new TIntermBranch(EOpBreak, nullptr));
    }

    // case kDitherControlDither565:
    //     fragmentOutput += vec4(ditherParam, ditherParam / 2, ditherParam, 0)
    {
        TIntermSequence ditherValueElements = {
            ditherParam->deepCopy(),
            new TIntermBinary(EOpMul, ditherParam->deepCopy(), CreateFloatNode(0.5f, EbpMedium)),
            ditherParam->deepCopy(),
            CreateFloatNode(0, EbpMedium),
        };
        TIntermAggregate *ditherValue = TIntermAggregate::CreateConstructor(
            *StaticType::GetBasic<EbtFloat, EbpMedium, 4>(), &ditherValueElements);

        TIntermTyped *addDitherNode =
            new TIntermBinary(EOpAddAssign, fragmentOutput->deepCopy(), ditherValue);

        switchBody->appendStatement(new TIntermCase(CreateUIntNode(vk::kDitherControlDither565)));
        switchBody->appendStatement(addDitherNode);
        switchBody->appendStatement(new TIntermBranch(EOpBreak, nullptr));
    }

    // switch (dither_i)
    // {
    //     ...
    // }
    TIntermSwitch *formatSwitch = new TIntermSwitch(thisDitherControl, switchBody);
    ditherBlock->appendStatement(formatSwitch);
}

void EmitFragmentVariableDither(TCompiler *compiler,
                                TSymbolTable *symbolTable,
                                TIntermBlock *ditherBlock,
                                TIntermTyped *ditherControl,
                                TIntermTyped *ditherParam,
                                const TVariable &fragmentVariable)
{
    const TType &type                       = fragmentVariable.getType();
    const TLayoutQualifier &layoutQualifier = type.getLayoutQualifier();

    const uint32_t location = layoutQualifier.locationsSpecified ? layoutQualifier.location : 0;

    // Fragment outputs cannot be an array of array.
    ASSERT(!type.isArrayOfArrays());

    // Emit one block of dithering output per element of array (if array).
    TIntermSymbol *fragmentOutputSymbol = new TIntermSymbol(&fragmentVariable);
    if (!type.isArray())
    {
        EmitFragmentOutputDither(compiler, symbolTable, ditherBlock, ditherControl, ditherParam,
                                 fragmentOutputSymbol, location);
        return;
    }

    for (uint32_t index = 0; index < type.getOutermostArraySize(); ++index)
    {
        TIntermBinary *element = new TIntermBinary(EOpIndexDirect, fragmentOutputSymbol->deepCopy(),
                                                   CreateIndexNode(index));
        EmitFragmentOutputDither(compiler, symbolTable, ditherBlock, ditherControl, ditherParam,
                                 element, location + static_cast<uint32_t>(index));
    }
}

TIntermNode *EmitDitheringBlock(TCompiler *compiler,
                                TSymbolTable *symbolTable,
                                SpecConst *specConst,
                                DriverUniform *driverUniforms,
                                const FragmentOutputVariableList &fragmentOutputVariables)
{
    // Add dithering code.  A specialization constant is taken (dither control) in the following
    // form:
    //
    //     0000000000000000dfdfdfdfdfdfdfdf
    //
    // Where every pair of bits df[i] means for attachment i:
    //
    //     00: no dithering
    //     01: dither for the R4G4B4A4 format
    //     10: dither for the R5G5B5A1 format
    //     11: dither for the R5G6B5 format
    //
    // Only the above formats are dithered to avoid paying a high cost on formats that usually don't
    // need dithering.  Applications that require dithering often perform the dithering themselves.
    //
    // The generated code is as such:
    //
    //     if (dither != 0)
    //     {
    //          const mediump float bayer[4] = { balanced 2x2 bayer divided by 32 }
    //          const mediump float b = bayer[(uint(gl_FragCoord.x) & 1) << 1 |
    //                                        (uint(gl_FragCoord.y) & 1)];
    //
    //          // for each attachment i
    //          uint ditheri = dither >> (2 * i) & 0x3;
    //          switch (ditheri)
    //          {
    //          case kDitherControlDither4444:
    //              colori += vec4(b * 2)
    //              break;
    //          case kDitherControlDither5551:
    //              colori += vec4(b, b, b, b * 32)
    //              break;
    //          case kDitherControlDither565:
    //              colori += vec4(b, b / 2, b, 0)
    //              break;
    //          }
    //     }

    TIntermTyped *ditherControl = specConst->getDither();
    if (ditherControl == nullptr)
    {
        ditherControl = driverUniforms->getDitherRef();
    }

    // if (dither != 0)
    TIntermTyped *ifAnyDitherCondition =
        new TIntermBinary(EOpNotEqual, ditherControl, CreateUIntNode(0));

    TIntermBlock *ditherBlock = new TIntermBlock;

    // The dithering (Bayer) matrix.  A 2x2 matrix is used which has acceptable results with minimal
    // impact on performance.  The 2x2 Bayer matrix is defined as:
    //
    //                [ 0  2 ]
    //     B = 0.25 * |      |
    //                [ 3  1 ]
    //
    // Using this matrix adds energy to the output however, and so it is balanced by subtracting the
    // elements by the average value:
    //
    //                         [ -1.5   0.5 ]
    //     B_balanced = 0.25 * |            |
    //                         [  1.5  -0.5 ]
    //
    // For each pixel, one of the four values in this matrix is selected (indexed by
    // gl_FragCoord.xy % 2), is scaled by the precision of the attachment format (per channel, if
    // different) and is added to the color output.  For example, if the value `b` is selected for a
    // pixel, and the attachment has the RGB565 format, then the following value is added to the
    // color output:
    //
    //      vec3(b/32, b/64, b/32)
    //
    // For RGBA5551, that would be:
    //
    //      vec4(b/32, b/32, b/32, b)
    //
    // And for RGBA4444, that would be:
    //
    //      vec4(b/16, b/16, b/16, b/16)
    //
    // Given the relative popularity of RGB565, and that b/32 is most often used in the above, the
    // Bayer matrix constant used here is pre-scaled by 1/32, avoiding further scaling in most
    // cases.
    TType *bayerType = new TType(*StaticType::GetBasic<EbtFloat, EbpMedium>());
    bayerType->setQualifier(EvqConst);
    bayerType->makeArray(4);

    TIntermSequence bayerElements = {
        CreateFloatNode(-1.5f * 0.25f / 32.0f, EbpMedium),
        CreateFloatNode(0.5f * 0.25f / 32.0f, EbpMedium),
        CreateFloatNode(1.5f * 0.25f / 32.0f, EbpMedium),
        CreateFloatNode(-0.5f * 0.25f / 32.0f, EbpMedium),
    };
    TIntermAggregate *bayerValue = TIntermAggregate::CreateConstructor(*bayerType, &bayerElements);

    // const float bayer[4] = { balanced 2x2 bayer divided by 32 }
    TIntermSymbol *bayer          = new TIntermSymbol(CreateTempVariable(symbolTable, bayerType));
    TIntermDeclaration *bayerDecl = CreateTempInitDeclarationNode(&bayer->variable(), bayerValue);
    ditherBlock->appendStatement(bayerDecl);

    // Take the coordinates of the pixel and determine which element of the bayer matrix should be
    // used:
    //
    //     (uint(gl_FragCoord.x) & 1) << 1 | (uint(gl_FragCoord.y) & 1)
    const TVariable *fragCoord = static_cast<const TVariable *>(
        symbolTable->findBuiltIn(ImmutableString("gl_FragCoord"), compiler->getShaderVersion()));

    TIntermTyped *fragCoordX          = new TIntermSwizzle(new TIntermSymbol(fragCoord), {0});
    TIntermSequence fragCoordXIntArgs = {
        fragCoordX,
    };
    TIntermTyped *fragCoordXInt = TIntermAggregate::CreateConstructor(
        *StaticType::GetBasic<EbtUInt, EbpMedium>(), &fragCoordXIntArgs);
    TIntermTyped *fragCoordXBit0 =
        new TIntermBinary(EOpBitwiseAnd, fragCoordXInt, CreateUIntNode(1));
    TIntermTyped *fragCoordXBit0Shifted =
        new TIntermBinary(EOpBitShiftLeft, fragCoordXBit0, CreateUIntNode(1));

    TIntermTyped *fragCoordY          = new TIntermSwizzle(new TIntermSymbol(fragCoord), {1});
    TIntermSequence fragCoordYIntArgs = {
        fragCoordY,
    };
    TIntermTyped *fragCoordYInt = TIntermAggregate::CreateConstructor(
        *StaticType::GetBasic<EbtUInt, EbpMedium>(), &fragCoordYIntArgs);
    TIntermTyped *fragCoordYBit0 =
        new TIntermBinary(EOpBitwiseAnd, fragCoordYInt, CreateUIntNode(1));

    TIntermTyped *bayerIndex =
        new TIntermBinary(EOpBitwiseOr, fragCoordXBit0Shifted, fragCoordYBit0);

    // const mediump float b = bayer[(uint(gl_FragCoord.x) & 1) << 1 |
    //                               (uint(gl_FragCoord.y) & 1)];
    TIntermSymbol *ditherParam = new TIntermSymbol(
        CreateTempVariable(symbolTable, StaticType::GetBasic<EbtFloat, EbpMedium>()));
    TIntermDeclaration *ditherParamDecl = CreateTempInitDeclarationNode(
        &ditherParam->variable(),
        new TIntermBinary(EOpIndexIndirect, bayer->deepCopy(), bayerIndex));
    ditherBlock->appendStatement(ditherParamDecl);

    // Dither blocks for each fragment output
    for (const TVariable *fragmentVariable : fragmentOutputVariables)
    {
        EmitFragmentVariableDither(compiler, symbolTable, ditherBlock, ditherControl, ditherParam,
                                   *fragmentVariable);
    }

    return new TIntermIfElse(ifAnyDitherCondition, ditherBlock, nullptr);
}
}  // anonymous namespace

bool EmulateDithering(TCompiler *compiler,
                      TIntermBlock *root,
                      TSymbolTable *symbolTable,
                      SpecConst *specConst,
                      DriverUniform *driverUniforms)
{
    FragmentOutputVariableList fragmentOutputVariables;
    GatherFragmentOutputs(root, &fragmentOutputVariables);

    TIntermNode *ditherCode = EmitDitheringBlock(compiler, symbolTable, specConst, driverUniforms,
                                                 fragmentOutputVariables);

    return RunAtTheEndOfShader(compiler, root, ditherCode, symbolTable);
}
}  // namespace sh
