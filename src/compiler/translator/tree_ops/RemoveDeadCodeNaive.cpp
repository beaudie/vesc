//
// Copyright 2020 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//
// RemoveDeadCodeNaive.cpp:
//
// - Add a pass that gathers this info:
//  1. For every node, what node it needs to do its calculations (for most cases, probably direct
//     children is enough. Function calls and code blocks are exceptions to that).
//  2. For function nodes, what functions it calls
//  3. For each function, what nodes are outputs (return, assignment to out parameter, or builtin
//     outputs)
//
// - Sort the functions with a topological sort (based on the graph created with information
//   gathered in #2 above. This is a DAG as there is no recursion in GLSL). `main` is necessarily
//   last (except unless there are unused functions).
//
// - Before processing each function, eliminate any that is not reached by main.
//
// - Process each function by starting from its outputs nodes, DFS and mark which nodes are reached.
//   Parameters that are not reached, mark them for elimination.
//  * When processing a function call node, don't mark the unused parameters during the DFS walk.
//    Note that we are walking over the functions after the topological sort, so when we reach a
//    function call, that function has already been processed and the unused parameters identified.
//
// - At the end, remove any node that's not marked. Change functions and call sites which have
//   removed parameters.
//  * Removing unmarked nodes should end up removing unused samplers.
//

#include "compiler/translator/tree_ops/RemoveDeadCodeNaive.h"

#include "compiler/translator/SymbolTable.h"
#include "compiler/translator/tree_util/IntermTraverse.h"

#include <unordered_set>

namespace sh
{

namespace
{

class ModuleTraverser : public TIntermTraverser
{
  public:
    ModuleTraverser(TIntermBlock *root);

    void visitFunctionPrototype(TIntermFunctionPrototype *node) override;
    bool visitFunctionDefinition(Visit visit, TIntermFunctionDefinition *node) override;
    bool visitAggregate(Visit visit, TIntermAggregate *node) override;

    void doTheWork(void);
    std::unordered_map<int, TIntermFunctionDefinition *> &getFunctionMap(void)
    {
        return mFunctionMap;
    }
    std::vector<int> &getFuncWorklist(void) { return mFuncWorklist; }
    std::unordered_map<int, bool> &getSideEffectMap(void) { return mSideEffectMap; }

  private:
    void internalSort(int uid, std::set<int> &visited);

    std::unordered_map<int, TIntermFunctionPrototype *> mFuncProtoMap;
    std::unordered_map<int, TIntermFunctionDefinition *> mFunctionMap;
    std::unordered_map<int, std::unordered_set<int>> mFuncDependency;
    std::unordered_map<int, std::unordered_set<TIntermAggregate *>> mFunctionCalls;
    std::set<int> mFunctionDefs;
    std::set<int> mFunctionVisited;
    std::vector<int> mFuncWorklist;

    std::unordered_map<int, bool> mSideEffectMap;

    TIntermBlock *mRootNode;

    int mCurrFunctionId;
    int mMainFunctionId;
};

ModuleTraverser::ModuleTraverser(TIntermBlock *root)
    : TIntermTraverser(true, false, false), mRootNode(root)
{}

void ModuleTraverser::internalSort(int uid, std::set<int> &visited)
{
    if (visited.count(uid) != 0)
    {
        return;
    }

    auto iter = mFuncDependency.find(uid);
    if (iter != mFuncDependency.end())
    {
        for (int child : iter->second)
        {
            internalSort(child, visited);
        }
    }

    mFuncWorklist.push_back(uid);
    visited.insert(uid);
}

void ModuleTraverser::doTheWork(void)
{
    internalSort(mMainFunctionId, mFunctionVisited);

    std::set<int> diffs;
    set_difference(mFunctionDefs.begin(), mFunctionDefs.end(), mFunctionVisited.begin(),
                   mFunctionVisited.end(), std::inserter(diffs, diffs.begin()));
    for (int uid : diffs)
    {
        TIntermSequence empty;
        mMultiReplacements.push_back(
            NodeReplaceWithMultipleEntry(mRootNode, mFunctionMap[uid], empty));
        if (mFuncProtoMap.find(uid) != mFuncProtoMap.end())
        {
            mMultiReplacements.push_back(
                NodeReplaceWithMultipleEntry(mRootNode, mFuncProtoMap[uid], empty));
        }
    }
}

void ModuleTraverser::visitFunctionPrototype(TIntermFunctionPrototype *node)
{
    const TFunction *function = node->getFunction();
    if (getParentNode() == mRootNode)
    {
        mFuncProtoMap[function->uniqueId().get()] = node;
    }
}

bool ModuleTraverser::visitFunctionDefinition(Visit visit, TIntermFunctionDefinition *node)
{
    const TFunction *function     = node->getFunction();
    mCurrFunctionId               = function->uniqueId().get();
    mFunctionMap[mCurrFunctionId] = node;
    mFunctionDefs.insert(mCurrFunctionId);
    if (function->isMain())
    {
        mMainFunctionId = mCurrFunctionId;
    }
    return true;
}

bool ModuleTraverser::visitAggregate(Visit visit, TIntermAggregate *node)
{
    TOperator op = node->getOp();
    if (op == EOpCallFunctionInAST || op == EOpCallInternalRawFunction)
    {
        int uid = node->getFunction()->uniqueId().get();
        mFuncDependency[mCurrFunctionId].insert(uid);
        mFunctionCalls[uid].insert(node);
    }
    return true;
}

bool isIndexNode(TIntermNode *node)
{
    TIntermBinary *binary = node->getAsBinaryNode();
    if (binary)
    {
        TOperator op = binary->getOp();
        if (op == EOpIndexDirect || op == EOpIndexIndirect || op == EOpIndexDirectStruct ||
            op == EOpIndexDirectInterfaceBlock)
        {
            return true;
        }
    }

    return false;
}

bool isComma(TIntermNode *node)
{
    TIntermBinary *binary = node->getAsBinaryNode();
    if (binary && binary->getOp() == EOpComma)
    {
        return true;
    }

    return false;
}

TIntermSymbol *getSymbolNode(TIntermNode *node)
{
    TIntermSymbol *symbol = nullptr;
    while (!(symbol = node->getAsSymbolNode()))
    {
        TIntermBinary *binary = node->getAsBinaryNode();
        if (binary)
        {
            node = binary->getLeft();
        }
        else
        {
            TIntermSwizzle *swizzle = node->getAsSwizzleNode();
            if (swizzle)
            {
                node = swizzle->getOperand();
            }
            else
            {
                break;
            }
        }
    }
    return symbol;
}

bool isFunctionOut(TIntermSymbol *symbol)
{
    TQualifier qual = symbol->getType().getQualifier();

    if (IsShaderOut(qual))
    {
        return true;
    }

    if (qual == EvqOut || qual == EvqInOut)
    {
        return true;
    }

    if (qual == EvqPosition || qual == EvqPointSize || qual == EvqFragColor ||
        qual == EvqFragData || qual == EvqFragDepth || qual == EvqFragDepthEXT ||
        qual == EvqSecondaryFragColorEXT || qual == EvqSecondaryFragDataEXT ||
        qual == EvqLastFragColor || qual == EvqLastFragData)
    {
        return true;
    }

    if (qual == EvqViewportIndex)
    {
        return true;
    }

    if (qual == EvqGlobal)
    {
        return true;
    }

    return false;
}

bool isImplicitOut(TIntermSymbol *symbol)
{
    TQualifier qual = symbol->getType().getQualifier();

    if (IsShaderOut(qual))
    {
        return true;
    }

    if (qual == EvqPosition || qual == EvqPointSize || qual == EvqFragColor ||
        qual == EvqFragData || qual == EvqFragDepth || qual == EvqFragDepthEXT ||
        qual == EvqSecondaryFragColorEXT || qual == EvqSecondaryFragDataEXT ||
        qual == EvqLastFragColor || qual == EvqLastFragData)
    {
        return true;
    }

    if (qual == EvqViewportIndex)
    {
        return true;
    }

    if (qual == EvqGlobal)
    {
        return true;
    }

    return false;
}

bool isGeneralAssignment(TIntermBinary *node)
{
    return node->isAssignment() || node->getOp() == EOpInitialize;
}

enum TWorkMode
{
    EmAnalyse,
    EmTryRemove
};

class FunctionTraverser : public TIntermTraverser
{
  public:
    FunctionTraverser(ModuleTraverser &module, int uid);

    void visitSymbol(TIntermSymbol *node) override;
    void visitConstantUnion(TIntermConstantUnion *node) override;
    bool visitSwizzle(Visit visit, TIntermSwizzle *node) override;
    bool visitBinary(Visit visit, TIntermBinary *node) override;
    bool visitUnary(Visit visit, TIntermUnary *node) override;
    bool visitTernary(Visit visit, TIntermTernary *node) override;
    bool visitAggregate(Visit visit, TIntermAggregate *node) override;

    bool visitIfElse(Visit visit, TIntermIfElse *node) override;
    bool visitSwitch(Visit visit, TIntermSwitch *node) override;
    bool visitLoop(Visit visit, TIntermLoop *node) override;
    bool visitBlock(Visit visit, TIntermBlock *node) override;
    bool visitBranch(Visit visit, TIntermBranch *node) override;

    void doTheWork(void);
    void setWorkMode(TWorkMode workMode) { mWorkMode = workMode; }

  private:
    void checkLoopCondition(Visit visit, TIntermNode *node);
    void analyseSymbol(TIntermSymbol *node);
    bool analyseBinary(Visit visit, TIntermBinary *node);
    void queueMultiReplacementWithParent(TIntermAggregateBase *parentIn,
                                         TIntermNode *originalIn,
                                         TIntermSequence replacementsIn);
    void cacheReplacementWithParent(TIntermNode *parent,
                                    TIntermNode *original,
                                    TIntermNode *replacement);
    void tryRemoveNodeCommon(TIntermNode *node);
    bool tryRemoveBinary(Visit vist, TIntermBinary *node);
    bool analyseUnary(Visit visit, TIntermUnary *node);
    bool tryRemoveUnary(Visit visit, TIntermUnary *node);
    bool analyseAggregate(Visit visit, TIntermAggregate *node);
    bool tryRemoveAggregate(Visit visit, TIntermAggregate *node);
    void internalMarkDeps(int uid, std::unordered_set<int> &workset);

    TWorkMode mWorkMode;
    ModuleTraverser &mModule;
    int mUid;

    std::map<int, std::unordered_set<int>> mVarDependency;
    std::unordered_set<int> mVarOuts;
    std::unordered_set<int> mVarImplicitOuts;
    std::unordered_set<int> mVarWorkset;

    std::pair<std::unordered_set<int>, TIntermNode *> mLoopCond;
    std::vector<std::pair<std::unordered_set<int>, TIntermNode *>> mConditions;
    std::vector<std::unordered_set<int>> mVarLoopOuts;
    std::vector<std::unordered_set<int>> mVarInExpr;
    std::vector<std::pair<bool, TIntermNode *>> mActiveNodes;
    size_t mDepthComma;
    size_t mDepthLoop;

    struct NodeUpdateCache
    {
        NodeUpdateCache(TIntermNode *_parent, TIntermNode *_original, TIntermNode *_replacement)
            : parent(_parent), original(_original), replacement(_replacement)
        {}

        TIntermNode *parent;
        TIntermNode *original;
        TIntermNode *replacement;
    };
    std::vector<NodeUpdateCache> mReplacementsCache;
};

FunctionTraverser::FunctionTraverser(ModuleTraverser &module, int uid)
    : TIntermTraverser(true, false, true),
      mWorkMode(EmAnalyse),
      mModule(module),
      mUid(uid),
      mDepthComma(0),
      mDepthLoop(0)
{}

void FunctionTraverser::checkLoopCondition(Visit visit, TIntermNode *node)
{
    TIntermLoop *loop;
    if (mDepthLoop > 0 && (loop = getParentNode()->getAsLoopNode()) && loop->getCondition() == node)
    {
        if (visit == PreVisit)
        {
            mVarInExpr.push_back(std::unordered_set<int>());
            return;
        }
        mLoopCond = std::make_pair(mVarInExpr.back(), loop);
        mVarInExpr.pop_back();
    }
}

void FunctionTraverser::analyseSymbol(TIntermSymbol *node)
{
    if (mVarInExpr.empty())
    {
        return;
    }

    unsigned int level;
    TIntermNode *parent;
    TIntermNode *operand;

    if (mDepthComma > 1)
    {
        return;
    }
    else if (mDepthComma == 1)
    {
        level   = 0;
        operand = node;
        parent  = getAncestorNode(level++);
        while (!isComma(parent))
        {
            operand = parent;
            parent  = getAncestorNode(level++);
        }
        if (parent->getAsBinaryNode()->getLeft() == operand)
        {
            return;
        }
    }

    level   = 0;
    operand = node;
    parent  = getParentNode();
    if (parent)
    {
        if (parent->getAsSwizzleNode())
        {
            level   = 1;
            operand = parent;
        }
        else
        {
            while (parent && isIndexNode(parent))
            {
                operand = parent;
                parent  = getAncestorNode(++level);
            }
        }
    }

    parent = getAncestorNode(level);
    if (parent)
    {
        TIntermAggregate *parentAggregate = parent->getAsAggregate();
        if (parentAggregate && parentAggregate->isFunctionCall())
        {
            TIntermSequence *args = parentAggregate->getSequence();
            size_t indexArg       = 0;
            for (TIntermNode *arg : *args)
            {
                if (arg == operand)
                {
                    break;
                }
                ++indexArg;
            }

            const TVariable *param = parentAggregate->getFunction()->getParam(indexArg);
            if (param->getType().getQualifier() == EvqOut)
            {
                return;
            }
        }
        else
        {
            TIntermBinary *parentBinary = parent->getAsBinaryNode();
            if (parentBinary && parentBinary->getOp() == EOpAssign)
            {
                if (parentBinary->getLeft() == operand)
                {
                    return;
                }
            }
        }
    }

    for (auto &expr : mVarInExpr)
    {
        expr.insert(node->variable().uniqueId().get());
    }
}

bool FunctionTraverser::analyseBinary(Visit visit, TIntermBinary *node)
{
    if (visit == PreVisit)
    {
        checkLoopCondition(visit, node);

        if (node->getOp() == EOpComma)
        {
            ++mDepthComma;
        }

        if (node->isAssignment() || node->getOp() == EOpInitialize)
        {
            mVarInExpr.push_back(std::unordered_set<int>());
        }

        return true;
    }

    if (node->getOp() == EOpComma)
    {
        --mDepthComma;
    }

    if (node->isAssignment() || node->getOp() == EOpInitialize)
    {

        TIntermSymbol *symbol = getSymbolNode(node->getLeft());

        int uid = symbol->variable().uniqueId().get();
        mVarDependency[uid].insert(mVarInExpr.back().begin(), mVarInExpr.back().end());
        mVarInExpr.pop_back();

        for (const auto &cond : mConditions)
        {
            mVarDependency[uid].insert(cond.first.begin(), cond.first.end());
        }

        for (auto &out : mVarLoopOuts)
        {
            out.insert(uid);
        }

        if (isFunctionOut(symbol))
        {
            mVarOuts.insert(uid);

            if (isImplicitOut(symbol))
            {
                mVarImplicitOuts.insert(uid);
            }
        }
    }

    checkLoopCondition(visit, node);

    return true;
}

void FunctionTraverser::queueMultiReplacementWithParent(TIntermAggregateBase *parentIn,
                                                        TIntermNode *originalIn,
                                                        TIntermSequence replacementsIn)
{
    mMultiReplacements.push_back(
        NodeReplaceWithMultipleEntry(parentIn, originalIn, replacementsIn));
}

void FunctionTraverser::cacheReplacementWithParent(TIntermNode *parent,
                                                   TIntermNode *original,
                                                   TIntermNode *replacement)
{
    mReplacementsCache.push_back(NodeUpdateCache(parent, original, replacement));
}

void FunctionTraverser::tryRemoveNodeCommon(TIntermNode *node)
{
    TIntermSequence empty;
    TIntermNode *parent   = getParentNode();
    TIntermNode *ancestor = getAncestorNode(1);

    TIntermBlock *block;
    TIntermLoop *loop;
    TIntermBinary *binary;

    if (parent && (block = parent->getAsBlock()))
    {
        queueMultiReplacementWithParent(block, node, empty);
    }
    else if (parent && (loop = parent->getAsLoopNode()) && loop->getCondition() != node)
    {
        queueReplacementWithParent(loop, node, nullptr, OriginalNode::IS_DROPPED);
    }
    else if (parent && (binary = parent->getAsBinaryNode()) && binary->getOp() == EOpComma)
    {
        if (binary->getLeft() == node)
        {
            cacheReplacementWithParent(ancestor, binary, binary->getRight());
        }
        else
        {
            if (ancestor->getAsBlock() ||
                ((loop = ancestor->getAsLoopNode()) && loop->getCondition() != parent) ||
                ((binary = ancestor->getAsBinaryNode()) && binary->getOp() == EOpComma))
            {
                bool siblingAlive = true;
                for (auto iter = mReplacementsCache.begin(); iter != mReplacementsCache.end();)
                {
                    if (iter->original == binary)
                    {
                        iter         = mReplacementsCache.erase(iter);
                        siblingAlive = false;
                    }
                    else
                    {
                        iter++;
                    }
                }
                if (siblingAlive)
                {
                    cacheReplacementWithParent(ancestor, binary, binary->getLeft());
                }
            }
        }
    }
    else if ((binary = node->getAsBinaryNode()) && binary->getOp() == EOpAssign)
    {
        queueReplacement(binary->getRight(), OriginalNode::IS_DROPPED);
    }
}

bool FunctionTraverser::tryRemoveBinary(Visit visit, TIntermBinary *node)
{
    if (visit == PreVisit)
    {
        mActiveNodes.push_back(std::make_pair(false, node));

        if (isGeneralAssignment(node))
        {
            TIntermSymbol *symbol = getSymbolNode(node->getLeft());
            if (mVarWorkset.find(symbol->variable().uniqueId().get()) != mVarWorkset.end())
            {
                for (auto iter = mActiveNodes.rbegin(); iter != mActiveNodes.rend(); iter++)
                {
                    iter->first = true;
                }
            }
        }

        return true;
    }

    if (node->getOp() == EOpComma)
    {
        if (mReplacementsCache.size() > 0)
        {
            for (const auto &repl : mReplacementsCache)
            {
                queueReplacementWithParent(repl.parent, repl.original, repl.replacement,
                                           OriginalNode::IS_DROPPED);
            }
            mReplacementsCache.clear();
        }
    }

    if (!mActiveNodes.back().first)
    {
        TIntermSequence empty;
        TIntermAggregateBase *ancestor;

        TIntermDeclaration *decl;
        TIntermLoop *loop;

        if ((decl = getParentNode()->getAsDeclarationNode()))
        {
            if (decl->getSequence()->size() > 1)
            {
                queueMultiReplacementWithParent(decl, node, empty);
            }
            else if ((ancestor = getAncestorNode(1)->getAsBlock()))
            {
                queueMultiReplacementWithParent(ancestor, decl, empty);
            }
            else if ((loop = getAncestorNode(1)->getAsLoopNode()))
            {
                queueReplacementWithParent(loop, decl, nullptr, OriginalNode::IS_DROPPED);
            }
        }
        else
        {
            tryRemoveNodeCommon(node);
        }
    }

    mActiveNodes.pop_back();

    return true;
}

bool FunctionTraverser::analyseUnary(Visit visit, TIntermUnary *node)
{
    if (visit == PreVisit)
    {
        checkLoopCondition(visit, node);

        return true;
    }

    if (node->isAssignment())
    {
        TIntermSymbol *symbol = getSymbolNode(node->getOperand());

        int uid = symbol->variable().uniqueId().get();
        for (const auto &cond : mConditions)
        {
            mVarDependency[uid].insert(cond.first.begin(), cond.first.end());
        }

        for (auto &out : mVarLoopOuts)
        {
            out.insert(uid);
        }

        if (isFunctionOut(symbol))
        {
            mVarOuts.insert(uid);

            if (isImplicitOut(symbol))
            {
                mVarImplicitOuts.insert(uid);
            }
        }
    }

    checkLoopCondition(visit, node);

    return true;
}

bool FunctionTraverser::tryRemoveUnary(Visit visit, TIntermUnary *node)
{
    if (visit == PreVisit)
    {
        mActiveNodes.push_back(std::make_pair(false, node));

        if (node->isAssignment())
        {
            TIntermSymbol *symbol = getSymbolNode(node->getOperand());
            if (mVarWorkset.find(symbol->variable().uniqueId().get()) != mVarWorkset.end())
            {
                for (auto iter = mActiveNodes.rbegin(); iter != mActiveNodes.rend(); iter++)
                {
                    iter->first = true;
                }
            }
        }

        return true;
    }

    if (!mActiveNodes.back().first)
    {
        tryRemoveNodeCommon(node);
    }

    mActiveNodes.pop_back();

    return true;
}

bool FunctionTraverser::analyseAggregate(Visit visit, TIntermAggregate *node)
{
    if (visit == PreVisit)
    {
        checkLoopCondition(visit, node);

        if (node->isFunctionCall())
        {
            for (auto &expr : mVarInExpr)
            {
                expr.insert(node->getFunction()->uniqueId().get());
            }

            mVarInExpr.push_back(std::unordered_set<int>());
        }

        return true;
    }

    if (node->isFunctionCall())
    {
        std::unordered_set<int> outs;
        std::unordered_set<int> ins;

        const TFunction *function = node->getFunction();
        for (size_t i = 0; i < function->getParamCount(); i++)
        {
            const TVariable *param = function->getParam(i);
            if (param->getType().getQualifier() == EvqOut ||
                param->getType().getQualifier() == EvqInOut)
            {
                TIntermSymbol *symbol = getSymbolNode((*node->getSequence())[i]);
                int uid               = symbol->variable().uniqueId().get();

                outs.insert(uid);

                if (isFunctionOut(symbol))
                {
                    mVarOuts.insert(uid);

                    if (isImplicitOut(symbol))
                    {
                        mVarImplicitOuts.insert(uid);
                    }
                }
            }
        }

        ins.insert(mVarInExpr.back().begin(), mVarInExpr.back().end());
        mVarInExpr.pop_back();

        int uid                                      = function->uniqueId().get();
        std::unordered_map<int, bool> &sideEffectMap = mModule.getSideEffectMap();
        if (sideEffectMap.find(uid) != sideEffectMap.end() && sideEffectMap[uid])
        {
            outs.insert(mUid);
            mVarOuts.insert(mUid);
            mVarImplicitOuts.insert(mUid);
        }

        for (int uid : outs)
        {
            mVarDependency[uid].insert(ins.begin(), ins.end());

            for (const auto &cond : mConditions)
            {
                mVarDependency[uid].insert(cond.first.begin(), cond.first.end());
            }

            for (auto &out : mVarLoopOuts)
            {
                out.insert(uid);
            }
        }
    }

    checkLoopCondition(visit, node);

    return true;
}

bool FunctionTraverser::tryRemoveAggregate(Visit visit, TIntermAggregate *node)
{
    if (visit == PreVisit)
    {
        mActiveNodes.push_back(std::make_pair(false, node));

        if (node->isFunctionCall())
        {
            const TFunction *function                    = node->getFunction();
            int uid                                      = function->uniqueId().get();
            bool shouldMark                              = false;
            std::unordered_map<int, bool> &sideEffectMap = mModule.getSideEffectMap();
            if (sideEffectMap.find(uid) != sideEffectMap.end() && sideEffectMap[uid])
            {
                shouldMark = true;
            }
            else
            {
                for (size_t i = 0; i < function->getParamCount(); i++)
                {
                    const TVariable *param = function->getParam(i);
                    if (param->getType().getQualifier() == EvqOut ||
                        param->getType().getQualifier() == EvqInOut)
                    {
                        TIntermSymbol *symbol = getSymbolNode((*node->getSequence())[i]);
                        if (mVarWorkset.find(symbol->variable().uniqueId().get()) !=
                            mVarWorkset.end())
                        {
                            shouldMark = true;
                            break;
                        }
                    }
                }
            }

            if (shouldMark)
            {
                for (auto iter = mActiveNodes.rbegin(); iter != mActiveNodes.rend(); iter++)
                {
                    iter->first = true;
                }
            }
        }

        return true;
    }

    if (!mActiveNodes.back().first)
    {
        tryRemoveNodeCommon(node);
    }

    mActiveNodes.pop_back();

    return true;
}

void FunctionTraverser::doTheWork(void)
{
    mModule.getSideEffectMap()[mUid] = mVarImplicitOuts.size() > 0;

    for (int uid : mVarOuts)
    {
        internalMarkDeps(uid, mVarWorkset);
    }
}

void FunctionTraverser::internalMarkDeps(int uid, std::unordered_set<int> &varWorkset)
{
    if (varWorkset.find(uid) != varWorkset.end())
    {
        return;
    }

    varWorkset.insert(uid);

    auto iter = mVarDependency.find(uid);
    if (iter != mVarDependency.end())
    {
        for (int uid : iter->second)
        {
            internalMarkDeps(uid, varWorkset);
        }
    }
}

void FunctionTraverser::visitSymbol(TIntermSymbol *node)
{
    if (mWorkMode == EmAnalyse)
    {
        checkLoopCondition(PreVisit, node);

        analyseSymbol(node);

        checkLoopCondition(PostVisit, node);

        return;
    }

    tryRemoveNodeCommon(node);
}

void FunctionTraverser::visitConstantUnion(TIntermConstantUnion *node)
{
    if (mWorkMode == EmAnalyse)
    {
        return;
    }

    tryRemoveNodeCommon(node);
}

bool FunctionTraverser::visitSwizzle(Visit visit, TIntermSwizzle *node)
{
    if (mWorkMode == EmAnalyse)
    {
        checkLoopCondition(visit, node);

        return true;
    }

    if (visit == PreVisit)
    {
        mActiveNodes.push_back(std::make_pair(false, node));

        return true;
    }

    if (!mActiveNodes.back().first)
    {
        tryRemoveNodeCommon(node);
    }

    mActiveNodes.pop_back();

    return true;
}

bool FunctionTraverser::visitBinary(Visit visit, TIntermBinary *node)
{
    if (mWorkMode == EmTryRemove)
    {
        tryRemoveBinary(visit, node);
    }
    else
    {
        analyseBinary(visit, node);
    }

    return true;
}

bool FunctionTraverser::visitUnary(Visit visit, TIntermUnary *node)
{
    if (mWorkMode == EmTryRemove)
    {
        tryRemoveUnary(visit, node);
    }
    else
    {
        analyseUnary(visit, node);
    }

    return true;
}

bool FunctionTraverser::visitTernary(Visit visit, TIntermTernary *node)
{
    if (mWorkMode == EmAnalyse)
    {
        checkLoopCondition(visit, node);

        return true;
    }

    if (visit == PreVisit)
    {
        mActiveNodes.push_back(std::make_pair(false, node));

        return true;
    }

    if (!mActiveNodes.back().first)
    {
        tryRemoveNodeCommon(node);
    }

    mActiveNodes.pop_back();

    return true;
}

bool FunctionTraverser::visitAggregate(Visit visit, TIntermAggregate *node)
{
    if (mWorkMode == EmTryRemove)
    {
        tryRemoveAggregate(visit, node);
    }
    else
    {
        analyseAggregate(visit, node);
    }

    return true;
}

bool FunctionTraverser::visitIfElse(Visit visit, TIntermIfElse *node)
{
    if (mWorkMode == EmAnalyse)
    {
        if (visit == PreVisit)
        {
            mVarInExpr.push_back(std::unordered_set<int>());

            return true;
        }

        return true;
    }

    if (visit == PreVisit)
    {
        mActiveNodes.push_back(std::make_pair(false, node));

        return true;
    }

    if (!mActiveNodes.back().first)
    {
        tryRemoveNodeCommon(node);
    }

    mActiveNodes.pop_back();

    return true;
}

bool FunctionTraverser::visitSwitch(Visit visit, TIntermSwitch *node)
{
    if (mWorkMode == EmAnalyse)
    {
        if (visit == PreVisit)
        {
            mVarInExpr.push_back(std::unordered_set<int>());

            return true;
        }

        return true;
    }

    if (visit == PreVisit)
    {
        mActiveNodes.push_back(std::make_pair(false, node));

        return true;
    }

    if (!mActiveNodes.back().first)
    {
        tryRemoveNodeCommon(node);
    }

    mActiveNodes.pop_back();

    return true;
}

bool FunctionTraverser::visitLoop(Visit visit, TIntermLoop *node)
{
    if (mWorkMode == EmAnalyse)
    {
        if (visit == PreVisit)
        {
            ++mDepthLoop;

            return true;
        }

        --mDepthLoop;

        return true;
    }

    if (visit == PreVisit)
    {
        mActiveNodes.push_back(std::make_pair(false, node));

        return true;
    }

    if (!mActiveNodes.back().first)
    {
        tryRemoveNodeCommon(node);
    }

    mActiveNodes.pop_back();

    return true;
}

bool FunctionTraverser::visitBlock(Visit visit, TIntermBlock *node)
{
    if (mWorkMode == EmAnalyse)
    {
        TIntermIfElse *selNode;

        if (visit == PreVisit)
        {
            if (((selNode = getParentNode()->getAsIfElseNode()) &&
                 selNode->getTrueBlock() == node) ||
                getParentNode()->getAsSwitchNode())
            {
                mConditions.push_back(std::make_pair(mVarInExpr.back(), getParentNode()));
                mVarInExpr.pop_back();
            }

            if (getParentNode()->getAsLoopNode())
            {
                mConditions.push_back(mLoopCond);
                mLoopCond = std::make_pair(std::unordered_set<int>(), nullptr);

                mVarLoopOuts.push_back(std::unordered_set<int>());
            }

            return true;
        }

        if (getParentNode()->getAsLoopNode())
        {
            mVarLoopOuts.pop_back();

            mConditions.pop_back();
        }

        if (((selNode = getParentNode()->getAsIfElseNode()) && selNode->getFalseBlock() == node) ||
            getParentNode()->getAsSwitchNode())
        {
            mConditions.pop_back();
        }

        return true;
    }

    if (visit == PreVisit)
    {
        mActiveNodes.push_back(std::make_pair(false, node));

        return true;
    }

    if (!mActiveNodes.back().first)
    {
        tryRemoveNodeCommon(node);
    }

    mActiveNodes.pop_back();

    return true;
}

bool FunctionTraverser::visitBranch(Visit visit, TIntermBranch *node)
{
    if (mWorkMode == EmTryRemove)
    {
        if (visit == PostVisit)
        {
            if (node->getChildCount() > 0)
            {
                mActiveNodes.pop_back();
            }
            return true;
        }

        if (node->getChildCount() > 0)
        {
            mActiveNodes.push_back(std::make_pair(false, node));
        }

        if (node->getFlowOp() == EOpReturn || node->getFlowOp() == EOpKill)
        {
            for (auto iter = mActiveNodes.rbegin(); iter != mActiveNodes.rend(); iter++)
            {
                iter->first = true;
            }
        }
        else if (node->getFlowOp() == EOpBreak || node->getFlowOp() == EOpContinue)
        {
            for (auto iter = mActiveNodes.rbegin(); iter != mActiveNodes.rend(); iter++)
            {
                if (iter->second->getAsLoopNode())
                {
                    break;
                }
                iter->first = true;
            }
        }

        return true;
    }

    if (visit == PreVisit)
    {
        if (node->getFlowOp() == EOpReturn)
        {
            if (node->getExpression())
            {
                mVarInExpr.push_back(std::unordered_set<int>());
            }
        }

        return true;
    }

    if (node->getFlowOp() == EOpReturn)
    {
        if (node->getExpression())
        {
            mVarDependency[mUid].insert(mVarInExpr.back().begin(), mVarInExpr.back().end());
            mVarInExpr.pop_back();
        }

        for (const auto &cond : mConditions)
        {
            mVarDependency[mUid].insert(cond.first.begin(), cond.first.end());
        }

        for (auto &out : mVarLoopOuts)
        {
            out.insert(mUid);
        }

        mVarOuts.insert(mUid);
    }
    else if (node->getFlowOp() == EOpKill)
    {
        for (const auto &cond : mConditions)
        {
            mVarDependency[mUid].insert(cond.first.begin(), cond.first.end());
        }

        for (auto &out : mVarLoopOuts)
        {
            out.insert(mUid);
        }

        mVarOuts.insert(mUid);

        mVarImplicitOuts.insert(mUid);
    }
    else if (node->getFlowOp() == EOpBreak || node->getFlowOp() == EOpContinue)
    {
        std::unordered_set<int> innerCond;
        for (auto iter = mConditions.rbegin(); iter != mConditions.rend(); iter++)
        {
            iter->first.insert(innerCond.begin(), innerCond.end());
            if (iter->second->getAsLoopNode())
            {
                break;
            }
            innerCond.insert(iter->first.begin(), iter->first.end());
        }

        for (int uid : mVarLoopOuts.back())
        {
            mVarDependency[uid].insert(innerCond.begin(), innerCond.end());
        }
    }

    return true;
}

}  // namespace

bool RemoveDeadCodeNaive(TCompiler *compiler, TIntermBlock *root, TSymbolTable *symbolTable)
{
    ModuleTraverser preModule(root);
    root->traverse(&preModule);
    preModule.doTheWork();
    if (!preModule.updateTree(compiler, root))
    {
        return false;
    }

    std::unordered_map<int, TIntermFunctionDefinition *> &functionMap = preModule.getFunctionMap();
    std::vector<int> &funcWorklist                                    = preModule.getFuncWorklist();
    for (int uid : funcWorklist)
    {
        FunctionTraverser function(preModule, uid);
        functionMap[uid]->traverse(&function);

        function.doTheWork();
        function.setWorkMode(EmTryRemove);

        functionMap[uid]->traverse(&function);
        if (!function.updateTree(compiler, root))
        {
            return false;
        }
    }

    ModuleTraverser postModule(root);
    root->traverse(&postModule);
    postModule.doTheWork();
    if (!postModule.updateTree(compiler, root))
    {
        return false;
    }

    return true;
}

}  // namespace sh
