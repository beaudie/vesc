//
// Copyright 2002 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//
#include "compiler/translator/tree_ops/SeparateDeclarations.h"

#include <unordered_map>
#include "compiler/translator/SymbolTable.h"

#include "compiler/translator/IntermRebuild.h"
#include "compiler/translator/util.h"

namespace sh
{
namespace
{

class Separator final : private TIntermRebuild
{
  public:
    Separator(TCompiler &compiler) : TIntermRebuild(compiler, true, true) {}
    using TIntermRebuild::rebuildRoot;

  private:
    void recordModifiedStructVariables(TIntermDeclaration &node)
    {
        ASSERT(!mStructure);  // No nested struct declarations.
        TIntermSequence &sequence = *node.getSequence();
        if (sequence.size() == 0)
        {
            return;
        }
        {
            TIntermTyped *declarator    = sequence.at(0)->getAsTyped();
            const TType &declaratorType = declarator->getType();
            const TStructure *structure = declaratorType.getStruct();
            // Rewrite variable declarations that specify structs.
            if (!structure || !declaratorType.isStructSpecifier())
            {
                return;
            }
            // When declaring just the struct, empty symbol type is used.
            Declaration decl = ViewDeclaration(node);
            if (decl.symbol.variable().symbolType() == SymbolType::Empty)
            {
                return;
            }
            if (structure->symbolType() == SymbolType::Empty)
            {
                TStructure *newStructure =
                    new TStructure(&mSymbolTable, kEmptyImmutableString, &structure->fields(),
                                   SymbolType::AngleInternal);
                newStructure->setAtGlobalScope(structure->atGlobalScope());
                structure = newStructure;
            }
            mStructure = structure;
        }
        for (uint32_t index = 0; index < sequence.size(); ++index)
        {
            Declaration decl     = ViewDeclaration(node, index);
            const TVariable &var = decl.symbol.variable();
            const TType &varType = var.getType();
            TType *newType       = new TType(mStructure, false);
            newType->setQualifier(varType.getQualifier());
            newType->makeArrays(varType.getArraySizes());
            TVariable *newVar = new TVariable(&mSymbolTable, var.name(), newType, var.symbolType());
            mStructVariables.insert(std::make_pair(&var, newVar));
        }
    }

    PreResult visitDeclarationPre(TIntermDeclaration &node) override
    {
        recordModifiedStructVariables(node);
        return node;
    }

    PostResult visitDeclarationPost(TIntermDeclaration &node) override
    {
        TIntermSequence &sequence = *node.getSequence();
        if (mStructure == nullptr && sequence.size() <= 1)
        {
            return node;
        }
        std::vector<TIntermNode *> replacements;
        if (mStructure)
        {
            TIntermDeclaration *replacement = new TIntermDeclaration;
            replacement->setLine(node.getLine());
            TType *namedType = new TType(mStructure, true);
            namedType->setQualifier(EvqGlobal);
            TVariable *structVariable =
                new TVariable(&mSymbolTable, kEmptyImmutableString, namedType, SymbolType::Empty);
            replacement->appendDeclarator(new TIntermSymbol(structVariable));
            replacements.push_back(replacement);
            mStructure = nullptr;
        }
        for (uint32_t index = 0; index < sequence.size(); ++index)
        {
            TIntermDeclaration *replacement = new TIntermDeclaration;
            TIntermTyped *declarator        = sequence.at(index)->getAsTyped();
            replacement->appendDeclarator(declarator);
            replacement->setLine(declarator->getLine());
            replacements.push_back(replacement);
        }
        return PostResult::Multi(std::move(replacements));
    }

    PreResult visitSymbolPre(TIntermSymbol &symbolNode) override
    {
        auto it = mStructVariables.find(&symbolNode.variable());
        if (it == mStructVariables.end())
        {
            return symbolNode;
        }
        return *new TIntermSymbol(it->second);
    }

    const TStructure *mStructure = nullptr;
    // Old struct variable to new struct variable mapping.
    std::unordered_map<const TVariable *, TVariable *> mStructVariables;
};

}  // namespace

bool SeparateDeclarations(TCompiler &compiler, TIntermBlock &root)
{
    Separator separator(compiler);
    return separator.rebuildRoot(root);
}

}  // namespace sh
