//
// Copyright (c) 2002-2014 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//

#include "compiler/translator/ScalarizeVecAndMatConstructorArgs.h"
#include "compiler/translator/compilerdebug.h"

#include <algorithm>

#include "common/angleutils.h"

namespace
{

bool ContainsMatrixNode(const TIntermSequence &sequence)
{
    for (size_t ii = 0; ii < sequence.size(); ++ii)
    {
        TIntermTyped *node = sequence[ii]->getAsTyped();
        if (node && node->isMatrix())
            return true;
    }
    return false;
}

bool ContainsVectorNode(const TIntermSequence &sequence)
{
    for (size_t ii = 0; ii < sequence.size(); ++ii)
    {
        TIntermTyped *node = sequence[ii]->getAsTyped();
        if (node && node->isVector())
            return true;
    }
    return false;
}

TIntermConstantUnion *ConstructIndexNode(int index)
{
    ConstantUnion *u = new ConstantUnion[1];
    u[0].setIConst(index);

    TType type(EbtInt, EbpUndefined, EvqConst, 1);
    TIntermConstantUnion *node = new TIntermConstantUnion(u, type);
    return node;
}

TIntermBinary *ConstructVectorIndexBinaryNode(
    const TString& varName, const TType& varType, int index)
{
    TIntermBinary *binary = new TIntermBinary(EOpIndexDirect);
    TIntermSymbol *symbolNode = new TIntermSymbol(-1, varName, varType);
    binary->setLeft(symbolNode);
    TIntermConstantUnion *indexNode = ConstructIndexNode(index);
    binary->setRight(indexNode);
    return binary;
}

TIntermBinary *ConstructMatrixIndexBinaryNode(
    const TString &varName, const TType &varType, int colIndex, int rowIndex)
{
    TIntermBinary *colVectorNode =
        ConstructVectorIndexBinaryNode(varName, varType, colIndex);

    TIntermBinary *binary = new TIntermBinary(EOpIndexDirect);
    binary->setLeft(colVectorNode);
    TIntermConstantUnion *rowIndexNode = ConstructIndexNode(rowIndex);
    binary->setRight(rowIndexNode);
    return binary;
}

}  // namespace anonymous

bool ScalarizeVecAndMatConstructorArgs::visitAggregate(Visit visit, TIntermAggregate *node)
{
    if (visit == PreVisit)
    {
        switch (node->getOp())
        {
          case EOpSequence:
            mSequenceStack.push_back(TIntermSequence());
            {
                for (TIntermSequence::const_iterator iter = node->getSequence().begin();
                     iter != node->getSequence().end(); ++iter)
                {
                    TIntermNode *child = *iter;
                    ASSERT(child != NULL);
                    child->traverse(this);
                    mSequenceStack.back().push_back(child);
                }
            }
            if (mSequenceStack.back().size() > node->getSequence().size())
            {
                node->getSequence().clear();
                node->getSequence() = mSequenceStack.back();
            }
            mSequenceStack.pop_back();
            return false;
          case EOpConstructVec2:
          case EOpConstructVec3:
          case EOpConstructVec4:
          case EOpConstructBVec2:
          case EOpConstructBVec3:
          case EOpConstructBVec4:
          case EOpConstructIVec2:
          case EOpConstructIVec3:
          case EOpConstructIVec4:
            if (ContainsMatrixNode(node->getSequence()))
                scalarizeArgs(node, false, true);
            break;
          case EOpConstructMat2:
          case EOpConstructMat3:
          case EOpConstructMat4:
            if (ContainsVectorNode(node->getSequence()))
                scalarizeArgs(node, true, false);
            break;
          default:
            break;
        }
    }
    return true;
}

void ScalarizeVecAndMatConstructorArgs::scalarizeArgs(
    TIntermAggregate *aggregate, bool scalarizeVector, bool scalarizeMatrix)
{
    ASSERT(aggregate);
    int size = 0;
    switch (aggregate->getOp())
    {
      case EOpConstructVec2:
      case EOpConstructBVec2:
      case EOpConstructIVec2:
        size = 2;
        break;
      case EOpConstructVec3:
      case EOpConstructBVec3:
      case EOpConstructIVec3:
        size = 3;
        break;
      case EOpConstructVec4:
      case EOpConstructBVec4:
      case EOpConstructIVec4:
      case EOpConstructMat2:
        size = 4;
        break;
      case EOpConstructMat3:
        size = 9;
        break;
      case EOpConstructMat4:
        size = 16;
        break;
      default:
        break;
    }
    TIntermSequence &sequence = aggregate->getSequence();
    TIntermSequence original(sequence);
    sequence.clear();
    for (size_t ii = 0; ii < original.size(); ++ii)
    {
        ASSERT(size > 0);
        TIntermTyped *node = original[ii]->getAsTyped();
        ASSERT(node);
        if (node->isScalar())
        {
            sequence.push_back(node);
            size--;
        }
        else if (node->isVector())
        {
            if (scalarizeVector)
            {
                int repeat = std::min(size, node->getNominalSize());
                size -= repeat;
                TString varName = createTempVariable(node);
                for (int index = 0; index < repeat; ++index)
                {
                    TIntermBinary *newNode = ConstructVectorIndexBinaryNode(
                        varName, node->getType(), index);
                    sequence.push_back(newNode);
                }
            }
            else
            {
                sequence.push_back(node);
                size -= node->getNominalSize();
            }
        }
        else
        {
            ASSERT(node->isMatrix());
            if (scalarizeMatrix)
            {
                int colIndex = 0, rowIndex = 0;
                int repeat = std::min(size, node->getCols() * node->getRows());
                size -= repeat;
                TString varName = createTempVariable(node);
                while (repeat > 0)
                {
                    TIntermBinary *newNode = ConstructMatrixIndexBinaryNode(
                        varName, node->getType(), colIndex, rowIndex);
                    sequence.push_back(newNode);
                    rowIndex++;
                    if (rowIndex >= node->getRows())
                    {
                        rowIndex = 0;
                        colIndex++;
                    }
                    repeat--;
                }
            }
            else
            {
                sequence.push_back(node);
                size -= node->getCols() * node->getRows();
            }
        }
    }
}

TString ScalarizeVecAndMatConstructorArgs::createTempVariable(TIntermTyped *original)
{
    TString tempVarName = "_webgl_tmp_";
    if (original->isMatrix())
        tempVarName += "mat_";
    else
        tempVarName += "vec_";
    tempVarName += Str(mTempVarCount).c_str();
    mTempVarCount++;

    ASSERT(original);
    TType type = original->getType();
    type.setQualifier(EvqTemporary);

    TIntermBinary *init = new TIntermBinary(EOpInitialize);
    TIntermSymbol *symbolNode = new TIntermSymbol(-1, tempVarName, type);
    init->setLeft(symbolNode);
    init->setRight(original->clone());
    init->setType(type);

    TIntermAggregate *decl = new TIntermAggregate(EOpDeclaration);
    decl->getSequence().push_back(init);

    ASSERT(mSequenceStack.size() > 0);
    TIntermSequence &sequence = mSequenceStack.back();
    sequence.push_back(decl);

    return tempVarName;
}
