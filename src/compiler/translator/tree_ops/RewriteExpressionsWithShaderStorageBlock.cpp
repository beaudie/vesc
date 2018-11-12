//
// Copyright (c) 2018 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//
// RewriteExpressionsWithShaderStorageBlock rewrites the expressions that contain shader storage
// block calls into several simple ones that can be easily handled in the HLSL translator. After the
// AST pass, all ssbo related blocks will be like below:
//     ssbo_access_chain = ssbo_access_chain;
//     ssbo_access_chain = expr_no_ssbo;
//     lvalue_no_ssbo    = ssbo_access_chain;
//

#include "compiler/translator/tree_ops/RewriteExpressionsWithShaderStorageBlock.h"

#include "compiler/translator/tree_util/IntermNode_util.h"
#include "compiler/translator/tree_util/IntermTraverse.h"
#include "compiler/translator/util.h"

namespace sh
{
namespace
{

bool IsSSBOAsReadonlyBinaryOperand(TIntermBinary *node)
{
    if (IsInShaderStorageBlock(node->getLeft()) || IsInShaderStorageBlock(node->getRight()))
    {
        switch (node->getOp())
        {
            case EOpAdd:
            case EOpSub:
            case EOpMul:
            case EOpDiv:
            case EOpIMod:
            case EOpBitShiftLeft:
            case EOpBitShiftRight:
            case EOpBitwiseAnd:
            case EOpBitwiseXor:
            case EOpBitwiseOr:
            case EOpEqual:
            case EOpNotEqual:
            case EOpLessThan:
            case EOpGreaterThan:
            case EOpLessThanEqual:
            case EOpGreaterThanEqual:
            case EOpVectorTimesScalar:
            case EOpMatrixTimesScalar:
            case EOpVectorTimesMatrix:
            case EOpMatrixTimesVector:
            case EOpMatrixTimesMatrix:
            case EOpLogicalOr:
            case EOpLogicalXor:
            case EOpLogicalAnd:
                return true;
            default:
                return false;
        }
    }
    return false;
}

class RewriteExpressionsWithShaderStorageBlockTraverser : public TIntermTraverser
{
  public:
    RewriteExpressionsWithShaderStorageBlockTraverser(TSymbolTable *symbolTable);
    void nextIteration();
    bool foundSSBO() const { return mFoundSSBO; }

  private:
    bool visitBinary(Visit, TIntermBinary *node) override;
    TIntermSymbol *insertInitStatementInParentBlockAndReturnTempSymbol(TIntermTyped *node,
                                                                       TIntermSequence *insertions);
    bool mFoundSSBO;
};

RewriteExpressionsWithShaderStorageBlockTraverser::
    RewriteExpressionsWithShaderStorageBlockTraverser(TSymbolTable *symbolTable)
    : TIntermTraverser(true, false, false, symbolTable), mFoundSSBO(false)
{}

TIntermSymbol *RewriteExpressionsWithShaderStorageBlockTraverser::
    insertInitStatementInParentBlockAndReturnTempSymbol(TIntermTyped *node,
                                                        TIntermSequence *insertions)
{
    TVariable *tempVariable            = CreateTempVariable(mSymbolTable, &node->getType());
    TIntermDeclaration *tempInitialize = CreateTempInitDeclarationNode(tempVariable, node);
    insertions->push_back(tempInitialize);
    return CreateTempSymbolNode(tempVariable);
}

bool RewriteExpressionsWithShaderStorageBlockTraverser::visitBinary(Visit, TIntermBinary *node)
{
    if (mFoundSSBO)
        return false;

    // case 1: Compound assigment operator
    if (node->isCompoundAssignment())
    {
        //  original:
        //      lssbo += expr_no_ssbo;
        //  new:
        //      var temp = lssbo;
        //      temp += expr_no_ssbo;
        //      lssbo = temp;
        //
        //  original:
        //      lssbo += rssbo;
        //  new:
        //      var temp0 = lssbo;
        //      var temp1 = rssbo;
        //      temp0 += temp1;
        //      lssbo = temp0;
        if (IsInShaderStorageBlock(node->getLeft()))
        {

            mFoundSSBO = true;

            TIntermSequence insertions;
            TIntermSymbol *temp0Symbol =
                insertInitStatementInParentBlockAndReturnTempSymbol(node->getLeft(), &insertions);

            bool rightSSBO             = IsInShaderStorageBlock(node->getRight());
            TIntermSymbol *temp1Symbol = nullptr;
            if (rightSSBO)
            {
                temp1Symbol = insertInitStatementInParentBlockAndReturnTempSymbol(node->getRight(),
                                                                                  &insertions);
            }
            TIntermBinary *tempCompondOperate = new TIntermBinary(
                node->getOp(), temp0Symbol, rightSSBO ? temp1Symbol : node->getRight());
            insertions.push_back(tempCompondOperate);
            insertStatementsInParentBlock(insertions);

            TIntermBinary *assignTempValueToSSBO =
                new TIntermBinary(EOpAssign, node->getLeft(), temp0Symbol);
            queueReplacement(assignTempValueToSSBO, OriginalNode::IS_DROPPED);
        }
        //  original:
        //      lvalue_no_ssbo += ssbo;
        //  new:
        //      var temp = ssbo;
        //      lvalue_no_ssbo += temp;
        else if (IsInShaderStorageBlock(node->getRight()))
        {
            mFoundSSBO = true;

            TIntermSequence insertions;
            TIntermSymbol *tempSymbol =
                insertInitStatementInParentBlockAndReturnTempSymbol(node->getRight(), &insertions);
            insertStatementsInParentBlock(insertions);

            TIntermBinary *assignTempValueToLValue =
                new TIntermBinary(node->getOp(), node->getLeft(), tempSymbol);
            queueReplacement(assignTempValueToLValue, OriginalNode::IS_DROPPED);
        }
    }
    // case 2: Readonly binary operator
    //  original:
    //      ssbo0 + ssbo1 + ssbo2;
    //  new:
    //      var temp0 = ssbo0;
    //      var temp1 = ssbo1;
    //      var temp2 = ssbo2;
    //      temp0 + temp1 + temp2;
    else if (IsSSBOAsReadonlyBinaryOperand(node))
    {
        mFoundSSBO                     = true;
        bool rightSSBO                 = IsInShaderStorageBlock(node->getRight());
        bool leftSSBO                  = IsInShaderStorageBlock(node->getLeft());
        TIntermSymbol *tempRightSymbol = nullptr;
        TIntermSymbol *tempLeftSymbol  = nullptr;
        TIntermSequence insertions;
        if (rightSSBO)
        {
            tempRightSymbol =
                insertInitStatementInParentBlockAndReturnTempSymbol(node->getRight(), &insertions);
        }
        if (leftSSBO)
        {
            tempLeftSymbol =
                insertInitStatementInParentBlockAndReturnTempSymbol(node->getLeft(), &insertions);
        }

        insertStatementsInParentBlock(insertions);
        TIntermBinary *newExpr =
            new TIntermBinary(node->getOp(), leftSSBO ? tempLeftSymbol : node->getLeft(),
                              rightSSBO ? tempRightSymbol : node->getRight());
        queueReplacement(newExpr, OriginalNode::IS_DROPPED);
    }
    return !mFoundSSBO;
}

void RewriteExpressionsWithShaderStorageBlockTraverser::nextIteration()
{
    mFoundSSBO = false;
}

}  // anonymous namespace

void RewriteExpressionsWithShaderStorageBlock(TIntermNode *root, TSymbolTable *symbolTable)
{
    RewriteExpressionsWithShaderStorageBlockTraverser traverser(symbolTable);
    do
    {
        traverser.nextIteration();
        root->traverse(&traverser);
        if (traverser.foundSSBO())
            traverser.updateTree();
    } while (traverser.foundSSBO());
}
}  // namespace sh
