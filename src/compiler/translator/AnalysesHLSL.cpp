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
class PullGradientFromBuiltins : public TIntermTraverser
{
public:
    PullGradientFromBuiltins(Analyses &analyses, size_t index, const CallDAG &dag)
        : TIntermTraverser(true, true, true),
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
        switch (node->getOp())
        {
          case EOpDFdx:
          case EOpDFdy:
            onGradient();
          default:
            break;
        }

        return true;
    }

    bool visitAggregate(Visit visit, TIntermAggregate *node) override
    {
        if (node->getOp() == EOpFunctionCall)
        {
            if (node->isUserDefined())
            {
                int calleeIndex = mDag.mangledNameToIndex(node->getName().data());
                ASSERT(calleeIndex >= 0);
                ASSERT(calleeIndex < mIndex);

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

        return true;
    }

private:
    Analyses &mAnalyses;
    AnalysesHLSLData &mAnalysis;
    int mIndex;
    const CallDAG &mDag;

    // Contains a stack of the control flow nodes that are parents of the node being
    // currently visited. It is used to mark control flows using a gradient.
    std::vector<TIntermNode*> mParents;
};

bool AnalysesHLSLData::hasGradientInCallGraph(TIntermSelection *node)
{
    return controlFlowsContainingGradient.count(node) > 0;
}

bool AnalysesHLSLData::hasGradientInCallGraph(TIntermLoop *node)
{
    return controlFlowsContainingGradient.count(node) > 0;
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
    for (size_t i = 0; i < callDag.size(); i++) {
        PullGradientFromBuiltins pull(analyses, i, callDag);
        pull.traverse(callDag.getRecord(i).node);
    }

    return analyses;
}