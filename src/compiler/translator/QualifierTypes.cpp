//
// Copyright (c) 2002-2016 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//

#include "QualifierTypes.h"

#include "Diagnostics.h"

namespace
{

bool IsScopeQualifier(const TQualifierWrapperBase *qualifier)
{
    if (qualifier->getType() != QtStorage)
        return false;
    const TStorageQualifierWrapper *storageQualifier =
        static_cast<const TStorageQualifierWrapper *>(qualifier);
    TQualifier q = storageQualifier->getQualifier();
    return q == EvqGlobal || q == EvqTemporary;
}

// Returns true if the invariant for the qualifier sequence holds
bool IsInvariantCorrect(const std::vector<const TQualifierWrapperBase *> &qualifiers)
{
    // We should have at least one qualifier.
    // The first qualifier always tells the scope.
    return qualifiers.size() >= 1 && IsScopeQualifier(qualifiers[0]);
}

// Returns true if there are qualifiers which have been specified multiple times
bool HasRepeatingQualifiers(const std::vector<const TQualifierWrapperBase *> &qualifiers,
                            std::string *errorMessage)
{
    bool invariantFound     = false;
    bool precisionFound     = false;
    bool layoutFound        = false;
    bool interpolationFound = false;

    // The iteration starts from one since the first qualifier only reveals the scope of the
    // expression.
    // It is inserted first whenever the sequence gets created.
    for (size_t i = 1; i < qualifiers.size(); ++i)
    {
        switch (qualifiers[i]->getType())
        {
            case QtInvariant:
            {
                if (invariantFound)
                {
                    *errorMessage = "The invariant qualifier specified multiple times.";
                    return true;
                }
                invariantFound = true;
                break;
            }
            case QtPrecision:
            {
                if (precisionFound)
                {
                    *errorMessage = "The precision qualifier specified multiple times.";
                    return true;
                }
                precisionFound = true;
                break;
            }
            case QtLayout:
            {
                if (layoutFound)
                {
                    *errorMessage = "The layout qualifier specified multiple times.";
                    return true;
                }
                layoutFound = true;
                break;
            }
            case QtInterpolation:
            {
                // 'centroid' is treated as a storage qualifier
                // 'flat centroid' will be squashed to 'flat'
                // 'smooth centroid' will be squashed to 'centroid'
                if (interpolationFound)
                {
                    *errorMessage = "The interpolation qualifier specified multiple times.";
                    return true;
                }
                interpolationFound = true;
                break;
            }
            case QtStorage:
            {
                // Go over all of the storage qualifiers up until the current one and check for
                // repetitions.
                TQualifier currentQualifier =
                    static_cast<const TStorageQualifierWrapper *>(qualifiers[i])->getQualifier();
                for (size_t j = 1; j < i; ++j)
                {
                    if (qualifiers[j]->getType() == QtStorage)
                    {
                        const TStorageQualifierWrapper *previousQualifierWrapper =
                            static_cast<const TStorageQualifierWrapper *>(qualifiers[j]);
                        TQualifier previousQualifier = previousQualifierWrapper->getQualifier();
                        if (currentQualifier == previousQualifier)
                        {
                            *errorMessage = previousQualifierWrapper->getQualifierString().c_str();
                            *errorMessage += " specified multiple times";
                            return true;
                        }
                    }
                }
                break;
            }
            default:
                UNREACHABLE();
        }
    }

    return false;
}

// GLSL ES 3.00_6, 4.7 Order of Qualification
// The correct order of qualifiers is:
// invariant-qualifier interpolation-qualifier storage-qualifier precision-qualifier
// layout-qualifier has to be before storage-qualifier.
bool AreQualifiersInOrder(const std::vector<const TQualifierWrapperBase *> &qualifiers,
                          std::string *errorMessage)
{
    bool foundInterpolation = false;
    bool foundStorage       = false;
    bool foundPrecision     = false;
    for (size_t i = 1; i < qualifiers.size(); ++i)
    {
        switch (qualifiers[i]->getType())
        {
            case QtInvariant:
                if (foundInterpolation || foundStorage || foundPrecision)
                {
                    *errorMessage = "The invariant qualifier has to be first in the expression.";
                    return false;
                }
                break;
            case QtInterpolation:
                if (foundStorage)
                {
                    *errorMessage = "Storage qualifiers have to be after interpolation qualifiers.";
                    return false;
                }
                else if (foundPrecision)
                {
                    *errorMessage =
                        "Precision qualifiers have to be after interpolation qualifiers.";
                    return false;
                }
                foundInterpolation = true;
                break;
            case QtLayout:
                if (foundStorage)
                {
                    *errorMessage = "Storage qualifiers have to be after layout qualifiers.";
                    return false;
                }
                else if (foundPrecision)
                {
                    *errorMessage = "Precision qualifiers have to be after layout qualifiers.";
                    return false;
                }
                break;
            case QtStorage:
                if (foundPrecision)
                {
                    *errorMessage = "Precision qualifiers have to be after storage qualifiers.";
                    return false;
                }
                foundStorage = true;
                break;
            case QtPrecision:
                foundPrecision = true;
                break;
            default:
                UNREACHABLE();
        }
    }
    return true;
}

bool JoinSmooth(TQualifier *joinedQualifier)
{
    switch (*joinedQualifier)
    {
        case EvqGlobal:
            *joinedQualifier = EvqSmooth;
            break;
        default:
            return false;
    }
    return true;
}

bool JoinFlat(TQualifier *joinedQualifier)
{
    switch (*joinedQualifier)
    {
        case EvqGlobal:
            *joinedQualifier = EvqFlat;
            break;
        default:
            return false;
    }
    return true;
}

bool JoinUniform(TQualifier *joinedQualifier)
{
    switch (*joinedQualifier)
    {
        case EvqGlobal:
            *joinedQualifier = EvqUniform;
            break;
        default:
            return false;
    }
    return true;
}

bool JoinAttribute(TQualifier *joinedQualifier)
{
    switch (*joinedQualifier)
    {
        case EvqGlobal:
            *joinedQualifier = EvqAttribute;
            break;
        default:
            return false;
    }
    return true;
}

bool JoinVaryingIn(TQualifier *joinedQualifier)
{
    switch (*joinedQualifier)
    {
        case EvqGlobal:
            *joinedQualifier = EvqVaryingIn;
            break;
        default:
            return false;
    }
    return true;
}

bool JoinVaryingOut(TQualifier *joinedQualifier)
{
    switch (*joinedQualifier)
    {
        case EvqGlobal:
            *joinedQualifier = EvqVaryingOut;
            break;
        default:
            return false;
    }
    return true;
}

bool JoinCentroid(TQualifier *joinedQualifier)
{
    switch (*joinedQualifier)
    {
        case EvqGlobal:
        case EvqSmooth:
            *joinedQualifier = EvqCentroid;
            break;
        case EvqFlat:
            *joinedQualifier = EvqFlat;
            break;
        default:
            return false;
    }
    return true;
}

bool JoinVertexIn(TQualifier *joinedQualifier)
{
    switch (*joinedQualifier)
    {
        case EvqGlobal:
            *joinedQualifier = EvqVertexIn;
            break;
        default:
            return false;
    }
    return true;
}

bool JoinVertexOut(TQualifier *joinedQualifier)
{
    switch (*joinedQualifier)
    {
        case EvqGlobal:
            *joinedQualifier = EvqVertexOut;
            break;
        case EvqCentroid:
            *joinedQualifier = EvqCentroidOut;
            break;
        case EvqSmooth:
            *joinedQualifier = EvqSmoothOut;
            break;
        case EvqFlat:
            *joinedQualifier = EvqFlatOut;
            break;
        default:
            return false;
    }
    return true;
}

bool JoinFragmentIn(TQualifier *joinedQualifier)
{
    switch (*joinedQualifier)
    {
        case EvqGlobal:
            *joinedQualifier = EvqFragmentIn;
            break;
        case EvqSmooth:
            *joinedQualifier = EvqSmoothIn;
            break;
        case EvqFlat:
            *joinedQualifier = EvqFlatIn;
            break;
        case EvqCentroid:
            *joinedQualifier = EvqCentroidIn;
            break;
        default:
            return false;
    }
    return true;
}

bool JoinFragmentOut(TQualifier *joinedQualifier)
{
    switch (*joinedQualifier)
    {
        case EvqGlobal:
            *joinedQualifier = EvqFragmentOut;
            break;
        default:
            return false;
    }
    return true;
}

bool JoinComputeIn(TQualifier *joinedQualifier)
{
    switch (*joinedQualifier)
    {
        case EvqGlobal:
            *joinedQualifier = EvqComputeIn;
            break;
        default:
            return false;
    }
    return true;
}

bool JoinConst(TQualifier *joinedQualifier)
{
    switch (*joinedQualifier)
    {
        case EvqTemporary:
        case EvqGlobal:
            *joinedQualifier = EvqConst;
            break;
        default:
            return false;
    }
    return true;
}

bool JoinIn(TQualifier *joinedQualifier)
{
    switch (*joinedQualifier)
    {
        case EvqTemporary:
            *joinedQualifier = EvqIn;
            break;
        case EvqConst:
            *joinedQualifier = EvqConstReadOnly;
            break;
        default:
            return false;
    }
    return true;
}

bool JoinOut(TQualifier *joinedQualifier)
{
    switch (*joinedQualifier)
    {
        case EvqTemporary:
            *joinedQualifier = EvqOut;
            break;
        default:
            return false;
    }
    return true;
}

bool JoinInOut(TQualifier *joinedQualifier)
{
    switch (*joinedQualifier)
    {
        case EvqTemporary:
            *joinedQualifier = EvqInOut;
            break;
        default:
            return false;
    }
    return true;
}

}  // namespace

TQualifierSequence::TQualifierSequence(const TStorageQualifierWrapper *scope)
{
    ASSERT(scope->getQualifier() == EvqGlobal || scope->getQualifier() == EvqTemporary);
    mQualifiers.push_back(scope);
}

void TQualifierSequence::appendQualifier(const TQualifierWrapperBase *qualifier)
{
    mQualifiers.push_back(qualifier);
}

bool TQualifierSequence::checkOrderIsValid(TDiagnostics *diagnostics) const
{
    std::string errorMessage;
    if (HasRepeatingQualifiers(mQualifiers, &errorMessage))
    {
        diagnostics->error(mQualifiers[0]->getLine(), "qualifier sequence", errorMessage.c_str(),
                           "");
        return false;
    }

    if (!AreQualifiersInOrder(mQualifiers, &errorMessage))
    {
        diagnostics->error(mQualifiers[0]->getLine(), "qualifier sequence", errorMessage.c_str(),
                           "");
        return false;
    }

    return true;
}

void TQualifierSequence::getParameterQualifiers(TDiagnostics *diagnostics,
                                                TQualifier *outQualifier,
                                                TPrecision *outPrecision) const
{
    ASSERT(IsInvariantCorrect(mQualifiers));
    ASSERT(static_cast<const TStorageQualifierWrapper *>(mQualifiers[0])->getQualifier() ==
           EvqTemporary);

    *outQualifier = EvqTemporary;
    *outPrecision = EbpUndefined;

    if (!checkOrderIsValid(diagnostics))
    {
        return;
    }

    for (size_t i = 1; i < mQualifiers.size(); ++i)
    {
        const TQualifierWrapperBase *qualifier = mQualifiers[i];
        bool isQualifierValid                  = false;
        switch (qualifier->getType())
        {
            case QtInvariant:
            case QtInterpolation:
            case QtLayout:
                break;
            case QtStorage:
                isQualifierValid = joinParameterStorageQualifier(
                    outQualifier,
                    static_cast<const TStorageQualifierWrapper *>(qualifier)->getQualifier());
                break;
            case QtPrecision:
                isQualifierValid = true;
                *outPrecision =
                    static_cast<const TPrecisionQualifierWrapper *>(qualifier)->getQualifier();
                ASSERT(*outPrecision != EbpUndefined);
                break;
            default:
                UNREACHABLE();
        }
        if (!isQualifierValid)
        {
            const TString &qualifierString = qualifier->getQualifierString();
            diagnostics->error(qualifier->getLine(), "invalid parameter qualifier",
                               qualifierString.c_str(), "");
            break;
        }
    }

    switch (*outQualifier)
    {
        case EvqIn:
        case EvqConstReadOnly:  // const in
        case EvqOut:
        case EvqInOut:
            break;
        case EvqConst:
            *outQualifier = EvqConstReadOnly;
            break;
        case EvqTemporary:
            // no qualifier has been specified, set it to EvqIn which is the default
            *outQualifier = EvqIn;
            break;
        default:
            diagnostics->error(mQualifiers[0]->getLine(), "Invalid parameter qualifier ",
                               getQualifierString(*outQualifier), "");
            return;
    }
}

TPublicType TQualifierSequence::getPublicType(TDiagnostics *diagnostics) const
{
    ASSERT(IsInvariantCorrect(mQualifiers));

    // Initialize the public type so that even if the generation fails, the type is initialized.
    TPublicType publicType;
    publicType.setBasic(
        EbtVoid, static_cast<const TStorageQualifierWrapper *>(mQualifiers[0])->getQualifier(),
        mQualifiers[0]->getLine());

    if (!checkOrderIsValid(diagnostics))
    {
        return publicType;
    }

    for (size_t i = 1; i < mQualifiers.size(); ++i)
    {
        const TQualifierWrapperBase *qualifier = mQualifiers[i];
        bool isQualifierValid                  = false;
        switch (qualifier->getType())
        {
            case QtInvariant:
                isQualifierValid     = true;
                publicType.invariant = true;
                break;
            case QtInterpolation:
                isQualifierValid = joinInterpolationQualifier(
                    &publicType.qualifier,
                    static_cast<const TInterpolationQualifierWrapper *>(qualifier)->getQualifier());
                break;
            case QtLayout:
                isQualifierValid = true;
                publicType.layoutQualifier =
                    static_cast<const TLayoutQualifierWrapper *>(qualifier)->getQualifier();
                break;
            case QtStorage:
                isQualifierValid = joinVariableStorageQualifier(
                    &publicType.qualifier,
                    static_cast<const TStorageQualifierWrapper *>(qualifier)->getQualifier());
                break;
            case QtPrecision:
                isQualifierValid = true;
                publicType.precision =
                    static_cast<const TPrecisionQualifierWrapper *>(qualifier)->getQualifier();
                ASSERT(publicType.precision != EbpUndefined);
                break;
            default:
                UNREACHABLE();
        }
        if (!isQualifierValid)
        {
            const TString &qualifierString = qualifier->getQualifierString();
            diagnostics->error(qualifier->getLine(), "invalid qualifier combination",
                               qualifierString.c_str(), "");
            break;
        }
    }
    return publicType;
}

bool TQualifierSequence::joinInterpolationQualifier(TQualifier *joinedQualifier,
                                                    TQualifier interpolationQualifier) const
{
    switch (interpolationQualifier)
    {
        case EvqSmooth:
            if (!JoinSmooth(joinedQualifier))
            {
                return false;
            }
            break;
        case EvqFlat:
            if (!JoinFlat(joinedQualifier))
            {
                return false;
            }
            break;
        default:
            UNREACHABLE();
    }
    return true;
}

bool TQualifierSequence::joinVariableStorageQualifier(TQualifier *joinedQualifier,
                                                      TQualifier storageQualifier) const
{
    switch (storageQualifier)
    {
        case EvqUniform:
            if (!JoinUniform(joinedQualifier))
            {
                return false;
            }
            break;
        case EvqAttribute:
            if (!JoinAttribute(joinedQualifier))
            {
                return false;
            }
            break;
        case EvqVaryingIn:
            if (!JoinVaryingIn(joinedQualifier))
            {
                return false;
            }
            break;
        case EvqVaryingOut:
            if (!JoinVaryingOut(joinedQualifier))
            {
                return false;
            }
            break;
        case EvqCentroid:
        {
            if (!JoinCentroid(joinedQualifier))
            {
                return false;
            }
            break;
        }
        case EvqFragmentIn:
        {
            if (!JoinFragmentIn(joinedQualifier))
            {
                return false;
            }
            break;
        }
        case EvqVertexIn:
        {
            if (!JoinVertexIn(joinedQualifier))
            {
                return false;
            }
            break;
        }
        case EvqComputeIn:
        {
            if (!JoinComputeIn(joinedQualifier))
            {
                return false;
            }
            break;
        }
        case EvqFragmentOut:
        {
            if (!JoinFragmentOut(joinedQualifier))
            {
                return false;
            }
            break;
        }
        case EvqVertexOut:
        {
            if (!JoinVertexOut(joinedQualifier))
            {
                return false;
            }
            break;
        }
        case EvqConst:
            if (!JoinConst(joinedQualifier))
            {
                return false;
            }
            break;
        default:
            UNREACHABLE();
    }
    return true;
}

bool TQualifierSequence::joinParameterStorageQualifier(TQualifier *joinedQualifier,
                                                       TQualifier storageQualifier) const
{
    switch (storageQualifier)
    {
        case EvqConst:
            if (!JoinConst(joinedQualifier))
            {
                return false;
            }
            break;
        case EvqIn:
        {
            if (!JoinIn(joinedQualifier))
            {
                return false;
            }
            break;
        }
        case EvqOut:
        {
            if (!JoinOut(joinedQualifier))
            {
                return false;
            }
            break;
        }
        case EvqInOut:
        {
            if (!JoinInOut(joinedQualifier))
            {
                return false;
            }
            break;
        }
        default:
            return false;
    }
    return true;
}
