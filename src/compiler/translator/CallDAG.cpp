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
  public:
    CallDAGCreator(TInfoSink *info)
        : mCurrentFunction(nullptr),
        mCurrentIndex(0),
        mCreationInfo(info)
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
        ASSERT(mCurrentIndex == mFunctions.size());
        return true;
    }

    void fillDataStructures(std::vector<Record> &records, std::map<std::string, int> &nameToIndex)
    {
        ASSERT(records.empty());
        ASSERT(nameToIndex.empty());

        records.resize(mFunctions.size());

        for (auto &it : mFunctions)
        {
            CreatorFunctionData &data = it.second;
            Record &record = records[data.index];

            record.name = data.name.data();
            record.node = data.node;

            record.callees.reserve(data.callees.size());
            for (auto &callee : data.callees)
            {
                record.callees.push_back(callee->index);
            }

            nameToIndex[data.name.data()] = data.index;
        }
    }

  private:
    TInfoSink *mCreationInfo;

    struct CreatorFunctionData
    {
        CreatorFunctionData()
            : node(nullptr),
              index(0),
              indexAssigned(false),
              visiting(false)
        {
        }

        std::set<CreatorFunctionData*> callees;
        TIntermAggregate *node;
        TString name;
        int index;
        bool indexAssigned;
        bool visiting;
    };

    std::map<TString, CreatorFunctionData> mFunctions;
    CreatorFunctionData *mCurrentFunction;
    size_t mCurrentIndex;

    // Aggregates the AST node for each function as well as the name of the functions called by it
    bool visitAggregate(Visit visit, TIntermAggregate *node) override
    {
        switch (node->getOp())
        {
          case EOpPrototype:
            // Function declaration, create an empty record.
            mFunctions[node->getName()];
            break;
          case EOpFunction:
            {
                // Function definition, create the record if need be and remember the node.
                if (visit == PreVisit)
                {
                    auto it = mFunctions.find(node->getName());

                    if (it == mFunctions.end())
                    {
                        mCurrentFunction = &mFunctions[node->getName()];
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
                    mCurrentFunction = nullptr;
                }
                break;
            }
          case EOpFunctionCall:
            {
                // Function call.
                if (visit == PreVisit)
                {
                    ASSERT(mCurrentFunction != nullptr);

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

        for (auto &callee : function->callees)
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

const CallDAG::Record &CallDAG::getRecord(size_t index) const
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
