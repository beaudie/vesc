//
// Copyright (c) 2016 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//
// AddDefaultReturnStatements.cpp: Add default return statements to functions that do not end in a
//                                 return.
//

#include "compiler/translator/AddDefaultReturnStatements.h"

#include "compiler/translator/IntermNode.h"

namespace
{

class AddDefaultReturnStatementsTraverser : private TIntermTraverser
{
  public:
    static void apply(TIntermNode *root)
    {
        AddDefaultReturnStatementsTraverser separateInit;
        root->traverse(&separateInit);
        separateInit.updateTree();
    }

  private:
    AddDefaultReturnStatementsTraverser() : TIntermTraverser(true, false, false) {}

    static bool isFunctionWithoutReturnStatement(TIntermAggregate *node, TType *returnType)
    {
        *returnType = node->getType();
        if (node->getOp() != EOpFunction || node->getType().getBasicType() == EbtVoid)
        {
            return false;
        }

        TIntermAggregate *lastNode = node->getSequence()->back()->getAsAggregate();
        if (lastNode == nullptr)
        {
            return true;
        }

        TIntermBranch *returnNode = lastNode->getSequence()->front()->getAsBranchNode();
        if (returnNode != nullptr && returnNode->getFlowOp() == EOpReturn)
        {
            return false;
        }

        return true;
    }

    static TOperator getConstructorOperator(const TType &type)
    {
        switch (type.getBasicType())
        {
            case EbtInt:
                ASSERT(type.getSecondarySize() == 1);
                switch (type.getNominalSize())
                {
                    case 1:
                        return EOpConstructInt;
                    case 2:
                        return EOpConstructIVec2;
                    case 3:
                        return EOpConstructIVec3;
                    case 4:
                        return EOpConstructIVec4;
                    default:
                        UNREACHABLE();
                        return EOpNull;
                }

            case EbtUInt:
                ASSERT(type.getSecondarySize() == 1);
                switch (type.getNominalSize())
                {
                    case 1:
                        return EOpConstructUInt;
                    case 2:
                        return EOpConstructUVec2;
                    case 3:
                        return EOpConstructUVec3;
                    case 4:
                        return EOpConstructUVec4;
                    default:
                        UNREACHABLE();
                        return EOpNull;
                }

            case EbtBool:
                ASSERT(type.getSecondarySize() == 1);
                switch (type.getNominalSize())
                {
                    case 1:
                        return EOpConstructBool;
                    case 2:
                        return EOpConstructBVec2;
                    case 3:
                        return EOpConstructBVec3;
                    case 4:
                        return EOpConstructBVec4;
                    default:
                        UNREACHABLE();
                        return EOpNull;
                }

            case EbtFloat:
                switch (type.getNominalSize())
                {
                    case 1:
                        ASSERT(type.getSecondarySize() == 1);
                        return EOpConstructFloat;
                    case 2:
                        switch (type.getSecondarySize())
                        {
                            case 1:
                                return EOpConstructVec2;
                            case 2:
                                return EOpConstructMat2;
                            case 3:
                                return EOpConstructMat2x3;
                            case 4:
                                return EOpConstructMat2x4;
                            default:
                                UNREACHABLE();
                                return EOpNull;
                        }
                    case 3:
                        switch (type.getSecondarySize())
                        {
                            case 1:
                                return EOpConstructVec3;
                            case 2:
                                return EOpConstructMat3x2;
                            case 3:
                                return EOpConstructMat3;
                            case 4:
                                return EOpConstructMat3x4;
                            default:
                                UNREACHABLE();
                                return EOpNull;
                        }
                    case 4:
                        switch (type.getSecondarySize())
                        {
                            case 1:
                                return EOpConstructVec4;
                            case 2:
                                return EOpConstructMat4x2;
                            case 3:
                                return EOpConstructMat4x3;
                            case 4:
                                return EOpConstructMat4;
                            default:
                                UNREACHABLE();
                                return EOpNull;
                        }
                }

            case EbtStruct:
                return EOpConstructStruct;

            default:
                UNREACHABLE();
                return EOpNull;
        }
    }

    TIntermTyped *generateTypeConstructor(const TType &returnType)
    {
        // Base case, constructing a single element
        if (!returnType.isArray())
        {
            size_t objectSize             = returnType.getObjectSize();
            TConstantUnion *constantUnion = new TConstantUnion[objectSize];
            for (size_t constantIdx = 0; constantIdx < objectSize; constantIdx++)
            {
                constantUnion[constantIdx].setFConst(0.0f);
            }

            TIntermConstantUnion *intermConstantUnion =
                new TIntermConstantUnion(constantUnion, returnType);
            return intermConstantUnion;
        }

        // Recursive case, construct an array of single elements
        TIntermAggregate *constructorAggrigate =
            new TIntermAggregate(getConstructorOperator(returnType));
        constructorAggrigate->setType(returnType);

        size_t arraySize = returnType.getArraySize();
        for (size_t arrayIdx = 0; arrayIdx < arraySize; arrayIdx++)
        {
            TType arrayElementType(returnType);
            arrayElementType.clearArrayness();

            constructorAggrigate->getSequence()->push_back(
                generateTypeConstructor(arrayElementType));
        }

        return constructorAggrigate;
    }

    bool visitAggregate(Visit, TIntermAggregate *node) override
    {
        TType returnType;
        if (isFunctionWithoutReturnStatement(node, &returnType))
        {
            TIntermBranch *branch =
                new TIntermBranch(EOpReturn, generateTypeConstructor(returnType));

            TIntermAggregate *lastNode = node->getSequence()->back()->getAsAggregate();
            lastNode->getSequence()->push_back(branch);

            return false;
        }

        return true;
    }
};
}

namespace sh
{

void AddDefaultReturnStatements(TIntermNode *node)
{
    AddDefaultReturnStatementsTraverser::apply(node);
}

}  // namespace sh
