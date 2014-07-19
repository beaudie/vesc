//
// Copyright (c) 2002-2014 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//

#ifndef COMPILER_TRANSLATOR_REGENERATE_STRUCT_NAMES_H_
#define COMPILER_TRANSLATOR_REGENERATE_STRUCT_NAMES_H_

#include "compiler/translator/intermediate.h"

class RegenerateStructNames : public TIntermTraverser
{
  public:
    RegenerateStructNames() {}

  protected:
    virtual void visitSymbol(TIntermSymbol *);
};

#endif  // COMPILER_TRANSLATOR_REGENERATE_STRUCT_NAMES_H_
