//
// Copyright (c) 2002-2017 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//
// The ValidateVaryingLocations function checks if there exists location conflicts on shader
// varyings.
//

#include "ValidateVaryingLocations.h"

#include "compiler/translator/Diagnostics.h"
#include "compiler/translator/InfoSink.h"
#include "compiler/translator/IntermTraverse.h"
#include "compiler/translator/util.h"

namespace sh
{

namespace
{

void error(const TIntermSymbol &symbol, const char *reason, TDiagnostics *diagnostics)
{
    diagnostics->error(symbol.getLine(), reason, symbol.getSymbol().c_str());
}

class ValidateVaryingLocationsTraverser : public TIntermTraverser
{
  public:
    ValidateVaryingLocationsTraverser();
    void validate(TDiagnostics *diagnostics);

  private:
    using VaryingVector = std::vector<TIntermSymbol *>;

    void visitSymbol(TIntermSymbol *symbol) override;
    static void ValidateShaderInterface(TDiagnostics *diagnostics,
                                        const VaryingVector &varyingVector);
    static int GetLocationCount(const TIntermSymbol *varying);

    VaryingVector mInputVaryingsWithLocation;
    VaryingVector mOutputVaryingsWithLocation;
    std::set<std::string> mVisitedSymbols;
};

ValidateVaryingLocationsTraverser::ValidateVaryingLocationsTraverser()
    : TIntermTraverser(true, false, false)
{
}

void ValidateVaryingLocationsTraverser::visitSymbol(TIntermSymbol *symbol)
{
    TString name = symbol->getSymbol();
    if (mVisitedSymbols.find(name.c_str()) != mVisitedSymbols.end())
    {
        return;
    }

    mVisitedSymbols.insert(name.c_str());

    TQualifier qualifier = symbol->getQualifier();
    if (symbol->getType().getLayoutQualifier().location != -1)
    {
        if (IsVaryingIn(qualifier))
        {
            mInputVaryingsWithLocation.push_back(symbol);
        }
        else if (IsVaryingOut(qualifier))
        {
            mOutputVaryingsWithLocation.push_back(symbol);
        }
    }
}

int ValidateVaryingLocationsTraverser::GetLocationCount(const TIntermSymbol *varying)
{
    const auto &varyingType = varying->getType();
    if (varyingType.getStruct() != nullptr)
    {
        ASSERT(!varyingType.isArray());
        int totalLocation = 0;
        for (const auto *field : varyingType.getStruct()->fields())
        {
            const auto *fieldType = field->type();
            ASSERT(fieldType->getStruct() == nullptr && !fieldType->isArray());

            totalLocation += fieldType->getSecondarySize();
        }
        return totalLocation;
    }
    else
    {
        return varyingType.getSecondarySize() * static_cast<int>(varyingType.getArraySizeProduct());
    }
}

void ValidateVaryingLocationsTraverser::ValidateShaderInterface(TDiagnostics *diagnostics,
                                                                const VaryingVector &varyingVector)
{
    std::map<int, const TIntermSymbol *> locationMap;
    for (const TIntermSymbol *varying : varyingVector)
    {
        const int location = varying->getType().getLayoutQualifier().location;
        ASSERT(location >= 0);

        const int elementCount = GetLocationCount(varying);
        for (int elementIndex = 0; elementIndex < elementCount; ++elementIndex)
        {
            const int offsetLocation = location + elementIndex;
            if (locationMap.find(offsetLocation) != locationMap.end())
            {
                std::stringstream strstr;
                strstr << "'" << varying->getSymbol()
                       << "' conflicting location with previously defined '"
                       << locationMap[offsetLocation]->getSymbol() << "'";
                error(*varying, strstr.str().c_str(), diagnostics);
            }
            else
            {
                locationMap[offsetLocation] = varying;
            }
        }
    }
}

void ValidateVaryingLocationsTraverser::validate(TDiagnostics *diagnostics)
{
    ASSERT(diagnostics);

    if (mInputVaryingsWithLocation.size() > 1)
    {
        ValidateShaderInterface(diagnostics, mInputVaryingsWithLocation);
    }

    if (mOutputVaryingsWithLocation.size() > 1)
    {
        ValidateShaderInterface(diagnostics, mOutputVaryingsWithLocation);
    }
}

}  // anonymous namespace

bool ValidateVaryingLocations(TIntermBlock *root, TDiagnostics *diagnostics)
{
    ValidateVaryingLocationsTraverser varyingValidator;
    root->traverse(&varyingValidator);
    int numErrorsBefore = diagnostics->numErrors();
    varyingValidator.validate(diagnostics);
    return (diagnostics->numErrors() == numErrorsBefore);
}

}  // namespace sh