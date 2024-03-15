//
// Copyright 2002 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//
#include "compiler/translator/tree_ops/SeparateDeclarations.h"

#include <unordered_map>
#include "compiler/translator/SymbolTable.h"

#include "compiler/translator/ImmutableStringBuilder.h"
#include "compiler/translator/IntermRebuild.h"
#include "compiler/translator/util.h"

namespace sh
{
namespace
{

class Separator final : private TIntermRebuild
{
  public:
    Separator(TCompiler &compiler, bool separateAnonymousInterfaceStructs)
        : TIntermRebuild(compiler, true, true),
          mSeparateAnonymousInterfaceStruct(separateAnonymousInterfaceStructs)
    {}
    using TIntermRebuild::rebuildRoot;

  private:
    bool shouldSeparateAnonymousStructAsNewStruct(const TType &type)
    {
        if (!mSeparateAnonymousInterfaceStruct)
        {
            return false;
        }
        // This is a workaround where the Separator guarantees that all declarations
        // are separated, but GLSL output vertex and fragment shaders needs types that match.
        return IsShaderIn(type.getQualifier()) || IsShaderOut(type.getQualifier());
    }

    void recordModifiedStructVariables(TIntermDeclaration &node)
    {
        TIntermSequence &sequence = *node.getSequence();
        if (sequence.size() <= 1)
        {
            return;
        }
        TIntermTyped *declarator    = sequence.at(0)->getAsTyped();
        const TType &declaratorType = declarator->getType();
        const TStructure *structure = declaratorType.getStruct();
        // Rewrite variable declarations that specify structs AND multiple variables at the same
        // time. Only one variable can specify the struct and rest must be rewritten with new
        // type.
        if (!structure || !declaratorType.isStructSpecifier())
        {
            return;
        }
        if (structure->symbolType() != SymbolType::Empty)
        {
            // Struct specifier changes for all variables except the first one.
            for (uint32_t index = 1; index < sequence.size(); ++index)
            {
                Declaration decl     = ViewDeclaration(node, index);
                const TVariable &var = decl.symbol.variable();
                const TType &varType = var.getType();
                TType *newType       = new TType(structure, false);
                newType->setQualifier(varType.getQualifier());
                newType->makeArrays(varType.getArraySizes());
                TVariable *newVar =
                    new TVariable(&mSymbolTable, var.name(), newType, var.symbolType());
                mStructVariables.insert(std::make_pair(&var, newVar));
            }
            return;
        }

        // Struct specifier changes for all variables.
        for (uint32_t index = 0; index < sequence.size(); ++index)
        {
            Declaration decl              = ViewDeclaration(node, index);
            const TVariable &var          = decl.symbol.variable();
            const TType &varType          = var.getType();
            const bool shouldSeparate     = shouldSeparateAnonymousStructAsNewStruct(varType);
            const bool newTypeIsSpecifier = index == 0 || shouldSeparate;
            if (newTypeIsSpecifier)
            {
                ImmutableString newName = kEmptyImmutableString;
                if (shouldSeparate)
                {
                    ImmutableStringBuilder str(2 + var.name().length());
                    str << "_i" << var.name();
                    newName = str;
                }
                TStructure *newStructure = new TStructure(
                    &mSymbolTable, newName, &structure->fields(), SymbolType::AngleInternal);
                newStructure->setAtGlobalScope(structure->atGlobalScope());
                structure = newStructure;
            }
            TType *newType = new TType(structure, newTypeIsSpecifier);
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
        if (sequence.size() <= 1)
        {
            return node;
        }
        std::vector<TIntermNode *> replacements;
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

    const bool mSeparateAnonymousInterfaceStruct;
    // Old struct variable to new struct variable mapping.
    std::unordered_map<const TVariable *, TVariable *> mStructVariables;
};

}  // namespace

bool SeparateDeclarations(TCompiler &compiler,
                          TIntermBlock &root,
                          bool separateAnonymousInterfaceStructs)
{
    Separator separator(compiler, separateAnonymousInterfaceStructs);
    return separator.rebuildRoot(root);
}

}  // namespace sh
