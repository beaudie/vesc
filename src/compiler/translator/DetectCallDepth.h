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

    // Returns false if it was not able to create the DAG, signals a recursion
    // FIXME proper error codes?
    bool create(TIntermNode* root);

    int mangledNameToIndex(const TString& name) const;
    const Record& getRecord(int index) const;
    int size() const;

private:
    std::vector<Record> records;
    std::unordered_map<TString, int> nameToIndex;

    class CallDAGCreator : public TIntermTraverser
    {
    public:
        CallDAGCreator();
        ~CallDAGCreator();

        virtual bool visitAggregate(Visit visit, TIntermAggregate* node);

        bool assignIndices();
        void fillDataStructures(std::vector<Record>& records, std::unordered_map<TString, int>& nameToIndex);

    private:
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

class TInfoSink;

// Traverses intermediate tree to detect function recursion.
class DetectCallDepth : public TIntermTraverser {
public:
    enum ErrorCode {
        kErrorMissingMain,
        kErrorRecursion,
        kErrorMaxDepthExceeded,
        kErrorNone
    };

    DetectCallDepth(TInfoSink& infoSync, bool limitCallStackDepth, int maxCallStackDepth);
    ~DetectCallDepth();

    virtual bool visitAggregate(Visit, TIntermAggregate*);

    bool checkExceedsMaxDepth(int depth);

    ErrorCode detectCallDepth();

private:
    class FunctionNode {
    public:
        static const int kInfiniteCallDepth = INT_MAX;

        FunctionNode(const TString& fname);

        const TString& getName() const;

        // If a function is already in the callee list, this becomes a no-op.
        void addCallee(FunctionNode* callee);

        // Returns kInifinityCallDepth if recursive function calls are detected.
        int detectCallDepth(DetectCallDepth* detectCallDepth, int depth);

        // Reset state.
        void reset();

    private:
        // mangled function name is unique.
        TString name;

        // functions that are directly called by this function.
        TVector<FunctionNode*> callees;

        Visit visit;
    };

    ErrorCode detectCallDepthForFunction(FunctionNode* func);
    FunctionNode* findFunctionByName(const TString& name);
    void resetFunctionNodes();

    TInfoSink& getInfoSink() { return infoSink; }

    TVector<FunctionNode*> functions;
    FunctionNode* currentFunction;
    TInfoSink& infoSink;
    int maxDepth;

    DetectCallDepth(const DetectCallDepth&);
    void operator=(const DetectCallDepth&);
};

DetectCallDepth::ErrorCode myDetectCallDepth(TIntermNode* root, bool limitCallStackDepth, int maxCallStackDepth, DetectCallDepth::ErrorCode expected);

#endif  // COMPILER_DETECT_RECURSION_H_
