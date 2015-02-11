//
// Copyright (c) 2002-2015 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//

#include "compiler/translator/CallDAG.h"
#include "compiler/translator/InfoSink.h"

// The CallDAGCreator does all the processing required to create the CallDAG
// structure so that the latter contains only the necessary variables.
class CallDAG::CallDAGCreator : public TIntermTraverser
{
public:
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

    std::map<TString, CreatorFunctionData> functions;
    CreatorFunctionData* currentFunction;
    int currentIndex;

    CallDAGCreator(TInfoSink* info) : currentFunction(NULL), currentIndex(0), creationInfo(info)
    {
    }

    // Aggregates the AST node for each function as well as the name of the functions called by it

    virtual bool visitAggregate(Visit visit, TIntermAggregate* node)
    {
        switch (node->getOp())
        {
        case EOpPrototype:
            // Function declaration, create an empty record.
            functions.emplace(node->getName(), CreatorFunctionData());
            break;
        case EOpFunction: {
            // Function definition, create the record if need be and remember the node.
            if (visit == PreVisit) {
                auto it = functions.find(node->getName());

                if (it == functions.end()) {
                    currentFunction = &functions.emplace(node->getName(), CreatorFunctionData()).first->second;
                }
                else {
                    currentFunction = &it->second;
                }

                currentFunction->node = node;
                currentFunction->name = node->getName();

            }
            else if (visit == PostVisit) {
                currentFunction = NULL;
            }
            break;
        }
        case EOpFunctionCall: {
            // Function call.
            if (visit == PreVisit) {
                ASSERT(currentFunction != NULL);

                // Do not handle calls to builtin functions
                if (node->isUserDefined()) {
                    auto it = functions.find(node->getName());
                    ASSERT(it != functions.end());

                    currentFunction->callees.insert(&it->second);
                }
            }
            break;
        }
        default:
            break;
        }
        return true;
    }

    bool assignIndices()
    {
        for (auto& it : functions) {
            if (!assignIndicesInternal(&it.second)) {
                return false;
            }
        }
        ASSERT(currentIndex == (int)functions.size());
        return true;
    }

    // Recursively assigns indices to a sub DAG
    bool assignIndicesInternal(CreatorFunctionData* function)
    {
        ASSERT(function);

        if (function->indexAssigned) {
            return true;
        }

        if (function->visiting) {
            if (creationInfo) {
                creationInfo->info << "Recursive function call in the following call chain: " << function->name;
            }
            return false;
        }
        function->visiting = true;

        for (auto& callee : function->callees) {
            if (!assignIndicesInternal(callee)) {
                // We know that there is a recursive function call chain in the AST,
                // print the link of the chain we were processing.
                if (creationInfo) {
                    creationInfo->info << " <- " << function->name;
                }
                return false;
            }
        }

        function->index = currentIndex++;
        function->indexAssigned = true;

        function->visiting = false;
        return true;
    }

    void fillDataStructures(std::vector<Record>& records, std::map<std::string, int>& nameToIndex)
    {
        ASSERT(records.empty());
        ASSERT(nameToIndex.empty());

        records.resize(functions.size());

        for (auto& it : functions) {
            CreatorFunctionData& data = it.second;
            Record& record = records[data.index];

            record.name = data.name.data();
            record.node = data.node;

            for (auto& callee : data.callees) {
                record.callees.push_back(callee->index);
            }

            nameToIndex.emplace(data.name.data(), data.index);
        }
    }
};

// CallDAG

CallDAG::CallDAG()
{
}

CallDAG::~CallDAG()
{
}

int CallDAG::mangledNameToIndex(const std::string& name) const
{
    auto it = nameToIndex.find(name);

    if (it == nameToIndex.end()) {
        return -1;
    } else {
        return it->second;
    }
}

const CallDAG::Record& CallDAG::getRecord(int index) const
{
    ASSERT(index >= 0);
    ASSERT(index < (int)records.size());
    return records[index];
}

int CallDAG::size() const
{
    return records.size();
}

void CallDAG::clear() {
    records.clear();
    nameToIndex.clear();
}

CallDAG::CreateResult CallDAG::create(TIntermNode* root, TInfoSink* info)
{
    CallDAGCreator creator(info);

    // Creates the mapping of functions to callees
    root->traverse(&creator);

    // Does the topological sort and detects recursions
    if (!creator.assignIndices()) {
        return CRRecursion;
    }

    creator.fillDataStructures(records, nameToIndex);
    return CRSuccess;
}
