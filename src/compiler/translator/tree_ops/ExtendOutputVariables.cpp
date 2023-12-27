//
// Copyright 2002 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//

#include "compiler/translator/tree_ops/ExtendOutputVariables.h"
#include "compiler/translator/tree_util/ReplaceVariable.h"

#include "angle_gl.h"
#include "common/debug.h"
#include "compiler/translator/Compiler.h"
#include "compiler/translator/StaticType.h"
#include "compiler/translator/SymbolTable.h"
#include "compiler/translator/tree_util/FindMain.h"
#include "compiler/translator/tree_util/FindSymbolNode.h"
#include "compiler/translator/tree_util/IntermNode_util.h"
#include "compiler/translator/tree_util/IntermTraverse.h"
#include "compiler/translator/util.h"

namespace sh
{

namespace
{

class ExtendOutputTraverser : public TIntermTraverser
{
  public:
    ExtendOutputTraverser(TSymbolTable &symbolTable)
        : TIntermTraverser(true, false, false, &symbolTable)
    {}

  protected:
    bool visitDeclaration(Visit visit, TIntermDeclaration *node) override
    {
        ASSERT(node->getChildCount() == 1);
        TIntermSymbol *symbol = node->getChildNode(0)->getAsSymbolNode();
        if (symbol)
        {
            const TVariable &var       = symbol->variable();
            const TType &type          = var.getType();
            const TQualifier qualifier = type.getQualifier();

            if (qualifier == TQualifier::EvqFragmentOut && type.getBasicType() == EbtFloat &&
                type.getNominalSize() < 4)
            {
                TType *newType = new TType(type);
                newType->setPrimarySize(4);
                const ImmutableString oldName = var.name();
                std::string newName(oldName.data(), oldName.length());
                newName += "_ANGLE_vec4";
                TVariable *newVar = new TVariable(mSymbolTable, ImmutableString(newName), newType,
                                                  SymbolType::UserDefined);
                mSymbolTable->declare(newVar);
                TIntermSymbol *newSymbol = new TIntermSymbol(newVar);
                mOutputsToExtend[&var]   = newSymbol;
                node->replaceChildNode(symbol, newSymbol);
            }
        }
        return false;
    }

    void visitSymbol(TIntermSymbol *node) override
    {
        if (mOutputsToExtend.contains(&node->variable()))
        {
            extendUseToVec4(node);
        }
    }

    bool visitFunctionDefinition(Visit visit, TIntermFunctionDefinition *node) override
    {
        mCurrentFunction = (visit == Visit::PreVisit) ? node : nullptr;
        return true;
    }

  private:
    void extendUseToVec4(TIntermSymbol *node)
    {
        uint32_t ancestorIndex    = 0;
        TIntermTyped *toReplace   = node;
        TIntermTyped *replacement = mOutputsToExtend[&node->variable()]->deepCopy();
        const TType &type         = node->getType();
        // const TType &newType = replacement->getType();

        size_t expectedIndices = type.isArray() ? type.getNumArraySizes() : 0;
        ASSERT(expectedIndices < 2);

        for (size_t idx = 0; idx < expectedIndices; idx++)
        {
            TIntermNode *ancestor = getAncestorNode(ancestorIndex);
            ASSERT(ancestor != nullptr);

            TIntermBinary *asBinary = ancestor->getAsBinaryNode();
            ASSERT(asBinary &&
                   (asBinary->getOp() == EOpIndexDirect || asBinary->getOp() == EOpIndexIndirect));
            if (asBinary == nullptr ||
                (asBinary->getOp() != EOpIndexDirect && asBinary->getOp() != EOpIndexIndirect))
            {
                toReplace = nullptr;
                break;
            }

            replacement = new TIntermBinary(asBinary->getOp(), replacement, asBinary->getRight());
            toReplace   = asBinary;

            ++ancestorIndex;
        }

        TIntermNode *ancestor = getAncestorNode(ancestorIndex);
        ASSERT(ancestor != nullptr);

        TIntermSwizzle *asSwizzle = ancestor->getAsSwizzleNode();
        TIntermBinary *asBinary   = ancestor->getAsBinaryNode();
        if (asSwizzle)
        {
            // Update swizzle expression because operand type has changed
            replacement = new TIntermSwizzle(replacement, asSwizzle->getSwizzleOffsets());
            toReplace   = ancestor->getAsTyped();
            ++ancestorIndex;
        }
        else if (asBinary &&
                 (asBinary->getOp() == EOpIndexDirect || asBinary->getOp() == EOpIndexIndirect))
        {
            // Update index expression because left operand type has changed
            // out[0] = redValue;
            // when out was extended from vec2 or vec3 to vec4
            replacement = new TIntermBinary(asBinary->getOp(), replacement, asBinary->getRight());
            toReplace   = ancestor->getAsTyped();
            ++ancestorIndex;
        }
        else
        {
            // Wrap the expression into a swizzle:
            // out = value ->
            //     out.r = redValue if value is just a float, or
            //     out.rg = redGreenValue if value is a vec2
            TVector<int> indices;
            for (int i = 0; i < type.getNominalSize(); i++)
            {
                indices.push_back(i);
            }
            replacement = new TIntermSwizzle(replacement, indices);
        }

        queueReplacementWithParent(getAncestorNode(ancestorIndex), toReplace, replacement,
                                   OriginalNode::IS_DROPPED);
    }
    VariableReplacementMap mOutputsToExtend;
    TIntermFunctionDefinition *mCurrentFunction = nullptr;
};
}  // namespace

bool ExtendOutputVariables(TCompiler *compiler, TIntermBlock *root)
{
    ExtendOutputTraverser traverser(compiler->getSymbolTable());
    root->traverse(static_cast<TIntermTraverser *>(&traverser));
    return traverser.updateTree(compiler, root);
}

}  // namespace sh
