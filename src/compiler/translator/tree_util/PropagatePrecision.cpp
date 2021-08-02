//
// Copyright 2021 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//
// PropagatePrecision.cpp: Propagates precision to AST nodes.
//

#include "compiler/translator/tree_util/PropagatePrecision.h"

#include "common/hash_utils.h"
#include "common/utilities.h"
#include "compiler/translator/Compiler.h"
#include "compiler/translator/IntermNode.h"
#include "compiler/translator/Symbol.h"
#include "compiler/translator/tree_util/IntermTraverse.h"

namespace sh
{

namespace
{

TPrecision GetHigherPrecision(TPrecision left, TPrecision right)
{
    return left > right ? left : right;
}

// A traverser that determines the precision of every node based on its operands following GLSL
// rules.  Every node is always post-visited, so the precision of the children would always be
// known.
//
// TODO: fold RecordConstantPrecision into this.  Yes in spirit; in that mediump+const_highp should
// be evaluated as highp in the same conditions as that change.
class PropagatePrecisionTraverser : public TIntermTraverser
{
  public:
    PropagatePrecisionTraverser(TCompiler *compiler)
        : TIntermTraverser(false, false, true), mCompiler(compiler)
    {}

    void visitSymbol(TIntermSymbol *node) override
    {
        // Symbols derive their precision from their declaration.
        deriveFromDeclaredPrecision(node, node->getType());
    }

    void visitConstantUnion(TIntermConstantUnion *node) override
    {
        // Constants are folded at highp, but their precision may be left undefined to allow
        // optimizations.  Where it matters, such as when outputting ESSL, constants are explicitly
        // given precision-qualified names to prevent this.
        node->setDerivedPrecision(node->getType().getDeclaredPrecision());
    }

    bool visitSwizzle(Visit visit, TIntermSwizzle *node) override
    {
        // Swizzles derive their precision from the expression they are swizzling.
        node->setDerivedPrecision(node->getOperand()->getDerivedPrecision());

        return true;
    }

    bool visitUnary(Visit visit, TIntermUnary *node) override
    {
        // Unary operators generally derive their precision from their operand, except for a few
        // built-ins where this is overriden.
        switch (node->getOp())
        {
            case EOpFloatBitsToInt:
            case EOpFloatBitsToUint:
            case EOpIntBitsToFloat:
            case EOpUintBitsToFloat:
            case EOpPackSnorm2x16:
            case EOpPackUnorm2x16:
            case EOpPackHalf2x16:
            case EOpPackUnorm4x8:
            case EOpPackSnorm4x8:
            case EOpUnpackSnorm2x16:
            case EOpUnpackUnorm2x16:
            case EOpBitfieldReverse:
                node->setDerivedPrecision(EbpHigh);
                break;
            case EOpUnpackHalf2x16:
            case EOpUnpackUnorm4x8:
            case EOpUnpackSnorm4x8:
                node->setDerivedPrecision(EbpMedium);
                break;
            case EOpBitCount:
            case EOpFindLSB:
            case EOpFindMSB:
                node->setDerivedPrecision(EbpLow);
                break;
            case EOpAny:
            case EOpAll:
            case EOpIsinf:
            case EOpIsnan:
                node->setDerivedPrecision(EbpUndefined);
                break;
            default:
                node->setDerivedPrecision(node->getOperand()->getDerivedPrecision());
                break;
        }

        return true;
    }

    bool visitBinary(Visit visit, TIntermBinary *node) override
    {
        TIntermTyped *left  = node->getLeft();
        TIntermTyped *right = node->getRight();

        const TPrecision higherPrecision =
            GetHigherPrecision(left->getDerivedPrecision(), right->getDerivedPrecision());

        switch (node->getOp())
        {
            case EOpComma:
                // Comma takes the right node's value.
                node->setDerivedPrecision(right->getDerivedPrecision());
                break;

            case EOpIndexDirect:
            case EOpIndexIndirect:
                // When indexing an array, the precision of the array is preserved.
                node->setDerivedPrecision(left->getDerivedPrecision());
                break;
            case EOpIndexDirectStruct:
            case EOpIndexDirectInterfaceBlock:
            {
                // When selecting the field of a block, the precision is taken from the field's
                // declaration.
                const TFieldList &fields = node->getOp() == EOpIndexDirectStruct
                                               ? left->getType().getStruct()->fields()
                                               : left->getType().getInterfaceBlock()->fields();
                const int fieldIndex = right->getAsConstantUnion()->getIConst(0);
                deriveFromDeclaredPrecision(node, *fields[fieldIndex]->type());
                break;
            }

            case EOpEqual:
            case EOpNotEqual:
            case EOpLessThan:
            case EOpGreaterThan:
            case EOpLessThanEqual:
            case EOpGreaterThanEqual:
            case EOpLogicalAnd:
            case EOpLogicalXor:
            case EOpLogicalOr:
                // No precision specified on bool results.
                node->setDerivedPrecision(EbpUndefined);
                break;

            default:
                // All other operations are evaluated at the higher of the two operands' precision.
                node->setDerivedPrecision(higherPrecision);
                break;
        }

        return true;
    }

    bool visitTernary(Visit visit, TIntermTernary *node) override
    {
        // Ternaries derive their precision from the true expression.
        node->setDerivedPrecision(node->getTrueExpression()->getDerivedPrecision());

        return true;
    }

    bool visitAggregate(Visit visit, TIntermAggregate *node) override
    {
        const TIntermSequence &args = *node->getSequence();

        if (node->getBasicType() == EbtBool || node->getBasicType() == EbtVoid ||
            node->getBasicType() == EbtStruct)
        {
            node->setDerivedPrecision(EbpUndefined);
            return true;
        }

        // For AST function calls, take the qualifier from the declared one.
        if (node->isFunctionCall())
        {
            deriveFromDeclaredPrecision(node, node->getType());
            return true;
        }

        // Some built-ins explicitly specify their precision.
        const TOperator op = node->getOp();
        switch (op)
        {
            case EOpBitfieldExtract:
                node->setDerivedPrecision(args[0]->getAsTyped()->getDerivedPrecision());
                return true;
            case EOpBitfieldInsert:
                node->setDerivedPrecision(
                    GetHigherPrecision(args[0]->getAsTyped()->getDerivedPrecision(),
                                       args[1]->getAsTyped()->getDerivedPrecision()));
                return true;
            case EOpTextureSize:
            case EOpImageSize:
            case EOpUaddCarry:
            case EOpUsubBorrow:
            case EOpUmulExtended:
            case EOpImulExtended:
            case EOpFrexp:
            case EOpLdexp:
                node->setDerivedPrecision(EbpHigh);
                return true;
            default:
                break;
        }

        // The rest of the math operations and constructors get their precision from their
        // arguments.
        if (BuiltInGroup::IsMath(op) || op == EOpConstruct)
        {
            TPrecision precision = EbpUndefined;
            for (TIntermNode *argument : args)
            {
                precision =
                    GetHigherPrecision(argument->getAsTyped()->getDerivedPrecision(), precision);
            }
            node->setDerivedPrecision(precision);
            return true;
        }

        // Image load and atomic operations return highp.
        if (BuiltInGroup::IsImageLoad(op) || BuiltInGroup::IsImageAtomic(op) ||
            BuiltInGroup::IsAtomicCounter(op) || BuiltInGroup::IsAtomicMemory(op))
        {
            node->setDerivedPrecision(EbpHigh);
            return true;
        }

        // Texture functions return the same precision as that of the sampler.  textureSize returns
        // highp, but that's handled above.  The same is true for dFd*, interpolateAt* and
        // subpassLoad operations.
        if (BuiltInGroup::IsTexture(op) || BuiltInGroup::IsDerivativesFS(op) ||
            BuiltInGroup::IsInterpolationFS(op) || op == EOpSubpassLoad)
        {
            node->setDerivedPrecision(args[0]->getAsTyped()->getDerivedPrecision());
            return true;
        }

        // Every possibility must be explicitly handled, except for desktop-GLSL-specific built-ins
        // for which precision does't matter.
        ASSERT(IsDesktopGLSpec(mCompiler->getShaderSpec()));
        node->setDerivedPrecision(EbpHigh);
        return true;
    }

  private:
    void deriveFromDeclaredPrecision(TIntermTyped *node, const TType &type)
    {
        // For symbols and constants, propagate the declaration's precision to the intermediate
        // node.  For floats, ints and uints, the precision is necessarily specified, except for
        // some built-ins.
        const TBasicType basicType = type.getBasicType();
        const TPrecision precision = type.getDeclaredPrecision();

        if (node->getAsSymbolNode() == nullptr ||
            !gl::IsBuiltInName(node->getAsSymbolNode()->getName().data()))
        {
            assertValidPrecision(basicType, precision);
        }

        node->setDerivedPrecision(precision);
    }

    void assertValidPrecision(TBasicType basicType, TPrecision precision)
    {
        ASSERT(IsDesktopGLSpec(mCompiler->getShaderSpec()) || precision != EbpUndefined ||
               (basicType != EbtFloat && basicType != EbtInt && basicType != EbtUInt));
    }

    TCompiler *mCompiler;
};
}  // anonymous namespace

void PropagatePrecision(TCompiler *compiler, TIntermBlock *root)
{
    PropagatePrecisionTraverser propagator(compiler);
    root->traverse(&propagator);

    // The AST nodes now contain information gathered by this post-processing step, and so the tree
    // must no longer be transformed.
    compiler->enableValidateNoMoreTransformations();
}

}  // namespace sh
