//
// Copyright (c) 2002-2015 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//
// RemoveDynamicIndexing is an AST traverser to remove dynamic indexing of vectors and matrices,
// replacing them with calls to functions that choose which component to return or write.
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
    TIntermSymbol *symbol = new TIntermSymbol(0, "base", type);
    symbol->setInternal(true);
    return symbol;
}

TIntermSymbol *CreateIndexSymbol()
{
    TIntermSymbol *symbol = new TIntermSymbol(0, "index", TType(EbtUInt, EbpHigh));
    symbol->setInternal(true);
    return symbol;
}

TIntermSymbol *CreateValueSymbol(const TType &type)
{
    TIntermSymbol *symbol = new TIntermSymbol(0, "value", type);
    symbol->setInternal(true);
    return symbol;
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
    TName name(GetIndexFunctionName(type, write));
    name.setInternal(true);
    indexingFunction->setNameObj(name);

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

class RemoveDynamicIndexingTraverser : public TLValueTrackingTraverser
{
  public:
    RemoveDynamicIndexingTraverser(const TSymbolTable &symbolTable, int shaderVersion);

    bool visitBinary(Visit visit, TIntermBinary *node) override;

    void insertHelperDefinitions(TIntermNode *root);

    void nextIteration();

    bool usedInsertion() const { return mUsedInsertion; }

  protected:
    std::set<TType> mIndexedTypes;
    std::set<TType> mWrittenTypes;

    bool mUsedInsertion;
};

RemoveDynamicIndexingTraverser::RemoveDynamicIndexingTraverser(const TSymbolTable &symbolTable,
                                                               int shaderVersion)
    : TLValueTrackingTraverser(true, false, false, symbolTable, shaderVersion),
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

// Create a call to dyn_index_*() based on an indirect indexing op node
TIntermAggregate *CreateIndexFunctionCall(TIntermBinary *node, TIntermTyped *index)
{
    ASSERT(node->getOp() == EOpIndexIndirect);
    TIntermAggregate *indexingCall = new TIntermAggregate(EOpFunctionCall);
    indexingCall->setLine(node->getLine());
    indexingCall->setUserDefined();
    TName name(GetIndexFunctionName(node->getLeft()->getType(), false));
    name.setInternal(true);
    indexingCall->setNameObj(name);
    indexingCall->getSequence()->push_back(node->getLeft());
    indexingCall->getSequence()->push_back(index);

    TType fieldType = GetFieldType(node->getLeft()->getType());
    indexingCall->setType(fieldType);
    return indexingCall;
}

TIntermAggregate *CreateIndexedWriteFunctionCall(TIntermBinary *node,
                                                 TIntermTyped *index,
                                                 TIntermTyped *writeTo)
{
    TIntermAggregate *indexedWriteCall = CreateIndexFunctionCall(node, index);
    indexedWriteCall->setName(GetIndexFunctionName(node->getLeft()->getType(), true));
    indexedWriteCall->setType(TType(EbtVoid));
    indexedWriteCall->getSequence()->push_back(writeTo);
    return indexedWriteCall;
}

bool RemoveDynamicIndexingTraverser::visitBinary(Visit visit, TIntermBinary *node)
{
    if (mUsedInsertion)
        return false;

    if (node->getOp() == EOpIndexIndirect && !node->getLeft()->isArray() &&
        node->getLeft()->getBasicType() != EbtStruct)
    {
        bool write = isLValueRequiredHere();

        TType type = node->getLeft()->getType();

        mIndexedTypes.insert(type);
        if (write)
        {
            // Convert:
            //   v[index_expr]++;
            // to
            //   int s0 = index_expr; float s1 = dyn_index(v, s0); s1++; dyn_index_write(v, s0, s1);
            // This works even if index_expr has some side effects.
            if (node->getLeft()->hasSideEffects())
            {
                // TODO(oetuaho@nvidia.com): This doesn't work if v has some side effects. For
                // example, it can be V[j++] where V is an array of vectors.
                // http://anglebug.com/1116
                UNIMPLEMENTED();
            }
            // TODO(oetuaho@nvidia.com): This is not optimal if the expression using the value only
            // writes it and doesn't need the previous value. http://anglebug.com/1116

            mWrittenTypes.insert(type);
            TType fieldType = GetFieldType(type);

            TIntermSequence insertionsBefore;
            TIntermSequence insertionsAfter;

            // Store the index in a temporary variable.
            TIntermAggregate *initIndex = createTempInitDeclaration(node->getRight());
            initIndex->setLine(node->getLine());
            insertionsBefore.push_back(initIndex);

            TIntermAggregate *indexingCall =
                CreateIndexFunctionCall(node, createTempSymbol(node->getRight()->getType()));

            // Create a node for referring to the index after the nextTemporaryIndex() call below.
            TIntermSymbol *tempIndex = createTempSymbol(node->getRight()->getType());

            nextTemporaryIndex();  // From now on, creating temporary symbols that refer to the
                                   // field value.
            insertionsBefore.push_back(createTempInitDeclaration(indexingCall));

            TIntermAggregate *indexedWriteCall =
                CreateIndexedWriteFunctionCall(node, tempIndex, createTempSymbol(fieldType));
            insertionsAfter.push_back(indexedWriteCall);
            insertStatementsInParentBlock(insertionsBefore, insertionsAfter);
            NodeUpdateEntry replaceIndex(getParentNode(), node, createTempSymbol(fieldType), false);
            mReplacements.push_back(replaceIndex);
            mUsedInsertion = true;
        }
        else
        {
            TIntermAggregate *indexingCall = CreateIndexFunctionCall(node, node->getRight());
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

}  // namespace

void RemoveDynamicIndexing(TIntermNode *root,
                           unsigned int *temporaryIndex,
                           const TSymbolTable &symbolTable,
                           int shaderVersion)
{
    RemoveDynamicIndexingTraverser traverser(symbolTable, shaderVersion);
    ASSERT(temporaryIndex != nullptr);
    traverser.useTemporaryIndex(temporaryIndex);
    do
    {
        traverser.nextIteration();
        root->traverse(&traverser);
        traverser.updateTree();
    } while (traverser.usedInsertion());
    traverser.insertHelperDefinitions(root);
    traverser.updateTree();
}
