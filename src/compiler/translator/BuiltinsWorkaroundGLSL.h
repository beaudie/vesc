//
// Copyright (c) 2002-2013 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//

#ifndef COMPILER_TRANSLATOR_BUILTINSWORKAROUNDGLSL_H_
#define COMPILER_TRANSLATOR_BUILTINSWORKAROUNDGLSL_H_

#include "compiler/translator/tree_util/IntermTraverse.h"

#include "compiler/translator/Pragma.h"

namespace sh
{

class TBuiltinsWorkaroundGLSL : public TIntermTraverser
{
  public:
    TBuiltinsWorkaroundGLSL(TSymbolTable *symbolTable, ShCompileOptions options, bool isVulkanGLSL);

    void visitSymbol(TIntermSymbol *node) override;
    bool visitDeclaration(Visit, TIntermDeclaration *node) override;

  private:
    void ensureVersionIsAtLeast(int version);

    ShCompileOptions mCompileOptions;

    bool isBaseInstanceDeclared = false;
    bool isBaseVertexDeclared   = false;

    bool isVulkan;
};

}  // namespace sh

#endif  // COMPILER_TRANSLATOR_BUILTINSWORKAROUNDGLSL_H_
