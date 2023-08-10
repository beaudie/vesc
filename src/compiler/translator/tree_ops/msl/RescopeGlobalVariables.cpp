//
// Copyright 2023 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//

#include "compiler/translator/tree_ops/msl/RescopeGlobalVariables.h"

using namespace sh;

////////////////////////////////////////////////////////////////////////////////

namespace
{

class Rescoper : private TIntermRebuild
{
  private:
    TUnorderedMap<TVariable *, std::pair<int, TFunctionDeclaration *>> mGlobalVarsNeedRescope;

  public:
    Rewriter(TCompiler &compiler) : TIntermRebuild(compiler, true, false) {}

    PreResult visitSymbolPre(TIntermSymbol &node) override
    {
        Declaration decl           = ViewDeclaration(node);
        const TVariable &var       = decl.symbol.variable();
        const TType &nodeType      = var.getType();
        const TQualifier qualifier = nodeType.getQualifier();
        if (nodeType != Type::NonConstantGlobals && qualifier != TQualifier::EvqGlobal)
        {
            return {node, VisitBits::Neither};
        }

        if (!mGlobalVarsNeedRescope.contains(&var))
        {
            mGlobalVarsNeedRescope.emplace(
                std::make_pair(&var, std::make_pair(0, new TFunctionDeclaration())));
        }

        const TFunction *func = getParentFunction();
        if (func)
        {
            const TFunctionDeclaration funcDecl             = ViewDeclaration(*func);
            std::pair<int, TFunctionDeclaration *> &varInfo = mGlobalVarsNeedRescope.at(&var);
            if (varInfo.first == 0)
            {
                varInfo.second = &funcDecl;
                varInfo.first++;
            }
            else if (varInfo.second != &funcDecl)
            {
                varInfo.first++;
            }
        }
        return {declNode, VisitBits::Neither};
    }

    bool rescope(TIntermBlock &root, IdGen &idGen)
    {
        if (!rebuildRoot(root))
        {
            return false;
        }

        if (mGlobalVarsNeedRescope.empty())
        {
            return true;
        }

        TSymbolTable &symbolTable = compiler.getSymbolTable();

        for (std::pair<TVariable *, std::pair<int, TFunctionDeclaration *>> &entry :
             mGlobalVarsNeedRescope)
        {
            if (entry.second.first != 1)
            {
                continue;
            }

            TType *replacementType = new TType(entry.first->getType());
            replacementType->setQualifier(EvqTemporary);
            TVariable *replacement = new TVariable(symbolTable, entry.first->name(),
                                                   replacementType, entry.first->symbolType());
            if (!ReplaceVariable(compiler, root, entry.first, replacement))
            {
                return false;
            }
        }

        return true;
    }
};

}  // anonymous namespace

////////////////////////////////////////////////////////////////////////////////

bool RescopeGlobalVariables(TCompiler &compiler, TIntermBlock &root, IdGen &idGen)
{
    return Rescoper(compiler).rescope(root, idGen);
}