//
// Copyright 2020 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//

#include "compiler/translator/TranslatorMetalDirect/RewriteUnaddressableReferences.h"
#include "compiler/translator/TranslatorMetalDirect/AstHelpers.h"
#include "compiler/translator/tree_util/AsNode.h"
#include "compiler/translator/tree_util/IntermRebuild.h"

using namespace sh;

namespace
{

bool IsOutParam(const TType &paramType)
{
    const TQualifier qual = paramType.getQualifier();
    switch (qual)
    {
        case TQualifier::EvqParamInOut:
        case TQualifier::EvqParamOut:
            return true;

        default:
            return false;
    }
}

bool IsVectorAccess(TIntermBinary &binary)
{
    TOperator op = binary.getOp();
    switch (op)
    {
        case TOperator::EOpIndexDirect:
        case TOperator::EOpIndexIndirect:
            break;

        default:
            return false;
    }

    const TType &leftType = binary.getLeft()->getType();
    if (!leftType.isVector() || leftType.isArray())
    {
        return false;
    }

    ASSERT(IsScalarBasicType(binary.getType()));

    return true;
}

// Differs from IsAssignment in that it does not include (++) or (--).
bool IsAssignEqualsSign(TOperator op)
{
    switch (op)
    {
        case TOperator::EOpAssign:
        case TOperator::EOpInitialize:
        case TOperator::EOpAddAssign:
        case TOperator::EOpSubAssign:
        case TOperator::EOpMulAssign:
        case TOperator::EOpVectorTimesMatrixAssign:
        case TOperator::EOpVectorTimesScalarAssign:
        case TOperator::EOpMatrixTimesScalarAssign:
        case TOperator::EOpMatrixTimesMatrixAssign:
        case TOperator::EOpDivAssign:
        case TOperator::EOpIModAssign:
        case TOperator::EOpBitShiftLeftAssign:
        case TOperator::EOpBitShiftRightAssign:
        case TOperator::EOpBitwiseAndAssign:
        case TOperator::EOpBitwiseXorAssign:
        case TOperator::EOpBitwiseOrAssign:
            return true;

        default:
            return false;
    }
}

bool ReturnsReference(TOperator op)
{
    switch (op)
    {
        case TOperator::EOpAssign:
        case TOperator::EOpInitialize:
        case TOperator::EOpAddAssign:
        case TOperator::EOpSubAssign:
        case TOperator::EOpMulAssign:
        case TOperator::EOpVectorTimesMatrixAssign:
        case TOperator::EOpVectorTimesScalarAssign:
        case TOperator::EOpMatrixTimesScalarAssign:
        case TOperator::EOpMatrixTimesMatrixAssign:
        case TOperator::EOpDivAssign:
        case TOperator::EOpIModAssign:
        case TOperator::EOpBitShiftLeftAssign:
        case TOperator::EOpBitShiftRightAssign:
        case TOperator::EOpBitwiseAndAssign:
        case TOperator::EOpBitwiseXorAssign:
        case TOperator::EOpBitwiseOrAssign:

        case TOperator::EOpPostIncrement:
        case TOperator::EOpPostDecrement:
        case TOperator::EOpPreIncrement:
        case TOperator::EOpPreDecrement:

        case TOperator::EOpIndexDirect:
        case TOperator::EOpIndexIndirect:
        case TOperator::EOpIndexDirectStruct:
        case TOperator::EOpIndexDirectInterfaceBlock:

            return true;

        default:
            return false;
    }
}

class Rewriter : public TIntermRebuild
{
    std::vector<bool> mRequiresAddressingStack;
    SymbolEnv &mSymbolEnv;

  private:
    bool requiresAddressing() const
    {
        if (mRequiresAddressingStack.empty())
        {
            return false;
        }
        return mRequiresAddressingStack.back();
    }

  public:
    ~Rewriter() override { ASSERT(mRequiresAddressingStack.empty()); }

    Rewriter(TCompiler &compiler, SymbolEnv &symbolEnv)
        : TIntermRebuild(compiler, true, true), mSymbolEnv(symbolEnv)
    {}

    PreResult visitAggregatePre(TIntermAggregate &aggregateNode) override
    {
        const TFunction *func = aggregateNode.getFunction();
        if (!func)
        {
            return aggregateNode;
        }

        TIntermSequence &args = *aggregateNode.getSequence();
        size_t argCount       = args.size();

        for (size_t i = 0; i < argCount; ++i)
        {
            const TVariable &param = *func->getParam(i);
            const TType &paramType = param.getType();
            TIntermNode *arg       = args[i];
            ASSERT(arg);

            mRequiresAddressingStack.push_back(IsOutParam(paramType));
            args[i] = rebuild(*arg).single();
            ASSERT(args[i]);
            ASSERT(!mRequiresAddressingStack.empty());
            mRequiresAddressingStack.pop_back();
        }

        return {aggregateNode, VisitBits::Neither};
    }

    PostResult visitSwizzlePost(TIntermSwizzle &swizzleNode) override
    {
        if (!requiresAddressing())
        {
            return swizzleNode;
        }

        TIntermTyped &vecNode         = *swizzleNode.getOperand();
        const TQualifierList &offsets = swizzleNode.getSwizzleOffsets();
        ASSERT(!offsets.empty());
        ASSERT(offsets.size() <= 4);

        auto &args = *new TIntermSequence();
        args.reserve(offsets.size() + 1);
        args.push_back(&vecNode);
        for (int offset : offsets)
        {
            args.push_back(new TIntermConstantUnion(new TConstantUnion(offset),
                                                    *new TType(TBasicType::EbtInt)));
        }

        return mSymbolEnv.callFunctionOverload(Name("swizzle_ref"), swizzleNode.getType(), args);
    }

    PreResult visitBinaryPre(TIntermBinary &binaryNode) override
    {
        const TOperator op = binaryNode.getOp();

        const bool isAccess = IsVectorAccess(binaryNode);

        const bool disableTop   = !ReturnsReference(op) || !requiresAddressing();
        const bool disableLeft  = disableTop;
        const bool disableRight = disableTop || isAccess || IsAssignEqualsSign(op);

        auto traverse = [&](TIntermTyped &node, const bool disable) -> TIntermTyped & {
            if (disable)
            {
                mRequiresAddressingStack.push_back(false);
            }
            auto *newNode = asNode<TIntermTyped>(rebuild(node).single());
            ASSERT(newNode);
            if (disable)
            {
                mRequiresAddressingStack.pop_back();
            }
            return *newNode;
        };

        TIntermTyped &leftNode  = *binaryNode.getLeft();
        TIntermTyped &rightNode = *binaryNode.getRight();

        TIntermTyped &newLeft  = traverse(leftNode, disableLeft);
        TIntermTyped &newRight = traverse(rightNode, disableRight);

        if (!isAccess || disableTop)
        {
            if (&leftNode == &newLeft && &rightNode == &newRight)
            {
                return {&binaryNode, VisitBits::Neither};
            }
            return {*new TIntermBinary(op, &newLeft, &newRight), VisitBits::Neither};
        }

        return {mSymbolEnv.callFunctionOverload(Name("elem_ref"), binaryNode.getType(),
                                                *new TIntermSequence{&newLeft, &newRight}),
                VisitBits::Neither};
    }
};

}  // anonymous namespace

bool sh::RewriteUnaddressableReferences(TCompiler &compiler,
                                        TIntermBlock &root,
                                        SymbolEnv &symbolEnv)
{
    if (!Rewriter(compiler, symbolEnv).rebuildRoot(root))
    {
        return false;
    }
    return true;
}
