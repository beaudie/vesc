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
#include "compiler/translator/tree_util/IntermNode_util.h"
#include "compiler/translator/tree_util/IntermTraverse.h"

namespace sh
{
namespace
{
// Helper method to get the sampler extracted struct type of a parameter.
TType *GetStructSamplerParameterType(TSymbolTable *symbolTable, const TVariable &param)
{
    const TStructure *structure = param.getType().getStruct();
    const TSymbol *structSymbol = symbolTable->findUserDefined(structure->name());
    ASSERT(structSymbol && structSymbol->isStruct());
    const TStructure *structVar = static_cast<const TStructure *>(structSymbol);
    TType *structType           = new TType(structVar, false);

    if (param.getType().isArray())
    {
        structType->makeArrays(*param.getType().getArraySizes());
    }

    ASSERT(!structType->isStructureContainingSamplers());

    return structType;
}

TIntermSymbol *ReplaceTypeOfSymbolNode(TIntermSymbol *symbolNode, TSymbolTable *symbolTable)
{
    const TVariable &oldVariable = symbolNode->variable();

    TType *newType = GetStructSamplerParameterType(symbolTable, oldVariable);

    TVariable *newVariable =
        new TVariable(oldVariable.uniqueId(), oldVariable.name(), oldVariable.symbolType(),
                      oldVariable.extension(), newType);
    return new TIntermSymbol(newVariable);
}

TIntermTyped *ReplaceTypeOfTypedStructNode(TIntermTyped *argument, TSymbolTable *symbolTable)
{
    TIntermSymbol *asSymbol = argument->getAsSymbolNode();
    if (asSymbol)
    {
        ASSERT(asSymbol->getType().getStruct());
        return ReplaceTypeOfSymbolNode(asSymbol, symbolTable);
    }

    TIntermTyped *replacement = argument->deepCopy();
    TIntermBinary *binary     = replacement->getAsBinaryNode();
    ASSERT(binary);

    while (binary)
    {
        ASSERT(binary->getOp() == EOpIndexDirectStruct || binary->getOp() == EOpIndexDirect);

        asSymbol = binary->getLeft()->getAsSymbolNode();

        if (asSymbol)
        {
            ASSERT(asSymbol->getType().getStruct());
            TIntermSymbol *newSymbol = ReplaceTypeOfSymbolNode(asSymbol, symbolTable);
            binary->replaceChildNode(binary->getLeft(), newSymbol);
            return replacement;
        }

        binary = binary->getLeft()->getAsBinaryNode();
    }

    UNREACHABLE();
    return nullptr;
}

// Maximum string size of a hex unsigned int.
constexpr size_t kHexSize = ImmutableStringBuilder::GetHexCharCount<unsigned int>();

// Generates names for array elements of the form
// prefix_<index0>_<index1>_...<indexN>
void GenerateArrayNames(const ImmutableString &prefix,
                        const TType *baseType,
                        std::vector<ImmutableStringBuilder> *elementNamesOut)
{
    ASSERT(baseType->isArray());
    const TVector<unsigned int> &arraySizes             = *baseType->getArraySizes();
    std::vector<ImmutableStringBuilder> &stringBuilders = *elementNamesOut;
    size_t numArrayElements                             = baseType->getArraySizeProduct();
    stringBuilders.reserve(numArrayElements);
    for (size_t i = 0; i < numArrayElements; i++)
    {
        stringBuilders.emplace_back(prefix.length() +
                                    (1 + kHexSize) * baseType->getNumArraySizes());
        stringBuilders[i] << prefix;
    }
    unsigned int stride = 1;
    for (size_t j = arraySizes.size(); j > 0; j--)
    {
        size_t arrayLevel      = j - 1;
        unsigned int arraySize = arraySizes[arrayLevel];
        for (unsigned int arrayElement = 0; arrayElement * stride < numArrayElements;
             arrayElement++)
        {
            for (size_t i = 0; i < stride; i++)
            {
                ImmutableStringBuilder &stringBuilder = stringBuilders[arrayElement * stride + i];
                stringBuilder << "_";
                stringBuilder.appendHex(arrayElement % arraySize);
            }
        }
        stride *= arraySize;
    }
}

class Traverser final : public TIntermTraverser
{
  public:
    explicit Traverser(TSymbolTable *symbolTable)
        : TIntermTraverser(true, false, true, symbolTable), mRemovedUniformsCount(0)
    {
        mSymbolTable->push();
        mCumulativeArraySizeStack.push_back(1);
    }

    ~Traverser() override { mSymbolTable->pop(); }

    int removedUniformsCount() const { return mRemovedUniformsCount; }

    // Each struct sampler declaration is stripped of its samplers. New uniforms are added for each
    // stripped struct sampler.
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

        if (type.isSampler() && type.isArrayOfArrays())
        {
            TIntermSequence *newSequence = new TIntermSequence;
            TIntermSymbol *asSymbol      = declarator->getAsSymbolNode();
            ASSERT(asSymbol);
            const TVariable &variable = asSymbol->variable();
            ASSERT(variable.symbolType() != SymbolType::Empty);
            extractSampler(variable.name(), variable.getType(), newSequence, 0);
            mMultiReplacements.emplace_back(getParentNode()->getAsBlock(), decl, *newSequence);
        }

        return true;
    }

    // Each struct sampler reference is replaced with a reference to the new extracted sampler.
    bool visitBinary(Visit visit, TIntermBinary *node) override
    {
        if (visit != PreVisit)
            return true;
        if (!node->getType().isSampler() || node->getType().isArray())
            return true;

        if (node->getOp() == EOpIndexDirect || node->getOp() == EOpIndexIndirect ||
            node->getOp() == EOpIndexDirectStruct)
        {
            ImmutableString newName = GetStructSamplerNameFromTypedNode(node);
            const TVariable *samplerReplacement =
                static_cast<const TVariable *>(mSymbolTable->findUserDefined(newName));
            ASSERT(samplerReplacement);

            TIntermTyped *replacement = new TIntermSymbol(samplerReplacement);

            // Add in an indirect index if contained in an array
            const auto &strides = mArraySizeMap[newName];
            ASSERT(!strides.empty());
            if (strides.size() > 1)
            {
                TIntermTyped *index = GetIndexExpressionFromTypedNode(node, strides);
                replacement         = new TIntermBinary(EOpIndexIndirect, replacement, index);
            }

            queueReplacement(replacement, OriginalNode::IS_DROPPED);
            return true;
        }

        return true;
    }

    // In we are passing references to structs containing samplers we must new additional
    // arguments. For each extracted struct sampler a new argument is added. This chains to nested
    // structs.
    void visitFunctionPrototype(TIntermFunctionPrototype *node) override
    {
        const TFunction *function = node->getFunction();

        if (!function->hasSamplerInStructParams())
        {
            return;
        }

        const TSymbol *foundFunction = mSymbolTable->findUserDefined(function->name());
        if (foundFunction)
        {
            ASSERT(foundFunction->isFunction());
            function = static_cast<const TFunction *>(foundFunction);
        }
        else
        {
            TFunction *newFunction = createStructSamplerFunction(function);
            mSymbolTable->declareUserDefinedFunction(newFunction, true);
            function = newFunction;
        }

        ASSERT(!function->hasSamplerInStructParams());
        TIntermFunctionPrototype *newProto = new TIntermFunctionPrototype(function);
        queueReplacement(newProto, OriginalNode::IS_DROPPED);
    }

    // We insert a new scope for each function definition so we can track the new parameters.
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

    // For function call nodes we pass references to the extracted struct samplers in that scope.
    bool visitAggregate(Visit visit, TIntermAggregate *node) override
    {
        if (visit != PreVisit)
            return true;

        if (!node->isFunctionCall())
            return true;

        const TFunction *function = node->getFunction();
        if (!function->hasSamplerInStructParams())
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
    // This returns the name of a struct sampler reference. References are always TIntermBinary.
    static ImmutableString GetStructSamplerNameFromTypedNode(TIntermTyped *node)
    {
        std::string stringBuilder;

        TIntermTyped *currentNode = node;
        while (currentNode->getAsBinaryNode())
        {
            TIntermBinary *asBinary = currentNode->getAsBinaryNode();

            switch (asBinary->getOp())
            {
                case EOpIndexDirectStruct:
                {
                    stringBuilder.insert(0, asBinary->getIndexStructFieldName().data());
                    stringBuilder.insert(0, "_");
                    break;
                }

                case EOpIndexDirect:
                case EOpIndexIndirect:
                    break;

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

    // This returns an expression representing the correct index using the array
    // index operations in node.
    static TIntermTyped *GetIndexExpressionFromTypedNode(TIntermTyped *node,
                                                         const std::vector<size_t> &strides)
    {
        TIntermTyped *result      = CreateIndexNode(0);
        TIntermTyped *currentNode = node;

        auto it = strides.begin();
        while (currentNode->getAsBinaryNode())
        {
            TIntermBinary *asBinary = currentNode->getAsBinaryNode();

            switch (asBinary->getOp())
            {
                case EOpIndexDirectStruct:
                    break;

                case EOpIndexDirect:
                case EOpIndexIndirect:
                {
                    TIntermBinary *multiply =
                        new TIntermBinary(EOpMul, CreateIndexNode(*it++), asBinary->getRight());
                    result = new TIntermBinary(EOpAdd, result, multiply);
                    break;
                }

                default:
                    UNREACHABLE();
                    break;
            }

            currentNode = asBinary->getLeft();
        }

        return result;
    }

    // Removes all the struct samplers from a struct specifier.
    void stripStructSpecifierSamplers(const TStructure *structure, TIntermSequence *newSequence)
    {
        TFieldList *newFieldList = new TFieldList;
        ASSERT(structure->containsSamplers());

        for (const TField *field : structure->fields())
        {
            const TType &fieldType = *field->type();
            if (!fieldType.isSampler() && !isRemovedStructType(fieldType))
            {
                TType *newType = nullptr;

                if (fieldType.isStructureContainingSamplers())
                {
                    const TSymbol *structSymbol =
                        mSymbolTable->findUserDefined(fieldType.getStruct()->name());
                    ASSERT(structSymbol && structSymbol->isStruct());
                    const TStructure *fieldStruct = static_cast<const TStructure *>(structSymbol);
                    newType                       = new TType(fieldStruct, true);
                    if (fieldType.isArray())
                    {
                        newType->makeArrays(*fieldType.getArraySizes());
                    }
                }
                else
                {
                    newType = new TType(fieldType);
                }

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

    // Returns true if the type is a struct that was removed because we extracted all the members.
    bool isRemovedStructType(const TType &type) const
    {
        const TStructure *structure = type.getStruct();
        return (structure && (mRemovedStructs.count(structure->name()) > 0));
    }

    // Removes samplers from struct uniforms. For each sampler removed also adds a new globally
    // defined sampler uniform.
    void extractStructSamplerUniforms(TIntermDeclaration *oldDeclaration,
                                      const TVariable &variable,
                                      const TStructure *structure,
                                      TIntermSequence *newSequence)
    {
        ASSERT(structure->containsSamplers());

        size_t nonSamplerCount = 0;

        enterArray(variable.getType());

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

        exitArray(variable.getType());
    }

    // Extracts samplers from a field of a struct. Works with nested structs and arrays.
    size_t extractFieldSamplers(const ImmutableString &prefix,
                                const TField *field,
                                const TType &containingType,
                                TIntermSequence *newSequence)
    {
        return extractFieldSamplersImpl(prefix, field, newSequence);
    }

    // Extracts samplers from a field of a struct. Works with nested structs and arrays.
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
                extractSampler(newPrefix, fieldType, newSequence, 0);
            }
            else
            {
                enterArray(fieldType);
                const TStructure *structure = fieldType.getStruct();
                for (const TField *nestedField : structure->fields())
                {
                    nonSamplerCount +=
                        extractFieldSamplers(newPrefix, nestedField, fieldType, newSequence);
                }
                exitArray(fieldType);
            }
        }
        else
        {
            nonSamplerCount++;
        }

        return nonSamplerCount;
    }

    void enterArray(const TType &arrayType)
    {
        if (!arrayType.isArray())
            return;
        size_t currentArraySize = mCumulativeArraySizeStack.back();
        const auto &arraySizes  = *arrayType.getArraySizes();
        for (auto it = arraySizes.rbegin(); it != arraySizes.rend(); ++it)
        {
            unsigned int arraySize = *it;
            currentArraySize *= arraySize;
            mArraySizeStack.push_back(arraySize);
            mCumulativeArraySizeStack.push_back(currentArraySize);
        }
    }

    void exitArray(const TType &arrayType)
    {
        if (!arrayType.isArray())
            return;
        mArraySizeStack.resize(mArraySizeStack.size() - arrayType.getNumArraySizes());
        mCumulativeArraySizeStack.resize(mCumulativeArraySizeStack.size() -
                                         arrayType.getNumArraySizes());
    }

    // Extracts a sampler from a struct. Declares the new extracted sampler.
    void extractSampler(const ImmutableString &newName,
                        const TType &fieldType,
                        TIntermSequence *newSequence,
                        size_t arrayLevel)
    {
        enterArray(fieldType);

        TType *newType = new TType(fieldType);
        while (newType->isArray())
        {
            newType->toArrayElementType();
        }
        if (!mArraySizeStack.empty())
        {
            newType->makeArray(static_cast<unsigned int>(mCumulativeArraySizeStack.back()));
        }
        newType->setQualifier(EvqUniform);
        TVariable *newVariable =
            new TVariable(mSymbolTable, newName, newType, SymbolType::AngleInternal);
        TIntermSymbol *newRef = new TIntermSymbol(newVariable);

        TIntermDeclaration *samplerDecl = new TIntermDeclaration;
        samplerDecl->appendDeclarator(newRef);

        newSequence->push_back(samplerDecl);

        mSymbolTable->declareInternal(newVariable);

        auto &strides = mArraySizeMap[newName];
        ASSERT(strides.empty());
        strides.reserve(mCumulativeArraySizeStack.size());

        size_t currentStride = 1;
        strides.push_back(1);
        for (auto it = mArraySizeStack.rbegin(); it != mArraySizeStack.rend(); ++it)
        {
            currentStride *= *it;
            strides.push_back(currentStride);
        }

        {
            // XXX XXX XXX
            WARN() << newName << " ->";
            size_t prev = 1;
            for (size_t a : strides)
            {
                WARN() << a << " (" << (a / prev) << ")";
                prev = a;
            }
            // XXX XXX XXX
        }

        exitArray(fieldType);
    }

    // Returns the chained name of a sampler uniform field.
    static ImmutableString GetFieldName(const ImmutableString &paramName, const TField *field)
    {
        ImmutableStringBuilder nameBuilder(paramName.length() + 1 + field->name().length());
        nameBuilder << paramName << "_";
        nameBuilder << field->name();

        return nameBuilder;
    }

    // A pattern that visits every parameter of a function call. Uses different handlers for struct
    // parameters, struct sampler parameters, and non-struct parameters.
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
                    if (traverseStructContainingSamplers(baseName, paramType))
                    {
                        visitStructParam(function, paramIndex);
                    }
                }
                else if (paramType.isArrayOfArrays() && paramType.isSampler())
                {
                    const ImmutableString &paramName = getNameFromIndex(function, paramIndex);
                    traverseLeafSampler(paramName, paramType, 0);
                }
                else
                {
                    visitNonStructParam(function, paramIndex);
                }
            }
        }

        virtual ImmutableString getNameFromIndex(const TFunction *function, size_t paramIndex) = 0;
        // Also includes samplers in arrays of arrays.
        virtual void visitSamplerInStructParam(const ImmutableString &name, const TType *type) = 0;
        virtual void visitStructParam(const TFunction *function, size_t paramIndex)            = 0;
        virtual void visitNonStructParam(const TFunction *function, size_t paramIndex)         = 0;

      private:
        bool traverseStructContainingSamplers(const ImmutableString &baseName,
                                              const TType &structType)
        {
            bool hasNonSamplerFields    = false;
            const TStructure *structure = structType.getStruct();
            for (const TField *field : structure->fields())
            {
                if (field->type()->isStructureContainingSamplers() || field->type()->isSampler())
                {
                    if (traverseSamplerInStruct(baseName, structType, field))
                    {
                        hasNonSamplerFields = true;
                    }
                }
                else
                {
                    hasNonSamplerFields = true;
                }
            }
            return hasNonSamplerFields;
        }

        bool traverseSamplerInStruct(const ImmutableString &baseName,
                                     const TType &baseType,
                                     const TField *field)
        {
            bool hasNonSamplerParams = false;

            if (baseType.isArray())
            {
                std::vector<ImmutableStringBuilder> elementNames;
                GenerateArrayNames(baseName, &baseType, &elementNames);

                for (auto &elementName : elementNames)
                {
                    ImmutableString name = GetFieldName(elementName, field);

                    if (field->type()->isStructureContainingSamplers())
                    {
                        if (traverseStructContainingSamplers(name, *field->type()))
                        {
                            hasNonSamplerParams = true;
                        }
                    }
                    else
                    {
                        ASSERT(field->type()->isSampler());
                        traverseLeafSampler(name, *field->type(), 0);
                    }
                }
            }
            else if (field->type()->isStructureContainingSamplers())
            {
                ImmutableString name = GetFieldName(baseName, field);
                hasNonSamplerParams = traverseStructContainingSamplers(name, *field->type());
            }
            else
            {
                ASSERT(field->type()->isSampler());
                ImmutableString name = GetFieldName(baseName, field);
                traverseLeafSampler(name, *field->type(), 0);
            }

            return hasNonSamplerParams;
        }

        void traverseLeafSampler(const ImmutableString &samplerName,
                                 const TType &samplerType,
                                 size_t arrayLevel)
        {
            size_t numArraySizes = samplerType.getNumArraySizes();
            if (numArraySizes <= arrayLevel + 1)
            {
                TType *finalType = new TType(samplerType);
                while (finalType->isArrayOfArrays())
                {
                    finalType->toArrayElementType();
                }
                visitSamplerInStructParam(samplerName, finalType);
                return;
            }
            unsigned int arraySize = (*samplerType.getArraySizes())[numArraySizes - arrayLevel - 1];
            for (unsigned int i = 0; i < arraySize; i++)
            {
                ImmutableStringBuilder stringBuilder(samplerName.length() + 1 + kHexSize);
                stringBuilder << samplerName << "_";
                stringBuilder.appendHex(i);
                ImmutableString samplerName = stringBuilder;
                traverseLeafSampler(samplerName, samplerType, arrayLevel + 1);
            }
            return;
        }
    };

    // A visitor that replaces functions with struct sampler references. The struct sampler
    // references are expanded to include new fields for the structs.
    class CreateStructSamplerFunctionVisitor final : public StructSamplerFunctionVisitor
    {
      public:
        CreateStructSamplerFunctionVisitor(TSymbolTable *symbolTable)
            : mSymbolTable(symbolTable), mNewFunction(nullptr)
        {}

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

        void visitSamplerInStructParam(const ImmutableString &name, const TType *type) override
        {
            TVariable *fieldSampler =
                new TVariable(mSymbolTable, name, type, SymbolType::AngleInternal);
            mNewFunction->addParameter(fieldSampler);
            mSymbolTable->declareInternal(fieldSampler);
        }

        void visitStructParam(const TFunction *function, size_t paramIndex) override
        {
            const TVariable *param = function->getParam(paramIndex);
            TType *structType      = GetStructSamplerParameterType(mSymbolTable, *param);
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

    // A visitor that replaces function calls with expanded struct sampler parameters.
    class GetSamplerArgumentsVisitor final : public StructSamplerFunctionVisitor
    {
      public:
        GetSamplerArgumentsVisitor(TSymbolTable *symbolTable, const TIntermSequence *arguments)
            : mSymbolTable(symbolTable), mArguments(arguments), mNewArguments(new TIntermSequence)
        {}

        ImmutableString getNameFromIndex(const TFunction *function, size_t paramIndex) override
        {
            TIntermTyped *argument = (*mArguments)[paramIndex]->getAsTyped();
            return GetStructSamplerNameFromTypedNode(argument);
        }

        void visitSamplerInStructParam(const ImmutableString &name, const TType *type) override
        {
            TVariable *argSampler =
                new TVariable(mSymbolTable, name, type, SymbolType::AngleInternal);
            TIntermSymbol *argSymbol = new TIntermSymbol(argSampler);
            mNewArguments->push_back(argSymbol);
        }

        void visitStructParam(const TFunction *function, size_t paramIndex) override
        {
            // The tree structure of the parameter is modified to point to the new type. This leaves
            // the tree in a consistent state.
            TIntermTyped *argument    = (*mArguments)[paramIndex]->getAsTyped();
            TIntermTyped *replacement = ReplaceTypeOfTypedStructNode(argument, mSymbolTable);
            mNewArguments->push_back(replacement);
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
    // The first element is 1; each successive element is the previous
    // multiplied by the size of the next nested array in the current sampler.
    // For example, with sampler2D foo[3][6], we would have {1, 3, 18}.
    std::vector<size_t> mCumulativeArraySizeStack;
    std::vector<size_t> mArraySizeStack;
    // The value consists of strides, starting from the outermost array.
    // For example, with sampler2D foo[3][6], we would have {1, 6, 18}.
    std::map<ImmutableString, std::vector<size_t>> mArraySizeMap;
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
