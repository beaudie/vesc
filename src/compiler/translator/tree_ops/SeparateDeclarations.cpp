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

class Separator final : public TIntermRebuild
{
  public:
    Separator(TCompiler &compiler) : TIntermRebuild(compiler, true, true) {}

    PreResult visitDeclarationPre(TIntermDeclaration &node) override
    {
        TIntermSequence &sequence = *node.getSequence();
        if (sequence.size() > 1)
        {
            TIntermTyped *declarator    = sequence.at(0)->getAsTyped();
            const TType &declaratorType = declarator->getType();
            const TStructure *structure = declaratorType.getStruct();
            // Rewrite variable declarations that specify structs AND multiple variables at the same
            // time. Only one variable can specify the struct and rest must be rewritten with new
            // type.
            if (structure && declaratorType.isStructSpecifier())
            {
                // Struct specifier changes for all variables except the first one.
                uint32_t index = 1;
                if (structure->symbolType() == SymbolType::Empty)
                {
                    ASSERT(!mNewStructure);  // No nested struct declarations.
                    mNewStructure = new TStructure(&mSymbolTable, kEmptyImmutableString,
                                                   &structure->fields(), SymbolType::AngleInternal);
                    mNewStructure->setAtGlobalScope(structure->atGlobalScope());
                    structure = mNewStructure;
                    // Struct type changes, so all variables need rewriting.
                    index = 0;
                }
                for (; index < sequence.size(); ++index)
                {
                    Declaration decl              = ViewDeclaration(node, index);
                    const TVariable &var          = decl.symbol.variable();
                    const TType &varType          = var.getType();
                    const bool newTypeIsSpecifier = index == 0;
                    TType *newType                = new TType(structure, newTypeIsSpecifier);
                    newType->setQualifier(varType.getQualifier());
                    newType->makeArrays(varType.getArraySizes());
                    TVariable *newVar = new TVariable(&mSymbolTable, decl.symbol.getName(), newType,
                                                      var.symbolType());
                    mStructVariables.insert(std::make_pair(&var, newVar));
                }
            }
        }
        return node;
    }

    PostResult visitDeclarationPost(TIntermDeclaration &node) override
    {
        TIntermSequence &sequence = *node.getSequence();
        if (sequence.size() <= 1)
        {
            return node;
        }
        std::vector<TIntermNode *> replacements;
        uint32_t index = 0;
        if (mNewStructure)
        {
            TType *namedType = new TType(mNewStructure, true);
            namedType->setQualifier(EvqGlobal);
            TIntermDeclaration *replacement = new TIntermDeclaration;
            replacement->appendDeclarator(sequence.at(0)->getAsTyped());
            replacement->setLine(node.getLine());
            replacements.push_back(replacement);
            mNewStructure = nullptr;
            index         = 1;
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

    TStructure *mNewStructure = nullptr;
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
