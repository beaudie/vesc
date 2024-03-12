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
    Separator(TCompiler &compiler, bool separateStructs)
        : TIntermRebuild(compiler, true, true), mSeparateStructs(separateStructs)
    {}
    using TIntermRebuild::rebuildRoot;

  private:
    void recordModifiedStructVariables(TIntermDeclaration &node)
    {
        ASSERT(!mNewStructure);  // No nested struct declarations.
        TIntermSequence &sequence = *node.getSequence();
        if (sequence.size() == 0)
        {
            return;
        }
        if (sequence.size() == 1 && !mSeparateStructs)
        {
            return;
        }
        TIntermTyped *declarator    = sequence.at(0)->getAsTyped();
        const TType &declaratorType = declarator->getType();
        const TStructure *structure = declaratorType.getStruct();
        // Rewrite variable declarations that specify structs.
        if (!structure || !declaratorType.isStructSpecifier())
        {
            return;
        }
        // When declaring just the struct, empty symbol type is used.
        {
            Declaration decl = ViewDeclaration(node);
            if (decl.symbol.variable().symbolType() == SymbolType::Empty)
            {
                return;
            }
        }
        // Struct specifier changes for all variables except the first one.
        uint32_t index = 1;
        if (structure->symbolType() == SymbolType::Empty)
        {

            TStructure *newStructure =
                new TStructure(&mSymbolTable, kEmptyImmutableString, &structure->fields(),
                               SymbolType::AngleInternal);
            newStructure->setAtGlobalScope(structure->atGlobalScope());
            structure     = newStructure;
            mNewStructure = newStructure;
            // Adding name causes the struct type change, so all variables need rewriting.
            index = 0;
        }
        else if (mSeparateStructs)
        {
            // Struct specifier changes for all declarator variables.
            mNewStructure = structure;
            // Struct type changes, so all variables need rewriting.
            index = 0;
        }
        for (; index < sequence.size(); ++index)
        {
            Declaration decl              = ViewDeclaration(node, index);
            const TVariable &var          = decl.symbol.variable();
            const TType &varType          = var.getType();
            const bool newTypeIsSpecifier = index == 0 && !mSeparateStructs;
            TType *newType                = new TType(structure, newTypeIsSpecifier);
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
        if (mNewStructure == nullptr && sequence.size() <= 1)
        {
            return node;
        }
        std::vector<TIntermNode *> replacements;
        uint32_t index = 0;
        if (mNewStructure)
        {
            TIntermDeclaration *replacement = new TIntermDeclaration;
            replacement->setLine(node.getLine());
            if (mSeparateStructs)
            {
                TType *namedType = new TType(mNewStructure, true);
                namedType->setQualifier(EvqGlobal);
                TVariable *structVariable = new TVariable(&mSymbolTable, kEmptyImmutableString,
                                                          namedType, SymbolType::Empty);
                replacement->appendDeclarator(new TIntermSymbol(structVariable));
            }
            else
            {
                replacement->appendDeclarator(sequence.at(0)->getAsTyped());
                index = 1;
            }
            replacements.push_back(replacement);
            mNewStructure = nullptr;
        }
        for (; index < sequence.size(); ++index)
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

    const TStructure *mNewStructure = nullptr;
    // Old struct variable to new struct variable mapping.
    std::unordered_map<const TVariable *, TVariable *> mStructVariables;
    // True if structs should not have declarators.
    const bool mSeparateStructs;
};

}  // namespace

bool SeparateDeclarations(TCompiler &compiler, TIntermBlock &root)
{
    Separator separator(compiler, false);
    return separator.rebuildRoot(root);
}

bool SeparateDeclarationsAndStructs(TCompiler &compiler, TIntermBlock &root)
{
    Separator separator(compiler, true);
    return separator.rebuildRoot(root);
}

}  // namespace sh
