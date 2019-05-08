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
#include "compiler/translator/tree_util/IntermTraverse.h"

namespace sh
{

namespace
{

class ReplaceRedefinedVariableTraverser : public TIntermTraverser
{
  public:
    ReplaceRedefinedVariableTraverser(TSymbolTable *symbolTable)
        : TIntermTraverser(true, false, false), mSymbolTable(symbolTable)
    {}

    bool visitFunctionDefinition(Visit visit, TIntermFunctionDefinition *node) override
    {
        const TFunction *func = node->getFunctionPrototype()->getFunction();
        // Grab all of the parameter names from the function prototype
        uint32_t paramCount = func->getParamCount();
        for (uint32_t i = 0; i < paramCount; ++i)
        {
            const TVariable *param = func->getParam(i);
            // TODO: Need to first detect if variable is redefined in function body, only then
            // replace if (IsVariableDefined(node->getBody(), param) // <- Need something like this
            // {
            // Create a var w/ substitute name & replace any matches in function body
            std::string baseName(param->name().data());
            std::string prefix("replace_");
            std::string replacementString = prefix + baseName;
            const ImmutableString replacementVariableName(replacementString.c_str());
            TVariable *replacementVar = new TVariable(mSymbolTable, replacementVariableName,
                                                      &param->getType(), SymbolType::UserDefined);
            ReplaceVariable(node->getBody(), param, replacementVar);
            // TODO: Do I need to do cleanup of replacementVar?
            // }
        }
        return true;
    }

  private:
    TSymbolTable *mSymbolTable;
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
