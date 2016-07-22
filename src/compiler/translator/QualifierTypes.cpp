//
// Copyright (c) 2002-2016 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//

#include "QualifierTypes.h"

#include "ParseContext.h"

namespace
{

// Returns true if there are qualifiers which have been specified multiple times
bool HasRepeatingQualifiers(const std::vector<const TQualifierWrapperBase *> &mQualifiers)
{
    bool invariantFound     = false;
    bool precisionFound     = false;
    bool layoutFound        = false;
    bool interpolationFound = false;

    for (size_t i = 1; i < mQualifiers.size(); ++i)
    {
        switch (mQualifiers[i]->getType())
        {
            case QtInvariant:
            {
                if (invariantFound)
                    return true;
                invariantFound = true;
                break;
            }
            case QtPrecision:
            {
                if (precisionFound)
                    return true;
                precisionFound = true;
                break;
            }
            case QtLayout:
            {
                if (layoutFound)
                    return true;
                layoutFound = true;
                break;
            }
            case QtInterpolation:
            {
                if (interpolationFound)
                    return true;
                interpolationFound = true;
                break;
            }
            case QtStorage:
            {
                // Go over all of the storage qualifiers up until the current one and check for
                // repetitions.
                for (size_t j = 0; j < i; ++j)
                {
                    TQualifier currentQualifier =
                        static_cast<const TStorageQualifierWrapper *>(mQualifiers[i])
                            ->getQualifier();
                    if (mQualifiers[j]->getType() == QtStorage)
                    {
                        TQualifier previousQualifier =
                            static_cast<const TStorageQualifierWrapper *>(mQualifiers[j])
                                ->getQualifier();
                        if (currentQualifier == previousQualifier)
                            return true;
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
bool AreQualifiersInOrder(const std::vector<const TQualifierWrapperBase *> &mQualifiers)
{
    bool foundInterpolation = false;
    bool foundStorage       = false;
    bool foundPrecision     = false;
    for (size_t i = 1; i < mQualifiers.size(); ++i)
    {
        switch (mQualifiers[i]->getType())
        {
            case QtInvariant:
                // No need to specify that we have found invariant as it has to be the first
                // qualifier anyway.
                if (foundInterpolation || foundStorage || foundPrecision)
                {
                    return false;
                }
                break;
            case QtInterpolation:
                if (foundStorage || foundPrecision)
                {
                    return false;
                }
                foundInterpolation = true;
                break;
            case QtLayout:
                // No need to specify that we have found layout as it can be anywhere to the left of
                // storage.
                if (foundStorage || foundPrecision)
                {
                    return false;
                }
                break;
            case QtStorage:
                if (foundPrecision)
                {
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

bool IsScopeQualifier(const TQualifierWrapperBase *qualifier)
{
    if (qualifier->getType() != QtStorage)
        return false;
    const TStorageQualifierWrapper *storageQualifier =
        static_cast<const TStorageQualifierWrapper *>(qualifier);
    TQualifier q = storageQualifier->getQualifier();
    return q == EvqGlobal || q == EvqTemporary;
}

bool JoinSmooth(const TQualifier leftQualifier, TQualifier *joinedQualifier)
{
    switch (leftQualifier)
    {
        case EvqGlobal:
            *joinedQualifier = EvqSmooth;
            break;
        default:
            return false;
    }
    return true;
}

bool JoinSmoothIn(const TQualifier leftQualifier, TQualifier *joinedQualifier)
{
    switch (leftQualifier)
    {
        case EvqGlobal:
            *joinedQualifier = EvqSmoothIn;
            break;
        default:
            return false;
    }
    return true;
}

bool JoinSmoothOut(const TQualifier leftQualifier, TQualifier *joinedQualifier)
{
    switch (leftQualifier)
    {
        case EvqGlobal:
            *joinedQualifier = EvqSmoothOut;
            break;
        default:
            return false;
    }
    return true;
}

bool JoinFlat(const TQualifier leftQualifier, TQualifier *joinedQualifier)
{
    switch (leftQualifier)
    {
        case EvqGlobal:
            *joinedQualifier = EvqFlat;
            break;
        default:
            return false;
    }
    return true;
}

bool JoinFlatIn(const TQualifier leftQualifier, TQualifier *joinedQualifier)
{
    switch (leftQualifier)
    {
        case EvqGlobal:
            *joinedQualifier = EvqFlatIn;
            break;
        default:
            return false;
    }
    return true;
}

bool JoinFlatOut(const TQualifier leftQualifier, TQualifier *joinedQualifier)
{
    switch (leftQualifier)
    {
        case EvqGlobal:
            *joinedQualifier = EvqFlatOut;
            break;
        default:
            return false;
    }
    return true;
}

bool JoinUniform(const TQualifier leftQualifier, TQualifier *joinedQualifier)
{
    switch (leftQualifier)
    {
        case EvqGlobal:
            *joinedQualifier = EvqUniform;
            break;
        default:
            return false;
    }
    return true;
}

bool JoinAttribute(const TQualifier leftQualifier, TQualifier *joinedQualifier)
{
    switch (leftQualifier)
    {
        case EvqGlobal:
            *joinedQualifier = EvqAttribute;
            break;
        default:
            return false;
    }
    return true;
}

bool JoinVaryingIn(const TQualifier leftQualifier, TQualifier *joinedQualifier)
{
    switch (leftQualifier)
    {
        case EvqGlobal:
            *joinedQualifier = EvqVaryingIn;
            break;
        default:
            return false;
    }
    return true;
}

bool JoinVaryingOut(const TQualifier leftQualifier, TQualifier *joinedQualifier)
{
    switch (leftQualifier)
    {
        case EvqGlobal:
            *joinedQualifier = EvqVaryingOut;
            break;
        default:
            return false;
    }
    return true;
}

bool JoinCentroid(const TQualifier leftQualifier, TQualifier *joinedQualifier)
{
    switch (leftQualifier)
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

bool JoinCentroidIn(const TQualifier leftQualifier, TQualifier *joinedQualifier)
{
    switch (leftQualifier)
    {
        case EvqGlobal:
        case EvqSmooth:
            *joinedQualifier = EvqCentroidIn;
            break;
        case EvqFlat:
            *joinedQualifier = EvqFlatIn;
            break;
        default:
            return false;
    }
    return true;
}

bool JoinCentroidOut(const TQualifier leftQualifier, TQualifier *joinedQualifier)
{
    switch (leftQualifier)
    {
        case EvqGlobal:
        case EvqSmooth:
            // GLSL 3.00, Revision 6, 4.3.9 Interpolation
            // When no interpolation qualifier is present, smooth interpolation is used.
            // This meaning that smooth is the default.
            *joinedQualifier = EvqCentroidOut;
            break;
        case EvqFlat:
            // GLSL 3.00, Revision 6, 4.3.9 Interpolation
            // A variable may be qualified as flat centroid, which will mean the same thing as
            // qualifying it only as flat.
            *joinedQualifier = EvqFlatOut;
            break;
        default:
            return false;
    }
    return true;
}

bool JoinVertexIn(const TQualifier leftQualifier, TQualifier *joinedQualifier)
{
    switch (leftQualifier)
    {
        case EvqGlobal:
            *joinedQualifier = EvqVertexIn;
            break;
        default:
            return false;
    }
    return true;
}

bool JoinVertexOut(const TQualifier leftQualifier, TQualifier *joinedQualifier)
{
    switch (leftQualifier)
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

bool JoinFragmentIn(const TQualifier leftQualifier, TQualifier *joinedQualifier)
{
    switch (leftQualifier)
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

bool JoinFragmentOut(const TQualifier leftQualifier, TQualifier *joinedQualifier)
{
    switch (leftQualifier)
    {
        case EvqGlobal:
            *joinedQualifier = EvqFragmentOut;
            break;
        default:
            return false;
    }
    return true;
}

bool JoinComputeIn(const TQualifier leftQualifier, TQualifier *joinedQualifier)
{
    switch (leftQualifier)
    {
        case EvqGlobal:
            *joinedQualifier = EvqComputeIn;
            break;
        default:
            return false;
    }
    return true;
}

bool JoinConst(const TQualifier leftQualifier, TQualifier *joinedQualifier)
{
    switch (leftQualifier)
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

bool JoinIn(const TQualifier leftQualifier, TQualifier *joinedQualifier)
{
    switch (leftQualifier)
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

bool JoinOut(const TQualifier leftQualifier, TQualifier *joinedQualifier)
{
    switch (leftQualifier)
    {
        case EvqTemporary:
            *joinedQualifier = EvqOut;
            break;
        default:
            return false;
    }
    return true;
}

bool JoinInOut(const TQualifier leftQualifier, TQualifier *joinedQualifier)
{
    switch (leftQualifier)
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

void TQualifierSequence::joinQualifier(const TQualifierWrapperBase *qualifier)
{
    mQualifiers.push_back(qualifier);
}

TPublicType TQualifierSequence::getPublicType(TParseContext *context) const
{
    // We should have at least one qualifier.
    ASSERT(mQualifiers.size() >= 1);

    // The first qualifier always tells the scope.
    ASSERT(IsScopeQualifier(mQualifiers[0]));

    // Initialize the public type so that even if the generation fails, the type is initialized.
    TPublicType publicType;
    publicType.setBasic(
        EbtVoid, static_cast<const TStorageQualifierWrapper *>(mQualifiers[0])->getQualifier(),
        mQualifiers[0]->getLine());

    if (HasRepeatingQualifiers(mQualifiers))
    {
        context->error(mQualifiers[0]->getLine(), "qualifier sequence",
                       "expression has repeating qualifiers");
        return publicType;
    }

    if (!AreQualifiersInOrder(mQualifiers))
    {
        context->error(mQualifiers[0]->getLine(), "qualifier sequence", "wrong order");
        return publicType;
    }

    for (size_t i = 1; i < mQualifiers.size(); ++i)
    {
        const TQualifierWrapperBase *qualifier = mQualifiers[i];
        bool tempResult                        = false;
        switch (qualifier->getType())
        {
            case QtInvariant:
                tempResult = joinInvariantQualifier(&publicType);
                break;
            case QtInterpolation:
                tempResult = joinInterpolationQualifier(
                    &publicType,
                    static_cast<const TInterpolationQualifierWrapper *>(qualifier)->getQualifier());
                break;
            case QtLayout:
                tempResult = joinLayoutQualifier(
                    &publicType,
                    static_cast<const TLayoutQualifierWrapper *>(qualifier)->getQualifier());
                break;
            case QtStorage:
                tempResult = joinStorageQualifier(
                    &publicType,
                    static_cast<const TStorageQualifierWrapper *>(qualifier)->getQualifier());
                break;
            case QtPrecision:
                tempResult = joinPrecisionQualifier(
                    &publicType,
                    static_cast<const TPrecisionQualifierWrapper *>(qualifier)->getQualifier());
                break;
            default:
                UNREACHABLE();
        }
        if (!tempResult)
        {
            const TString &qualifierString = qualifier->getQualifierString();
            context->error(qualifier->getLine(), "invalid qualifier combination",
                           qualifierString.c_str());
            break;
        }
    }
    return publicType;
}

bool TQualifierSequence::joinInvariantQualifier(TPublicType *publicType) const
{
    publicType->invariant = true;
    return true;
}

bool TQualifierSequence::joinInterpolationQualifier(TPublicType *publicType,
                                                    TQualifier interpolationQualifier) const
{
    switch (interpolationQualifier)
    {
        case EvqSmooth:
            if (!JoinSmooth(publicType->qualifier, &publicType->qualifier))
            {
                return false;
            }
            break;
        case EvqFlat:
            if (!JoinFlat(publicType->qualifier, &publicType->qualifier))
            {
                return false;
            }
            break;
        default:
            UNREACHABLE();
    }
    return true;
}

bool TQualifierSequence::joinStorageQualifier(TPublicType *publicType,
                                              TQualifier storageQualifier) const
{
    switch (storageQualifier)
    {
        case EvqUniform:
            if (!JoinUniform(publicType->qualifier, &publicType->qualifier))
            {
                return false;
            }
            break;
        case EvqAttribute:
            if (!JoinAttribute(publicType->qualifier, &publicType->qualifier))
            {
                return false;
            }
            break;
        case EvqVaryingIn:
            if (!JoinVaryingIn(publicType->qualifier, &publicType->qualifier))
            {
                return false;
            }
            break;
        case EvqVaryingOut:
            if (!JoinVaryingOut(publicType->qualifier, &publicType->qualifier))
            {
                return false;
            }
            break;
        case EvqCentroid:
        {
            if (!JoinCentroid(publicType->qualifier, &publicType->qualifier))
            {
                return false;
            }
            break;
        }
        case EvqFragmentIn:
        {
            if (!JoinFragmentIn(publicType->qualifier, &publicType->qualifier))
            {
                return false;
            }
            break;
        }
        case EvqVertexIn:
        {
            if (!JoinVertexIn(publicType->qualifier, &publicType->qualifier))
            {
                return false;
            }
            break;
        }
        case EvqComputeIn:
        {
            if (!JoinComputeIn(publicType->qualifier, &publicType->qualifier))
            {
                return false;
            }
            break;
        }
        case EvqFragmentOut:
        {
            if (!JoinFragmentOut(publicType->qualifier, &publicType->qualifier))
            {
                return false;
            }
            break;
        }
        case EvqVertexOut:
        {
            if (!JoinVertexOut(publicType->qualifier, &publicType->qualifier))
            {
                return false;
            }
            break;
        }
        // already combined types
        case EvqCentroidOut:
        {
            if (!JoinCentroidOut(publicType->qualifier, &publicType->qualifier))
            {
                return false;
            }
            break;
        }
        case EvqCentroidIn:
        {
            if (!JoinCentroidIn(publicType->qualifier, &publicType->qualifier))
            {
                return false;
            }
            break;
        }
        case EvqFlatIn:
        {
            if (!JoinCentroidIn(publicType->qualifier, &publicType->qualifier))
            {
                return false;
            }
            break;
        }
        case EvqSmoothIn:
        {
            if (!JoinSmoothIn(publicType->qualifier, &publicType->qualifier))
            {
                return false;
            }
            break;
        }
        case EvqFlatOut:
        {
            if (!JoinFlatOut(publicType->qualifier, &publicType->qualifier))
            {
                return false;
            }
            break;
        }
        case EvqSmoothOut:
        {
            if (!JoinSmoothOut(publicType->qualifier, &publicType->qualifier))
            {
                return false;
            }
            break;
        }
        // function parameter qualifiers
        case EvqConst:
            if (!JoinConst(publicType->qualifier, &publicType->qualifier))
            {
                return false;
            }
            break;
        case EvqIn:
        {
            if (!JoinIn(publicType->qualifier, &publicType->qualifier))
            {
                return false;
            }
            break;
        }
        case EvqOut:
        {
            if (!JoinOut(publicType->qualifier, &publicType->qualifier))
            {
                return false;
            }
            break;
        }
        case EvqInOut:
        {
            if (!JoinInOut(publicType->qualifier, &publicType->qualifier))
            {
                return false;
            }
            break;
        }
        default:
            UNREACHABLE();
    }
    return true;
}

bool TQualifierSequence::joinPrecisionQualifier(TPublicType *publicType,
                                                TPrecision precisionQualifier) const
{
    if (precisionQualifier != EbpUndefined)
    {
        publicType->precision = precisionQualifier;
    }
    return true;
}

bool TQualifierSequence::joinLayoutQualifier(TPublicType *publicType,
                                             const TLayoutQualifier &layoutQualifier) const
{
    if (!layoutQualifier.isEmpty())
    {
        publicType->layoutQualifier = layoutQualifier;
    }
    return true;
}
