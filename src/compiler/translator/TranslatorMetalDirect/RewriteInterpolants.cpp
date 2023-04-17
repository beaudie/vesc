//
// Copyright 2023 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//

#include "compiler/translator/TranslatorMetalDirect/RewriteInterpolants.h"

#include "compiler/translator/StaticType.h"
#include "compiler/translator/TranslatorMetalDirect/AstHelpers.h"
#include "compiler/translator/tree_util/BuiltIn.h"
#include "compiler/translator/tree_util/IntermNode_util.h"
#include "compiler/translator/tree_util/ReplaceVariable.h"

namespace sh
{

namespace
{

class FindInterpolantsTraverser : public TIntermTraverser
{
  public:
    FindInterpolantsTraverser()
        : TIntermTraverser(true, false, false), mUsesSampleInterpolation(false)
    {}

    bool visitDeclaration(Visit, TIntermDeclaration *node) override
    {
        const TIntermSequence &sequence = *(node->getSequence());
        ASSERT(!sequence.empty());

        const TIntermTyped &typedNode = *(sequence.front()->getAsTyped());
        TQualifier qualifier          = typedNode.getQualifier();
        if (qualifier == EvqSampleIn || qualifier == EvqNoPerspectiveSampleIn)
        {
            mUsesSampleInterpolation = true;
        }

        return true;
    }

    bool visitAggregate(Visit visit, TIntermAggregate *node) override
    {
        if (!BuiltInGroup::IsInterpolationFS(node->getOp()))
        {
            return true;
        }

        TIntermNode *operand = node->getSequence()->at(0);
        ASSERT(operand);

        // For all of the interpolation functions, <interpolant> must be an input
        // variable or an element of an input variable declared as an array.
        const TIntermSymbol *symbolNode = operand->getAsSymbolNode();
        if (!symbolNode)
        {
            const TIntermBinary *binaryNode = operand->getAsBinaryNode();
            if (binaryNode &&
                (binaryNode->getOp() == EOpIndexDirect || binaryNode->getOp() == EOpIndexIndirect))
            {
                symbolNode = binaryNode->getLeft()->getAsSymbolNode();
            }
        }
        ASSERT(symbolNode);

        // If <interpolant> is declared with a "flat" qualifier, the interpolated
        // value will have the same value everywhere for a single primitive, so
        // the location used for the interpolation has no effect and the functions
        // just return that same value.
        const TVariable *variable = &symbolNode->variable();
        if (variable->getType().getQualifier() != EvqFlatIn)
        {
            mInterpolants.insert(variable);
        }

        return true;
    }

    bool usesSampleInterpolation() const { return mUsesSampleInterpolation; }

    const std::unordered_set<const TVariable *> &getInterpolants() const { return mInterpolants; }

  private:
    bool mUsesSampleInterpolation;
    std::unordered_set<const TVariable *> mInterpolants;
};

class WrapInterpolantsTraverser : public TIntermTraverser
{
  public:
    WrapInterpolantsTraverser(TSymbolTable *symbolTable)
        : TIntermTraverser(true, false, false, symbolTable), mUsesSampleInterpolant(false)
    {}

    void visitSymbol(TIntermSymbol *node) override
    {
        const TType &type = node->variable().getType();
        if (!type.isInterpolant())
        {
            return;
        }

        TIntermNode *ancestor = getAncestorNode(0);
        ASSERT(ancestor);

        // Skip symbols inside declarations
        if (ancestor->getAsDeclarationNode())
        {
            return;
        }

        auto checkSkip = [](TIntermNode *node, TIntermNode *parentNode) {
            if (TIntermAggregate *callNode = parentNode->getAsAggregate())
            {
                if (BuiltInGroup::IsInterpolationFS(callNode->getOp()) &&
                    callNode->getSequence()->at(0) == node)
                {
                    return true;
                }
            }
            return false;
        };

        // Skip symbols used as the first operand of interpolation functions
        if (checkSkip(node, ancestor))
        {
            return;
        }

        TIntermNode *original = node;
        if (TIntermBinary *binaryNode = ancestor->getAsBinaryNode())
        {
            if (binaryNode->getOp() == EOpIndexDirect || binaryNode->getOp() == EOpIndexIndirect)
            {
                ancestor = getAncestorNode(1);
                ASSERT(ancestor);

                // Skip array elements used as the first operand of interpolation functions
                if (checkSkip(binaryNode, ancestor))
                {
                    return;
                }
                original = binaryNode;
            }
        }

        const char *functionName   = nullptr;
        TIntermSequence *arguments = new TIntermSequence{original};
        switch (type.getQualifier())
        {
            case EvqFragmentIn:
            case EvqSmoothIn:
            case EvqNoPerspectiveIn:
                functionName = "interpolateAtCenter";
                break;
            case EvqCentroidIn:
            case EvqNoPerspectiveCentroidIn:
                functionName = "interpolateAtCentroid";
                break;
            case EvqSampleIn:
            case EvqNoPerspectiveSampleIn:
                functionName = "interpolateAtSample";
                arguments->push_back(new TIntermSymbol(BuiltInVariable::gl_SampleID()));
                mUsesSampleInterpolant = true;
                break;
            default:
                UNREACHABLE();
                break;
        }
        TIntermTyped *replacement = CreateBuiltInFunctionCallNode(
            functionName, arguments, *mSymbolTable, kESSLInternalBackendBuiltIns);

        queueReplacementWithParent(ancestor, original, replacement, OriginalNode::BECOMES_CHILD);
    }

    bool usesSampleInterpolant() const { return mUsesSampleInterpolant; }

  private:
    bool mUsesSampleInterpolant;
};

}  // anonymous namespace

[[nodiscard]] bool RewriteInterpolants(TCompiler &compiler,
                                       TIntermBlock &root,
                                       TSymbolTable &symbolTable,
                                       bool *outUsesSampleInterpolation,
                                       bool *outUsesSampleInterpolant)
{
    // Find all fragment inputs used with interpolation functions.
    FindInterpolantsTraverser findInterpolantsTraverser;
    root.traverse(&findInterpolantsTraverser);
    if (!findInterpolantsTraverser.updateTree(&compiler, &root))
    {
        return false;
    }
    *outUsesSampleInterpolation = findInterpolantsTraverser.usesSampleInterpolation();

    // Skip further operations when interpolation functions are not used.
    if (findInterpolantsTraverser.getInterpolants().empty())
    {
        return true;
    }

    // Adjust variable types as per MSL requirements
    //
    // * Inputs with omitted and smooth interpolation qualifiers will be written as
    //       metal::interpolant<T, metal::interpolation::perspective>
    //
    // * Inputs with noperspective interpolation qualifiers will be written as
    //       metal::interpolant<T, metal::interpolation::no_perspective>
    for (const TVariable *var : findInterpolantsTraverser.getInterpolants())
    {
        TType *replacementType = new TType(var->getType());
        replacementType->setInterpolant(true);
        TVariable *replacement =
            new TVariable(&symbolTable, var->name(), replacementType, var->symbolType());
        if (!ReplaceVariable(&compiler, &root, var, replacement))
        {
            return false;
        }
    }

    // Wrap direct usages of interpolants with explicit interpolation
    // functions depending on their auxiliary qualifiers
    //            in vec4 interpolant -> ANGLE_interpolateAtCenter(interpolant)
    //   centroid in vec4 interpolant -> ANGLE_interpolateAtCentroid(interpolant)
    //     sample in vec4 interpolant -> ANGLE_interpolateAtSample(interpolant, gl_SampleID)
    WrapInterpolantsTraverser wrapInterpolantsTraverser(&symbolTable);
    root.traverse(&wrapInterpolantsTraverser);
    if (!wrapInterpolantsTraverser.updateTree(&compiler, &root))
    {
        return false;
    }
    *outUsesSampleInterpolant = wrapInterpolantsTraverser.usesSampleInterpolant();

    return true;
}

}  // namespace sh
