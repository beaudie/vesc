//
// Copyright (c) 2002-2013 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//

#include "compiler/translator/InitializeVariables.h"

#include "angle_gl.h"
#include "common/debug.h"

namespace
{

TType constructType(sh::GLenum type)
{
    switch (type)
    {
        case GL_FLOAT:
            return TType(EbtFloat);
        case GL_FLOAT_VEC2:
            return TType(EbtFloat, 2);
        case GL_FLOAT_VEC3:
            return TType(EbtFloat, 3);
        case GL_FLOAT_VEC4:
            return TType(EbtFloat, 4);
        case GL_FLOAT_MAT2:
            return TType(EbtFloat, 2, 2);
        case GL_FLOAT_MAT3:
            return TType(EbtFloat, 3, 3);
        case GL_FLOAT_MAT4:
            return TType(EbtFloat, 4, 4);
        case GL_FLOAT_MAT2x3:
            return TType(EbtFloat, 2, 3);
        case GL_FLOAT_MAT2x4:
            return TType(EbtFloat, 2, 4);
        case GL_FLOAT_MAT3x2:
            return TType(EbtFloat, 3, 2);
        case GL_FLOAT_MAT3x4:
            return TType(EbtFloat, 3, 4);
        case GL_FLOAT_MAT4x2:
            return TType(EbtFloat, 4, 2);
        case GL_FLOAT_MAT4x3:
            return TType(EbtFloat, 4, 3);
        case GL_INT:
            return TType(EbtInt);
        case GL_INT_VEC2:
            return TType(EbtInt, 2);
        case GL_INT_VEC3:
            return TType(EbtInt, 3);
        case GL_INT_VEC4:
            return TType(EbtInt, 4);
        case GL_UNSIGNED_INT:
            return TType(EbtUInt);
        case GL_UNSIGNED_INT_VEC2:
            return TType(EbtUInt, 2);
        case GL_UNSIGNED_INT_VEC3:
            return TType(EbtUInt, 3);
        case GL_UNSIGNED_INT_VEC4:
            return TType(EbtUInt, 4);
        default:
            ASSERT(false);
            return TType();
    }
}

TIntermConstantUnion *constructConstUnionNode(const TType &type)
{
    TType myType = type;
    int size          = type.getNominalSize() * type.getSecondarySize();
    TConstantUnion *u = new TConstantUnion[size];
    for (int ii = 0; ii < size; ++ii)
    {
        switch (type.getBasicType())
        {
            case EbtFloat:
                u[ii].setFConst(0.0f);
                break;
            case EbtInt:
                u[ii].setIConst(0);
                break;
            case EbtUInt:
                u[ii].setUConst(0u);
                break;
            default:
                ASSERT(false);
                return nullptr;
        }
    }

    myType.clearArrayness();
    myType.setQualifier(EvqConst);
    TIntermConstantUnion *node = new TIntermConstantUnion(u, myType);
    return node;
}

TIntermConstantUnion *constructIndexNode(int index)
{
    TConstantUnion *u = new TConstantUnion[1];
    u[0].setIConst(index);

    TType type(EbtInt, EbpUndefined, EvqConst, 1);
    TIntermConstantUnion *node = new TIntermConstantUnion(u, type);
    return node;
}

}  // namespace anonymous

bool InitializeVariables::visitAggregate(Visit visit, TIntermAggregate *node)
{
    bool visitChildren = !mCodeInserted;
    switch (node->getOp())
    {
      case EOpSequence:
        break;
      case EOpFunction:
      {
        // Function definition.
        ASSERT(visit == PreVisit);
        if (node->getName() == "main(")
        {
            TIntermSequence *sequence = node->getSequence();
            ASSERT((sequence->size() == 1) || (sequence->size() == 2));
            TIntermAggregate *body = NULL;
            if (sequence->size() == 1)
            {
                body = new TIntermAggregate(EOpSequence);
                sequence->push_back(body);
            }
            else
            {
                body = (*sequence)[1]->getAsAggregate();
            }
            ASSERT(body);
            insertInitCode(body->getSequence());
            mCodeInserted = true;
        }
        break;
      }
      default:
        visitChildren = false;
        break;
    }
    return visitChildren;
}

void InitializeVariables::insertInitCode(TIntermSequence *sequence)
{
    for (size_t ii = 0; ii < mVariables.size(); ++ii)
    {
        const sh::ShaderVariable &var = mVariables[ii];
        ASSERT(!var.isStruct());
        TType type   = constructType(var.type);
        TString name = TString(var.name.c_str());
        if (var.isArray())
        {
            for (int index = static_cast<int>(var.arraySize) - 1; index >= 0; --index)
            {
                TIntermBinary *assign = new TIntermBinary(EOpAssign);
                sequence->insert(sequence->begin(), assign);

                TIntermBinary *indexDirect = new TIntermBinary(EOpIndexDirect);
                TIntermSymbol *symbol      = new TIntermSymbol(0, name, type);
                indexDirect->setLeft(symbol);
                TIntermConstantUnion *indexNode = constructIndexNode(index);
                indexDirect->setRight(indexNode);

                assign->setLeft(indexDirect);

                TIntermConstantUnion *zeroConst = constructConstUnionNode(type);
                assign->setRight(zeroConst);
            }
        }
        else
        {
            TIntermBinary *assign = new TIntermBinary(EOpAssign);
            sequence->insert(sequence->begin(), assign);
            TIntermSymbol *symbol = new TIntermSymbol(0, name, type);
            assign->setLeft(symbol);
            TIntermConstantUnion *zeroConst = constructConstUnionNode(type);
            assign->setRight(zeroConst);
        }

    }
}

