//
// Copyright 2016 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//

#include "compiler/translator/UseInterfaceBlockFields.h"

#include "angle_gl.h"
#include "compiler/translator/IntermNode.h"
#include "compiler/translator/util.h"

namespace sh
{

namespace
{

class UseUniformBlockMembers : public TIntermTraverser
{
  public:
    UseUniformBlockMembers(const UseBlockFieldList &blocks)
        : TIntermTraverser(true, false, false), mBlocks(blocks), mCodeInserted(false)
    {
    }

  protected:
    bool visitAggregate(Visit visit, TIntermAggregate *node) override;

  private:
    void insertUseCode(TIntermSequence *sequence);

    const UseBlockFieldList &mBlocks;
    bool mCodeInserted;
};

bool UseUniformBlockMembers::visitAggregate(Visit visit, TIntermAggregate *node)
{
    bool visitChildren = !mCodeInserted;
    switch (node->getOp())
    {
        case EOpSequence:
            break;
        case EOpFunction:
        {
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
                insertUseCode(body->getSequence());
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

void UseUniformBlockMembers::insertUseCode(TIntermSequence *sequence)
{
    for (const auto &block : mBlocks)
    {
        if (block.instanceName.empty())
        {
            const unsigned int numMembers = static_cast<unsigned int>(block.fields.size());
            for (unsigned int memberIndex = 0; memberIndex < numMembers; memberIndex++)
            {
                sh::ShaderVariable var = block.fields[memberIndex];
                TString name           = TString(var.name.c_str());
                TType type             = sh::GetShaderVariableType(var);

                if (var.isArray())
                {
                    size_t pos = name.find_last_of('[');
                    if (pos != TString::npos)
                    {
                        name = name.substr(0, pos);
                    }
                    TType elementType = type;
                    elementType.clearArrayness();

                    for (unsigned int i = 0; i < var.arraySize; ++i)
                    {
                        TIntermSymbol *arraySymbol = new TIntermSymbol(0, name, type);
                        TIntermBinary *element     = new TIntermBinary(
                            EOpIndexDirect, arraySymbol, TIntermTyped::CreateIndexNode(i));

                        TIntermAggregate *tempDeclaration = nullptr;
                        {
                            tempDeclaration = createTempInitDeclaration(element);
                        }

                        sequence->insert(sequence->begin(), tempDeclaration);
                        nextTemporaryIndex();
                    }
                }
                else
                {
                    TIntermSymbol *symbol = new TIntermSymbol(0, name, type);

                    TIntermAggregate *tempDeclaration = nullptr;
                    {
                        tempDeclaration = createTempInitDeclaration(symbol);
                    }

                    sequence->insert(sequence->begin(), tempDeclaration);
                    nextTemporaryIndex();
                }
            }
        }
        else
        {
            // TODO(Jiajia): When the instance name is not empty, how to get the feild variables.
        }
    }
}

}  // namespace anonymous

void UseInterfaceBlockFields(TIntermNode *root,
                             const UseBlockFieldList &blocks,
                             unsigned int *temporaryIndex)
{
    ASSERT(temporaryIndex != 0);
    UseUniformBlockMembers useUniformBlock(blocks);
    useUniformBlock.useTemporaryIndex(temporaryIndex);
    root->traverse(&useUniformBlock);
}

}  // namespace sh
