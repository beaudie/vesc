//
// Copyright 2023 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//

#include "compiler/translator/tree_ops/msl/RescopeGlobalVariables.h"
#include "compiler/translator/tree_util/IntermTraverse.h"

using namespace sh;

////////////////////////////////////////////////////////////////////////////////

namespace
{

class Rescoper : private TIntermTraverser
{
  private:
    TUnorderedMap<TVariable*, TVector<TFunction*>> mGlobalVarsNeedRescope;
    TUnorderedMap<TVariable*, TIntermDeclaration*> mGlobalVarsDeclarations;
    TVector<TFunction*> mCurrentFunctionStack;

  public:
    Rescoper(TSymbolTable *symbolTable) : TIntermTraverser(false, true, true, symbolTable) {}

    bool rescope()
    {
        if (mGlobalVarsNeedRescope.empty())
        {
            return true;
        }

        for (std::pair<TVariable*, TVector<TFunction*>> pair : mGlobalVarsNeedRescope)
        {

        }
    }

  protected:
    void visitSymbol(TIntermSymbol *node) override
    {
        TVariable& var = node->variable();
        TFunction* func = mCurrentFunctionStack.back();
        if (func && mGlobalVarsNeedRescope.contains(&var))
        {
            TVector<TFunction*>& list = mGlobalVarsNeedRescope.at(&var);
            if (!list.contains(func))
            {
                list.push_back(func);
            }
        }
    }

    bool visitDeclaration(Visit visit, TIntermDeclaration *node) override
    {
        if (visit == Visit::InVisit)
        {
            Declaration decl           = ViewDeclaration(node);
            const TVariable &var       = decl.symbol.variable();
            const TType &nodeType      = var.getType();
            const TQualifier qualifier = nodeType.getQualifier();
            if (nodeType == Type::NonConstantGlobals && qualifier == TQualifier::EvqGlobal)
            {
                if (decl.initExpr)
                {
                    const TVariable *var2 = decl.initExpr->getAsSymbolNode();
                    if (var2)
                    {
                        if(mGlobalVarsNeedRescope.contains(var2))
                        {
                            std::erase(mGlobalVarsNeedRescope, var2);
                            return true;
                        }
                    }
                }
                mGlobalVarsNeedRescope.emplace(&var, TVector<TFunction*>());
                mGlobalVarsDeclaration.emplace(&var, node);
            }
        }
        return true;
    }

    bool visitFunctionDefinition(Visit visit, TIntermFunctionDefinition *node) override
    {
        if (visit == Visit::InVisit)
        {
            TFunctionDefinition* func = node->getAsFunctionDefinition();
            if (func)
            {
                mCurrentFunction.push_back(func->getFunction());
            }
        }
        else if (visit == Visit::PostVisit)
        {
            if (mCurrentFunction.back() == node->getFunction())
            {
                mCurrentFunction.pop_back()
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
    root.traverse(rescoper);
    return rescoper.rescope();
}