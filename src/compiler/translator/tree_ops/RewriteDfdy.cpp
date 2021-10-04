//
// Copyright 2019 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//
// Implementation of dFdy viewport transformation.
// See header for more info.

#include "compiler/translator/tree_ops/RewriteDfdy.h"

#include "common/angleutils.h"
#include "compiler/translator/SymbolTable.h"
#include "compiler/translator/TranslatorVulkan.h"
#include "compiler/translator/tree_util/DriverUniform.h"
#include "compiler/translator/tree_util/IntermNode_util.h"
#include "compiler/translator/tree_util/IntermTraverse.h"
#include "compiler/translator/tree_util/SpecializationConstant.h"

namespace sh
{

namespace
{

class Traverser : public TIntermTraverser
{
  public:
    ANGLE_NO_DISCARD static bool Apply(TCompiler *compiler,
                                       ShCompileOptions compileOptions,
                                       TIntermNode *root,
                                       const TSymbolTable &symbolTable,
                                       SpecConst *specConst);

  private:
    Traverser(TSymbolTable *symbolTable, ShCompileOptions compileOptions, SpecConst *specConst);
    bool visitAggregate(Visit visit, TIntermAggregate *node) override;

    bool visitAggregateWithRotation(Visit visit, TIntermAggregate *node);
    bool visitAggregateWithoutRotation(Visit visit, TIntermAggregate *node);

    SpecConst *mRotationSpecConst = nullptr;
    bool mUsePreRotation          = false;
};

Traverser::Traverser(TSymbolTable *symbolTable,
                     ShCompileOptions compileOptions,
                     SpecConst *specConst)
    : TIntermTraverser(true, false, false, symbolTable),
      mRotationSpecConst(specConst),
      mUsePreRotation((compileOptions & SH_ADD_PRE_ROTATION) != 0)
{}

// static
bool Traverser::Apply(TCompiler *compiler,
                      ShCompileOptions compileOptions,
                      TIntermNode *root,
                      const TSymbolTable &symbolTable,
                      SpecConst *specConst)
{
    TSymbolTable *pSymbolTable = const_cast<TSymbolTable *>(&symbolTable);
    Traverser traverser(pSymbolTable, compileOptions, specConst);
    root->traverse(&traverser);
    return traverser.updateTree(compiler, root);
}

bool Traverser::visitAggregate(Visit visit, TIntermAggregate *node)
{
    if (mUsePreRotation)
    {
        return visitAggregateWithRotation(visit, node);
    }
    return visitAggregateWithoutRotation(visit, node);
}

bool Traverser::visitAggregateWithRotation(Visit visit, TIntermAggregate *node)
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

    TIntermTyped *multiplierX;
    TIntermTyped *multiplierY;
    if (node->getOp() == EOpDFdx)
    {
        multiplierX = mRotationSpecConst->getMultiplierXForDFdx();
        multiplierY = mRotationSpecConst->getMultiplierYForDFdx();
    }
    else
    {
        multiplierX = mRotationSpecConst->getMultiplierXForDFdy();
        multiplierY = mRotationSpecConst->getMultiplierYForDFdy();
    }

    // Get the results of dFdx(operand) and dFdy(operand), and multiply them by the swizzles
    TIntermTyped *operand = node->getChildNode(0)->getAsTyped();

    TIntermTyped *dFdx =
        CreateBuiltInUnaryFunctionCallNode("dFdx", operand->deepCopy(), *mSymbolTable, 300);
    TIntermTyped *dFdy =
        CreateBuiltInUnaryFunctionCallNode("dFdy", operand->deepCopy(), *mSymbolTable, 300);

    size_t objectSize                 = node->getType().getObjectSize();
    TOperator multiplyOp              = (objectSize == 1) ? EOpMul : EOpVectorTimesScalar;
    TIntermBinary *rotatedFlippedDfdx = new TIntermBinary(multiplyOp, dFdx, multiplierX);
    TIntermBinary *rotatedFlippedDfdy = new TIntermBinary(multiplyOp, dFdy, multiplierY);

    // Sum them together into the result:
    TIntermBinary *correctedResult =
        new TIntermBinary(EOpAdd, rotatedFlippedDfdx, rotatedFlippedDfdy);

    // Replace the old dFdx() or dFdy() node with the new node that contains the corrected value
    queueReplacement(correctedResult, OriginalNode::IS_DROPPED);

    return true;
}

bool Traverser::visitAggregateWithoutRotation(Visit visit, TIntermAggregate *node)
{
    // Decide if the node represents a call to dFdy()
    if (node->getOp() != EOpDFdy)
    {
        return true;
    }

    // Copy the dFdy node so we can replace it with the corrected value
    TIntermAggregate *newDfdy = node->deepCopy()->getAsAggregate();

    size_t objectSize    = node->getType().getObjectSize();
    TOperator multiplyOp = (objectSize == 1) ? EOpMul : EOpVectorTimesScalar;

    TIntermTyped *flipY = mRotationSpecConst->getFlipY();

    // Correct dFdy()'s value:
    // (dFdy() * mFlipXY.y)
    TIntermBinary *correctedDfdy = new TIntermBinary(multiplyOp, newDfdy, flipY);

    // Replace the old dFdy node with the new node that contains the corrected value
    queueReplacement(correctedDfdy, OriginalNode::IS_DROPPED);

    return true;
}
}  // anonymous namespace

bool RewriteDfdy(TCompiler *compiler,
                 ShCompileOptions compileOptions,
                 TIntermNode *root,
                 const TSymbolTable &symbolTable,
                 int shaderVersion,
                 SpecConst *specConst)
{
    // dFdy is only valid in GLSL 3.0 and later.
    if (shaderVersion < 300)
        return true;

    return Traverser::Apply(compiler, compileOptions, root, symbolTable, specConst);
}

}  // namespace sh
