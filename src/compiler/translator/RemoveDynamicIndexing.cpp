//
// Copyright (c) 2002-2015 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//
// RemoveDynamicIndexing is an AST traverser to remove dynamic indexing of vectors and matrices,
// replacing them with calls to a function that chooses which component to return.
//

#include "compiler/translator/RemoveDynamicIndexing.h"

#include "compiler/translator/InfoSink.h"
#include "compiler/translator/IntermNode.h"
#include "compiler/translator/SymbolTable.h"

namespace
{

TString GetIndexFunctionName(const TType &type, bool write)
{
    TInfoSinkBase nameSink;
    nameSink << "dyn_index_";
    if (write)
    {
        nameSink << "write_";
    }
    if (type.isMatrix())
    {
        nameSink << "mat" << type.getCols() << "x" << type.getRows();
    }
    else
    {
        switch (type.getBasicType())
        {
          case EbtInt:
            nameSink << "ivec";
            break;
          case EbtBool:
            nameSink << "bvec";
            break;
          case EbtUInt:
            nameSink << "uvec";
            break;
          case EbtFloat:
            nameSink << "vec";
            break;
          default:
            UNREACHABLE();
        }
        nameSink << type.getNominalSize();
    }
    return TFunction::mangleName(nameSink.c_str());
}

TIntermSymbol *CreateBaseSymbol(const TType &type)
{
    return new TIntermSymbol(0, "base", type);
}

TIntermSymbol *CreateIndexSymbol()
{
    return new TIntermSymbol(0, "index", TType(EbtUInt, EbpHigh));
}

TIntermSymbol *CreateValueSymbol(const TType &type)
{
    return new TIntermSymbol(0, "value", type);
}

TIntermConstantUnion *CreateUintConstantNode(unsigned int u)
{
    TConstantUnion *constant = new TConstantUnion();
    constant->setUConst(u);
    return new TIntermConstantUnion(constant, TType(EbtUInt, EbpHigh));
}

TType GetFieldType(const TType &indexedType)
{
    if (indexedType.isMatrix())
    {
        TType fieldType = TType(indexedType.getBasicType(), EbpHigh);
        fieldType.setPrimarySize(unsigned char(indexedType.getRows()));
        return fieldType;
    }
    else
    {
        return TType(indexedType.getBasicType(), EbpHigh);
    }
}

TIntermAggregate *GetIndexFunctionDefinition(const TType &type, bool write)
{
    ASSERT(!type.isArray());
    TIntermAggregate *indexingFunction = new TIntermAggregate(EOpFunction);
    indexingFunction->setName(GetIndexFunctionName(type, write));

    TType fieldType = GetFieldType(type);
    int numCases = 0;
    if (type.isMatrix())
    {
        numCases = type.getCols();
    }
    else
    {
        numCases = type.getNominalSize();
    }
    if (write)
    {
        indexingFunction->setType(TType(EbtVoid));
    }
    else
    {
        indexingFunction->setType(fieldType);
    }

    TIntermAggregate *paramsNode = new TIntermAggregate(EOpParameters);
    TIntermSymbol *baseParam = CreateBaseSymbol(type);
    if (write)
        baseParam->getTypePointer()->setQualifier(EvqInOut);
    else
        baseParam->getTypePointer()->setQualifier(EvqIn);
    paramsNode->getSequence()->push_back(baseParam);
    TIntermSymbol *indexParam = CreateIndexSymbol();
    indexParam->getTypePointer()->setQualifier(EvqIn);
    paramsNode->getSequence()->push_back(indexParam);
    if (write)
    {
        TIntermSymbol *valueParam = CreateValueSymbol(fieldType);
        valueParam->getTypePointer()->setQualifier(EvqIn);
        paramsNode->getSequence()->push_back(valueParam);
    }
    indexingFunction->getSequence()->push_back(paramsNode);

    TIntermAggregate *statementList = new TIntermAggregate(EOpSequence);
    for (int i = 0; i < numCases; ++i)
    {
        TIntermCase *caseNode = new TIntermCase(CreateUintConstantNode(i));
        statementList->getSequence()->push_back(caseNode);

        TIntermBinary *indexNode = new TIntermBinary(EOpIndexDirect);
        indexNode->setType(fieldType);
        indexNode->setLeft(CreateBaseSymbol(type));
        indexNode->setRight(CreateUintConstantNode(i));
        if (write)
        {
            TIntermBinary *assignNode = new TIntermBinary(EOpAssign);
            assignNode->setType(fieldType);
            assignNode->setLeft(indexNode);
            assignNode->setRight(CreateValueSymbol(fieldType));
            statementList->getSequence()->push_back(assignNode);
            TIntermBranch *breakNode = new TIntermBranch(EOpBreak, nullptr);
            statementList->getSequence()->push_back(breakNode);
        }
        else
        {
            TIntermBranch *returnNode = new TIntermBranch(EOpReturn, indexNode);
            statementList->getSequence()->push_back(returnNode);
        }
    }
    TIntermCase *defaultNode = new TIntermCase(nullptr);
    statementList->getSequence()->push_back(defaultNode);
    if (write)
    {
        TIntermBranch *breakNode = new TIntermBranch(EOpBreak, nullptr);
        statementList->getSequence()->push_back(breakNode);
    }
    else
    {
        TIntermBinary *returnValue = new TIntermBinary(EOpIndexDirect);
        returnValue->setType(fieldType);
        returnValue->setLeft(CreateBaseSymbol(type));
        returnValue->setRight(CreateUintConstantNode(0));
        TIntermBranch *returnNode = new TIntermBranch(EOpReturn, returnValue);
        statementList->getSequence()->push_back(returnNode);
    }
    TIntermSwitch *switchNode = new TIntermSwitch(CreateIndexSymbol(), statementList);

    TIntermAggregate *bodyNode = new TIntermAggregate(EOpSequence);
    bodyNode->getSequence()->push_back(switchNode);
    indexingFunction->getSequence()->push_back(bodyNode);

    return indexingFunction;
}

class RemoveDynamicIndexingTraverser : public TIntermTraverser
{
  public:
    RemoveDynamicIndexingTraverser();

    bool visitBinary(Visit visit, TIntermBinary *node) override;

    void insertHelperDefinitions(TIntermNode *root);

    void nextIteration();

    bool usedInsertion() const
    {
        return mUsedInsertion;
    }

  protected:
    std::set<TType> mIndexedTypes;
    std::set<TType> mWrittenTypes;

    bool mUsedInsertion;
};

RemoveDynamicIndexingTraverser::RemoveDynamicIndexingTraverser()
    : TIntermTraverser(true, false, false),
      mUsedInsertion(false)
{
}

void RemoveDynamicIndexingTraverser::insertHelperDefinitions(TIntermNode *root)
{
    TIntermAggregate *rootAgg = root->getAsAggregate();
    ASSERT(rootAgg != nullptr && rootAgg->getOp() == EOpSequence);
    TIntermSequence insertions;
    for (TType type : mIndexedTypes)
    {
        insertions.push_back(GetIndexFunctionDefinition(type, false));
    }
    for (TType type : mWrittenTypes)
    {
        insertions.push_back(GetIndexFunctionDefinition(type, true));
    }
    mInsertions.push_back(NodeInsertMultipleEntry(rootAgg, 0, insertions, TIntermSequence()));
}

bool RemoveDynamicIndexingTraverser::visitBinary(Visit visit, TIntermBinary *node)
{
    if (mUsedInsertion)
        return false;

    if (node->getOp() == EOpIndexIndirect && !node->getLeft()->isArray() && node->getLeft()->getBasicType() != EbtStruct)
    {
        bool write = isInAssignmentTarget();

        TType type = node->getLeft()->getType();
        TType fieldType = GetFieldType(type);

        TIntermAggregate *indexingCall = new TIntermAggregate(EOpInternalFunctionCall);
        indexingCall->setLine(node->getLine());
        indexingCall->setUserDefined();
        indexingCall->setName(GetIndexFunctionName(node->getLeft()->getType(), false));
        indexingCall->getSequence()->push_back(node->getLeft());
        indexingCall->getSequence()->push_back(node->getRight());
        indexingCall->setType(fieldType);

        mIndexedTypes.insert(type);
        if (write)
        {
            mWrittenTypes.insert(type);
            // Convert:
            //   a[i]++;
            // to
            //   float b = dyn_index(a, i); b++; dyn_index_write(a, i, b);
            TIntermSequence insertionsBefore;
            TIntermSequence insertionsAfter;
            insertionsBefore.push_back(createTempInitDeclaration(indexingCall));
            TIntermAggregate *indexedWriteCall = new TIntermAggregate(EOpInternalFunctionCall);
            indexedWriteCall->setLine(node->getLine());
            indexedWriteCall->setUserDefined();
            indexedWriteCall->setName(GetIndexFunctionName(node->getLeft()->getType(), true));
            indexedWriteCall->getSequence()->push_back(node->getLeft());
            indexedWriteCall->getSequence()->push_back(node->getRight());
            indexedWriteCall->getSequence()->push_back(createTempSymbol(fieldType));
            insertionsAfter.push_back(indexedWriteCall);
            insertStatementsInParentBlock(insertionsBefore, insertionsAfter);
            NodeUpdateEntry replaceIndex(getParentNode(), node, createTempSymbol(fieldType), false);
            mReplacements.push_back(replaceIndex);
            mUsedInsertion = true;
        }
        else
        {
            NodeUpdateEntry replaceIndex(getParentNode(), node, indexingCall, false);
            mReplacements.push_back(replaceIndex);
        }
    }
    return true;
}

void RemoveDynamicIndexingTraverser::nextIteration()
{
    mUsedInsertion = false;
    nextTemporaryIndex();
}

} // namespace

void RemoveDynamicIndexing(TIntermNode *root, unsigned int *temporaryIndex)
{
    RemoveDynamicIndexingTraverser traverser;
    ASSERT(temporaryIndex != nullptr);
    traverser.useTemporaryIndex(temporaryIndex);
    do
    {
        traverser.nextIteration();
        root->traverse(&traverser);
        traverser.updateTree();
    }
    while (traverser.usedInsertion());
    traverser.insertHelperDefinitions(root);
    traverser.updateTree();
}
