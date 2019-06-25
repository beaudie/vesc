//
// Copyright 2019 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//

#include "compiler/translator/BuiltinsWorkaroundGLSL.h"

#include "angle_gl.h"
#include "compiler/translator/StaticType.h"
#include "compiler/translator/Symbol.h"
#include "compiler/translator/SymbolTable.h"
#include "compiler/translator/tree_util/BuiltIn.h"

namespace sh
{

namespace
{
constexpr const ImmutableString kGlInstanceIDString("gl_InstanceID");
constexpr const ImmutableString kGlVertexIDString("gl_VertexID");
}  // anonymous namespace

TBuiltinsWorkaroundGLSL::TBuiltinsWorkaroundGLSL(TSymbolTable *symbolTable,
                                                 ShCompileOptions options)
    : TIntermTraverser(true, false, false, symbolTable), mCompileOptions(options)
{}

void TBuiltinsWorkaroundGLSL::visitSymbol(TIntermSymbol *node)
{
    if (node->variable().symbolType() == SymbolType::BuiltIn)
    {
        if (node->getName() == kGlInstanceIDString)
        {
            TIntermSymbol *instanceIndexRef =
                new TIntermSymbol(BuiltInVariable::gl_InstanceIndex());

            if (isBaseInstanceDeclared)
            {
                TIntermSymbol *baseInstanceRef =
                    new TIntermSymbol(BuiltInVariable::angle_BaseInstance());

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
            TIntermSymbol *vertexIndexRef = new TIntermSymbol(BuiltInVariable::gl_VertexIndex());
            queueReplacement(vertexIndexRef, OriginalNode::IS_DROPPED);
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
