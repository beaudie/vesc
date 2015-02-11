//
// Copyright (c) 2002-2015 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//

#ifndef COMPILER_TRANSLATOR_CALLDAG_H_
#define COMPILER_TRANSLATOR_CALLDAG_H_

#include <limits.h>
#include "compiler/translator/IntermNode.h"
#include "compiler/translator/VariableInfo.h"

#include <unordered_map>

// We need to have check that analyze the graph of the function calls
// and in GLSL recursion is not allowed, so that graph is a DAG.
// This class is used to precompute that function call DAG so that it
// can be reused by multiple analysis.
//
// It stores a vector of function record, with one record per function and
// records are accessed by index but a mangled function name can be converted
// to the index of the corresponding record. The records mostly contain the
// AST node of the function and the indices of the function's callees.
//
// In addition records are in reverse topological order: a function F being
// called by a function G will have index index(F) < index(G), that way
// depth-first analysis become analysis in the order of indices.

class CallDAG
{
public:
    CallDAG();
    ~CallDAG();

    struct Record {
        std::string name;
        TIntermAggregate* node;
        std::vector<int> callees;
    };

    enum CreateResult {
        CRSuccess,
        CRRecursion,
    };

    // Returns false if it was not able to create the DAG, signals a recursion
    // if present, the recursion chain will be output to info
    CreateResult create(TIntermNode* root, TInfoSink* info);

    // Returns -1 if the function wasn't found
    int mangledNameToIndex(const std::string& name) const;
    const Record& getRecord(int index) const;
    int size() const;
    void clear();

private:
    std::vector<Record> records;
    std::unordered_map<std::string, int> nameToIndex;

    class CallDAGCreator;

    CallDAG(const CallDAG&);
    void operator=(const CallDAG&);
};

#endif  // COMPILER_TRANSLATOR_CALLDAG_H_
