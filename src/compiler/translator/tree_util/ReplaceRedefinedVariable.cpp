//
// Copyright (c) 2018 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//
// ReplaceVariable.cpp: Replace all references to a specific variable in the AST with references to
// another variable.

#include "compiler/translator/tree_util/ReplaceRedefinedVariable.h"
#include "compiler/translator/tree_util/ReplaceVariable.h"

#include "compiler/translator/IntermNode.h"
#include "compiler/translator/Symbol.h"
#include "compiler/translator/SymbolTable.h"
#include "compiler/translator/tree_util/IntermNode_util.h"
#include "compiler/translator/tree_util/IntermTraverse.h"

#include <unordered_set>

namespace sh
{

namespace
{

class ReplaceRedefinedVariableTraverser : public TIntermTraverser
{
  public:
    ReplaceRedefinedVariableTraverser(TSymbolTable *symbolTable)
        : TIntermTraverser(true, true, true),
          mSymbolTable(symbolTable),
          mParameterNames{},
          mFunctionBody(nullptr)
    {}

    bool visitFunctionDefinition(Visit visit, TIntermFunctionDefinition *node) override
    {
        // In pre-visit of function, record params
        if (visit == PreVisit)
        {
            ASSERT(mParameterNames.size() == 0);
            const TFunction *func = node->getFunctionPrototype()->getFunction();
            // Grab all of the parameter names from the function prototype
            uint32_t paramCount = func->getParamCount();
            for (uint32_t i = 0; i < paramCount; ++i)
            {
                mParameterNames.emplace(std::string(func->getParam(i)->name().data()));
            }
            if (mParameterNames.size() > 0)
                mFunctionBody = node->getBody();
        }
        else if (visit == PostVisit)
        {
            // Clear data saved from function definition
            mParameterNames.clear();
            mFunctionBody = nullptr;
        }
        return true;
    }
    bool visitDeclaration(Visit visit, TIntermDeclaration *node) override
    {
        if (visit == PreVisit && mParameterNames.size() != 0)
        {
            TIntermSequence *decls = node->getSequence();
            for (auto &declVector : *decls)
            {
                // no init case
                TIntermSymbol *symNode = declVector->getAsSymbolNode();
                if (symNode == nullptr)
                {
                    // init case
                    TIntermBinary *binaryNode = declVector->getAsBinaryNode();
                    symNode                   = binaryNode->getLeft()->getAsSymbolNode();
                }
                ASSERT(symNode != nullptr);
                std::string varName = std::string(symNode->variable().name().data());
                if (mParameterNames.count(varName) > 0)
                {
                    // We found a redefined var so replace w/ new name to avoid later conflict
                    TVariable *replaceVariable =
                        CreateTempVariable(mSymbolTable, &symNode->variable().getType());
                    ReplaceVariable(mFunctionBody, &symNode->variable(), replaceVariable);
                }
            }
        }
        return true;
    }

  private:
    TSymbolTable *mSymbolTable;
    std::unordered_set<std::string> mParameterNames;
    TIntermBlock *mFunctionBody;
};

}  // anonymous namespace

// Replaces every occurrence of a variable with another variable.
void ReplaceRedefinedVariable(TIntermBlock *root, TSymbolTable *symbolTable)
{
    ReplaceRedefinedVariableTraverser traverser(symbolTable);
    root->traverse(&traverser);
    traverser.updateTree();
}

}  // namespace sh
