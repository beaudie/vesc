//
// Copyright (c) 2002-2014 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//

#include "compiler/translator/RewriteConstructor.h"
#include "compiler/translator/compilerdebug.h"

#include <algorithm>

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
    TIntermTyped *vector, int index)
{
    TIntermBinary *binary = new TIntermBinary(EOpIndexDirect);
    binary->setLeft(vector);
    TIntermConstantUnion *indexNode = ConstructIndexNode(index);
    binary->setRight(indexNode);
    return binary;
}

TIntermBinary *ConstructMatrixIndexBinaryNode(
    TIntermTyped *matrix, int colIndex, int rowIndex)
{
    TIntermBinary *colVectorNode =
        ConstructVectorIndexBinaryNode(matrix, colIndex);
    return ConstructVectorIndexBinaryNode(colVectorNode, rowIndex);
}

void ReconstructSequence(TOperator op, TIntermSequence *sequence,
                         bool replaceVector, bool replaceMatrix)
{
    int size = 0;
    switch (op)
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
    TIntermSequence original(*sequence);
    sequence->clear();
    for (size_t ii = 0; ii < original.size(); ++ii)
    {
        ASSERT(size > 0);
        TIntermTyped *node = original[ii]->getAsTyped();
        ASSERT(node);
        if (node->isScalar())
        {
            sequence->push_back(node);
            size--;
        }
        else if (node->isVector())
        {
            if (replaceVector)
            {
                int repeat = std::min(size, node->getNominalSize());
                size -= repeat;
                for (int index = 0; index < repeat; ++index)
                {
                    TIntermBinary *newNode = ConstructVectorIndexBinaryNode(
                        index == 0 ? node : node->clone(), index);
                    sequence->push_back(newNode);
                }
            }
            else
            {
                sequence->push_back(node);
                size -= node->getNominalSize();
            }
        }
        else
        {
            ASSERT(node->isMatrix());
            if (replaceMatrix)
            {
                int colIndex = 0, rowIndex = 0;
                int repeat = std::min(size, node->getCols() * node->getRows());
                size -= repeat;
                while (repeat > 0)
                {
                    TIntermBinary *newNode = ConstructMatrixIndexBinaryNode(
                        (rowIndex == 0 && colIndex == 0) ? node : node->clone(),
                        colIndex, rowIndex);
                    sequence->push_back(newNode);
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
                sequence->push_back(node);
                size -= node->getCols() * node->getRows();
            }
        }
    }
}

}  // namespace anonymous

bool RewriteConstructor::visitAggregate(Visit visit, TIntermAggregate *node)
{
    if (visit == PreVisit)
    {
        switch (node->getOp())
        {
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
            {
                ReconstructSequence(
                    node->getOp(), &(node->getSequence()), false, true);
            }
            break;
          case EOpConstructMat2:
          case EOpConstructMat3:
          case EOpConstructMat4:
            if (ContainsVectorNode(node->getSequence()))
            {
                ReconstructSequence(
                    node->getOp(), &(node->getSequence()), true, false);
            }
            break;
          default:
            break;
        }
    }
    return true;
}
