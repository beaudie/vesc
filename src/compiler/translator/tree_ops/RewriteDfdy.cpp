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
#ifdef OLD_CODE
#else  // OLD_CODE
#    include "compiler/translator/StaticType.h"
#endif  // OLD_CODE
#include "compiler/translator/tree_util/IntermNode_util.h"
#include "compiler/translator/tree_util/IntermTraverse.h"

namespace sh
{

namespace
{

class Traverser : public TIntermTraverser
{
  public:
    ANGLE_NO_DISCARD static bool Apply(TCompiler *compiler,
                                       TIntermNode *root,
                                       const TSymbolTable &symbolTable,
#ifdef OLD_CODE
                                       TIntermSwizzle *viewportYScale);
#else   // OLD_CODE
                                       TIntermBinary *flipXY,
                                       TIntermTyped *fragRotation);
#endif  // OLD_CODE

  private:
#ifdef OLD_CODE
    Traverser(TIntermSwizzle *viewportYScale, TSymbolTable *symbolTable);
#else   // OLD_CODE
    Traverser(TIntermBinary *flipXY, TIntermTyped *fragRotation, TSymbolTable *symbolTable);
#endif  // OLD_CODE
    bool visitUnary(Visit visit, TIntermUnary *node) override;

#ifdef OLD_CODE
    TIntermSwizzle *mViewportYScale = nullptr;
#else   // OLD_CODE
    TIntermBinary *mFlipXY = nullptr;
    TIntermTyped *mFragRotation = nullptr;
#endif  // OLD_CODE
};

#ifdef OLD_CODE
Traverser::Traverser(TIntermSwizzle *viewportYScale, TSymbolTable *symbolTable)
    : TIntermTraverser(true, false, false, symbolTable), mViewportYScale(viewportYScale)
{}
#else   // OLD_CODE
Traverser::Traverser(TIntermBinary *flipXY, TIntermTyped *fragRotation, TSymbolTable *symbolTable)
    : TIntermTraverser(true, false, false, symbolTable),
      mFlipXY(flipXY),
      mFragRotation(fragRotation)
{}
#endif  // OLD_CODE

// static
#ifdef OLD_CODE
bool Traverser::Apply(TCompiler *compiler,
                      TIntermNode *root,
                      const TSymbolTable &symbolTable,
                      TIntermSwizzle *viewportYScale)
{
    TSymbolTable *pSymbolTable = const_cast<TSymbolTable *>(&symbolTable);
    Traverser traverser(viewportYScale, pSymbolTable);
    root->traverse(&traverser);
    return traverser.updateTree(compiler, root);
}

bool Traverser::visitUnary(Visit visit, TIntermUnary *node)
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

    // Correct dFdy()'s value:
    // (dFdy() * ANGLEUniforms.viewportYScale)
    TIntermBinary *correctedDfdy =
        new TIntermBinary(multiplyOp, newDfdy, mViewportYScale->deepCopy());

    // Replace the old dFdy node with the new node that contains the corrected value
    queueReplacement(correctedDfdy, OriginalNode::IS_DROPPED);

    return true;
}
#else   // OLD_CODE
bool Traverser::Apply(TCompiler *compiler,
                      TIntermNode *root,
                      const TSymbolTable &symbolTable,
                      TIntermBinary *flipXY,
                      TIntermTyped *fragRotation)
{
    TSymbolTable *pSymbolTable = const_cast<TSymbolTable *>(&symbolTable);
    Traverser traverser(flipXY, fragRotation, pSymbolTable);
    root->traverse(&traverser);
    return traverser.updateTree(compiler, root);
}

bool Traverser::visitUnary(Visit visit, TIntermUnary *node)
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

    // Multiply halfRotationMat by ANGLEUniforms.flipXY:
    TIntermBinary *rotatedFlipXY = new TIntermBinary(EOpMul, mFlipXY, halfRotationMat);
    TVector<int> swizzleOffsetX = {0};
    TVector<int> swizzleOffsetY = {1};
    TIntermSwizzle *multiplierX = new TIntermSwizzle(rotatedFlipXY, swizzleOffsetX);
    TIntermSwizzle *multiplierY = new TIntermSwizzle(rotatedFlipXY, swizzleOffsetY);

    // Get the results of dFdx(operand) and dFdy(operand), and multiply them by multipliers
    TIntermTyped *operandCopy = node->getOperand();
    TIntermUnary *dFdx = new TIntermUnary(EOpDFdx, operandCopy->deepCopy(), node->getFunction());
    TIntermUnary *dFdy = new TIntermUnary(EOpDFdy, operandCopy->deepCopy(), node->getFunction());
    size_t objectSize = node->getType().getObjectSize();
    TOperator multiplyOp = (objectSize == 1) ? EOpMul : EOpVectorTimesScalar;
    TIntermBinary *rotatedFlippedDfdx =
        new TIntermBinary(multiplyOp, dFdx->deepCopy(), multiplierX->deepCopy());
    TIntermBinary *rotatedFlippedDfdy =
        new TIntermBinary(multiplyOp, dFdy->deepCopy(), multiplierY->deepCopy());

    // Sum them together into the result:
    TIntermBinary *correctedDfdy =
        new TIntermBinary(EOpAdd, rotatedFlippedDfdx->deepCopy(), rotatedFlippedDfdy->deepCopy());

    // Replace the old dFdy node with the new node that contains the corrected value
    queueReplacement(correctedDfdy, OriginalNode::IS_DROPPED);

    return true;
}
#endif  // OLD_CODE

}  // anonymous namespace

bool RewriteDfdy(TCompiler *compiler,
                 TIntermNode *root,
                 const TSymbolTable &symbolTable,
                 int shaderVersion,
#ifdef OLD_CODE
                 TIntermSwizzle *viewportYScale)
#else   // OLD_CODE
                 TIntermBinary *flipXY,
                 TIntermTyped *fragRotation)
#endif  // OLD_CODE
{
    // dFdy is only valid in GLSL 3.0 and later.
    if (shaderVersion < 300)
        return true;

#ifdef OLD_CODE
    return Traverser::Apply(compiler, root, symbolTable, viewportYScale);
#else   // OLD_CODE
    return Traverser::Apply(compiler, root, symbolTable, flipXY, fragRotation);
#endif  // OLD_CODE
}

}  // namespace sh
