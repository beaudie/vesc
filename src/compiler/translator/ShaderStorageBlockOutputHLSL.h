//
// Copyright 2018 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//
// ShaderStorageBlockOutputHLSL: A traverser to translate a buffer variable of shader storage block
// to an offset of RWByteAddressBuffer.
//

#ifndef COMPILER_TRANSLATOR_SHADERSTORAGEBLOCKOUTPUTHLSL_H_
#define COMPILER_TRANSLATOR_SHADERSTORAGEBLOCKOUTPUTHLSL_H_

#include "compiler/translator/ShaderStorageBlockFunctionHLSL.h"
#include "compiler/translator/tree_util/IntermTraverse.h"
#include "compiler/translator/util.h"

namespace sh
{
class TSymbolTable;
class UniformHLSL;
class OutputHLSL;

class ShaderStorageBlockOutputHLSL : public TIntermTraverser
{
  public:
    ShaderStorageBlockOutputHLSL(OutputHLSL *outputHLSL,
                                 TSymbolTable *symbolTable,
                                 UniformHLSL *uniformHLSL);

    ~ShaderStorageBlockOutputHLSL();

    void setAssignLeft(bool inLeft);
    TString shaderStorageBlocksHeader();
    ShaderStorageBlockFunctionHLSL *getSSBOFunctionHLSL() const { return mSSBOFunctionHLSL; }

  protected:
    void visitSymbol(TIntermSymbol *) override;
    void visitConstantUnion(TIntermConstantUnion *) override;
    bool visitSwizzle(Visit visit, TIntermSwizzle *node) override;
    bool visitBinary(Visit visit, TIntermBinary *) override;

  private:
    const TType &ancestorEvaluatesBufferVariableType(const TType &type);
    bool isLastNodeInSSBO();
    void writeEOpIndexOutput(TInfoSinkBase &out, Visit visit, TIntermBinary *node);

    bool mBufferVariableInAssignLeft;
    OutputHLSL *mOutputHLSL;
    ShaderStorageBlockFunctionHLSL *mSSBOFunctionHLSL;
    UniformHLSL *mUniformHLSL;
    ReferencedInterfaceBlocks mReferencedShaderStorageBlocks;
};
}

#endif  // COMPILER_TRANSLATOR_SHADERSTORAGEBLOCKOUTPUTHLSL_H_
