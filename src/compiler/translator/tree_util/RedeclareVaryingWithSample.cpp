//
// Copyright 2020 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//
// RedeclareVaryingWithSample.h: Find varyings at the fragment shader which doesn't have any
// auxiliary storage qulifiers or interpolation qualifiers, and add sample qualifier to the varying
//

#include "compiler/translator/tree_util/RedeclareVaryingWithSample.h"

#include "common/debug.h"
#include "compiler/translator/Compiler.h"
#include "compiler/translator/SymbolTable.h"
#include "compiler/translator/tree_util/IntermNode_util.h"
#include "compiler/translator/tree_util/IntermTraverse.h"

namespace sh
{
namespace
{

// If the input of the fragment shader has no other decorator and is not an integer type or double
// float type, then the input will be decorated with 'sample' keyword
class RedeclareVaryingWithSampleTraverser : public TIntermTraverser
{
  public:
    RedeclareVaryingWithSampleTraverser(TSymbolTable *symbolTable)
        : TIntermTraverser(true, false, false, symbolTable)
    {}

    bool visitDeclaration(Visit visit, TIntermDeclaration *node) override
    {
        const TIntermSequence &sequence = *(node->getSequence());
        ASSERT(sequence.size() == 1);

        TIntermTyped *variable = sequence.front()->getAsTyped();
        TIntermSymbol *symbol  = variable->getAsSymbolNode();

        if (symbol == nullptr || symbol->variable().symbolType() != sh::SymbolType::UserDefined)
        {
            return true;
        }

        bool typeContainsIntegers =
            (symbol->getBasicType() == EbtInt || symbol->getBasicType() == EbtUInt ||
             symbol->getType().isStructureContainingType(EbtInt) ||
             symbol->getType().isStructureContainingType(EbtUInt));
        bool typeContainsDoubles = (symbol->getBasicType() == EbtDouble ||
                                    symbol->getType().isStructureContainingType(EbtDouble));
        if (!typeContainsIntegers && !typeContainsDoubles &&
            symbol->getQualifier() == EvqFragmentIn)
        {
            TIntermDeclaration *replaceDeclaration = new TIntermDeclaration();

            TType *newType = new TType(symbol->getType());
            newType->setQualifier(EvqSampleIn);

            TVariable *newVariable =
                new TVariable(mSymbolTable, symbol->getName(), newType,
                              symbol->variable().symbolType(), symbol->variable().extension());
            TIntermSymbol *newSymbol = new TIntermSymbol(newVariable);

            replaceDeclaration->appendDeclarator(newSymbol);
            replaceDeclaration->setLine(node->getLine());

            queueReplacement(replaceDeclaration, OriginalNode::IS_DROPPED);
        }

        return true;
    }
};

}  // anonymous namespace

ANGLE_NO_DISCARD bool RedeclareVaryingWithSample(TCompiler *compiler,
                                                 TIntermBlock *root,
                                                 TSymbolTable *symbolTable)
{
    RedeclareVaryingWithSampleTraverser varyingTraverser(symbolTable);
    root->traverse(&varyingTraverser);
    if (!varyingTraverser.updateTree(compiler, root))
    {
        return false;
    }

    return true;
}

}  // namespace sh
