//
// Copyright (c) 2002-2016 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//

#include "QualifierTypes.h"

#include "Diagnostics.h"

namespace
{

bool IsScopeQualifier(TQualifier qualifier)
{
    return qualifier == EvqGlobal || qualifier == EvqTemporary;
}

bool IsScopeQualifierWrapper(const TQualifierWrapperBase *qualifier)
{
    if (qualifier->getType() != QtStorage)
        return false;
    const TStorageQualifierWrapper *storageQualifier =
        static_cast<const TStorageQualifierWrapper *>(qualifier);
    TQualifier q = storageQualifier->getQualifier();
    return IsScopeQualifier(q);
}

// Returns true if the invariant for the qualifier sequence holds
bool IsInvariantCorrect(const std::vector<const TQualifierWrapperBase *> &qualifiers)
{
    // We should have at least one qualifier.
    // The first qualifier always tells the scope.
    return qualifiers.size() >= 1 && IsScopeQualifierWrapper(qualifiers[0]);
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
    // expression. It is inserted first whenever the sequence gets created.
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
            case QtMemory:
            {
                // Go over all of the memory qualifiers up until the current one and check for
                // repetitions.
                // Having both readonly and writeonly in a sequence is valid.
                // GLSL ES 3.10 Revision 4, 4.9 Memory Access Qualifiers
                TQualifier currentQualifier =
                    static_cast<const TMemoryQualifierWrapper *>(qualifiers[i])->getQualifier();
                for (size_t j = 1; j < i; ++j)
                {
                    if (qualifiers[j]->getType() == QtMemory)
                    {
                        const TMemoryQualifierWrapper *previousQualifierWrapper =
                            static_cast<const TMemoryQualifierWrapper *>(qualifiers[j]);
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
    bool foundMemory        = false;
    for (size_t i = 1; i < qualifiers.size(); ++i)
    {
        switch (qualifiers[i]->getType())
        {
            case QtInvariant:
                if (foundInterpolation || foundStorage || foundPrecision || foundMemory)
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
                if (foundMemory)
                {
                    *errorMessage = "Memory qualifiers have to be after interpolation qualifiers.";
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
                if (foundMemory)
                {
                    *errorMessage = "Memory qualifiers have to be after layout qualifiers.";
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
            case QtMemory:
                if (foundPrecision)
                {
                    *errorMessage = "Precision qualifiers have to be after memory qualifiers.";
                    return false;
                }
                foundMemory = true;
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

}  // namespace

TTypeQualifier::TTypeQualifier(TQualifier scope, const TSourceLoc &loc)
    : layoutQualifier(TLayoutQualifier::create()),
      memoryQualifier(TMemoryQualifier::create()),
      precision(EbpUndefined),
      qualifier(scope),
      invariant(false),
      line(loc)
{
    ASSERT(IsScopeQualifier(qualifier));
}

TTypeQualifierBuilder::TTypeQualifierBuilder(const TStorageQualifierWrapper *scope)
{
    ASSERT(IsScopeQualifier(scope->getQualifier()));
    mQualifiers.push_back(scope);
}

void TTypeQualifierBuilder::appendQualifier(const TQualifierWrapperBase *qualifier)
{
    mQualifiers.push_back(qualifier);
}

bool TTypeQualifierBuilder::checkOrderIsValid(TDiagnostics *diagnostics) const
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

TTypeQualifier TTypeQualifierBuilder::getParameterTypeQualifier(TDiagnostics *diagnostics) const
{
    ASSERT(IsInvariantCorrect(mQualifiers));
    ASSERT(static_cast<const TStorageQualifierWrapper *>(mQualifiers[0])->getQualifier() ==
           EvqTemporary);

    TTypeQualifier typeQualifier(EvqTemporary, mQualifiers[0]->getLine());

    if (!checkOrderIsValid(diagnostics))
    {
        return typeQualifier;
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
            case QtMemory:
                isQualifierValid = joinMemoryQualifier(
                    &typeQualifier.memoryQualifier,
                    static_cast<const TMemoryQualifierWrapper *>(qualifier)->getQualifier());
                break;
            case QtStorage:
                isQualifierValid = joinParameterStorageQualifier(
                    &typeQualifier.qualifier,
                    static_cast<const TStorageQualifierWrapper *>(qualifier)->getQualifier());
                break;
            case QtPrecision:
                isQualifierValid = true;
                typeQualifier.precision =
                    static_cast<const TPrecisionQualifierWrapper *>(qualifier)->getQualifier();
                ASSERT(typeQualifier.precision != EbpUndefined);
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

    switch (typeQualifier.qualifier)
    {
        case EvqIn:
        case EvqConstReadOnly:  // const in
        case EvqOut:
        case EvqInOut:
            break;
        case EvqConst:
            typeQualifier.qualifier = EvqConstReadOnly;
            break;
        case EvqTemporary:
            // no qualifier has been specified, set it to EvqIn which is the default
            typeQualifier.qualifier = EvqIn;
            break;
        default:
            diagnostics->error(mQualifiers[0]->getLine(), "Invalid parameter qualifier ",
                               getQualifierString(typeQualifier.qualifier), "");
    }
    return typeQualifier;
}

TTypeQualifier TTypeQualifierBuilder::getVariableTypeQualifier(TDiagnostics *diagnostics) const
{
    ASSERT(IsInvariantCorrect(mQualifiers));

    TQualifier scope =
        static_cast<const TStorageQualifierWrapper *>(mQualifiers[0])->getQualifier();
    TTypeQualifier typeQualifier = TTypeQualifier(scope, mQualifiers[0]->getLine());

    if (!checkOrderIsValid(diagnostics))
    {
        return typeQualifier;
<<<<<<< HEAD
=======
=======
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

<<<<<<< HEAD
=======
bool JoinReadOnly(bool readOnlyQualifier, bool *joinedReadOnlyQualifier)
{
    if (readOnlyQualifier)
    {
        return false;
    }
    *joinedReadOnlyQualifier = true;
    return true;
}

bool JoinWriteOnly(bool writeOnlyQualifier, bool *joinedWriteOnlyQualifier)
{
    if (writeOnlyQualifier)
    {
        return false;
    }
    *joinedWriteOnlyQualifier = true;
    return true;
}

bool JoinShared(const TQualifier leftQualifier, TQualifier *joinedQualifier)
{
      switch (leftQualifier)
      {
          case EvqGlobal:
              *joinedQualifier = EvqShared;
              break;
          default:
              return false;
      }
      return true;
}

>>>>>>> 512e071... Add support for shared memory
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
>>>>>>> Change grammar to support features from es31
>>>>>>> da42fa5... Add support for shared memory
    }

    for (size_t i = 1; i < mQualifiers.size(); ++i)
    {
        const TQualifierWrapperBase *qualifier = mQualifiers[i];
        bool isQualifierValid                  = false;
        switch (qualifier->getType())
        {
            case QtInvariant:
                isQualifierValid        = true;
                typeQualifier.invariant = true;
                break;
            case QtInterpolation:
            {
                switch (typeQualifier.qualifier)
                {
                    case EvqGlobal:
                        isQualifierValid = true;
                        typeQualifier.qualifier =
                            static_cast<const TInterpolationQualifierWrapper *>(qualifier)
                                ->getQualifier();
                        break;
                    default:
                        isQualifierValid = false;
                }
                break;
            }
            case QtLayout:
                isQualifierValid = true;
                typeQualifier.layoutQualifier =
                    static_cast<const TLayoutQualifierWrapper *>(qualifier)->getQualifier();
                break;
            case QtStorage:
                isQualifierValid = joinVariableStorageQualifier(
                    &typeQualifier.qualifier,
                    static_cast<const TStorageQualifierWrapper *>(qualifier)->getQualifier());
                break;
            case QtPrecision:
                isQualifierValid = true;
                typeQualifier.precision =
                    static_cast<const TPrecisionQualifierWrapper *>(qualifier)->getQualifier();
                ASSERT(typeQualifier.precision != EbpUndefined);
                break;
            case QtMemory:
                isQualifierValid = joinMemoryQualifier(
                    &typeQualifier.memoryQualifier,
                    static_cast<const TMemoryQualifierWrapper *>(qualifier)->getQualifier());
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
    return typeQualifier;
}

bool TTypeQualifierBuilder::joinMemoryQualifier(TMemoryQualifier *joinedMemoryQualifier,
                                                TQualifier memoryQualifier) const
{
    switch (memoryQualifier)
    {
        case EvqReadOnly:
            joinedMemoryQualifier->readonly = true;
            break;
        case EvqWriteOnly:
            joinedMemoryQualifier->writeonly = true;
            break;
        default:
            UNREACHABLE();
    }
    return true;
}

bool TTypeQualifierBuilder::joinVariableStorageQualifier(TQualifier *joinedQualifier,
                                                         TQualifier storageQualifier) const
{
    switch (*joinedQualifier)
    {
        case EvqGlobal:
            *joinedQualifier = storageQualifier;
            break;
        case EvqTemporary:
        {
            switch (storageQualifier)
            {
                case EvqConst:
                    *joinedQualifier = storageQualifier;
                    break;
                default:
                    return false;
            }
            break;
        }
        case EvqSmooth:
        {
            switch (storageQualifier)
            {
                case EvqCentroid:
                    *joinedQualifier = EvqCentroid;
                    break;
                case EvqVertexOut:
                    *joinedQualifier = EvqSmoothOut;
                    break;
                case EvqFragmentIn:
                    *joinedQualifier = EvqSmoothIn;
                    break;
                default:
                    return false;
            }
            break;
        }
        case EvqFlat:
        {
            switch (storageQualifier)
            {
                case EvqCentroid:
                    *joinedQualifier = EvqFlat;
                    break;
                case EvqVertexOut:
                    *joinedQualifier = EvqFlatOut;
                    break;
                case EvqFragmentIn:
                    *joinedQualifier = EvqFlatIn;
                    break;
                default:
                    return false;
            }
            break;
        }
        case EvqCentroid:
        {
            switch (storageQualifier)
            {
                case EvqVertexOut:
                    *joinedQualifier = EvqCentroidOut;
                    break;
                case EvqFragmentIn:
                    *joinedQualifier = EvqCentroidIn;
                    break;
                default:
                    return false;
            }
            break;
        }
        default:
            return false;
    }
    return true;
}

bool TTypeQualifierBuilder::joinParameterStorageQualifier(TQualifier *joinedQualifier,
                                                          TQualifier storageQualifier) const
{
    switch (*joinedQualifier)
    {
        case EvqTemporary:
            *joinedQualifier = storageQualifier;
            break;
        case EvqConst:
        {
            switch (storageQualifier)
            {
                case EvqIn:
                    *joinedQualifier = EvqConstReadOnly;
                    break;
                default:
                    return false;
<<<<<<< HEAD
=======
=======
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
        case EvqShared:
            if (!JoinShared(publicType->qualifier, &publicType->qualifier))
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
>>>>>>> Change grammar to support features from es31
>>>>>>> da42fa5... Add support for shared memory
            }
            break;
        }
        default:
            return false;
    }
    return true;
}
