//
// Copyright (c) 2016 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//
// Implementation of texelFetchOffset translation issue workaround.
// See header for more info.

#include "compiler/translator/RewriteTexelFetchOffset.h"

#include "common/angleutils.h"
#include "compiler/translator/IntermNode.h"
#include "compiler/translator/SymbolTable.h"

namespace sh
{

namespace
{

// Referenced from ExpandIntegerPowExpressions.cpp.
class Traverser : public TIntermTraverser
{
public:
    static void Apply(TIntermNode *root,
        unsigned int *tempIndex,
        const TSymbolTable &symbolTable,
        int shaderVersion);

private:
    Traverser(const TSymbolTable &symbolTable, int shaderVersion);
    bool visitAggregate(Visit visit, TIntermAggregate *node) override;
    void nextIteration();

    struct TexelFetchInfo
    {
        const char *functionName;
        int uniqueId;
        bool is2DArray;
        TexelFetchInfo(const char *functionName, int uniqueId, bool is2DArray)
            : functionName(functionName), uniqueId(uniqueId), is2DArray(is2DArray)
        {
        }
    };

    typedef TMap<TString, TexelFetchInfo> TexelFetchTable;
    typedef TexelFetchTable::value_type TexelFetchPair;

    TexelFetchTable texelFetchTable;

    bool mFound = false;

    const static int TEXELFETCHOFFSET_COUNT = 9;
    const static int TEXELFETCHOFFSET_2D_ARRAY_COUNT = 3;
    const static char *TEXELFETCHOFFSET_LIST[];
    const static char *TEXELFETCH_LIST[];
};

// Names of texelFetchOffset.
const char *Traverser::TEXELFETCHOFFSET_LIST [] =
{
    "texelFetchOffset(is2a1;vi3;i1;vi2;",
    "texelFetchOffset(s2a1;vi3;i1;vi2;",
    "texelFetchOffset(us2a1;vi3;i1;vi2;",
    "texelFetchOffset(is21;vi2;i1;vi2;",
    "texelFetchOffset(is31;vi3;i1;vi3;",
    "texelFetchOffset(s21;vi2;i1;vi2;",
    "texelFetchOffset(s31;vi3;i1;vi3;",
    "texelFetchOffset(us21;vi2;i1;vi2;",
    "texelFetchOffset(us31;vi3;i1;vi3;",
};

// Names of texelFetch.
const char *Traverser::TEXELFETCH_LIST[] =
{
    "texelFetch(is2a1;vi3;i1;",
    "texelFetch(s2a1;vi3;i1;",
    "texelFetch(us2a1;vi3;i1;",
    "texelFetch(is21;vi2;i1;",
    "texelFetch(is31;vi3;i1;",
    "texelFetch(s21;vi2;i1;",
    "texelFetch(s31;vi3;i1;",
    "texelFetch(us21;vi2;i1;",
    "texelFetch(us31;vi3;i1;"
};

Traverser::Traverser(const TSymbolTable &symbolTable, int shaderVersion)
    : TIntermTraverser(true, false, false)
{
    // TEXELFETCHOFFSET_LIST[i] will be translated into TEXELFETCHOFFSET_LIST[i].
    for (int i = 0; i < TEXELFETCHOFFSET_COUNT; i++)
    {
        const char *texelFetch = TEXELFETCH_LIST[i];
        TSymbol *symbol = symbolTable.findBuiltIn(texelFetch, shaderVersion);
        if (symbol == nullptr)
            continue;

        const char *texelFetchOffset = TEXELFETCHOFFSET_LIST[i];
        int uniqueId = symbol->getUniqueId();
        bool is2DArray = i < TEXELFETCHOFFSET_2D_ARRAY_COUNT;
        TexelFetchInfo texelFetchInfo(texelFetch, uniqueId, is2DArray);
        texelFetchTable.insert(TexelFetchPair(texelFetchOffset, texelFetchInfo));
    }
}


// static
void Traverser::Apply(TIntermNode *root,
                      unsigned int *tempIndex,
                      const TSymbolTable &symbolTable,
                      int shaderVersion)
{
    Traverser traverser(symbolTable, shaderVersion);
    traverser.useTemporaryIndex(tempIndex);
    do
    {
        traverser.nextIteration();
        root->traverse(&traverser);
        if (traverser.mFound)
        {
            traverser.updateTree();
        }
    } while (traverser.mFound);
}

void Traverser::nextIteration()
{
    mFound = false;
    nextTemporaryIndex();
}

bool Traverser::visitAggregate(Visit visit, TIntermAggregate *node)
{
    if (mFound)
    {
        return false;
    }

    // Decide if the node represents the call of texelFetchOffset.
    if (node->getOp() != EOpFunctionCall)
    {
        return true;
    }

    TexelFetchTable::const_iterator it = texelFetchTable.find(node->getName());
    if (it == texelFetchTable.end())
    {
        return true;
    }

    // Potential problem case detected, apply workaround.
    const TIntermSequence *sequence = node->getSequence();
    ASSERT(sequence->size() == 4);
    nextTemporaryIndex();

    // Create new node that represents the call of function texelFetch.
    TIntermAggregate *texelFetchNode = new TIntermAggregate(EOpFunctionCall);
    texelFetchNode->setName(it->second.functionName);
    texelFetchNode->setFunctionId(it->second.uniqueId);
    texelFetchNode->setType(node->getType());
    texelFetchNode->setLine(node->getLine());

    // Create argument List of texelFetch(sampler, Position+offset, lod).
    TIntermSequence newsequence;

    // sampler
    newsequence.push_back(sequence->at(0));

    // Position+offset
    TIntermBinary *add = new TIntermBinary(EOpAdd);
    add->setType(node->getType());
    // Position
    TIntermTyped *texCoordNode = sequence->at(1)->getAsTyped();
    ASSERT(texCoordNode);
    add->setLine(texCoordNode->getLine());
    add->setType(texCoordNode->getType());
    add->setLeft(texCoordNode);
    // offset
    ASSERT(sequence->at(3)->getAsTyped());
    if (it->second.is2DArray)
    {
        // For 2DArray samplers, Position is ivec3 and offset is ivec2;
        // So offset must be converted into an ivec3 before being added to Position.
        TIntermAggregate *constructIVec3Node = new TIntermAggregate(EOpConstructIVec3);
        constructIVec3Node->setLine(texCoordNode->getLine());
        constructIVec3Node->setType(texCoordNode->getType());

        TIntermSequence ivec3Sequence;
        ivec3Sequence.push_back(sequence->at(3)->getAsTyped());

        TConstantUnion *zero = new TConstantUnion();
        zero->setIConst(0);
        TType *intType = new TType(EbtInt);

        TIntermConstantUnion *zeroNode = new TIntermConstantUnion(zero, *intType);
        ivec3Sequence.push_back(zeroNode);
        constructIVec3Node->insertChildNodes(0, ivec3Sequence);

        add->setRight(constructIVec3Node);
    }
    else
    {
        add->setRight(sequence->at(3)->getAsTyped());
    }
    newsequence.push_back(add);

    // lod
    newsequence.push_back(sequence->at(2));
    texelFetchNode->insertChildNodes(0, newsequence);

    // Replace the old node by this new node.
    queueReplacement(node, texelFetchNode, OriginalNode::IS_DROPPED);
    mFound = true;
    return false;
}

}

void RewriteTexelFetchOffset(TIntermNode *root,
                             unsigned int *tempIndex,
                             const TSymbolTable &symbolTable,
                             int shaderVersion)
{
    Traverser::Apply(root, tempIndex, symbolTable, shaderVersion);
}

}  // namespace sh