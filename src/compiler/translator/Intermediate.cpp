//
// Copyright (c) 2002-2014 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//

//
// Build the intermediate representation.
//

#include <float.h>
#include <limits.h>
#include <algorithm>

#include "compiler/translator/Intermediate.h"
#include "compiler/translator/SymbolTable.h"

namespace sh
{

////////////////////////////////////////////////////////////////////////////
//
// First set of functions are to help build the intermediate representation.
// These functions are not member functions of the nodes.
// They are called from parser productions.
//
/////////////////////////////////////////////////////////////////////////////

//
// Connect two nodes through an index operator, where the left node is the base
// of an array or struct, and the right node is a direct or indirect offset.
//
// Returns the added node.
// The caller should set the type of the returned node.
//
TIntermTyped *TIntermediate::addIndex(TOperator op,
                                      TIntermTyped *base,
                                      TIntermTyped *index,
                                      const TSourceLoc &line,
                                      TDiagnostics *diagnostics)
{
    TIntermBinary *node = new TIntermBinary(op, base, index);
    node->setLine(line);

    TIntermTyped *folded = node->fold(diagnostics);
    if (folded)
    {
        return folded;
    }

    return node;
}

// If the input node is nullptr, return nullptr.
// If the input node is a block node, return it.
// If the input node is not a block node, put it inside a block node and return that.
TIntermBlock *TIntermediate::EnsureBlock(TIntermNode *node)
{
    if (node == nullptr)
        return nullptr;
    TIntermBlock *blockNode = node->getAsBlock();
    if (blockNode != nullptr)
        return blockNode;

    blockNode = new TIntermBlock();
    blockNode->setLine(node->getLine());
    blockNode->appendStatement(node);
    return blockNode;
}

//
// Constant terminal nodes.  Has a union that contains bool, float or int constants
//
// Returns the constant union node created.
//

TIntermConstantUnion *TIntermediate::addConstantUnion(const TConstantUnion *constantUnion,
                                                      const TType &type,
                                                      const TSourceLoc &line)
{
    TIntermConstantUnion *node = new TIntermConstantUnion(constantUnion, type);
    node->setLine(line);

    return node;
}

TIntermTyped *TIntermediate::foldAggregateBuiltIn(TIntermAggregate *aggregate,
                                                  TDiagnostics *diagnostics)
{
    switch (aggregate->getOp())
    {
        case EOpAtan:
        case EOpPow:
        case EOpMod:
        case EOpMin:
        case EOpMax:
        case EOpClamp:
        case EOpMix:
        case EOpStep:
        case EOpSmoothStep:
        case EOpLdexp:
        case EOpMulMatrixComponentWise:
        case EOpOuterProduct:
        case EOpEqualComponentWise:
        case EOpNotEqualComponentWise:
        case EOpLessThanComponentWise:
        case EOpLessThanEqualComponentWise:
        case EOpGreaterThanComponentWise:
        case EOpGreaterThanEqualComponentWise:
        case EOpDistance:
        case EOpDot:
        case EOpCross:
        case EOpFaceforward:
        case EOpReflect:
        case EOpRefract:
        case EOpBitfieldExtract:
        case EOpBitfieldInsert:
            return aggregate->fold(diagnostics);
        default:
            // TODO: Add support for folding array constructors
            if (aggregate->isConstructor() && !aggregate->isArray())
            {
                return aggregate->fold(diagnostics);
            }
            // Constant folding not supported for the built-in.
            return aggregate;
    }
}

}  // namespace sh
