//
// Copyright (c) 2002-2014 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//

#ifndef COMPILER_TRANSLATOR_REWRITE_CONSTRUCTOR_H_
#define COMPILER_TRANSLATOR_REWRITE_CONSTRUCTOR_H_

#include "compiler/translator/intermediate.h"

class RewriteConstructor : public TIntermTraverser
{
  public:
    RewriteConstructor() {}

  protected:
    virtual bool visitAggregate(Visit visit, TIntermAggregate *node);
};

#endif  // COMPILER_TRANSLATOR_REWRITE_CONSTRUCTOR_H_
