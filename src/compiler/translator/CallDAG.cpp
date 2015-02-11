//
// Copyright (c) 2002-2015 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//

// CallDAG.h: Implements a call graph DAG of functions to be re-used accross
// analyses, allows to efficiently traverse the functions in topological
// order.

#include "compiler/translator/CallDAG.h"
#include "compiler/translator/InfoSink.h"

// The CallDAGCreator does all the processing required to create the CallDAG
// structure so that the latter contains only the necessary variables.
class CallDAG::CallDAGCreator : public TIntermTraverser
{
  private:
    TInfoSink *mCreationInfo;

    struct CreatorFunctionData
    {
        std::set<CreatorFunctionData*> callees;
        TIntermAggregate *node;
        TString name;
        int index = 0;
        bool indexAssigned = false;
        bool visiting = false;
    };

    std::map<TString, CreatorFunctionData> mFunctions;
    CreatorFunctionData *mCurrentFunction;
    int mCurrentIndex;

    // Aggregates the AST node for each function as well as the name of the functions called by it

    virtual bool visitAggregate(Visit visit, TIntermAggregate *node)
    {
        switch (node->getOp())
        {
          case EOpPrototype:
            // Function declaration, create an empty record.
            mFunctions.emplace(node->getName(), CreatorFunctionData());
            break;
          case EOpFunction:
            {
                // Function definition, create the record if need be and remember the node.
                if (visit == PreVisit) {
                    auto it = mFunctions.find(node->getName());

                    if (it == mFunctions.end())
                    {
                        mCurrentFunction = &mFunctions.emplace(node->getName(), CreatorFunctionData()).first->second;
                    }
                    else
                    {
                        mCurrentFunction = &it->second;
                    }

                    mCurrentFunction->node = node;
                    mCurrentFunction->name = node->getName();

                }
                else if (visit == PostVisit)
                {
                    mCurrentFunction = NULL;
                }
                break;
            }
          case EOpFunctionCall:
            {
                // Function call.
                if (visit == PreVisit)
                {
                    ASSERT(mCurrentFunction != NULL);

                    // Do not handle calls to builtin functions
                    if (node->isUserDefined())
                    {
                        auto it = mFunctions.find(node->getName());
                        ASSERT(it != mFunctions.end());

                        mCurrentFunction->callees.insert(&it->second);
                    }
                }
                break;
            }
          default:
            break;
        }
        return true;
    }

    // Recursively assigns indices to a sub DAG
    bool assignIndicesInternal(CreatorFunctionData *function)
    {
        ASSERT(function);

        if (function->indexAssigned)
        {
            return true;
        }

        if (function->visiting)
        {
            if (mCreationInfo)
            {
                mCreationInfo->info << "Recursive function call in the following call chain: " << function->name;
            }
            return false;
        }
        function->visiting = true;

        for (auto& callee : function->callees)
        {
            if (!assignIndicesInternal(callee))
            {
                // We know that there is a recursive function call chain in the AST,
                // print the link of the chain we were processing.
                if (mCreationInfo)
                {
                    mCreationInfo->info << " <- " << function->name;
                }
                return false;
            }
        }

        function->index = mCurrentIndex++;
        function->indexAssigned = true;

        function->visiting = false;
        return true;
    }

  public:
    CallDAGCreator(TInfoSink *info)
        : mCurrentFunction(NULL), mCurrentIndex(0), mCreationInfo(info)
    {
    }

    bool assignIndices()
    {
        for (auto &it : mFunctions)
        {
            if (!assignIndicesInternal(&it.second))
            {
                return false;
            }
        }
        ASSERT(mCurrentIndex == (int)mFunctions.size());
        return true;
    }

    void fillDataStructures(std::vector<Record> &records, std::map<std::string, int> &nameToIndex)
    {
        ASSERT(records.empty());
        ASSERT(nameToIndex.empty());

        records.resize(mFunctions.size());

        for (auto& it : mFunctions)
        {
            CreatorFunctionData& data = it.second;
            Record& record = records[data.index];

            record.name = data.name.data();
            record.node = data.node;

            for (auto& callee : data.callees)
            {
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

int CallDAG::mangledNameToIndex(const std::string &name) const
{
    auto it = mNameToIndex.find(name);

    if (it == mNameToIndex.end())
    {
        return -1;
    }
    else
    {
        return it->second;
    }
}

const CallDAG::Record& CallDAG::getRecord(size_t index) const
{
    ASSERT(index >= 0);
    ASSERT(index < mRecords.size());
    return mRecords[index];
}

size_t CallDAG::size() const
{
    return mRecords.size();
}

void CallDAG::clear()
{
    mRecords.clear();
    mNameToIndex.clear();
}

CallDAG::InitResult CallDAG::init(TIntermNode *root, TInfoSink *info)
{
    CallDAGCreator creator(info);

    // Creates the mapping of functions to callees
    root->traverse(&creator);

    // Does the topological sort and detects recursions
    if (!creator.assignIndices())
    {
        return INITDAG_RECURSION;
    }

    creator.fillDataStructures(mRecords, mNameToIndex);
    return INITDAG_SUCCESS;
}
