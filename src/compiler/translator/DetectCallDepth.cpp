//
// Copyright (c) 2002-2011 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//

#include "compiler/translator/DetectCallDepth.h"
#include "compiler/translator/InfoSink.h"

CallDAG::CallDAG()
{
}

CallDAG::~CallDAG()
{
}

int CallDAG::mangledNameToIndex(const TString& name) const
{
    auto& it = nameToIndex.find(name);

    if (it == nameToIndex.end()) {
        return -1;
    } else {
        return it->second;
    }
}

const CallDAG::Record& CallDAG::getRecord(int index) const
{
    ASSERT(index >= 0);
    ASSERT(index < records.size());
    return records[index];
}

int CallDAG::size() const
{
    return records.size();
}

//TODO(kangz) return an error code?
bool CallDAG::create(TIntermNode* root)
{
    CallDAGCreator creator;

    // Creates the mapping of functions to callees
    root->traverse(&creator);

    // Does the topological sort and detects recursions
    if (!creator.assignIndices()) {
        return false;
    }

    creator.fillDataStructures(records, nameToIndex);
    return true;
}

// CallDAGCreator

CallDAG::CallDAGCreator::CallDAGCreator()
: currentFunction(NULL), currentIndex(0)
{
}

CallDAG::CallDAGCreator::~CallDAGCreator()
{
}

// Aggregates the AST node for each function as well as the functions called by it
bool CallDAG::CallDAGCreator::visitAggregate(Visit visit, TIntermAggregate* node)
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
            auto& it = functions.find(node->getName());

            if (it == functions.end()) {
                currentFunction = &functions.emplace(node->getName(), CreatorFunctionData()).first->second;
            } else {
                currentFunction = &it->second;
            }

            currentFunction->node = node;
            currentFunction->name = node->getName();

        } else if (visit == PostVisit) {
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
                auto& it = functions.find(node->getName());
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

bool CallDAG::CallDAGCreator::assignIndices()
{
    for (auto& it : functions) {
        if (!assignIndicesInternal(&it.second)) {
            return false;
        }
    }
    ASSERT(currentIndex == functions.size());
    return true;
}

bool CallDAG::CallDAGCreator::assignIndicesInternal(CreatorFunctionData* function)
{
    ASSERT(function);

    if (function->indexAssigned) {
        return true;
    }

    if (function->visiting) {
        return false;
    }
    function->visiting = true;

    for (auto& callee : function->callees) {
        if (!assignIndicesInternal(callee)) {
            return false;
        }
    }
    function->index = currentIndex ++;
    function->indexAssigned = true;

    function->visiting = false;
    return true;
}

void CallDAG::CallDAGCreator::fillDataStructures(std::vector<Record>& records,
                                                 std::unordered_map<TString, int>& nameToIndex)
{
    ASSERT(records.empty());
    ASSERT(nameToIndex.empty());

    records.resize(functions.size());

    for (auto& it : functions) {
        CreatorFunctionData& data = it.second;
        Record& record = records[data.index];

        record.name = data.name;
        record.node = data.node;

        for (auto& callee : data.callees) {
            record.callees.push_back(callee->index);
        }

        nameToIndex.emplace(data.name, data.index);
    }
}





// TODO doesn't handle builtins
DetectCallDepth::ErrorCode myDetectCallDepth(TIntermNode* root, bool limitCallStackDepth, int maxCallStackDepth, DetectCallDepth::ErrorCode expected) {
    CallDAG dag;
    if (!dag.create(root)) {
        return DetectCallDepth::kErrorRecursion;
    }

    std::vector<int> depths(dag.size());
    bool foundMain = false;

    for (int i = 0; i < dag.size(); i++) {
        int depth = 0;
        auto& record = dag.getRecord(i);

        for (auto& calleeIndex :record.callees) {
            depth = std::max(depths[i], depths[calleeIndex + 1]);
        }

        if (limitCallStackDepth && depth >= maxCallStackDepth) {
            return DetectCallDepth::kErrorMaxDepthExceeded;
        }

        if (record.name == "main(") {
            foundMain = true;
        }
    }

    if (!foundMain) {
        return DetectCallDepth::kErrorMissingMain;
    }

    return DetectCallDepth::kErrorNone;
}






DetectCallDepth::FunctionNode::FunctionNode(const TString& fname)
    : name(fname),
      visit(PreVisit)
{
}

const TString& DetectCallDepth::FunctionNode::getName() const
{
    return name;
}

void DetectCallDepth::FunctionNode::addCallee(
    DetectCallDepth::FunctionNode* callee)
{
    for (size_t i = 0; i < callees.size(); ++i) {
        if (callees[i] == callee)
            return;
    }
    callees.push_back(callee);
}

int DetectCallDepth::FunctionNode::detectCallDepth(DetectCallDepth* detectCallDepth, int depth)
{
    ASSERT(visit == PreVisit);
    ASSERT(detectCallDepth);

    int maxDepth = depth;
    visit = InVisit;
    for (size_t i = 0; i < callees.size(); ++i) {
        switch (callees[i]->visit) {
            case InVisit:
                // cycle detected, i.e., recursion detected.
                return kInfiniteCallDepth;
            case PostVisit:
                break;
            case PreVisit: {
                // Check before we recurse so we don't go too depth
                if (detectCallDepth->checkExceedsMaxDepth(depth))
                    return depth;
                int callDepth = callees[i]->detectCallDepth(detectCallDepth, depth + 1);
                // Check after we recurse so we can exit immediately and provide info.
                if (detectCallDepth->checkExceedsMaxDepth(callDepth)) {
                    detectCallDepth->getInfoSink().info << "<-" << callees[i]->getName();
                    return callDepth;
                }
                maxDepth = std::max(callDepth, maxDepth);
                break;
            }
            default:
                UNREACHABLE();
                break;
        }
    }
    visit = PostVisit;
    return maxDepth;
}

void DetectCallDepth::FunctionNode::reset()
{
    visit = PreVisit;
}

DetectCallDepth::DetectCallDepth(TInfoSink& infoSink, bool limitCallStackDepth, int maxCallStackDepth)
    : TIntermTraverser(true, false, true, false),
      currentFunction(NULL),
      infoSink(infoSink),
      maxDepth(limitCallStackDepth ? maxCallStackDepth : FunctionNode::kInfiniteCallDepth)
{
}

DetectCallDepth::~DetectCallDepth()
{
    for (size_t i = 0; i < functions.size(); ++i)
        delete functions[i];
}

bool DetectCallDepth::visitAggregate(Visit visit, TIntermAggregate* node)
{
    switch (node->getOp())
    {
        case EOpPrototype:
            // Function declaration.
            // Don't add FunctionNode here because node->getName() is the
            // unmangled function name.
            break;
        case EOpFunction: {
            // Function definition.
            if (visit == PreVisit) {
                currentFunction = findFunctionByName(node->getName());
                if (currentFunction == NULL) {
                    currentFunction = new FunctionNode(node->getName());
                    functions.push_back(currentFunction);
                }
            } else if (visit == PostVisit) {
                currentFunction = NULL;
            }
            break;
        }
        case EOpFunctionCall: {
            // Function call.
            if (visit == PreVisit) {
                FunctionNode* func = findFunctionByName(node->getName());
                if (func == NULL) {
                    func = new FunctionNode(node->getName());
                    functions.push_back(func);
                }
                if (currentFunction)
                    currentFunction->addCallee(func);
            }
            break;
        }
        default:
            break;
    }
    return true;
}

bool DetectCallDepth::checkExceedsMaxDepth(int depth)
{
    return depth >= maxDepth;
}

void DetectCallDepth::resetFunctionNodes()
{
    for (size_t i = 0; i < functions.size(); ++i) {
        functions[i]->reset();
    }
}

DetectCallDepth::ErrorCode DetectCallDepth::detectCallDepthForFunction(FunctionNode* func)
{
    currentFunction = NULL;
    resetFunctionNodes();

    int maxCallDepth = func->detectCallDepth(this, 1);

    if (maxCallDepth == FunctionNode::kInfiniteCallDepth)
        return kErrorRecursion;

    if (maxCallDepth >= maxDepth)
        return kErrorMaxDepthExceeded;

    return kErrorNone;
}

DetectCallDepth::ErrorCode DetectCallDepth::detectCallDepth()
{
    if (maxDepth != FunctionNode::kInfiniteCallDepth) {
        // Check all functions because the driver may fail on them
        // TODO: Before detectingRecursion, strip unused functions.
        for (size_t i = 0; i < functions.size(); ++i) {
            ErrorCode error = detectCallDepthForFunction(functions[i]);
            if (error != kErrorNone)
                return error;
        }
    } else {
        FunctionNode* main = findFunctionByName("main(");
        if (main == NULL)
            return kErrorMissingMain;

        return detectCallDepthForFunction(main);
    }

    return kErrorNone;
}

DetectCallDepth::FunctionNode* DetectCallDepth::findFunctionByName(
    const TString& name)
{
    for (size_t i = 0; i < functions.size(); ++i) {
        if (functions[i]->getName() == name)
            return functions[i];
    }
    return NULL;
}

