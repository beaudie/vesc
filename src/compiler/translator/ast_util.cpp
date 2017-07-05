//
// Copyright (c) 2017 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//

#include "compiler/translator/ast_util.h"

namespace sh
{

namespace
{

TName GetInternalFunctionName(const char *name)
{
    TString nameStr(name);
    TName nameObj(nameStr);
    nameObj.setInternal(true);
    return nameObj;
}
}

TIntermFunctionPrototype *CreateInternalFunctionPrototypeNode(const TType &returnType,
                                                              const char *name,
                                                              const TSymbolUniqueId &functionId)
{
    TIntermFunctionPrototype *functionNode = new TIntermFunctionPrototype(returnType, functionId);
    functionNode->getFunctionSymbolInfo()->setNameObj(GetInternalFunctionName(name));
    return functionNode;
}

TIntermFunctionDefinition *CreateInternalFunctionDefinitionNode(const TType &returnType,
                                                                const char *name,
                                                                TIntermBlock *functionBody,
                                                                const TSymbolUniqueId &functionId)
{
    TIntermFunctionPrototype *prototypeNode =
        CreateInternalFunctionPrototypeNode(returnType, name, functionId);
    return new TIntermFunctionDefinition(prototypeNode, functionBody);
}

TIntermAggregate *CreateInternalFunctionCallNode(const TType &returnType,
                                                 const char *name,
                                                 const TSymbolUniqueId &functionId,
                                                 TIntermSequence *arguments)
{
    TIntermAggregate *functionNode = TIntermAggregate::CreateFunctionCall(
        returnType, functionId, GetInternalFunctionName(name), arguments);
    return functionNode;
}

TIntermConstantUnion *CreateIndexNode(int index)
{
    TConstantUnion *u = new TConstantUnion[1];
    u[0].setIConst(index);

    TType type(EbtInt, EbpUndefined, EvqConst, 1);
    TIntermConstantUnion *node = new TIntermConstantUnion(u, type);
    return node;
}

TIntermTyped *CreateZero(const TType &type)
{
    TType constType(type);
    constType.setQualifier(EvqConst);

    if (!type.isArray() && type.getBasicType() != EbtStruct)
    {
        size_t size       = constType.getObjectSize();
        TConstantUnion *u = new TConstantUnion[size];
        for (size_t i = 0; i < size; ++i)
        {
            switch (type.getBasicType())
            {
                case EbtFloat:
                    u[i].setFConst(0.0f);
                    break;
                case EbtInt:
                    u[i].setIConst(0);
                    break;
                case EbtUInt:
                    u[i].setUConst(0u);
                    break;
                case EbtBool:
                    u[i].setBConst(false);
                    break;
                default:
                    // CreateZero is called by ParseContext that keeps parsing even when an error
                    // occurs, so it is possible for CreateZero to be called with non-basic types.
                    // This happens only on error condition but CreateZero needs to return a value
                    // with the correct type to continue the typecheck. That's why we handle
                    // non-basic type by setting whatever value, we just need the type to be right.
                    u[i].setIConst(42);
                    break;
            }
        }

        TIntermConstantUnion *node = new TIntermConstantUnion(u, constType);
        return node;
    }

    if (type.getBasicType() == EbtVoid)
    {
        // Void array. This happens only on error condition, similarly to the case above. We don't
        // have a constructor operator for void, so this needs special handling. We'll end up with a
        // value without the array type, but that should not be a problem.
        constType.clearArrayness();
        return CreateZero(constType);
    }

    TIntermSequence *arguments = new TIntermSequence();

    if (type.isArray())
    {
        TType elementType(type);
        elementType.clearArrayness();

        size_t arraySize = type.getArraySize();
        for (size_t i = 0; i < arraySize; ++i)
        {
            arguments->push_back(CreateZero(elementType));
        }
    }
    else
    {
        ASSERT(type.getBasicType() == EbtStruct);

        TStructure *structure = type.getStruct();
        for (const auto &field : structure->fields())
        {
            arguments->push_back(CreateZero(*field->type()));
        }
    }

    return TIntermAggregate::CreateConstructor(constType, arguments);
}

TIntermConstantUnion *CreateBool(bool value)
{
    TConstantUnion *u = new TConstantUnion[1];
    u[0].setBConst(value);

    TType type(EbtBool, EbpUndefined, EvqConst, 1);
    TIntermConstantUnion *node = new TIntermConstantUnion(u, type);
    return node;
}

}  // namespace sh
