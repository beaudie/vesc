//
// Copyright 2024 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//
// SeparateStructFromFunctionDeclarations: Separate struct declarations from function declaration
// return type.
//

#include "compiler/translator/tree_ops/SeparateStructFromFunctionDeclarations.h"
#include "compiler/translator/IntermRebuild.h"
#include "compiler/translator/SymbolTable.h"

namespace sh
{
namespace
{
class SeparateStructFromFunctionDeclarationsTraverser : public TIntermRebuild
{
  public:
    explicit SeparateStructFromFunctionDeclarationsTraverser(TCompiler &compiler)
        : TIntermRebuild(compiler, true, false)
    {}

    PreResult visitFunctionPrototypePre(TIntermFunctionPrototype &node) override
    {
        const TFunction *function = node.getFunction();
        if (mFunctionsToReplace.count(function) > 0)
        {
            TIntermFunctionPrototype *newFuncProto =
                new TIntermFunctionPrototype(mFunctionsToReplace[function]);
            return newFuncProto;
        }
        else if (node.getType().isStructSpecifier())
        {
            const TType &oldType        = node.getType();
            const TStructure *structure = oldType.getStruct();
            // Name unnamed inline structs
            if (structure->symbolType() == SymbolType::Empty)
            {
                structure = new TStructure(&mSymbolTable, kEmptyImmutableString,
                                           &structure->fields(), SymbolType::AngleInternal);
            }

            TVariable *structVar = new TVariable(&mSymbolTable, ImmutableString(""),
                                                 new TType(structure, true), SymbolType::Empty);

            TType *returnType = new TType(structure, false);
            if (oldType.isArray())
            {
                returnType->makeArrays(oldType.getArraySizes());
            }
            returnType->setQualifier(oldType.getQualifier());

            const TFunction *oldFunc = function;
            ASSERT(oldFunc->symbolType() == SymbolType::UserDefined);

            const TFunction *newFunc     = cloneFunctionAndChangeReturnType(oldFunc, returnType);
            mFunctionsToReplace[oldFunc] = newFunc;

            return PreResult::Multi(
                {new TIntermDeclaration({structVar}), new TIntermFunctionPrototype(newFunc)});
        }

        return node;
    }

    PreResult visitAggregatePre(TIntermAggregate &node) override
    {
        const TFunction *function = node.getFunction();
        if (mFunctionsToReplace.count(function) > 0)
        {
            TIntermAggregate *replacementNode = TIntermAggregate::CreateFunctionCall(
                *mFunctionsToReplace[function], node.getSequence());

            return PreResult(replacementNode, VisitBits::Children);
        }

        return node;
    }

  private:
    const TFunction *cloneFunctionAndChangeReturnType(const TFunction *oldFunc,
                                                      const TType *newReturnType)

    {
        ASSERT(oldFunc->symbolType() == SymbolType::UserDefined);

        TFunction *newFunc = new TFunction(&mSymbolTable, oldFunc->name(), oldFunc->symbolType(),
                                           newReturnType, oldFunc->isKnownToNotHaveSideEffects());

        if (oldFunc->isDefined())
        {
            newFunc->setDefined();
        }

        if (oldFunc->hasPrototypeDeclaration())
        {
            newFunc->setHasPrototypeDeclaration();
        }

        const size_t paramCount = oldFunc->getParamCount();
        for (size_t i = 0; i < paramCount; ++i)
        {
            const TVariable *var = oldFunc->getParam(i);
            newFunc->addParameter(var);
        }

        return newFunc;
    }

    using FunctionReplacement = angle::HashMap<const TFunction *, const TFunction *>;
    FunctionReplacement mFunctionsToReplace;
};

}  // anonymous namespace

bool SeparateStructFromFunctionDeclarations(TCompiler &compiler, TIntermBlock &root)
{
    SeparateStructFromFunctionDeclarationsTraverser separateStructDecls(compiler);
    return separateStructDecls.rebuildRoot(root);
}
}  // namespace sh
