//
// Copyright 2018 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//
// RewriteStructSamplers: Extract structs from samplers.
//

#include "compiler/translator/tree_ops/RewriteStructSamplers.h"

#include "compiler/translator/ImmutableStringBuilder.h"
#include "compiler/translator/SymbolTable.h"
#include "compiler/translator/tree_util/IntermTraverse.h"

namespace sh
{
namespace
{
class Traverser final : public TIntermTraverser
{
  public:
    explicit Traverser(TSymbolTable *symbolTable)
        : TIntermTraverser(true, false, true, symbolTable), mRemovedUniformsCount(0)
    {
        mSymbolTable->push();
    }

    ~Traverser() override { mSymbolTable->pop(); }

    int removedUniformsCount() const { return mRemovedUniformsCount; }

    bool visitDeclaration(Visit visit, TIntermDeclaration *decl) override
    {
        if (visit != PreVisit)
            return true;

        if (!mInGlobalScope)
        {
            return true;
        }

        const TIntermSequence &sequence = *(decl->getSequence());
        TIntermTyped *declarator        = sequence.front()->getAsTyped();
        const TType &type               = declarator->getType();

        if (type.isStructureContainingSamplers())
        {
            TIntermSequence *newSequence = new TIntermSequence;

            if (type.isStructSpecifier())
            {
                stripStructSpecifierSamplers(type.getStruct(), newSequence);
            }
            else
            {
                TIntermSymbol *asSymbol = declarator->getAsSymbolNode();
                ASSERT(asSymbol);
                const TVariable &variable = asSymbol->variable();
                ASSERT(variable.symbolType() != SymbolType::Empty);
                extractStructSamplerUniforms(decl, variable, type.getStruct(), newSequence);
            }

            mMultiReplacements.emplace_back(getParentNode()->getAsBlock(), decl, *newSequence);
        }

        return true;
    }

    bool visitBinary(Visit visit, TIntermBinary *node) override
    {
        if (visit != PreVisit)
            return true;

        if (node->getOp() == EOpIndexDirectStruct && node->getType().isSampler())
        {
            ImmutableString newName = GetStructSamplerNameFromTyped(node);
            const TVariable *samplerReplacement =
                static_cast<const TVariable *>(mSymbolTable->findUser(newName));
            ASSERT(samplerReplacement);

            TIntermSymbol *replacement = new TIntermSymbol(samplerReplacement);

            queueReplacement(replacement, OriginalNode::IS_DROPPED);
            return true;
        }

        return true;
    }

    void visitFunctionPrototype(TIntermFunctionPrototype *node) override
    {
        const TFunction *function = node->getFunction();

        if (!function->containsStructSamplerParams())
        {
            return;
        }

        TFunction *newFunction = createStructSamplerFunction(function);
        mSymbolTable->declareUserDefinedFunction(newFunction, true);
        TIntermFunctionPrototype *newProto = new TIntermFunctionPrototype(newFunction);
        queueReplacement(newProto, OriginalNode::IS_DROPPED);
    }

    bool visitFunctionDefinition(Visit visit, TIntermFunctionDefinition *node) override
    {
        if (visit == PreVisit)
        {
            mSymbolTable->push();
        }
        else
        {
            ASSERT(visit == PostVisit);
            mSymbolTable->pop();
        }
        return true;
    }

    bool visitAggregate(Visit visit, TIntermAggregate *node) override
    {
        if (visit != PreVisit)
            return true;

        if (!node->isFunctionCall())
            return true;

        const TFunction *function = node->getFunction();
        if (!function->containsStructSamplerParams())
            return true;

        ASSERT(node->getOp() == EOpCallFunctionInAST);
        TFunction *newFunction        = mSymbolTable->findUserDefinedFunction(function->name());
        TIntermSequence *newArguments = getStructSamplerArguments(function, node->getSequence());

        TIntermAggregate *newCall =
            TIntermAggregate::CreateFunctionCall(*newFunction, newArguments);
        queueReplacement(newCall, OriginalNode::IS_DROPPED);
        return true;
    }

  private:
    static ImmutableString GetStructSamplerNameFromTyped(TIntermTyped *node)
    {
        std::string stringBuilder;

        TIntermTyped *currentNode = node;
        while (currentNode->getAsBinaryNode())
        {
            TIntermBinary *asBinary = currentNode->getAsBinaryNode();

            switch (asBinary->getOp())
            {
                case EOpIndexDirect:
                {
                    const int index = asBinary->getRight()->getAsConstantUnion()->getIConst(0);
                    const std::string strInt = Str(index);
                    stringBuilder.insert(0, strInt);
                    stringBuilder.insert(0, "_");
                    break;
                }
                case EOpIndexDirectStruct:
                {
                    stringBuilder.insert(0, asBinary->getIndexStructFieldName().data());
                    stringBuilder.insert(0, "_");
                    break;
                }

                default:
                    UNREACHABLE();
                    break;
            }

            currentNode = asBinary->getLeft();
        }

        const ImmutableString &variableName = currentNode->getAsSymbolNode()->variable().name();
        stringBuilder.insert(0, variableName.data());

        return stringBuilder;
    }

    void stripStructSpecifierSamplers(const TStructure *structure, TIntermSequence *newSequence)
    {
        TFieldList *newFieldList = new TFieldList;
        ASSERT(structure->containsSamplers());

        for (const TField *field : structure->fields())
        {
            const TType &fieldType = *field->type();
            if (!fieldType.isSampler() && !isRemovedStructType(fieldType))
            {
                TType *newType = new TType(fieldType);
                TField *newField =
                    new TField(newType, field->name(), field->line(), field->symbolType());
                newFieldList->push_back(newField);
            }
        }

        // Prune empty structs.
        if (newFieldList->empty())
        {
            mRemovedStructs.insert(structure->name());
            return;
        }

        TStructure *newStruct =
            new TStructure(mSymbolTable, structure->name(), newFieldList, structure->symbolType());
        TType *newStructType = new TType(newStruct, true);
        TVariable *newStructVar =
            new TVariable(mSymbolTable, kEmptyImmutableString, newStructType, SymbolType::Empty);
        TIntermSymbol *newStructRef = new TIntermSymbol(newStructVar);

        TIntermDeclaration *structDecl = new TIntermDeclaration;
        structDecl->appendDeclarator(newStructRef);

        newSequence->push_back(structDecl);

        mSymbolTable->declare(newStruct);
    }

    bool isRemovedStructType(const TType &type) const
    {
        const TStructure *structure = type.getStruct();
        return (structure && (mRemovedStructs.count(structure->name()) > 0));
    }

    void extractStructSamplerUniforms(TIntermDeclaration *oldDeclaration,
                                      const TVariable &variable,
                                      const TStructure *structure,
                                      TIntermSequence *newSequence)
    {
        ASSERT(structure->containsSamplers());

        size_t nonSamplerCount = 0;

        for (const TField *field : structure->fields())
        {
            nonSamplerCount +=
                extractFieldSamplers(variable.name(), field, variable.getType(), newSequence);
        }

        if (nonSamplerCount > 0)
        {
            // Keep the old declaration around if it has other members.
            newSequence->push_back(oldDeclaration);
        }
        else
        {
            mRemovedUniformsCount++;
        }
    }

    size_t extractFieldSamplers(const ImmutableString &prefix,
                                const TField *field,
                                const TType &containingType,
                                TIntermSequence *newSequence)
    {
        if (containingType.isArray())
        {
            size_t nonSamplerCount = 0;

            // Name the samplers internally as varName_<index>_fieldName
            const TVector<unsigned int> &arraySizes = *containingType.getArraySizes();
            for (unsigned int arrayElement = 0; arrayElement < arraySizes[0]; ++arrayElement)
            {
                ImmutableStringBuilder stringBuilder(prefix.length() + 10);
                stringBuilder << prefix << "_";
                stringBuilder.appendHex(arrayElement);
                nonSamplerCount = extractFieldSamplersImpl(stringBuilder, field, newSequence);
            }

            return nonSamplerCount;
        }

        return extractFieldSamplersImpl(prefix, field, newSequence);
    }

    size_t extractFieldSamplersImpl(const ImmutableString &prefix,
                                    const TField *field,
                                    TIntermSequence *newSequence)
    {
        size_t nonSamplerCount = 0;

        const TType &fieldType = *field->type();
        if (fieldType.isSampler() || fieldType.isStructureContainingSamplers())
        {
            ImmutableStringBuilder stringBuilder(prefix.length() + field->name().length() + 1);
            stringBuilder << prefix << "_" << field->name();
            ImmutableString newPrefix(stringBuilder);

            if (fieldType.isSampler())
            {
                extractSampler(newPrefix, fieldType, newSequence);
            }
            else
            {
                const TStructure *structure = fieldType.getStruct();
                for (const TField *nestedField : structure->fields())
                {
                    nonSamplerCount +=
                        extractFieldSamplers(newPrefix, nestedField, fieldType, newSequence);
                }
            }
        }
        else
        {
            nonSamplerCount++;
        }

        return nonSamplerCount;
    }

    void extractSampler(const ImmutableString &newName,
                        const TType &fieldType,
                        TIntermSequence *newSequence) const
    {
        TType *newType = new TType(fieldType);
        newType->setQualifier(EvqUniform);
        TVariable *newVariable =
            new TVariable(mSymbolTable, newName, newType, SymbolType::AngleInternal);
        TIntermSymbol *newRef = new TIntermSymbol(newVariable);

        TIntermDeclaration *samplerDecl = new TIntermDeclaration;
        samplerDecl->appendDeclarator(newRef);

        newSequence->push_back(samplerDecl);

        mSymbolTable->declareInternal(newVariable);
    }

    static ImmutableString GetFieldName(const ImmutableString &paramName,
                                        const TField *field,
                                        unsigned arrayIndex)
    {
        ImmutableStringBuilder nameBuilder(paramName.length() + 10 + field->name().length());
        nameBuilder << paramName << "_";

        if (arrayIndex < std::numeric_limits<unsigned>::max())
        {
            nameBuilder.appendHex(arrayIndex);
            nameBuilder << "_";
        }
        nameBuilder << field->name();

        return nameBuilder;
    }

    class StructSamplerFunctionVisitor : angle::NonCopyable
    {
      public:
        StructSamplerFunctionVisitor()          = default;
        virtual ~StructSamplerFunctionVisitor() = default;

        virtual void traverse(const TFunction *function)
        {
            size_t paramCount = function->getParamCount();

            for (size_t paramIndex = 0; paramIndex < paramCount; ++paramIndex)
            {
                const TVariable *param = function->getParam(paramIndex);
                const TType &paramType = param->getType();

                if (paramType.isStructureContainingSamplers())
                {
                    const ImmutableString &baseName = getNameFromIndex(function, paramIndex);
                    if (traverseStructSamplers(baseName, paramType))
                    {
                        visitStructParam(function, paramIndex);
                    }
                }
                else
                {
                    visitNonStructParam(function, paramIndex);
                }
            }
        }

        virtual ImmutableString getNameFromIndex(const TFunction *function, size_t paramIndex) = 0;
        virtual void visitStructSamplerParam(const ImmutableString &name, const TField *field) = 0;
        virtual void visitStructParam(const TFunction *function, size_t paramIndex)            = 0;
        virtual void visitNonStructParam(const TFunction *function, size_t paramIndex)         = 0;

      private:
        bool traverseStructSamplers(const ImmutableString &baseName, const TType &structType)
        {
            bool hasNonSamplerParams    = false;
            const TStructure *structure = structType.getStruct();
            for (const TField *field : structure->fields())
            {
                if (field->type()->isStructureContainingSamplers() || field->type()->isSampler())
                {
                    if (traverseStructSampler(baseName, structType, field))
                    {
                        hasNonSamplerParams = true;
                    }
                }
                else
                {
                    hasNonSamplerParams = true;
                }
            }
            return hasNonSamplerParams;
        }

        bool traverseStructSampler(const ImmutableString &baseName,
                                   const TType &baseType,
                                   const TField *field)
        {
            bool hasNonSamplerParams = false;

            if (baseType.isArray())
            {
                const TVector<unsigned int> &arraySizes = *baseType.getArraySizes();
                ASSERT(arraySizes.size() == 1);

                for (unsigned int arrayIndex = 0; arrayIndex < arraySizes[0]; ++arrayIndex)
                {
                    ImmutableString name = GetFieldName(baseName, field, arrayIndex);

                    if (field->type()->isStructureContainingSamplers())
                    {
                        if (traverseStructSamplers(name, *field->type()))
                        {
                            hasNonSamplerParams = true;
                        }
                    }
                    else
                    {
                        visitStructSamplerParam(name, field);
                    }
                }
            }
            else if (field->type()->isStructureContainingSamplers())
            {
                ImmutableString name =
                    GetFieldName(baseName, field, std::numeric_limits<unsigned>::max());
                hasNonSamplerParams = traverseStructSamplers(name, *field->type());
            }
            else
            {
                ImmutableString name =
                    GetFieldName(baseName, field, std::numeric_limits<unsigned>::max());
                visitStructSamplerParam(name, field);
            }

            return hasNonSamplerParams;
        }
    };

    class CreateStructSamplerFunctionVisitor final : public StructSamplerFunctionVisitor
    {
      public:
        CreateStructSamplerFunctionVisitor(TSymbolTable *symbolTable)
            : mSymbolTable(symbolTable), mNewFunction(nullptr)
        {
        }

        ImmutableString getNameFromIndex(const TFunction *function, size_t paramIndex) override
        {
            const TVariable *param = function->getParam(paramIndex);
            return param->name();
        }

        void traverse(const TFunction *function) override
        {
            mNewFunction =
                new TFunction(mSymbolTable, function->name(), function->symbolType(),
                              &function->getReturnType(), function->isKnownToNotHaveSideEffects());

            StructSamplerFunctionVisitor::traverse(function);
        }

        void visitStructSamplerParam(const ImmutableString &name, const TField *field) override
        {
            TVariable *fieldSampler =
                new TVariable(mSymbolTable, name, field->type(), SymbolType::AngleInternal);
            mNewFunction->addParameter(fieldSampler);
            mSymbolTable->declareInternal(fieldSampler);
        }

        void visitStructParam(const TFunction *function, size_t paramIndex) override
        {
            const TVariable *param      = function->getParam(paramIndex);
            const TStructure *structure = param->getType().getStruct();
            const TSymbol *structSymbol = mSymbolTable->findUser(structure->name());
            ASSERT(structSymbol && structSymbol->isStruct());
            const TStructure *structVar = static_cast<const TStructure *>(structSymbol);
            TType *structType           = new TType(structVar, false);

            TVariable *newParam =
                new TVariable(mSymbolTable, param->name(), structType, param->symbolType());
            mNewFunction->addParameter(newParam);
        }

        void visitNonStructParam(const TFunction *function, size_t paramIndex) override
        {
            const TVariable *param = function->getParam(paramIndex);
            mNewFunction->addParameter(param);
        }

        TFunction *getNewFunction() const { return mNewFunction; }

      private:
        TSymbolTable *mSymbolTable;
        TFunction *mNewFunction;
    };

    TFunction *createStructSamplerFunction(const TFunction *function) const
    {
        CreateStructSamplerFunctionVisitor visitor(mSymbolTable);
        visitor.traverse(function);
        return visitor.getNewFunction();
    }

    class GetSamplerArgumentsVisitor final : public StructSamplerFunctionVisitor
    {
      public:
        GetSamplerArgumentsVisitor(TSymbolTable *symbolTable, const TIntermSequence *arguments)
            : mSymbolTable(symbolTable), mArguments(arguments), mNewArguments(new TIntermSequence)
        {
        }

        ImmutableString getNameFromIndex(const TFunction *function, size_t paramIndex) override
        {
            TIntermTyped *argument = (*mArguments)[paramIndex]->getAsTyped();
            return GetStructSamplerNameFromTyped(argument);
        }

        void visitStructSamplerParam(const ImmutableString &name, const TField *field) override
        {
            TVariable *argSampler =
                new TVariable(mSymbolTable, name, field->type(), SymbolType::AngleInternal);
            TIntermSymbol *argSymbol = new TIntermSymbol(argSampler);
            mNewArguments->push_back(argSymbol);
        }

        void visitStructParam(const TFunction *function, size_t paramIndex) override
        {
            TIntermTyped *argument = (*mArguments)[paramIndex]->getAsTyped();
            mNewArguments->push_back(argument);
        }

        void visitNonStructParam(const TFunction *function, size_t paramIndex) override
        {
            TIntermTyped *argument = (*mArguments)[paramIndex]->getAsTyped();
            mNewArguments->push_back(argument);
        }

        TIntermSequence *getNewArguments() const { return mNewArguments; }

      private:
        TSymbolTable *mSymbolTable;
        const TIntermSequence *mArguments;
        TIntermSequence *mNewArguments;
    };

    TIntermSequence *getStructSamplerArguments(const TFunction *function,
                                               const TIntermSequence *arguments) const
    {
        GetSamplerArgumentsVisitor visitor(mSymbolTable, arguments);
        visitor.traverse(function);
        return visitor.getNewArguments();
    }

    int mRemovedUniformsCount;
    std::set<ImmutableString> mRemovedStructs;
};
}  // anonymous namespace

int RewriteStructSamplers(TIntermBlock *root, TSymbolTable *symbolTable)
{
    Traverser rewriteStructSamplers(symbolTable);
    root->traverse(&rewriteStructSamplers);
    rewriteStructSamplers.updateTree();

    return rewriteStructSamplers.removedUniformsCount();
}
}  // namespace sh
