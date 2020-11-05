//
// Copyright 2019 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//
// Implementation of dFdy viewport transformation.
// See header for more info.

#include "compiler/translator/tree_ops/RewriteDfdy.h"

#include "common/PackedEnums.h"
#include "common/angleutils.h"
#include "compiler/translator/StaticType.h"
#include "compiler/translator/SymbolTable.h"
#include "compiler/translator/tree_util/IntermNode_util.h"
#include "compiler/translator/tree_util/IntermTraverse.h"

namespace sh
{

namespace
{
// This is 2x2 matrix in column major. The first column is for dFdx and second column is for dFdy.
using PreRotationMatrixValues        = std::array<float, 4>;
using PreRotationMatrixValuesEnumMap = angle::PackedEnumMap<vk::SurfaceRotation,
                                                            PreRotationMatrixValues,
                                                            angle::EnumSize<vk::SurfaceRotation>()>;
constexpr PreRotationMatrixValuesEnumMap kFragRotationMatrices = {
    {{vk::SurfaceRotation::Identity, {{1.0f, 0.0f, 0.0f, 1.0f}}},
     {vk::SurfaceRotation::Rotated90Degrees, {{0.0f, 1.0f, 1.0f, 0.0f}}},
     {vk::SurfaceRotation::Rotated180Degrees, {{1.0f, 0.0f, 0.0f, 1.0f}}},
     {vk::SurfaceRotation::Rotated270Degrees, {{0.0f, 1.0f, 1.0f, 0.0f}}},
     {vk::SurfaceRotation::FlippedIdentity, {{1.0f, 0.0f, 0.0f, 1.0f}}},
     {vk::SurfaceRotation::FlippedRotated90Degrees, {{0.0f, 1.0f, 1.0f, 0.0f}}},
     {vk::SurfaceRotation::FlippedRotated180Degrees, {{1.0f, 0.0f, 0.0f, 1.0f}}},
     {vk::SurfaceRotation::FlippedRotated270Degrees, {{0.0f, 1.0f, 1.0f, 0.0f}}}}};

// Create mat2 node from kFragRotationMatrices
TIntermAggregate *CreateFragRotationMatrixNode(vk::SurfaceRotation rotation)
{
    auto mat2Type             = new TType(EbtFloat, 2, 2);
    TIntermSequence *mat2Args = new TIntermSequence();
    mat2Args->push_back(CreateFloatNode(kFragRotationMatrices[rotation][0]));
    mat2Args->push_back(CreateFloatNode(kFragRotationMatrices[rotation][1]));
    mat2Args->push_back(CreateFloatNode(kFragRotationMatrices[rotation][2]));
    mat2Args->push_back(CreateFloatNode(kFragRotationMatrices[rotation][3]));
    TIntermAggregate *constVarConstructor =
        TIntermAggregate::CreateConstructor(*mat2Type, mat2Args);
    return constVarConstructor;
}

using FlipXYValues = std::array<float, 2>;
using FlipXYValuesEnumaMap =
    angle::PackedEnumMap<vk::SurfaceRotation, FlipXYValues, angle::EnumSize<vk::SurfaceRotation>()>;
constexpr FlipXYValuesEnumaMap kFlipXY = {
    {{vk::SurfaceRotation::Identity, {{1.0f, 1.0f}}},
     {vk::SurfaceRotation::Rotated90Degrees, {{1.0f, 1.0f}}},
     {vk::SurfaceRotation::Rotated180Degrees, {{-1.0f, 1.0f}}},
     {vk::SurfaceRotation::Rotated270Degrees, {{-1.0f, -1.0f}}},
     {vk::SurfaceRotation::FlippedIdentity, {{1.0f, -1.0f}}},
     {vk::SurfaceRotation::FlippedRotated90Degrees, {{1.0f, -1.0f}}},
     {vk::SurfaceRotation::FlippedRotated180Degrees, {{-1.0f, -1.0f}}},
     {vk::SurfaceRotation::FlippedRotated270Degrees, {{-1.0f, 1.0f}}}}};

// Create vec2(flip.x, flip.y) or vec2(flip.x, -flip.y)
TIntermAggregate *CreateFlipXYNode(vk::SurfaceRotation rotation, bool negFlipY)
{
    auto vec2Type             = new TType(EbtFloat, 2);
    TIntermSequence *vec2Args = new TIntermSequence();
    vec2Args->push_back(CreateFloatNode(kFlipXY[rotation][0]));
    if (negFlipY)
    {
        vec2Args->push_back(CreateFloatNode(-kFlipXY[rotation][1]));
    }
    else
    {
        vec2Args->push_back(CreateFloatNode(kFlipXY[rotation][1]));
    }
    TIntermAggregate *constVarConstructor =
        TIntermAggregate::CreateConstructor(*vec2Type, vec2Args);
    return constVarConstructor;
}

// Calculate [flipX*m0, flipY*m1]. [m0 m1] is the first column of kFragRotation matrix.
constexpr FlipXYValues calculateRotatedFlipXYForDFdx(vk::SurfaceRotation rotation)
{
    return FlipXYValues({kFlipXY[rotation][0] * kFragRotationMatrices[rotation][0],
                         kFlipXY[rotation][1] * kFragRotationMatrices[rotation][1]});
}
constexpr FlipXYValuesEnumaMap kRotatedFlipXYForDFdx = {
    {{vk::SurfaceRotation::Identity, calculateRotatedFlipXYForDFdx(vk::SurfaceRotation::Identity)},
     {vk::SurfaceRotation::Rotated90Degrees,
      calculateRotatedFlipXYForDFdx(vk::SurfaceRotation::Rotated90Degrees)},
     {vk::SurfaceRotation::Rotated180Degrees,
      calculateRotatedFlipXYForDFdx(vk::SurfaceRotation::Rotated180Degrees)},
     {vk::SurfaceRotation::Rotated270Degrees,
      calculateRotatedFlipXYForDFdx(vk::SurfaceRotation::Rotated270Degrees)},
     {vk::SurfaceRotation::FlippedIdentity,
      calculateRotatedFlipXYForDFdx(vk::SurfaceRotation::FlippedIdentity)},
     {vk::SurfaceRotation::FlippedRotated90Degrees,
      calculateRotatedFlipXYForDFdx(vk::SurfaceRotation::FlippedRotated90Degrees)},
     {vk::SurfaceRotation::FlippedRotated180Degrees,
      calculateRotatedFlipXYForDFdx(vk::SurfaceRotation::FlippedRotated180Degrees)},
     {vk::SurfaceRotation::FlippedRotated270Degrees,
      calculateRotatedFlipXYForDFdx(vk::SurfaceRotation::FlippedRotated270Degrees)}}};

// Calculate [flipX*m2, flipY*m3]. [m2 m3] is the second column of kFragRotation matrix.
constexpr FlipXYValues calculateRotatedFlipXYForDFdy(vk::SurfaceRotation rotation)
{
    return FlipXYValues({kFlipXY[rotation][0] * kFragRotationMatrices[rotation][2],
                         kFlipXY[rotation][1] * kFragRotationMatrices[rotation][3]});
}
constexpr FlipXYValuesEnumaMap kRotatedFlipXYForDFdy = {
    {{vk::SurfaceRotation::Identity, calculateRotatedFlipXYForDFdy(vk::SurfaceRotation::Identity)},
     {vk::SurfaceRotation::Rotated90Degrees,
      calculateRotatedFlipXYForDFdy(vk::SurfaceRotation::Rotated90Degrees)},
     {vk::SurfaceRotation::Rotated180Degrees,
      calculateRotatedFlipXYForDFdy(vk::SurfaceRotation::Rotated180Degrees)},
     {vk::SurfaceRotation::Rotated270Degrees,
      calculateRotatedFlipXYForDFdy(vk::SurfaceRotation::Rotated270Degrees)},
     {vk::SurfaceRotation::FlippedIdentity,
      calculateRotatedFlipXYForDFdy(vk::SurfaceRotation::FlippedIdentity)},
     {vk::SurfaceRotation::FlippedRotated90Degrees,
      calculateRotatedFlipXYForDFdy(vk::SurfaceRotation::FlippedRotated90Degrees)},
     {vk::SurfaceRotation::FlippedRotated180Degrees,
      calculateRotatedFlipXYForDFdy(vk::SurfaceRotation::FlippedRotated180Degrees)},
     {vk::SurfaceRotation::FlippedRotated270Degrees,
      calculateRotatedFlipXYForDFdy(vk::SurfaceRotation::FlippedRotated270Degrees)}}};

// Generates an array of float and then use rotation to retrieve the desired float value out.
TIntermTyped *GenerateMultiplier(const FlipXYValuesEnumaMap &valuesEnumaMap,
                                 TIntermSymbol *rotation,
                                 int index)
{
    const TType *floatType = StaticType::GetBasic<EbtFloat>();
    TType *typeFloat8      = new TType(*floatType);
    typeFloat8->makeArray(static_cast<unsigned int>(vk::SurfaceRotation::EnumCount));

    TIntermSequence *sequences;
    sequences = new TIntermSequence(
        {CreateFloatNode(valuesEnumaMap[vk::SurfaceRotation::Identity][index]),
         CreateFloatNode(valuesEnumaMap[vk::SurfaceRotation::Rotated90Degrees][index]),
         CreateFloatNode(valuesEnumaMap[vk::SurfaceRotation::Rotated180Degrees][index]),
         CreateFloatNode(valuesEnumaMap[vk::SurfaceRotation::Rotated270Degrees][index]),
         CreateFloatNode(valuesEnumaMap[vk::SurfaceRotation::FlippedIdentity][index]),
         CreateFloatNode(valuesEnumaMap[vk::SurfaceRotation::FlippedRotated90Degrees][index]),
         CreateFloatNode(valuesEnumaMap[vk::SurfaceRotation::FlippedRotated180Degrees][index]),
         CreateFloatNode(valuesEnumaMap[vk::SurfaceRotation::FlippedRotated270Degrees][index])});
    TIntermTyped *multiplersArray = TIntermAggregate::CreateConstructor(*typeFloat8, sequences);

    TIntermTyped *multiplier =
        new TIntermBinary(EOpIndexDirect, multiplersArray, rotation->deepCopy());
    return multiplier;
}

class Traverser : public TIntermTraverser
{
  public:
    ANGLE_NO_DISCARD static bool Apply(TCompiler *compiler,
                                       TIntermNode *root,
                                       const TSymbolTable &symbolTable,
                                       TIntermSymbol *surfaceRotationSpecConst,
                                       bool usePreRotation,
                                       TIntermBinary *flipXY,
                                       TIntermTyped *fragRotation);

  private:
    Traverser(TIntermSymbol *surfaceRotationSpecConst,
              bool usePreRotation,
              TIntermBinary *flipXY,
              TIntermTyped *fragRotation,
              TSymbolTable *symbolTable);
    bool visitUnary(Visit visit, TIntermUnary *node) override;

    bool visitUnaryWithRotation(Visit visit, TIntermUnary *node);
    bool visitUnaryWithoutRotation(Visit visit, TIntermUnary *node);

    TIntermSymbol *mRotationSpecConst = nullptr;
    bool mUsePreRotation              = false;
    TIntermBinary *mFlipXY            = nullptr;
    TIntermTyped *mFragRotation       = nullptr;
};

Traverser::Traverser(TIntermSymbol *surfaceRotationSpecConst,
                     bool usePreRotation,
                     TIntermBinary *flipXY,
                     TIntermTyped *fragRotation,
                     TSymbolTable *symbolTable)
    : TIntermTraverser(true, false, false, symbolTable),
      mRotationSpecConst(surfaceRotationSpecConst),
      mUsePreRotation(usePreRotation),
      mFlipXY(flipXY),
      mFragRotation(fragRotation)
{}

// static
bool Traverser::Apply(TCompiler *compiler,
                      TIntermNode *root,
                      const TSymbolTable &symbolTable,
                      TIntermSymbol *surfaceRotationSpecConst,
                      bool usePreRotation,
                      TIntermBinary *flipXY,
                      TIntermTyped *fragRotation)
{
    TSymbolTable *pSymbolTable = const_cast<TSymbolTable *>(&symbolTable);
    Traverser traverser(surfaceRotationSpecConst, usePreRotation, flipXY, fragRotation,
                        pSymbolTable);
    root->traverse(&traverser);
    return traverser.updateTree(compiler, root);
}

bool Traverser::visitUnary(Visit visit, TIntermUnary *node)
{
    if (mUsePreRotation)
    {
        ASSERT(mRotationSpecConst || mFragRotation);
        return visitUnaryWithRotation(visit, node);
    }
    ASSERT(!mFragRotation);
    return visitUnaryWithoutRotation(visit, node);
}

bool Traverser::visitUnaryWithRotation(Visit visit, TIntermUnary *node)
{
    // Decide if the node represents a call to dFdx() or dFdy()
    if ((node->getOp() != EOpDFdx) && (node->getOp() != EOpDFdy))
    {
        return true;
    }

    // Prior to supporting Android pre-rotation, dFdy() needed to be multiplied by mFlipXY.y:
    //
    //   correctedDfdy(operand) = dFdy(operand) * mFlipXY.y
    //
    // For Android pre-rotation, both dFdx() and dFdy() need to be "rotated" and multiplied by
    // mFlipXY.  "Rotation" means to swap them for 90 and 270 degrees, or to not swap them for 0
    // and 180 degrees.  This rotation is accomplished with mFragRotation, which is a 2x2 matrix
    // used for fragment shader rotation.  The 1st half (a vec2 that is either (1,0) or (0,1)) is
    // used for rewriting dFdx() and the 2nd half (either (0,1) or (1,0)) is used for rewriting
    // dFdy().  Otherwise, the formula for the rewrite is the same:
    //
    //     result = ((dFdx(operand) * (mFragRotation[half] * mFlipXY).x) +
    //               (dFdy(operand) * (mFragRotation[half] * mFlipXY).y))
    //
    // For dFdx(), half is 0 (the 1st half).  For dFdy(), half is 1 (the 2nd half).  Depending on
    // the rotation, mFragRotation[half] will cause either dFdx(operand) or dFdy(operand) to be
    // zeroed-out.  That effectively means that the above code results in the following for 0 and
    // 180 degrees:
    //
    //   correctedDfdx(operand) = dFdx(operand) * mFlipXY.x
    //   correctedDfdy(operand) = dFdy(operand) * mFlipXY.y
    //
    // and the following for 90 and 270 degrees:
    //
    //   correctedDfdx(operand) = dFdy(operand) * mFlipXY.y
    //   correctedDfdy(operand) = dFdx(operand) * mFlipXY.x
    //
    // TODO(ianelliott): Look at the performance of this approach and potentially optimize it
    // http://anglebug.com/4678

    TIntermTyped *multiplierX;
    TIntermTyped *multiplierY;
    if (mRotationSpecConst)
    {
        if (node->getOp() == EOpDFdx)
        {
            multiplierX = GenerateMultiplier(kRotatedFlipXYForDFdx, mRotationSpecConst, 0);
            multiplierY = GenerateMultiplier(kRotatedFlipXYForDFdx, mRotationSpecConst, 1);
        }
        else
        {
            multiplierX = GenerateMultiplier(kRotatedFlipXYForDFdy, mRotationSpecConst, 0);
            multiplierY = GenerateMultiplier(kRotatedFlipXYForDFdy, mRotationSpecConst, 1);
        }
    }
    else
    {
        // Get a vec2 with the correct half of ANGLEUniforms.fragRotation
        TIntermBinary *halfRotationMat = nullptr;
        if (node->getOp() == EOpDFdx)
        {
            halfRotationMat =
                new TIntermBinary(EOpIndexDirect, mFragRotation->deepCopy(), CreateIndexNode(0));
        }
        else
        {
            halfRotationMat =
                new TIntermBinary(EOpIndexDirect, mFragRotation->deepCopy(), CreateIndexNode(1));
        }

        // Multiply halfRotationMat by ANGLEUniforms.flipXY and store in a temporary variable
        TIntermBinary *rotatedFlipXY =
            new TIntermBinary(EOpMul, mFlipXY->deepCopy(), halfRotationMat);
        const TType *vec2Type       = StaticType::GetBasic<EbtFloat, 2>();
        TIntermSymbol *tmpRotFlipXY = new TIntermSymbol(CreateTempVariable(mSymbolTable, vec2Type));
        TIntermSequence *tmpDecl    = new TIntermSequence;
        tmpDecl->push_back(CreateTempInitDeclarationNode(&tmpRotFlipXY->variable(), rotatedFlipXY));
        insertStatementsInParentBlock(*tmpDecl);

        // Get the .x and .y swizzles to use as multipliers
        TVector<int> swizzleOffsetX = {0};
        TVector<int> swizzleOffsetY = {1};
        multiplierX                 = new TIntermSwizzle(tmpRotFlipXY, swizzleOffsetX);
        multiplierY                 = new TIntermSwizzle(tmpRotFlipXY->deepCopy(), swizzleOffsetY);
    }

    // Get the results of dFdx(operand) and dFdy(operand), and multiply them by the swizzles
    TIntermTyped *operand = node->getOperand();
    TIntermUnary *dFdx    = new TIntermUnary(EOpDFdx, operand->deepCopy(), node->getFunction());
    TIntermUnary *dFdy    = new TIntermUnary(EOpDFdy, operand->deepCopy(), node->getFunction());
    size_t objectSize     = node->getType().getObjectSize();
    TOperator multiplyOp  = (objectSize == 1) ? EOpMul : EOpVectorTimesScalar;
    TIntermBinary *rotatedFlippedDfdx = new TIntermBinary(multiplyOp, dFdx, multiplierX);
    TIntermBinary *rotatedFlippedDfdy = new TIntermBinary(multiplyOp, dFdy, multiplierY);

    // Sum them together into the result:
    TIntermBinary *correctedResult =
        new TIntermBinary(EOpAdd, rotatedFlippedDfdx, rotatedFlippedDfdy);

    // Replace the old dFdx() or dFdy() node with the new node that contains the corrected value
    queueReplacement(correctedResult, OriginalNode::IS_DROPPED);

    return true;
}

bool Traverser::visitUnaryWithoutRotation(Visit visit, TIntermUnary *node)
{
    // Decide if the node represents a call to dFdy()
    if (node->getOp() != EOpDFdy)
    {
        return true;
    }

    // Copy the dFdy node so we can replace it with the corrected value
    TIntermUnary *newDfdy = node->deepCopy()->getAsUnaryNode();

    size_t objectSize    = node->getType().getObjectSize();
    TOperator multiplyOp = (objectSize == 1) ? EOpMul : EOpVectorTimesScalar;

    TIntermTyped *flipY;
    if (mRotationSpecConst)
    {
        flipY = GenerateMultiplier(kFlipXY, mRotationSpecConst, 1);
    }
    else
    {
        flipY = new TIntermBinary(EOpIndexDirect, mFlipXY->deepCopy(), CreateIndexNode(1));
    }
    // Correct dFdy()'s value:
    // (dFdy() * mFlipXY.y)
    TIntermBinary *correctedDfdy = new TIntermBinary(multiplyOp, newDfdy, flipY);

    // Replace the old dFdy node with the new node that contains the corrected value
    queueReplacement(correctedDfdy, OriginalNode::IS_DROPPED);

    return true;
}
}  // anonymous namespace

bool RewriteDfdy(TCompiler *compiler,
                 TIntermNode *root,
                 const TSymbolTable &symbolTable,
                 int shaderVersion,
                 TIntermSymbol *surfaceRotationSpecConst,
                 bool usePreRotation,
                 TIntermBinary *flipXY,
                 TIntermTyped *fragRotation)
{
    // dFdy is only valid in GLSL 3.0 and later.
    if (shaderVersion < 300)
        return true;

    return Traverser::Apply(compiler, root, symbolTable, surfaceRotationSpecConst, usePreRotation,
                            flipXY, fragRotation);
}

// Generates an array of vec2 and then use rotation to retrieve the desired flipXY out.
TIntermTyped *GenerateFlipXY(TIntermSymbol *rotation, bool negFlipY)
{
    auto vec2Type          = new TType(EbtFloat, 2);
    TType *typeFlipXYArray = new TType(*vec2Type);
    typeFlipXYArray->makeArray(static_cast<unsigned int>(vk::SurfaceRotation::EnumCount));

    TIntermSequence *sequences;
    sequences = new TIntermSequence(
        {CreateFlipXYNode(vk::SurfaceRotation::Identity, negFlipY),
         CreateFlipXYNode(vk::SurfaceRotation::Rotated90Degrees, negFlipY),
         CreateFlipXYNode(vk::SurfaceRotation::Rotated180Degrees, negFlipY),
         CreateFlipXYNode(vk::SurfaceRotation::Rotated270Degrees, negFlipY),
         CreateFlipXYNode(vk::SurfaceRotation::FlippedIdentity, negFlipY),
         CreateFlipXYNode(vk::SurfaceRotation::FlippedRotated90Degrees, negFlipY),
         CreateFlipXYNode(vk::SurfaceRotation::FlippedRotated180Degrees, negFlipY),
         CreateFlipXYNode(vk::SurfaceRotation::FlippedRotated270Degrees, negFlipY)});
    TIntermTyped *flipXYArray = TIntermAggregate::CreateConstructor(*typeFlipXYArray, sequences);

    TIntermTyped *flipXY = new TIntermBinary(EOpIndexDirect, flipXYArray, rotation->deepCopy());
    return flipXY;
}

// Generates an array of vec2 and then use rotation to retrieve the desired flipXY out.
TIntermTyped *GenerateFragRotationMatrix(TIntermSymbol *rotation)
{
    auto mat2Type        = new TType(EbtFloat, 2, 2);
    TType *typeMat2Array = new TType(*mat2Type);
    typeMat2Array->makeArray(static_cast<unsigned int>(vk::SurfaceRotation::EnumCount));

    TIntermSequence *sequences;
    sequences = new TIntermSequence(
        {CreateFragRotationMatrixNode(vk::SurfaceRotation::Identity),
         CreateFragRotationMatrixNode(vk::SurfaceRotation::Rotated90Degrees),
         CreateFragRotationMatrixNode(vk::SurfaceRotation::Rotated180Degrees),
         CreateFragRotationMatrixNode(vk::SurfaceRotation::Rotated270Degrees),
         CreateFragRotationMatrixNode(vk::SurfaceRotation::FlippedIdentity),
         CreateFragRotationMatrixNode(vk::SurfaceRotation::FlippedRotated90Degrees),
         CreateFragRotationMatrixNode(vk::SurfaceRotation::FlippedRotated180Degrees),
         CreateFragRotationMatrixNode(vk::SurfaceRotation::FlippedRotated270Degrees)});
    TIntermTyped *fragRotationMatrixArray =
        TIntermAggregate::CreateConstructor(*typeMat2Array, sequences);

    TIntermTyped *fragRotationMatrix =
        new TIntermBinary(EOpIndexDirect, fragRotationMatrixArray, rotation->deepCopy());
    return fragRotationMatrix;
}
}  // namespace sh
