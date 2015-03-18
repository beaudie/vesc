//
// Copyright (c) 2002-2015 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//

// Defines several analyses of the AST needed for HLSL generation

#ifndef COMPILER_TRANSLATOR_ANALYSESHLSL_H_
#define COMPILER_TRANSLATOR_ANALYSESHLSL_H_

#include <set>
#include <vector>

class CallDAG;
class TIntermNode;
class TIntermSelection;
class TIntermLoop;

struct AnalysesHLSLData
{
    AnalysesHLSLData()
        : usesGradient(false)
    {
    }

    // Here "something uses a gradient" means here that it either contains a
    // gradient operation, or a call to a function that uses a gradient.

    // Does the function use a gradient.
    bool usesGradient;

    // Even if usesGradient is true, some control flow might not use a gradient
    // so we store the set of all gradient-using control flows.
    bool hasGradientInCallGraph(TIntermSelection *node);
    bool hasGradientInCallGraph(TIntermLoop *node);
    std::set<TIntermNode*> controlFlowsContainingGradient;
};

// Return the AST analysis result, in the order defined by the call DAG
std::vector<AnalysesHLSLData> createHLSLAnalyses(TIntermNode *root, const CallDAG &callDag);

#endif // COMPILER_TRANSLATOR_ANALYSESHLSL_H_
