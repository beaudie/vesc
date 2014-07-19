//
// Copyright (c) 2002-2014 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//

#ifndef COMPILER_TRANSLATOR_REGENERATE_STRUCT_NAMES_H_
#define COMPILER_TRANSLATOR_REGENERATE_STRUCT_NAMES_H_

#include "compiler/translator/intermediate.h"
#include "compiler/translator/SymbolTable.h"

class RegenerateStructNames : public TIntermTraverser
{
  public:
    RegenerateStructNames(TSymbolTable &symbolTable,
                          int shaderVersion)
        : mSymbolTable(symbolTable),
          mShaderVersion(shaderVersion) {}

  protected:
    virtual void visitSymbol(TIntermSymbol *);

  private:
    TSymbolTable &mSymbolTable;
    int mShaderVersion;
};

#endif  // COMPILER_TRANSLATOR_REGENERATE_STRUCT_NAMES_H_
