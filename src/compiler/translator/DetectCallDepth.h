//
// Copyright (c) 2002-2011 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//

#ifndef COMPILER_DETECT_RECURSION_H_
#define COMPILER_DETECT_RECURSION_H_

#include <limits.h>
#include "compiler/translator/IntermNode.h"
#include "compiler/translator/VariableInfo.h"


//TODO move to its own file, make a higher lvel API?
//TODO document better

#include "unordered_map"

// Basically a vector of function records, one per function in the IR.
// Records in the vector are in reverse topological order, that is a function
// caller is always at a later position in the vector.
class CallDAG : public TIntermTraverser
{
public:
    CallDAG();
    ~CallDAG();

    // FIXME should we include the index too?
    struct Record {
        TString name;
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
    int mangledNameToIndex(const TString& name) const;
    const Record& getRecord(int index) const;
    int size() const;

private:
    std::vector<Record> records;
    std::unordered_map<TString, int> nameToIndex;

    class CallDAGCreator : public TIntermTraverser
    {
    public:
        CallDAGCreator(TInfoSink* info);
        ~CallDAGCreator();

        virtual bool visitAggregate(Visit visit, TIntermAggregate* node);

        bool assignIndices();
        void fillDataStructures(std::vector<Record>& records, std::unordered_map<TString, int>& nameToIndex);

    private:
        TInfoSink* creationInfo;

        struct CreatorFunctionData
        {
            std::set<CreatorFunctionData*> callees;
            TIntermAggregate* node;
            TString name;
            int index = 0;
            bool indexAssigned = false;
            bool visiting = false;
        };

        bool assignIndicesInternal(CreatorFunctionData* function);

        std::unordered_map<TString, CreatorFunctionData> functions;
        CreatorFunctionData* currentFunction;

        int currentIndex;
    };

    CallDAG(const CallDAG&);
    void operator=(const CallDAG&);
};

#endif  // COMPILER_DETECT_RECURSION_H_
