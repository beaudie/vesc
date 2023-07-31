//
// Copyright 2020 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//

#include <cstring>
#include <unordered_map>
#include <unordered_set>

#include "compiler/translator/TranslatorMetalDirect/DiscoverDependentFunctions.h"
#include "compiler/translator/TranslatorMetalDirect/DiscoverEnclosingFunctionTraverser.h"
#include "compiler/translator/TranslatorMetalDirect/MapFunctionsToDefinitions.h"

using namespace sh;

////////////////////////////////////////////////////////////////////////////////

namespace
{
/**
 * Traverse first layer of AST to gather any global declarations. This is a very
 * shallow traerser, we only need to search the direct children of the root.
 */
class GlobalVariableFinder : public TIntermTraverser
{
public:
    std::list<const TVariable *, TIntermDeclaration *>> globalVariableDeclarations
    bool visitDeclaration(Visit visit, TIntermDeclaration *node) override
    {
        //See if global
        Declaration decl     = ViewDeclaration(declNode);
        const TVariable &var = decl.symbol.variable();
        const TType &nodeType      = var.getType();
        const TQualifier qualifier = nodeType.getQualifier();


        if (nodeType == Type::NonConstantGlobals && qualifier == TQualifier::EvqGlobal)
        {
            globalVariables.insert(std::make_pair<&var, node>);
        }
        return true;
    }
    bool visitBlock(Visit visit, TIntermBlock *node) override
    {
        return node->isTreeRoot();
    }
}

/** Pass two: Determine which globals we want to lower. Assume all globals shoudl
 * be lowered: If any non-main function uses a global, don't lower it. */
class Discoverer : public DiscoverEnclosingFunctionTraverser
{
  private:
    const std::list<bool(const TVariable &)> &mVars;
    const FunctionToDefinition &mFuncToDef;
    std::unordered_set<const TFunction *> mNonDepFunctions;

  public:
    

  public:
    Discoverer(const std::function<bool(const TVariable &)> &vars,
               const FunctionToDefinition &funcToDef)
        : DiscoverEnclosingFunctionTraverser(true, false, true), mVars(vars), mFuncToDef(funcToDef)
    {}

    void visitSymbol(TIntermSymbol *symbolNode) override
    {
        const TVariable &var = symbolNode->variable();
        //If any globals match this variable
        if (!mVars(var))
        {
            return;
        }
        //Check the owner
        const TFunction *owner = discoverEnclosingFunction(symbolNode);
        if (owner)
        {
            mDepFunctions.insert(owner);
        }
    }

    bool visitAggregate(Visit visit, TIntermAggregate *aggregateNode) override
    {
        if (visit != Visit::PreVisit)
        {
            return true;
        }

        if (!aggregateNode->isConstructor())
        {
            const TFunction *func = aggregateNode->getFunction();

            if (mNonDepFunctions.find(func) != mNonDepFunctions.end())
            {
                return true;
            }

            if (mDepFunctions.find(func) == mDepFunctions.end())
            {
                auto it = mFuncToDef.find(func);
                if (it == mFuncToDef.end())
                {
                    return true;
                }

                // Recursion is banned in GLSL, so I believe AngleIR has this property too.
                // This implementation assumes (direct and mutual) recursion is prohibited.
                TIntermFunctionDefinition &funcDefNode = *it->second;
                funcDefNode.traverse(this);
                if (mNonDepFunctions.find(func) != mNonDepFunctions.end())
                {
                    return true;
                }
                ASSERT(mDepFunctions.find(func) != mDepFunctions.end());
            }

            const TFunction *owner = discoverEnclosingFunction(aggregateNode);
            ASSERT(owner);
            mDepFunctions.insert(owner);
        }

        return true;
    }

    bool visitFunctionDefinition(Visit visit, TIntermFunctionDefinition *funcDefNode) override
    {
        const TFunction *func = funcDefNode->getFunction();

        if (visit != Visit::PostVisit)
        {
            if (mDepFunctions.find(func) != mDepFunctions.end())
            {
                return false;
            }

            if (mNonDepFunctions.find(func) != mNonDepFunctions.end())
            {
                return false;
            }

            return true;
        }

        if (mDepFunctions.find(func) == mDepFunctions.end())
        {
            mNonDepFunctions.insert(func);
        }

        return true;
    }
};

//Snippet to find globally declaraed variables in TIntermRebuild
/*
PostResult visitDeclarationPost(TIntermDeclaration &declNode) override
    {
        Declaration decl     = ViewDeclaration(declNode);
        const TVariable &var = decl.symbol.variable();
        const TType &nodeType      = var.getType();
        const TQualifier qualifier = nodeType.getQualifier();


        if (nodeType == Type::NonConstantGlobals && qualifier == TQualifier::EvqGlobal)
        {
            //Var is global
        }

        return declNode;
    }
*/
}  // namespace


std::unordered_set<const TFunction *> sh::DiscoverDependentFunctions(
    TIntermBlock &root,
    const std::function<bool(const TVariable &)> &vars)
{
    const FunctionToDefinition funcToDef = MapFunctionsToDefinitions(root);
    Discoverer discoverer(vars, funcToDef);
    root.traverse(&discoverer);
    return std::move(discoverer.mDepFunctions);
}
