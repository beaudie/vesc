//
// Copyright 2020 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//
// DriverUniformMetal:
//   Struct defining the default driver uniforms for direct and SpirV based ANGLE translation
//

#include "compiler/translator/DriverUniformMetal.h"
#include "compiler/translator/tree_util/BuiltIn.h"
#include "compiler/translator/tree_util/DriverUniform.h"
#include "compiler/translator/tree_util/IntermNode_util.h"

namespace sh
{

namespace
{

// Metal specific driver uniforms
constexpr const char kEmulatedInstanceID[] = "emulatedInstanceID";
constexpr const char kCoverageMask[]       = "coverageMask";

}  // namespace

// class DriverUniformMetal
// The fields here must match the DriverUniforms structure defined in ContextMtl.h.
TFieldList *DriverUniformMetal::createUniformFields(TSymbolTable *symbolTable)
{
    TFieldList *driverFieldList = DriverUniform::createUniformFields(symbolTable);

    constexpr size_t kNumGraphicsDriverUniformsMetal = 2;
    constexpr std::array<const char *, kNumGraphicsDriverUniformsMetal>
        kGraphicsDriverUniformNamesMetal = {{kEmulatedInstanceID, kCoverageMask}};

    const std::array<TType *, kNumGraphicsDriverUniformsMetal> kDriverUniformTypesMetal = {{
        new TType(EbtUInt, EbpHigh,
                  EvqGlobal),  // kEmulatedInstanceID - unused in SPIR-V Metal compiler
        new TType(EbtUInt, EbpHigh, EvqGlobal),  // kCoverageMask
    }};

    for (size_t uniformIndex = 0; uniformIndex < kNumGraphicsDriverUniformsMetal; ++uniformIndex)
    {
        TField *driverUniformField =
            new TField(kDriverUniformTypesMetal[uniformIndex],
                       ImmutableString(kGraphicsDriverUniformNamesMetal[uniformIndex]),
                       TSourceLoc(), SymbolType::AngleInternal);
        driverFieldList->push_back(driverUniformField);
    }

    return driverFieldList;
}

TIntermBinary *DriverUniformMetal::getEmulatedInstanceId() const
{
    return createDriverUniformRef(kEmulatedInstanceID);
}

TIntermBinary *DriverUniformMetal::getCoverageMaskFieldRef() const
{
    return createDriverUniformRef(kCoverageMask);
}

}  // namespace sh
