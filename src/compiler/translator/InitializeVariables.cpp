//
// Copyright (c) 2002-2013 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//

#include "compiler/translator/InitializeVariables.h"

#include "angle_gl.h"
#include "common/debug.h"
#include "compiler/translator/FindMain.h"
#include "compiler/translator/IntermNode.h"
#include "compiler/translator/SymbolTable.h"
#include "compiler/translator/util.h"

namespace sh
{

namespace
{

bool IsNamelessStruct(TIntermTyped *node)
{
    return (node->getBasicType() == EbtStruct && node->getType().getStruct()->name() == "");
}

TIntermSequence *CreateArrayZeroInitSequence(TIntermTyped *initializedNode);

TIntermBinary *CreateZeroInitAssignment(TIntermTyped *initializedNode)
{
    TIntermTyped *zero = TIntermTyped::CreateZero(initializedNode->getType());
    return new TIntermBinary(EOpAssign, initializedNode, zero);
}

TIntermSequence *CreateStructZeroInitSequence(TIntermTyped *initializedNode)
{
    ASSERT(initializedNode->getBasicType() == EbtStruct);
    TStructure *structType        = initializedNode->getType().getStruct();
    TIntermSequence *initSequence = new TIntermSequence();
    for (int i = 0; i < static_cast<int>(structType->fields().size()); ++i)
    {
        TIntermBinary *element = new TIntermBinary(
            EOpIndexDirectStruct, initializedNode->deepCopy(), TIntermTyped::CreateIndexNode(i));
        if (element->isArray())
        {
            TIntermSequence *arrayInit = CreateArrayZeroInitSequence(element);
            initSequence->insert(initSequence->end(), arrayInit->begin(), arrayInit->end());
        }
        else if (element->getType().isStructureContainingArrays())
        {
            TIntermSequence *structInit = CreateStructZeroInitSequence(element);
            initSequence->insert(initSequence->end(), structInit->begin(), structInit->end());
        }
        else
        {
            // Structs can't be defined inside structs, so the type of a struct field can't be a
            // nameless struct.
            ASSERT(!IsNamelessStruct(element));
            initSequence->push_back(CreateZeroInitAssignment(element));
        }
    }
    return initSequence;
}

TIntermSequence *CreateArrayZeroInitSequence(TIntermTyped *initializedNode)
{
    ASSERT(initializedNode->isArray());
    // Assign the array elements one by one to keep the AST compatible with ESSL 1.00 which
    // doesn't have array assignment.
    // Note that it is important to have the array init in the right order to workaround
    // http://crbug.com/709317
    TIntermSequence *initSequence = new TIntermSequence();
    for (unsigned int i = 0; i < initializedNode->getArraySize(); ++i)
    {
        TIntermBinary *element = new TIntermBinary(EOpIndexDirect, initializedNode->deepCopy(),
                                                   TIntermTyped::CreateIndexNode(i));
        if (element->getType().isStructureContainingArrays())
        {
            TIntermSequence *structInit = CreateStructZeroInitSequence(element);
            initSequence->insert(initSequence->end(), structInit->begin(), structInit->end());
        }
        else
        {
            initSequence->push_back(CreateZeroInitAssignment(element));
        }
    }
    return initSequence;
}

void InsertInitCode(TIntermSequence *mainBody,
                    const InitVariableList &variables,
                    const TSymbolTable &symbolTable)
{
    for (const auto &var : variables)
    {
        TString name = TString(var.name.c_str());

        TIntermSymbol *initializedSymbol = nullptr;
        if (var.isArray())
        {
            size_t pos = name.find_last_of('[');
            if (pos != TString::npos)
            {
                name = name.substr(0, pos);
            }
            TType arrayType = sh::GetShaderVariableBasicType(var);
            arrayType.setArraySize(var.elementCount());
            initializedSymbol = new TIntermSymbol(0, name, arrayType);
        }
        else if (var.isStruct())
        {
            TVariable *structInfo = reinterpret_cast<TVariable *>(symbolTable.findGlobal(name));
            ASSERT(structInfo);

            initializedSymbol = new TIntermSymbol(0, name, structInfo->getType());
        }
        else
        {
            TType type        = sh::GetShaderVariableBasicType(var);
            initializedSymbol = new TIntermSymbol(0, name, type);
        }
        TIntermSequence *initCode = CreateInitCode(initializedSymbol);
        mainBody->insert(mainBody->begin(), initCode->begin(), initCode->end());
    }
}

class InitializeLocalsTraverser : public TIntermTraverser
{
  public:
    InitializeLocalsTraverser(int shaderVersion)
        : TIntermTraverser(true, false, false), mShaderVersion(shaderVersion)
    {
    }

  protected:
    bool visitDeclaration(Visit visit, TIntermDeclaration *node) override
    {
        for (TIntermNode *declarator : *node->getSequence())
        {
            if (!mInGlobalScope && !declarator->getAsBinaryNode())
            {
                TIntermSymbol *symbol = declarator->getAsSymbolNode();
                ASSERT(symbol);
                if (symbol->getSymbol() == "")
                {
                    continue;
                }

                // Arrays may need to be initialized one element at a time, since ESSL 1.00 does not
                // support array constructors or assigning arrays.
                bool arrayConstructorUnavailable =
                    (symbol->isArray() || symbol->getType().isStructureContainingArrays()) &&
                    mShaderVersion == 100;
                // Nameless struct constructors can't be referred to, so they also need to be
                // initialized one element at a time.
                if (arrayConstructorUnavailable || IsNamelessStruct(symbol))
                {
                    // SimplifyLoopConditions should have been run so the parent node of this node
                    // should not be a loop.
                    ASSERT(getParentNode()->getAsLoopNode() == nullptr);
                    // SeparateDeclarations should have already been run, so we don't need to worry
                    // about further declarators in this declaration depending on the effects of
                    // this declarator.
                    ASSERT(node->getSequence()->size() == 1);
                    insertStatementsInParentBlock(TIntermSequence(), *CreateInitCode(symbol));
                }
                else
                {
                    TIntermBinary *init = new TIntermBinary(
                        EOpInitialize, symbol, TIntermTyped::CreateZero(symbol->getType()));
                    queueReplacementWithParent(node, symbol, init, OriginalNode::BECOMES_CHILD);
                }
            }
        }
        return false;
    }

  private:
    int mShaderVersion;
};

}  // namespace anonymous

TIntermSequence *CreateInitCode(TIntermSymbol *initializedSymbol)
{
    if (initializedSymbol->getType().isStructureContainingArrays() ||
        IsNamelessStruct(initializedSymbol))
    {
        return CreateStructZeroInitSequence(initializedSymbol);
    }
    if (initializedSymbol->isArray())
    {
        return CreateArrayZeroInitSequence(initializedSymbol);
    }

    TIntermSequence *initCode = new TIntermSequence();
    initCode->push_back(CreateZeroInitAssignment(initializedSymbol));
    return initCode;
}

void InitializeUninitializedLocals(TIntermBlock *root, int shaderVersion)
{
    InitializeLocalsTraverser traverser(shaderVersion);
    root->traverse(&traverser);
    traverser.updateTree();
}

void InitializeVariables(TIntermBlock *root,
                         const InitVariableList &vars,
                         const TSymbolTable &symbolTable)
{
    TIntermFunctionDefinition *main = FindMain(root);
    ASSERT(main != nullptr);
    TIntermBlock *body = main->getBody();
    InsertInitCode(body->getSequence(), vars, symbolTable);
}

}  // namespace sh
