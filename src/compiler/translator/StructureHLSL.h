//
// Copyright (c) 2014 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//
// StructureHLSL.h:
//   HLSL translation of GLSL constructors and structures.
//

#ifndef COMPILER_TRANSLATOR_STRUCTUREHLSL_H_
#define COMPILER_TRANSLATOR_STRUCTUREHLSL_H_

#include "compiler/translator/Common.h"
#include "compiler/translator/IntermNode.h"

#include <set>

class TInfoSinkBase;
class TScopeBracket;

namespace sh
{

// This helper class assists structure and interface block definitions in determining
// how to pack std140 structs within HLSL's packing rules.
class Std140PaddingHelper
{
  public:
    explicit Std140PaddingHelper(const std::map<ImmutableString, int> &structElementIndexes,
                                 unsigned int *uniqueCounter);
    Std140PaddingHelper(const Std140PaddingHelper &other);
    Std140PaddingHelper &operator=(const Std140PaddingHelper &other);

    int elementIndex() const { return mElementIndex; }
    int prePadding(const TType &type);
    std::string prePaddingString(const TType &type);
    std::string postPaddingString(const TType &type, bool useHLSLRowMajorPacking);

  private:
    unsigned next();

    unsigned *mPaddingCounter;
    int mElementIndex;
    const std::map<ImmutableString, int> *mStructElementIndexes;
};

class StructureHLSL : angle::NonCopyable
{
  public:
    StructureHLSL();

    // Returns the name of the constructor function.
    ImmutableString addStructConstructor(const TStructure &structure);
    ImmutableString addBuiltInConstructor(const TType &type, const TIntermSequence *parameters);

    static ImmutableString defineNameless(const TStructure &structure);
    void ensureStructDefined(const TStructure &structure);

    void structsHeader(TInfoSinkBase &out) const;

    Std140PaddingHelper getPaddingHelper();

  private:
    unsigned mUniquePaddingCounter;

    std::map<ImmutableString, int> mStd140StructElementIndexes;

    struct TStructProperties : public angle::NonCopyable
    {
        POOL_ALLOCATOR_NEW_DELETE();

        TStructProperties() : constructor(nullptr) {}

        // Constructor is an empty string in case the struct doesn't have a constructor yet.
        ImmutableString constructor;
    };

    // Map from struct name to struct properties.
    typedef std::map<ImmutableString, TStructProperties *> DefinedStructs;
    DefinedStructs mDefinedStructs;

    // Struct declarations need to be kept in a vector instead of having them inside mDefinedStructs
    // since maintaining the original order is necessary for nested structs.
    typedef std::vector<ImmutableString> StructDeclarations;
    StructDeclarations mStructDeclarations;

    typedef std::set<std::string> BuiltInConstructors;
    BuiltInConstructors mBuiltInConstructors;

    void storeStd140ElementIndex(const TStructure &structure, bool useHLSLRowMajorPacking);
    ImmutableString defineQualified(const TStructure &structure,
                                    bool useHLSLRowMajorPacking,
                                    bool useStd140Packing);
    DefinedStructs::iterator defineVariants(const TStructure &structure,
                                            const ImmutableString &name);
};
}

#endif  // COMPILER_TRANSLATOR_STRUCTUREHLSL_H_
