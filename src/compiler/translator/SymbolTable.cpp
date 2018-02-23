//
// Copyright (c) 2002-2013 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//
// Symbol table for parsing. The design principles and most of the functionality are documented in
// the header file.
//

#if defined(_MSC_VER)
#pragma warning(disable : 4718)
#endif

#include "compiler/translator/SymbolTable.h"

#include <algorithm>
#include <set>

#include "angle_gl.h"
#include "compiler/translator/BuiltIn_autogen.h"
#include "compiler/translator/ImmutableString.h"
#include "compiler/translator/IntermNode.h"
#include "compiler/translator/StaticType.h"

namespace sh
{

class TSymbolTable::TSymbolTableLevel
{
  public:
    TSymbolTableLevel() : mGlobalInvariant(false) {}

    bool insert(TSymbol *symbol);

    // Insert a function using its unmangled name as the key.
    void insertUnmangled(TFunction *function);

    TSymbol *find(const ImmutableString &name) const;

    void addInvariantVarying(const ImmutableString &name) { mInvariantVaryings.insert(name); }

    bool isVaryingInvariant(const ImmutableString &name)
    {
        return (mGlobalInvariant || mInvariantVaryings.count(name) > 0);
    }

    void setGlobalInvariant(bool invariant) { mGlobalInvariant = invariant; }

  private:
    using tLevel        = TUnorderedMap<ImmutableString,
                                 TSymbol *,
                                 ImmutableString::FowlerNollVoHash<sizeof(size_t)>>;
    using tLevelPair    = const tLevel::value_type;
    using tInsertResult = std::pair<tLevel::iterator, bool>;

    tLevel level;

    std::set<ImmutableString> mInvariantVaryings;
    bool mGlobalInvariant;
};

class TSymbolTable::TSymbolTableBuiltInLevel
{
  public:
    TSymbolTableBuiltInLevel() = default;

    bool insert(const TSymbol *symbol);

    const TSymbol *find(const ImmutableString &name) const;

    void insertUnmangledBuiltIn(const ImmutableString &name, TExtension ext);

  private:
    using tLevel        = TUnorderedMap<ImmutableString,
                                 const TSymbol *,
                                 ImmutableString::FowlerNollVoHash<sizeof(size_t)>>;
    using tLevelPair    = const tLevel::value_type;
    using tInsertResult = std::pair<tLevel::iterator, bool>;

    tLevel mLevel;
};

bool TSymbolTable::TSymbolTableLevel::insert(TSymbol *symbol)
{
    // returning true means symbol was added to the table
    tInsertResult result = level.insert(tLevelPair(symbol->getMangledName(), symbol));
    return result.second;
}

void TSymbolTable::TSymbolTableLevel::insertUnmangled(TFunction *function)
{
    level.insert(tLevelPair(function->name(), function));
}

TSymbol *TSymbolTable::TSymbolTableLevel::find(const ImmutableString &name) const
{
    tLevel::const_iterator it = level.find(name);
    if (it == level.end())
        return nullptr;
    else
        return (*it).second;
}

bool TSymbolTable::TSymbolTableBuiltInLevel::insert(const TSymbol *symbol)
{
    // returning true means symbol was added to the table
    tInsertResult result = mLevel.insert(tLevelPair(symbol->getMangledName(), symbol));
    return result.second;
}

const TSymbol *TSymbolTable::TSymbolTableBuiltInLevel::find(const ImmutableString &name) const
{
    tLevel::const_iterator it = mLevel.find(name);
    if (it == mLevel.end())
        return nullptr;
    else
        return (*it).second;
}

TSymbolTable::TSymbolTable() : mUniqueIdCounter(0), mUserDefinedUniqueIdsStart(-1)
{
}

TSymbolTable::~TSymbolTable() = default;

void TSymbolTable::pushBuiltInLevel()
{
    mBuiltInTable.push_back(
        std::unique_ptr<TSymbolTableBuiltInLevel>(new TSymbolTableBuiltInLevel));
}

void TSymbolTable::push()
{
    mTable.push_back(std::unique_ptr<TSymbolTableLevel>(new TSymbolTableLevel));
    mPrecisionStack.push_back(std::unique_ptr<PrecisionStackLevel>(new PrecisionStackLevel));
}

void TSymbolTable::pop()
{
    mTable.pop_back();
    mPrecisionStack.pop_back();
}

const TFunction *TSymbolTable::markFunctionHasPrototypeDeclaration(
    const ImmutableString &mangledName,
    bool *hadPrototypeDeclarationOut)
{
    TFunction *function         = findUserDefinedFunction(mangledName);
    *hadPrototypeDeclarationOut = function->hasPrototypeDeclaration();
    function->setHasPrototypeDeclaration();
    return function;
}

const TFunction *TSymbolTable::setFunctionParameterNamesFromDefinition(const TFunction *function,
                                                                       bool *wasDefinedOut)
{
    TFunction *firstDeclaration = findUserDefinedFunction(function->getMangledName());
    ASSERT(firstDeclaration);
    // Note: 'firstDeclaration' could be 'function' if this is the first time we've seen function as
    // it would have just been put in the symbol table. Otherwise, we're looking up an earlier
    // occurance.
    if (function != firstDeclaration)
    {
        // The previous declaration should have the same parameters as the function definition
        // (parameter names may differ).
        firstDeclaration->shareParameters(*function);
    }

    *wasDefinedOut = firstDeclaration->isDefined();
    firstDeclaration->setDefined();
    return firstDeclaration;
}

const TSymbol *TSymbolTable::find(const ImmutableString &name, int shaderVersion) const
{
    int userDefinedLevel = static_cast<int>(mTable.size()) - 1;
    while (userDefinedLevel >= 0)
    {
        const TSymbol *symbol = mTable[userDefinedLevel]->find(name);
        if (symbol)
        {
            return symbol;
        }
        userDefinedLevel--;
    }

    return findBuiltIn(name, shaderVersion, false);
}

TFunction *TSymbolTable::findUserDefinedFunction(const ImmutableString &name) const
{
    // User-defined functions are always declared at the global level.
    ASSERT(!mTable.empty());
    return static_cast<TFunction *>(mTable[0]->find(name));
}

const TSymbol *TSymbolTable::findGlobal(const ImmutableString &name) const
{
    ASSERT(!mTable.empty());
    return mTable[0]->find(name);
}

const TSymbol *TSymbolTable::findBuiltIn(const ImmutableString &name, int shaderVersion) const
{
    return findBuiltIn(name, shaderVersion, false);
}

const TSymbol *TSymbolTable::findBuiltIn(const ImmutableString &name,
                                         int shaderVersion,
                                         bool includeGLSLBuiltins) const
{
    for (int level = LAST_BUILTIN_LEVEL; level >= 0; level--)
    {
        if (level == GLSL_BUILTINS && !includeGLSLBuiltins)
            level--;
        if (level == ESSL3_1_BUILTINS && shaderVersion != 310)
            level--;
        if (level == ESSL3_BUILTINS && shaderVersion < 300)
            level--;
        if (level == ESSL1_BUILTINS && shaderVersion != 100)
            level--;

        const TSymbol *symbol = mBuiltInTable[level]->find(name);

        if (symbol)
            return symbol;
    }

    return nullptr;
}

bool TSymbolTable::declare(TSymbol *symbol)
{
    ASSERT(!mTable.empty());
    ASSERT(symbol->symbolType() == SymbolType::UserDefined);
    ASSERT(!symbol->isFunction());
    return mTable.back()->insert(symbol);
}

void TSymbolTable::declareUserDefinedFunction(TFunction *function, bool insertUnmangledName)
{
    ASSERT(!mTable.empty());
    if (insertUnmangledName)
    {
        // Insert the unmangled name to detect potential future redefinition as a variable.
        mTable[0]->insertUnmangled(function);
    }
    mTable[0]->insert(function);
}

void TSymbolTable::insertVariable(ESymbolLevel level,
                                  const ImmutableString &name,
                                  const TType *type)
{
    ASSERT(type->isRealized());
    TVariable *var = new TVariable(this, name, type, SymbolType::BuiltIn);
    insertBuiltIn(level, var);
}

void TSymbolTable::insertVariableExt(ESymbolLevel level,
                                     TExtension ext,
                                     const ImmutableString &name,
                                     const TType *type)
{
    ASSERT(type->isRealized());
    TVariable *var = new TVariable(this, name, type, SymbolType::BuiltIn, ext);
    insertBuiltIn(level, var);
}

void TSymbolTable::insertBuiltIn(ESymbolLevel level, const TSymbol *symbol)
{
    ASSERT(level <= LAST_BUILTIN_LEVEL);
    ASSERT(symbol);
    mBuiltInTable[level]->insert(symbol);
}

void TSymbolTable::setDefaultPrecision(TBasicType type, TPrecision prec)
{
    int indexOfLastElement = static_cast<int>(mPrecisionStack.size()) - 1;
    // Uses map operator [], overwrites the current value
    (*mPrecisionStack[indexOfLastElement])[type] = prec;
}

TPrecision TSymbolTable::getDefaultPrecision(TBasicType type) const
{
    if (!SupportsPrecision(type))
        return EbpUndefined;

    // unsigned integers use the same precision as signed
    TBasicType baseType = (type == EbtUInt) ? EbtInt : type;

    int level = static_cast<int>(mPrecisionStack.size()) - 1;
    ASSERT(level >= 0);  // Just to be safe. Should not happen.
    // If we dont find anything we return this. Some types don't have predefined default precision.
    TPrecision prec = EbpUndefined;
    while (level >= 0)
    {
        PrecisionStackLevel::iterator it = mPrecisionStack[level]->find(baseType);
        if (it != mPrecisionStack[level]->end())
        {
            prec = (*it).second;
            break;
        }
        level--;
    }
    return prec;
}

void TSymbolTable::addInvariantVarying(const ImmutableString &originalName)
{
    ASSERT(atGlobalLevel());
    mTable.back()->addInvariantVarying(originalName);
}

bool TSymbolTable::isVaryingInvariant(const ImmutableString &originalName) const
{
    ASSERT(atGlobalLevel());
    return mTable.back()->isVaryingInvariant(originalName);
}

void TSymbolTable::setGlobalInvariant(bool invariant)
{
    ASSERT(atGlobalLevel());
    mTable.back()->setGlobalInvariant(invariant);
}

void TSymbolTable::markBuiltInInitializationFinished()
{
    mUserDefinedUniqueIdsStart = mUniqueIdCounter;
}

void TSymbolTable::clearCompilationResults()
{
    mUniqueIdCounter = mUserDefinedUniqueIdsStart;

    // User-defined scopes should have already been cleared when the compilation finished.
    ASSERT(mTable.size() == 0u);
}

int TSymbolTable::nextUniqueIdValue()
{
    ASSERT(mUniqueIdCounter < std::numeric_limits<int>::max());
    return ++mUniqueIdCounter;
}

void TSymbolTable::initializeBuiltIns(sh::GLenum type,
                                      ShShaderSpec spec,
                                      const ShBuiltInResources &resources)
{
    mShaderType = type;

    ASSERT(isEmpty());
    pushBuiltInLevel();  // COMMON_BUILTINS
    pushBuiltInLevel();  // ESSL1_BUILTINS
    pushBuiltInLevel();  // ESSL3_BUILTINS
    pushBuiltInLevel();  // ESSL3_1_BUILTINS
    pushBuiltInLevel();  // GLSL_BUILTINS

    // We need just one precision stack level for predefined precisions.
    mPrecisionStack.push_back(std::unique_ptr<PrecisionStackLevel>(new PrecisionStackLevel));

    switch (type)
    {
        case GL_FRAGMENT_SHADER:
            setDefaultPrecision(EbtInt, EbpMedium);
            break;
        case GL_VERTEX_SHADER:
        case GL_COMPUTE_SHADER:
        case GL_GEOMETRY_SHADER_EXT:
            setDefaultPrecision(EbtInt, EbpHigh);
            setDefaultPrecision(EbtFloat, EbpHigh);
            break;
        default:
            UNREACHABLE();
    }
    // Set defaults for sampler types that have default precision, even those that are
    // only available if an extension exists.
    // New sampler types in ESSL3 don't have default precision. ESSL1 types do.
    initSamplerDefaultPrecision(EbtSampler2D);
    initSamplerDefaultPrecision(EbtSamplerCube);
    // SamplerExternalOES is specified in the extension to have default precision.
    initSamplerDefaultPrecision(EbtSamplerExternalOES);
    // SamplerExternal2DY2YEXT is specified in the extension to have default precision.
    initSamplerDefaultPrecision(EbtSamplerExternal2DY2YEXT);
    // It isn't specified whether Sampler2DRect has default precision.
    initSamplerDefaultPrecision(EbtSampler2DRect);

    setDefaultPrecision(EbtAtomicCounter, EbpHigh);

    insertStaticBuiltInVariables(type, spec, resources);
    insertStaticBuiltInFunctions(type);
    mUniqueIdCounter = kLastStaticBuiltInId + 1;

    initializeBuiltInVariables(type, spec, resources);
    markBuiltInInitializationFinished();
}

void TSymbolTable::initSamplerDefaultPrecision(TBasicType samplerType)
{
    ASSERT(samplerType > EbtGuardSamplerBegin && samplerType < EbtGuardSamplerEnd);
    setDefaultPrecision(samplerType, EbpLow);
}

void TSymbolTable::initializeBuiltInVariables(sh::GLenum type,
                                              ShShaderSpec spec,
                                              const ShBuiltInResources &resources)
{
    // Here we initialize built-ins that involve structs or interface blocks, as we don't have
    // support for them in the symbol table code generation scripts yet.

    const TSourceLoc zeroSourceLoc = {0, 0, 0, 0};

    // Depth range in window coordinates
    TFieldList *fields = new TFieldList();
    auto highpFloat1   = new TType(EbtFloat, EbpHigh, EvqGlobal, 1);
    TField *near       = new TField(highpFloat1, ImmutableString("near"), zeroSourceLoc);
    TField *far        = new TField(highpFloat1, ImmutableString("far"), zeroSourceLoc);
    TField *diff       = new TField(highpFloat1, ImmutableString("diff"), zeroSourceLoc);
    fields->push_back(near);
    fields->push_back(far);
    fields->push_back(diff);
    TStructure *depthRangeStruct = new TStructure(this, ImmutableString("gl_DepthRangeParameters"),
                                                  fields, SymbolType::BuiltIn);
    insertBuiltIn(COMMON_BUILTINS, depthRangeStruct);
    TType *depthRangeType = new TType(depthRangeStruct);
    depthRangeType->setQualifier(EvqUniform);
    depthRangeType->realize();
    insertVariable(COMMON_BUILTINS, ImmutableString("gl_DepthRange"), depthRangeType);

    switch (type)
    {
        case GL_GEOMETRY_SHADER_EXT:
        {
            TExtension extension = TExtension::EXT_geometry_shader;

            // Add built-in interface block gl_PerVertex and the built-in array gl_in.
            // TODO(jiawei.shao@intel.com): implement GL_EXT_geometry_point_size.
            TFieldList *glPerVertexFieldList = new TFieldList();
            const TType *positionType = StaticType::Get<EbtFloat, EbpHigh, EvqPosition, 4, 1>();
            TField *glPositionField =
                new TField(new TType(*positionType), ImmutableString("gl_Position"), zeroSourceLoc);
            glPerVertexFieldList->push_back(glPositionField);

            const ImmutableString glPerVertexString("gl_PerVertex");
            TInterfaceBlock *glPerVertexInBlock =
                new TInterfaceBlock(this, glPerVertexString, glPerVertexFieldList,
                                    TLayoutQualifier::Create(), SymbolType::BuiltIn, extension);
            insertBuiltIn(ESSL3_1_BUILTINS, glPerVertexInBlock);

            // The array size of gl_in is undefined until we get a valid input primitive
            // declaration.
            TType *glInType =
                new TType(glPerVertexInBlock, EvqPerVertexIn, TLayoutQualifier::Create());
            glInType->makeArray(0u);
            glInType->realize();
            insertVariableExt(ESSL3_1_BUILTINS, extension, ImmutableString("gl_in"), glInType);

            TInterfaceBlock *glPerVertexOutBlock =
                new TInterfaceBlock(this, glPerVertexString, glPerVertexFieldList,
                                    TLayoutQualifier::Create(), SymbolType::BuiltIn);
            TType *glPositionInBlockType = new TType(EbtFloat, EbpHigh, EvqPosition, 4);
            glPositionInBlockType->setInterfaceBlock(glPerVertexOutBlock);
            glPositionInBlockType->realize();
            insertVariableExt(ESSL3_1_BUILTINS, extension, ImmutableString("gl_Position"),
                              glPositionInBlockType);

            break;
        }
        default:
            ASSERT(type == GL_VERTEX_SHADER || type == GL_FRAGMENT_SHADER ||
                   type == GL_COMPUTE_SHADER);
    }
}

}  // namespace sh
