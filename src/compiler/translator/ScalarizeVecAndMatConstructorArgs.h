//
// Copyright (c) 2002-2014 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//

#ifndef COMPILER_TRANSLATOR_SCALARIZEVECANDMATCONSTRUCTORARGS_H_
#define COMPILER_TRANSLATOR_SCALARIZEVECANDMATCONSTRUCTORARGS_H_

#include "compiler/translator/IntermNode.h"

class ScalarizeVecAndMatConstructorArgs : public TIntermTraverser
{
  public:
    ScalarizeVecAndMatConstructorArgs(sh::GLenum shaderType,
                                      bool fragmentPrecisionHigh,
                                      unsigned int *temporaryIndex)
        : TIntermTraverser(true, false, false),
          mTempVarCount(0),
          mShaderType(shaderType),
          mFragmentPrecisionHigh(fragmentPrecisionHigh)
    {
        useTemporaryIndex(temporaryIndex);
    }

  protected:
    bool visitAggregate(Visit visit, TIntermAggregate *node) override;
    bool visitBlock(Visit visit, TIntermBlock *node) override;

  private:
    void scalarizeArgs(TIntermAggregate *aggregate,
                       bool scalarizeVector, bool scalarizeMatrix);

    // If we have the following code:
    //   mat4 m(0);
    //   vec4 v(1, m);
    // We will rewrite to:
    //   mat4 m(0);
    //   mat4 s0 = m;
    //   vec4 v(1, s0[0][0], s0[0][1], s0[0][2]);
    // This function is to create nodes for "mat4 s0 = m;" and insert it to
    // the code sequence.
    void createTempVariable(TIntermTyped *original);

    std::vector<TIntermSequence> mBlockStack;
    int mTempVarCount;

    sh::GLenum mShaderType;
    bool mFragmentPrecisionHigh;
};

#endif  // COMPILER_TRANSLATOR_SCALARIZEVECANDMATCONSTRUCTORARGS_H_
