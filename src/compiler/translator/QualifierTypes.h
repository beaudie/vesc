//
// Copyright (c) 2002-2016 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//

#ifndef COMPILER_TRANSLATOR_QUALIFIER_TYPES_H_
#define COMPILER_TRANSLATOR_QUALIFIER_TYPES_H_

#include "BaseTypes.h"
#include "common/angleutils.h"
#include "Types.h"

#include <vector>

class TParseContext;

enum TQualifierType
{
    QtInvariant,
    QtInterpolation,
    QtLayout,
    QtStorage,
    QtPrecision
};

class TQualifierWrapperBase : angle::NonCopyable
{

  public:
    TQualifierWrapperBase(const TSourceLoc &line) : mLine(line) {}
    virtual ~TQualifierWrapperBase(){};
    virtual TQualifierType getType() const     = 0;
    virtual TString getQualifierString() const = 0;
    const TSourceLoc &getLine() const { return mLine; }
  private:
    TSourceLoc mLine;
};

class TInvariantQualifierWrapper final : public TQualifierWrapperBase
{
  public:
    TInvariantQualifierWrapper(const TSourceLoc &line) : TQualifierWrapperBase(line) {}
    ~TInvariantQualifierWrapper() {}

    TQualifierType getType() const { return QtInvariant; }
    TString getQualifierString() const { return "invariant"; }
};

class TInterpolationQualifierWrapper final : public TQualifierWrapperBase
{
  public:
    TInterpolationQualifierWrapper(TQualifier interpolationQualifier, const TSourceLoc &line)
        : TQualifierWrapperBase(line), mInterpolationQualifier(interpolationQualifier)
    {
    }
    ~TInterpolationQualifierWrapper() {}

    TQualifierType getType() const { return QtInterpolation; }
    TString getQualifierString() const { return ::getQualifierString(mInterpolationQualifier); }
    TQualifier getQualifier() const { return mInterpolationQualifier; }

  private:
    TQualifier mInterpolationQualifier;
};

class TLayoutQualifierWrapper final : public TQualifierWrapperBase
{
  public:
    TLayoutQualifierWrapper(TLayoutQualifier layoutQualifier, const TSourceLoc &line)
        : TQualifierWrapperBase(line), mLayoutQualifier(layoutQualifier)
    {
    }
    ~TLayoutQualifierWrapper() {}

    TQualifierType getType() const { return QtLayout; }
    TString getQualifierString() const { return "layout"; }
    const TLayoutQualifier &getQualifier() const { return mLayoutQualifier; }
  private:
    TLayoutQualifier mLayoutQualifier;
};

class TStorageQualifierWrapper final : public TQualifierWrapperBase
{
  public:
    TStorageQualifierWrapper(TQualifier storageQualifier, const TSourceLoc &line)
        : TQualifierWrapperBase(line), mStorageQualifier(storageQualifier)
    {
    }
    ~TStorageQualifierWrapper() {}

    TQualifierType getType() const { return QtStorage; }
    TString getQualifierString() const { return ::getQualifierString(mStorageQualifier); }
    TQualifier getQualifier() const { return mStorageQualifier; }
  private:
    TQualifier mStorageQualifier;
};

class TPrecisionQualifierWrapper final : public TQualifierWrapperBase
{
  public:
    TPrecisionQualifierWrapper(TPrecision precisionQualifier, const TSourceLoc &line)
        : TQualifierWrapperBase(line), mPrecisionQualifier(precisionQualifier)
    {
    }
    ~TPrecisionQualifierWrapper() {}

    TQualifierType getType() const { return QtPrecision; }
    TString getQualifierString() const { return ::getPrecisionString(mPrecisionQualifier); }
    TPrecision getQualifier() const { return mPrecisionQualifier; }
  private:
    TPrecision mPrecisionQualifier;
};

class TQualifierSequence final : angle::NonCopyable
{
  public:
    TQualifierSequence(const TStorageQualifierWrapper *scope);

    void joinQualifier(const TQualifierWrapperBase *qualifier);
    // Joins all of the qualifiers to fill in the members in TPublicType.
    // It returns true on success and false if the qualifier combination is not valid.
    TPublicType getPublicType(TParseContext *context) const;

  private:
    std::vector<const TQualifierWrapperBase *> mQualifiers;
    bool joinInvariantQualifier(TPublicType *publicType) const;
    bool joinInterpolationQualifier(TPublicType *publicType,
                                    TQualifier interpolationQualifier) const;
    bool joinStorageQualifier(TPublicType *publicType, TQualifier storageQualifier) const;
    bool joinPrecisionQualifier(TPublicType *publicType, TPrecision precisionQualifier) const;
    bool joinLayoutQualifier(TPublicType *publicType,
                             const TLayoutQualifier &layoutQualifier) const;
    bool joinParameterQualifier(TPublicType *publicType, TQualifier parameterQualifier) const;
};

#endif  // COMPILER_TRANSLATOR_QUALIFIER_TYPES_H_
