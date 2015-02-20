//
// Copyright (c) 2002-2015 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//

#ifndef COMPILER_TRANSLATOR_VALIDATESWITCH_H_
#define COMPILER_TRANSLATOR_VALIDATESWITCH_H_

#include "compiler/translator/IntermNode.h"

struct TParseContext;

class ValidateSwitch : public TIntermTraverser
{
  public:
    ValidateSwitch(TBasicType switchType, TParseContext *);

    virtual bool visitBinary(Visit, TIntermBinary *);
    virtual bool visitUnary(Visit, TIntermUnary *);
    virtual bool visitSelection(Visit, TIntermSelection *);
    virtual bool visitSwitch(Visit, TIntermSwitch *);
    virtual bool visitCase(Visit, TIntermCase *);
    virtual bool visitLoop(Visit, TIntermLoop *);
    virtual bool visitBranch(Visit, TIntermBranch *);

    // Check for errors and output messages for any remaining errors.
    // Returns true if there are no errors.
    bool validate(const TSourceLoc &line);

  private:
    TBasicType mSwitchType;
    TParseContext *mContext;
    bool mCaseTypeMismatch;
    bool mFirstCaseFound;
    bool mStatementBeforeCase;
    bool mLastStatementWasCase;
    int mControlFlowDepth;
    bool mCaseInsideControlFlow;
    int mDefaultCount;
    std::map<int, unsigned int> mCasesSigned;
    std::map<unsigned int, unsigned int> mCasesUnsigned;
    bool mDuplicateCases;
};

#endif // COMPILER_TRANSLATOR_VALIDATESWITCH_H_
