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
class ResourcesHLSL;
class OutputHLSL;
class TSymbolTable;

class ShaderStorageBlockOutputHLSL : public TIntermTraverser
{
  public:
    ShaderStorageBlockOutputHLSL(OutputHLSL *outputHLSL,
                                 TSymbolTable *symbolTable,
                                 ResourcesHLSL *resourcesHLSL);

    ~ShaderStorageBlockOutputHLSL();

    void traverseAssignementLeftHandSide(TIntermBinary *node);
    void traverseAssignementRightHandSide(TIntermBinary *node);
    void shaderStorageBlocksHeader();

  protected:
    void visitSymbol(TIntermSymbol *) override;
    void visitConstantUnion(TIntermConstantUnion *) override;
    bool visitSwizzle(Visit visit, TIntermSwizzle *node) override;
    bool visitBinary(Visit visit, TIntermBinary *) override;

  private:
    void traverseAssignementInternal(TIntermTyped *node, SSBOMethod method);
    bool isEndOfSSBOAccessChain();
    void writeEOpIndexDirectOrIndirectOutput(TInfoSinkBase &out, Visit visit, TIntermBinary *node);
    // Common part in dot operations.
    void writeDotOperatorOutput(TInfoSinkBase &out, const TField *field);

    bool mBufferVariableInAssignLeft;
    OutputHLSL *mOutputHLSL;
    ShaderStorageBlockFunctionHLSL *mSSBOFunctionHLSL;
    ResourcesHLSL *mResourcesHLSL;
    ReferencedInterfaceBlocks mReferencedShaderStorageBlocks;
};
}

#endif  // COMPILER_TRANSLATOR_SHADERSTORAGEBLOCKOUTPUTHLSL_H_
