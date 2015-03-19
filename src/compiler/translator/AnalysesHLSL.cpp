//
// Copyright (c) 2002-2015 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//

// Defines several analyses of the AST needed for HLSL generation

#include "compiler/translator/AnalysesHLSL.h"

#include "compiler/translator/CallDAG.h"
#include "compiler/translator/SymbolTable.h"

typedef std::vector<AnalysesHLSLData> Analyses;

// Class used to traverse the AST of a function definition, checking if the
// function uses a gradient, and writing the set of control flow using gradients.
// It assumes that the analysis has already been made for the function's
// callees.
class PullGradient : public TIntermTraverser
{
  public:
    PullGradient(Analyses &analyses, size_t index, const CallDAG &dag)
        : TIntermTraverser(true, false, true),
          mAnalyses(analyses),
          mAnalysis(analyses[index]),
          mIndex(index),
          mDag(dag)
    {
        ASSERT(index < analyses.size());
    }

    void traverse(TIntermAggregate *node)
    {
        node->traverse(this);
        ASSERT(mParents.empty());
    }

    // Called when a gradient operation or a call to a function using a gradient is found.
    void onGradient()
    {
        mAnalysis.usesGradient = true;
        // Mark the latest control flow as using a gradient.
        if (!mParents.empty())
        {
            mAnalysis.controlFlowsContainingGradient.insert(mParents.back());
        }
    }

    void visitControlFlow(Visit visit, TIntermNode *node)
    {
        if (visit == PreVisit)
        {
            mParents.push_back(node);
        }
        else if (visit == PostVisit)
        {
            ASSERT(mParents.back() == node);
            mParents.pop_back();
            // A control flow's using a gradient means its parents are too.
            if (mAnalysis.controlFlowsContainingGradient.count(node)> 0 && !mParents.empty())
            {
                mAnalysis.controlFlowsContainingGradient.insert(mParents.back());
            }
        }
    }

    bool visitLoop(Visit visit, TIntermLoop *loop)
    {
        visitControlFlow(visit, loop);
        return true;
    }

    bool visitSelection(Visit visit, TIntermSelection *selection)
    {
        visitControlFlow(visit, selection);
        return true;
    }

    bool visitUnary(Visit visit, TIntermUnary *node) override
    {
        if (visit == PreVisit)
        {
            switch (node->getOp())
            {
              case EOpDFdx:
              case EOpDFdy:
                onGradient();
              default:
                break;
            }
        }

        return true;
    }

    bool visitAggregate(Visit visit, TIntermAggregate *node) override
    {
        if (visit == PreVisit)
        {
            if (node->getOp() == EOpFunctionCall)
            {
                if (node->isUserDefined())
                {
                    size_t calleeIndex = mDag.findIndex(node);
                    ASSERT(calleeIndex != CallDAG::InvalidIndex && calleeIndex < mIndex);

                    if (mAnalyses[calleeIndex].usesGradient) {
                        onGradient();
                    }
                }
                else
                {
                    TString name = TFunction::unmangleName(node->getName());

                    if (name == "texture2D" ||
                        name == "texture2DProj" ||
                        name == "textureCube")
                    {
                        onGradient();
                    }
                }
            }
        }

        return true;
    }

  private:
    Analyses &mAnalyses;
    AnalysesHLSLData &mAnalysis;
    size_t mIndex;
    const CallDAG &mDag;

    // Contains a stack of the control flow nodes that are parents of the node being
    // currently visited. It is used to mark control flows using a gradient.
    std::vector<TIntermNode*> mParents;
};

// Traverses the AST of a function definition, assuming it has already been used to
// traverse the callees of that function; computes the discontinuous loops and the if
// statements that contain a discontinuous loop in their call graph.
class PullComputeDiscontinuousLoops : public TIntermTraverser
{
  public:
    PullComputeDiscontinuousLoops(Analyses &analyses, size_t index, const CallDAG &dag)
        : TIntermTraverser(true, false, true),
          mAnalyses(analyses),
          mAnalysis(analyses[index]),
          mIndex(index),
          mDag(dag)
    {
    }

    void traverse(TIntermAggregate *node)
    {
        node->traverse(this);
        ASSERT(mLoops.empty());
        ASSERT(mIfs.empty());
    }

    // Called when a discontinuous loop or a call to a function with a discontinuous loop
    // in its call graph is found.
    void onDiscontinuousLoop()
    {
        mAnalysis.hasDiscontinuousLoopInCallGraph = true;
        // Mark the latest if as using a discontinuous loop.
        if (!mIfs.empty())
        {
            mAnalysis.ifsContainingDiscontinuousLoop.insert(mIfs.back());
        }
    }

    bool visitLoop(Visit visit, TIntermLoop *loop)
    {
        if (visit == PreVisit)
        {
            mLoops.push_back(loop);
        }
        else if (visit == PostVisit)
        {
            ASSERT(mLoops.back() == loop);
            mLoops.pop_back();
        }

        return true;
    }

    bool visitSelection(Visit visit, TIntermSelection *node)
    {
        if (visit == PreVisit)
        {
            mIfs.push_back(node);
        }
        else if (visit == PostVisit)
        {
            ASSERT(mIfs.back() == node);
            mIfs.pop_back();
            // An if using a discontinuous loop means its parents ifs are too.
            if (mAnalysis.ifsContainingDiscontinuousLoop.count(node)> 0 && !mIfs.empty())
            {
                mAnalysis.ifsContainingDiscontinuousLoop.insert(mIfs.back());
            }
        }

        return true;
    }

    bool visitBranch(Visit visit, TIntermBranch *node)
    {
        if (visit == PreVisit)
        {
            switch (node->getFlowOp())
            {
              case EOpKill:
                break;
              case EOpBreak:
              case EOpContinue:
                ASSERT(!mLoops.empty());
                mAnalysis.discontinuousLoops.insert(mLoops.back());
                onDiscontinuousLoop();
                break;
              case EOpReturn:
                // A return jumps out of all the enclosing loops
                if (!mLoops.empty())
                {
                    for (TIntermLoop* loop : mLoops)
                    {
                        mAnalysis.discontinuousLoops.insert(loop);
                    }
                    onDiscontinuousLoop();
                }
                break;
              default:
                UNREACHABLE();
            }
        }

        return true;
    }

    bool visitAggregate(Visit visit, TIntermAggregate *node) override
    {
        if (visit == PreVisit && node->getOp() == EOpFunctionCall)
        {
            if (node->isUserDefined())
            {
                size_t calleeIndex = mDag.findIndex(node);
                ASSERT(calleeIndex != CallDAG::InvalidIndex && calleeIndex < mIndex);

                if (mAnalyses[calleeIndex].hasDiscontinuousLoopInCallGraph) {
                    onDiscontinuousLoop();
                }
            }
        }

        return true;
    }

  private:
    Analyses &mAnalyses;
    AnalysesHLSLData &mAnalysis;
    size_t mIndex;
    const CallDAG &mDag;

    std::vector<TIntermLoop*> mLoops;
    std::vector<TIntermSelection*> mIfs;
};

// Tags all the functions called in a discontinuous loop
class PushDiscontinuousLoops : public TIntermTraverser
{
public:
    PushDiscontinuousLoops(Analyses &analyses, size_t index, const CallDAG &dag)
        : TIntermTraverser(true, true, true),
          mAnalyses(analyses),
          mAnalysis(analyses[index]),
          mIndex(index),
          mDag(dag),
          mNestedDiscont(mAnalysis.calledInDiscontinuousLoop ? 1 : 0)
    {
    }

    void traverse(TIntermAggregate *node)
    {
        node->traverse(this);
        ASSERT(mNestedDiscont == (mAnalysis.calledInDiscontinuousLoop ? 1 : 0));
    }

    bool visitLoop(Visit visit, TIntermLoop *loop)
    {
        bool isDiscontinuous = mAnalysis.discontinuousLoops.count(loop) > 0;

        if (visit == PreVisit && isDiscontinuous)
        {
            mNestedDiscont ++;
        }
        else if (visit == PostVisit && isDiscontinuous)
        {
            mNestedDiscont --;
        }

        return true;
    }

    bool visitAggregate(Visit visit, TIntermAggregate *node) override
    {
        switch (node->getOp())
        {
          case EOpFunctionCall:
            if (visit == PreVisit)
            {
                // Do not handle calls to builtin functions
                if (node->isUserDefined())
                {
                    size_t calleeIndex = mDag.findIndex(node);
                    ASSERT(calleeIndex != CallDAG::InvalidIndex && calleeIndex < mIndex);

                    mAnalyses[calleeIndex].calledInDiscontinuousLoop = true;
                }
            }
            break;
          default:
            break;
        }
        return true;
    }

private:
    Analyses &mAnalyses;
    AnalysesHLSLData &mAnalysis;
    size_t mIndex;
    const CallDAG &mDag;

    int mNestedDiscont;
};
bool AnalysesHLSLData::hasGradientInCallGraph(TIntermSelection *node)
{
    return controlFlowsContainingGradient.count(node) > 0;
}

bool AnalysesHLSLData::hasGradientInCallGraph(TIntermLoop *node)
{
    return controlFlowsContainingGradient.count(node) > 0;
}

bool AnalysesHLSLData::hasDiscontinuousLoop(TIntermSelection *node)
{
    return ifsContainingDiscontinuousLoop.count(node) > 0;
}

std::vector<AnalysesHLSLData> createHLSLAnalyses(TIntermNode *root, const CallDAG &callDag) {
    std::vector<AnalysesHLSLData> analyses(callDag.size());

    // Compute all the information related to when gradient operations are used.
    // We want to know for each function and control flow operation if they have
    // a gradient operation in their call graph (shortened to "using a gradient"
    // in the rest of the file).
    //
    // This computation is logically split in three steps:
    //  1 - For each function compute if it uses a gradient in its body, ignoring
    // calls to other user-defined functions.
    //  2 - For each function determine if it uses a gradient, using the result of
    // step 1.
    //  3 - For each control flow statement of each function, check if it uses a
    // gradient in the function's body, or if it calls a user-defined function that
    // uses a gradient.
    //
    // We take advantage of the call graph being a DAG and instead compute 1, 2 and 3
    // for leaves first, then going down the tree. This is correct because 1 doesn't
    // depend on other functions, and 2 and 3 depend only on callees.
    for (size_t i = 0; i < callDag.size(); i++)
    {
        PullGradient pull(analyses, i, callDag);
        pull.traverse(callDag.getRecordFromIndex(i).node);
    }

    // Compute which loops are discontinuous and which function are called in
    // these loops. The same way computing gradient usage is a "pull" process,
    // computing "bing used in a discont. loop" is a push process. However we also
    // need to know what ifs have a discontinuous loop inside so we do the same type
    // of callgraph analysis as for the gradient.

    // First compute which loops are discontinuous (no specific order) and pull
    // the ifs and functions using a discontinuous loop.
    for (size_t i = 0; i < callDag.size(); i++)
    {
        PullComputeDiscontinuousLoops pull(analyses, i, callDag);
        pull.traverse(callDag.getRecordFromIndex(i).node);
    }

    // Then push the information to callees, either from the a local discontinuous
    // loop or from the caller being called in a discontinuous loop already
    for (int i = callDag.size(); i-- > 0;)
    {
        PushDiscontinuousLoops push(analyses, i, callDag);
        push.traverse(callDag.getRecordFromIndex(i).node);
    }

    // We create "Lod0" version of functions with the gradient operations replaced
    // by non-gradient operations so that the D3D compiler is happier with discont
    // loops.
    for (auto &analysis : analyses)
    {
        analysis.needsLod0 = analysis.calledInDiscontinuousLoop && analysis.usesGradient;
    }

    return analyses;
}