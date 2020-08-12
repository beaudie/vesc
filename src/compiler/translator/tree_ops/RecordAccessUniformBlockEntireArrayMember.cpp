//
// Copyright 2020 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//
// RecordAccessUniformBlockEntireArrayMember.h:
// Collect all uniform blocks which have been accessed their entire array member.
//

#include "compiler/translator/tree_ops/RecordAccessUniformBlockEntireArrayMember.h"

#include "compiler/translator/Compiler.h"
#include "compiler/translator/tree_util/IntermNode_util.h"
#include "compiler/translator/tree_util/IntermTraverse.h"

namespace sh
{

namespace
{

// Traverser that collects all uniform blocks which have been accessed their entire array member.
class RecordAccessUniformBlockEntireArrayMemberTraverser : public TIntermTraverser
{
  public:
    RecordAccessUniformBlockEntireArrayMemberTraverser(
        std::map<int, const TInterfaceBlock *> &accessUniformBlockEntireArrayMember);

    bool visitAggregate(Visit visit, TIntermAggregate *node) override;
    bool visitBinary(Visit visit, TIntermBinary *node) override;
    bool visitBranch(Visit visit, TIntermBranch *node) override;

  private:
    void parseAccessWholeUniformBlock(TIntermTyped *node);
    std::map<int, const TInterfaceBlock *> &mAccessUniformBlockEntireArrayMember;
};

RecordAccessUniformBlockEntireArrayMemberTraverser::
    RecordAccessUniformBlockEntireArrayMemberTraverser(
        std::map<int, const TInterfaceBlock *> &accessUniformBlockEntireArrayMember)
    : TIntermTraverser(true, false, false),
      mAccessUniformBlockEntireArrayMember(accessUniformBlockEntireArrayMember)
{}

bool RecordAccessUniformBlockEntireArrayMemberTraverser::visitAggregate(Visit visit,
                                                                        TIntermAggregate *node)
{
    if (node->getOp() == EOpCallFunctionInAST || node->getOp() == EOpCallInternalRawFunction)
    {
        TIntermSequence *arguments = node->getSequence();
        for (TIntermSequence::iterator arg = arguments->begin(); arg != arguments->end(); arg++)
        {
            parseAccessWholeUniformBlock((*arg)->getAsTyped());
        }
    }

    return true;
}

bool RecordAccessUniformBlockEntireArrayMemberTraverser::visitBinary(Visit visit,
                                                                     TIntermBinary *node)
{
    switch (node->getOp())
    {
        case EOpAssign:
            parseAccessWholeUniformBlock(node->getRight());
            break;
        case EOpEqual:
        case EOpNotEqual:
            parseAccessWholeUniformBlock(node->getLeft());
            parseAccessWholeUniformBlock(node->getRight());
            break;
        default:
            break;
    }

    return true;
}

bool RecordAccessUniformBlockEntireArrayMemberTraverser::visitBranch(Visit visit,
                                                                     TIntermBranch *node)
{
    if (node->getFlowOp() == EOpReturn && node->getExpression())
    {
        parseAccessWholeUniformBlock(node->getExpression());
    }

    return true;
}

void RecordAccessUniformBlockEntireArrayMemberTraverser::parseAccessWholeUniformBlock(
    TIntermTyped *node)
{
    // There are three situations which access the entire array member of uniform block, for
    // example, uniform UBO1{
    //     mat4x4 buf1[90];
    // };

    // uniform UBO2{
    //     mat4x4 buf2[90];
    // }instance2;

    // uniform UBO3{
    //    mat4x4 buf3[90];
    // }instance3[2];

    // a. buf1;
    // b. instance2.buf2;
    // c. instance3[1].buf3.

    TIntermSymbol *symbolNode             = node->getAsSymbolNode();
    const TInterfaceBlock *interfaceBlock = nullptr;
    if (symbolNode)
    {
        const TVariable &variable = symbolNode->variable();
        const TType &variableType = variable.getType();
        TQualifier qualifier      = variableType.getQualifier();
        if (qualifier == EvqUniform && variableType.isArray())
        {
            interfaceBlock = variableType.getInterfaceBlock();
        }
    }
    else
    {
        TIntermBinary *binaryNode = node->getAsBinaryNode();
        if (binaryNode && binaryNode->getOp() == EOpIndexDirectInterfaceBlock &&
            binaryNode->getType().isArray())
        {
            symbolNode = binaryNode->getLeft()->getAsSymbolNode();
            if (!symbolNode)
            {
                binaryNode = binaryNode->getLeft()->getAsBinaryNode();
                if (binaryNode && binaryNode->getOp() == EOpIndexDirect)
                {
                    symbolNode = binaryNode->getLeft()->getAsSymbolNode();
                }
            }

            if (symbolNode)
            {
                interfaceBlock = symbolNode->variable().getType().getInterfaceBlock();
            }
        }
    }

    if (interfaceBlock)
    {
        if (mAccessUniformBlockEntireArrayMember.count(interfaceBlock->uniqueId().get()) == 0)
        {
            mAccessUniformBlockEntireArrayMember[interfaceBlock->uniqueId().get()] = interfaceBlock;
        }
    }
}

}  // namespace

bool RecordAccessUniformBlockEntireArrayMember(
    TIntermNode *root,
    std::map<int, const TInterfaceBlock *> &accessUniformBlockEntireArrayMember)
{
    RecordAccessUniformBlockEntireArrayMemberTraverser traverser(
        accessUniformBlockEntireArrayMember);
    root->traverse(&traverser);
    return true;
}

}  // namespace sh
