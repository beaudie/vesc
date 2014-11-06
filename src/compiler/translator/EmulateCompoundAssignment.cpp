//
// Copyright (c) 2002-2014 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//

#include "compiler/translator/EmulateCompoundAssignment.h"

namespace
{

static void writeCompoundAssignmentPrecisionEmulation(TInfoSinkBase& sink, const char *lType, const char *rType, const char *opStr, const char *opNameStr)
{
    // Note that y should be passed through webgl_frm at the function call site,
    // but x can't be passed through webgl_frm there since it is an inout parameter.
    // So only pass x and the result through webgl_frm here.
    sink <<
    lType << " webgl_compound_" << opNameStr << "_frm(inout " << lType << " x, in " << rType << " y) {\n"
    "    x = webgl_frm(webgl_frm(x) " << opStr << " y);\n"
    "    return x;\n"
    "}\n";
    sink <<
    lType << " webgl_compound_" << opNameStr << "_frl(inout " << lType << " x, in " << rType << " y) {\n"
    "    x = webgl_frl(webgl_frm(x) " << opStr << " y);\n"
    "    return x;\n"
    "}\n";
}

const char *getFloatTypeStr(const TType& type)
{
    switch (type.getNominalSize())
    {
      case 1:
        return "float";
      case 2:
        return type.getSecondarySize() > 1 ? "mat2" : "vec2";
      case 3:
        return type.getSecondarySize() > 1 ? "mat3" : "vec3";
      case 4:
        return type.getSecondarySize() > 1 ? "mat4" : "vec4";
      default:
        UNREACHABLE();
        return NULL;
    }
}

}  // namespace anonymous


bool EmulateCompoundAssignment::visitBinary(Visit visit, TIntermBinary *node)
{
    const TType& type = node->getType();
    bool roundFloat = type.getBasicType() == EbtFloat &&
        !type.isNonSquareMatrix() && !type.isArray() &&
        (type.getPrecision() == EbpMedium || type.getPrecision() == EbpLow);

    if (roundFloat) {
        switch (node->getOp())
        {
          case EOpAddAssign:
            mEmulateAdd.insert(TypePair(getFloatTypeStr(type), getFloatTypeStr(node->getRight()->getType())));
            break;
          case EOpSubAssign:
            mEmulateSub.insert(TypePair(getFloatTypeStr(type), getFloatTypeStr(node->getRight()->getType())));
            break;
          case EOpMulAssign:
          case EOpVectorTimesMatrixAssign:
          case EOpVectorTimesScalarAssign:
          case EOpMatrixTimesScalarAssign:
          case EOpMatrixTimesMatrixAssign:
            mEmulateMul.insert(TypePair(getFloatTypeStr(type), getFloatTypeStr(node->getRight()->getType())));
            break;
          case EOpDivAssign:
            mEmulateDiv.insert(TypePair(getFloatTypeStr(type), getFloatTypeStr(node->getRight()->getType())));
            break;
          default:
            break;
        }
    }
    return true;
}

void EmulateCompoundAssignment::writeEmulation(TInfoSinkBase& sink)
{
    EmulationSet::const_iterator it;
    for (it = mEmulateAdd.begin(); it != mEmulateAdd.end(); it++) {
        writeCompoundAssignmentPrecisionEmulation(sink, it->lType, it->rType, "+", "add");
    }
    for (it = mEmulateSub.begin(); it != mEmulateSub.end(); it++) {
        writeCompoundAssignmentPrecisionEmulation(sink, it->lType, it->rType, "-", "sub");
    }
    for (it = mEmulateDiv.begin(); it != mEmulateDiv.end(); it++) {
        writeCompoundAssignmentPrecisionEmulation(sink, it->lType, it->rType, "/", "div");
    }
    for (it = mEmulateMul.begin(); it != mEmulateMul.end(); it++) {
        writeCompoundAssignmentPrecisionEmulation(sink, it->lType, it->rType, "*", "mul");
    }
}
