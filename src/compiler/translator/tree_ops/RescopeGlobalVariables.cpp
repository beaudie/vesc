//
// Copyright 2023 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//

#include "compiler/translator/util.h"
#include "compiler/translator/tree_ops/RescopeGlobalVariables.h"
#include "compiler/translator/tree_util/IntermTraverse.h"

namespace sh
{

////////////////////////////////////////////////////////////////////////////////

namespace
{

class Rescoper : public TIntermTraverser
{
  private:

    struct VariableInfo
    {
        TVector<TIntermFunctionDefinition*> functions;
        TIntermBlock* blockDeclared;
        TIntermDeclaration* declaration;
    };

    TUnorderedMap<const TVariable*, VariableInfo> mGlobalVarsNeedRescope;
    TIntermFunctionDefinition* mCurrentFunction;
    TVector<TIntermBlock*> mCurrentBlockStack;

  public:
    Rescoper(TSymbolTable *symbolTable) : TIntermTraverser(false, true, true, symbolTable) {}

    bool rescope()
    {
        if (mGlobalVarsNeedRescope.empty())
        {
            return true;
        }

        for (auto& pair : mGlobalVarsNeedRescope)
        {
            if (pair.second.functions.size() == 1)
            {
                TIntermSequence* funcSequence = pair.second.functions[0]->getBody()->getSequence();
                auto it = funcSequence->begin();
                funcSequence->insert(it, pair.second.declaration);
                // pair.second.blockDeclared->replaceChildNode(pair.second.declaration, nullptr);
            }
        }
        return true;
    }

  protected:
    void visitSymbol(TIntermSymbol *node) override
    {
        const TVariable& var = node->variable();
        if (mCurrentFunction && mGlobalVarsNeedRescope.contains(&var))
        {
            TVector<TIntermFunctionDefinition*>& list = mGlobalVarsNeedRescope.at(&var).functions;
            if (std::find(list.begin(), list.end(), mCurrentFunction) == list.end())
            {
                list.push_back(mCurrentFunction);
            }
        }
    }

    bool visitDeclaration(Visit visit, TIntermDeclaration *node) override
    {
        Declaration decl           = ViewDeclaration(*node);
        const TVariable &var       = decl.symbol.variable();
        const TType &nodeType      = var.getType();
        const TQualifier qualifier = nodeType.getQualifier();
        if (qualifier == TQualifier::EvqGlobal && !nodeType.isStructSpecifier())
        {
            mGlobalVarsNeedRescope.emplace(&var, VariableInfo());
            mGlobalVarsNeedRescope.at(&var).declaration = node;
            if (!mCurrentBlockStack.empty())
            {
                mGlobalVarsNeedRescope.at(&var).blockDeclared = mCurrentBlockStack.back();
            }
        }
        return true;
    }

    bool visitFunctionDefinition(Visit visit, TIntermFunctionDefinition *node) override
    {
        if (visit == Visit::InVisit)
        {
            TIntermFunctionDefinition* func = node->getAsFunctionDefinition();
            if (func)
            {
                mCurrentFunction = func;
            }
        }
        else if (visit == Visit::PostVisit)
        {
            if (mCurrentFunction && mCurrentFunction == node->getAsFunctionDefinition())
            {
                mCurrentFunction = nullptr;
            }
        }
        return true;
    }

    bool visitBlock(Visit visit, TIntermBlock *node) override
    {
        if (visit == Visit::InVisit)
        {
            TIntermBlock* block = node->getAsBlock();
            if (block && (mCurrentBlockStack.empty() || block != mCurrentBlockStack.back()))
            {
                mCurrentBlockStack.push_back(block);
            }
        }
        else if (visit == Visit::PostVisit)
        {
            if (!mCurrentBlockStack.empty() && mCurrentBlockStack.back() == node->getAsBlock())
            {
                mCurrentBlockStack.pop_back();
            }
        }
        return true;
    }
};

}  // anonymous namespace

////////////////////////////////////////////////////////////////////////////////

bool RescopeGlobalVariables(TCompiler &compiler, TIntermBlock &root)
{
    TSymbolTable &symbolTable = compiler.getSymbolTable();
    Rescoper rescoper(&symbolTable);
    root.traverse(&rescoper);
    return rescoper.rescope();
}

} // namespace sh

