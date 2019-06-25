//
// Copyright (c) 2002-2012 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//

#include "compiler/translator/BuiltinsWorkaroundGLSL.h"

#include "angle_gl.h"
#include "compiler/translator/StaticType.h"
#include "compiler/translator/Symbol.h"
#include "compiler/translator/SymbolTable.h"

namespace sh
{

namespace
{
constexpr const ImmutableString kGlInstanceIDString("gl_InstanceID");
constexpr const ImmutableString kGlVertexIDString("gl_VertexID");
}  // anonymous namespace

TBuiltinsWorkaroundGLSL::TBuiltinsWorkaroundGLSL(TSymbolTable *symbolTable,
                                                 ShCompileOptions options,
                                                 bool isVulkanGLSL)
    : TIntermTraverser(true, false, false, symbolTable),
      mCompileOptions(options),
      isVulkan(isVulkanGLSL)
{}

void TBuiltinsWorkaroundGLSL::visitSymbol(TIntermSymbol *node)
{
    if (node->variable().symbolType() == SymbolType::BuiltIn)
    {
        if (isVulkan)
        {
            if (node->getName() == kGlInstanceIDString)
            {
                TVariable *instanceIndex = new TVariable(
                    mSymbolTable, ImmutableString("gl_InstanceIndex"),
                    StaticType::Get<EbtInt, EbpHigh, EvqUniform, 1, 1>(), SymbolType::BuiltIn);
                TIntermSymbol *instanceIndexRef = new TIntermSymbol(instanceIndex);

                if (isBaseInstanceDeclared)
                {
                    TVariable *baseInstance =
                        new TVariable(mSymbolTable, ImmutableString("angle_BaseInstance"),
                                      StaticType::Get<EbtInt, EbpHigh, EvqUniform, 1, 1>(),
                                      SymbolType::AngleInternal);
                    TIntermSymbol *baseInstanceRef = new TIntermSymbol(baseInstance);

                    TIntermBinary *subBaseInstance =
                        new TIntermBinary(EOpSub, instanceIndexRef, baseInstanceRef);
                    queueReplacement(subBaseInstance, OriginalNode::IS_DROPPED);
                }
                else
                {
                    queueReplacement(instanceIndexRef, OriginalNode::IS_DROPPED);
                }
            }
            else if (node->getName() == kGlVertexIDString)
            {
                TVariable *vertexIndex = new TVariable(
                    mSymbolTable, ImmutableString("gl_VertexIndex"),
                    StaticType::Get<EbtInt, EbpHigh, EvqUniform, 1, 1>(), SymbolType::BuiltIn);
                TIntermSymbol *vertexIndexRef = new TIntermSymbol(vertexIndex);
                queueReplacement(vertexIndexRef, OriginalNode::IS_DROPPED);
            }
        }
    }
}

bool TBuiltinsWorkaroundGLSL::visitDeclaration(Visit, TIntermDeclaration *node)
{
    const TIntermSequence &sequence = *(node->getSequence());
    ASSERT(!sequence.empty());

    for (TIntermNode *variableNode : sequence)
    {
        TIntermSymbol *variable = variableNode->getAsSymbolNode();
        if (variable && variable->variable().symbolType() == SymbolType::AngleInternal)
        {
            if (variable->getName() == "angle_BaseInstance")
            {
                isBaseInstanceDeclared = true;
            }
        }
    }
    return true;
}

}  // namespace sh
