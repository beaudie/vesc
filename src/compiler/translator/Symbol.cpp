//
// Copyright (c) 2017 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//
// Symbol.cpp: Symbols representing variables, functions, structures and interface blocks.
//

#if defined(_MSC_VER)
#pragma warning(disable : 4718)
#endif

#include "compiler/translator/Symbol.h"

#include "compiler/translator/ImmutableStringBuilder.h"
#include "compiler/translator/SymbolTable.h"

namespace sh
{

namespace
{

constexpr const ImmutableString kMainName("main");
constexpr const ImmutableString kImageLoadName("imageLoad");
constexpr const ImmutableString kImageStoreName("imageStore");
constexpr const ImmutableString kImageSizeName("imageSize");

static const char kFunctionMangledNameSeparator = '(';

}  // anonymous namespace

TSymbol::TSymbol(TSymbolTable *symbolTable,
                 const ImmutableString &name,
                 SymbolType symbolType,
                 TExtension extension)
    : mName(name),
      mUniqueId(symbolTable->nextUniqueId()),
      mSymbolType(symbolType),
      mExtension(extension)
{
    ASSERT(mSymbolType == SymbolType::BuiltIn || mExtension == TExtension::UNDEFINED);
    ASSERT(mName != "" || mSymbolType == SymbolType::AngleInternal ||
           mSymbolType == SymbolType::Empty);
}

ImmutableString TSymbol::name() const
{
    if (mName != "")
    {
        return mName;
    }
    ASSERT(mSymbolType == SymbolType::AngleInternal);
    int uniqueId = mUniqueId.get();
    ImmutableStringBuilder symbolNameOut(sizeof(uniqueId) * 2u + 1u);
    symbolNameOut << 's';
    symbolNameOut.appendHex(mUniqueId.get());
    return symbolNameOut;
}

ImmutableString TSymbol::getMangledName() const
{
    ASSERT(mSymbolType != SymbolType::Empty);
    return name();
}

TVariable::TVariable(TSymbolTable *symbolTable,
                     const ImmutableString &name,
                     const TType *type,
                     SymbolType symbolType,
                     TExtension extension)
    : TSymbol(symbolTable, name, symbolType, extension), mType(type), unionArray(nullptr)
{
    ASSERT(mType);
}

TStructure::TStructure(TSymbolTable *symbolTable,
                       const ImmutableString &name,
                       const TFieldList *fields,
                       SymbolType symbolType)
    : TSymbol(symbolTable, name, symbolType), TFieldListCollection(fields)
{
}

void TStructure::createSamplerSymbols(const char *namePrefix,
                                      const TString &apiNamePrefix,
                                      TVector<const TVariable *> *outputSymbols,
                                      TMap<const TVariable *, TString> *outputSymbolsToAPINames,
                                      TSymbolTable *symbolTable) const
{
    ASSERT(containsSamplers());
    for (const auto *field : *mFields)
    {
        const TType *fieldType = field->type();
        if (IsSampler(fieldType->getBasicType()) || fieldType->isStructureContainingSamplers())
        {
            std::stringstream fieldName;
            fieldName << namePrefix << "_" << field->name();
            TString fieldApiName = apiNamePrefix + ".";
            fieldApiName += field->name().data();
            fieldType->createSamplerSymbols(ImmutableString(fieldName.str()), fieldApiName,
                                            outputSymbols, outputSymbolsToAPINames, symbolTable);
        }
    }
}

void TStructure::setName(const ImmutableString &name)
{
    ImmutableString *mutableName = const_cast<ImmutableString *>(&mName);
    *mutableName         = name;
}

TInterfaceBlock::TInterfaceBlock(TSymbolTable *symbolTable,
                                 const ImmutableString &name,
                                 const TFieldList *fields,
                                 const TLayoutQualifier &layoutQualifier,
                                 SymbolType symbolType,
                                 TExtension extension)
    : TSymbol(symbolTable, name, symbolType, extension),
      TFieldListCollection(fields),
      mBlockStorage(layoutQualifier.blockStorage),
      mBinding(layoutQualifier.binding)
{
    ASSERT(name != nullptr);
}

TFunctionBase::TFunctionBase(TSymbolTable *symbolTable,
                             const ImmutableString &name,
                             SymbolType symbolType,
                             TExtension extension,
                             const TType *returnType,
                             bool knownToNotHaveSideEffects)
    : TSymbol(symbolTable, name, symbolType, extension),
      mReturnType(returnType),
      mKnownToNotHaveSideEffects(knownToNotHaveSideEffects)
{
    // Functions with an empty name are not allowed.
    ASSERT(symbolType != SymbolType::Empty);
}

ImmutableString TFunctionBase::buildMangledName() const
{
    std::string newName(name().data(), name().length());
    newName += kFunctionMangledNameSeparator;

    size_t paramCount = getParamCount();
    for (size_t paramIndex = 0u; paramIndex < paramCount; ++paramIndex)
    {
        newName += getParam(paramIndex).type->getMangledName();
    }
    return ImmutableString(newName);
}

bool TFunctionBase::isMain() const
{
    return symbolType() == SymbolType::UserDefined && name() == kMainName;
}

bool TFunctionBase::isImageFunction() const
{
    return symbolType() == SymbolType::BuiltIn &&
           (name() == kImageSizeName || name() == kImageLoadName || name() == kImageStoreName);
}

TBuiltInFunction::TBuiltInFunction(TSymbolTable *symbolTable,
                                   const ImmutableString &name,
                                   SymbolType symbolType,
                                   TExtension extension,
                                   const TType *returnType,
                                   bool knownToNotHaveSideEffects,
                                   TOperator op,
                                   size_t paramCount,
                                   const ParamList &parameters)
    : TFunctionBase(symbolTable,
                    name,
                    symbolType,
                    extension,
                    returnType,
                    knownToNotHaveSideEffects),
      mMangledName(""),
      mOp(op),
      mParamCount(paramCount),
      mParameters(parameters)
{
    ASSERT(symbolType == SymbolType::BuiltIn);
    mMangledName = buildMangledName();
}

TFunction::TFunction(TSymbolTable *symbolTable,
                     const ImmutableString &name,
                     SymbolType symbolType,
                     const TType *returnType,
                     bool knownToNotHaveSideEffects)
    : TFunctionBase(symbolTable,
                    name,
                    symbolType,
                    TExtension::UNDEFINED,
                    returnType,
                    knownToNotHaveSideEffects),
      mMangledName(""),
      mDefined(false),
      mHasPrototypeDeclaration(false)
{
    ASSERT(symbolType != SymbolType::BuiltIn);
    ASSERT(name != nullptr || symbolType == SymbolType::AngleInternal);
}

void TFunction::clearParameters()
{
    mParameters.clear();
    mMangledName = ImmutableString("");
}

void TFunction::swapParameters(const TFunction &parametersSource)
{
    clearParameters();
    for (auto parameter : parametersSource.mParameters)
    {
        addParameter(parameter);
    }
}

}  // namespace sh
