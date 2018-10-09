//
// Copyright (c) 2002-2015 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//
// EmulateGLDrawID is an AST traverser to convert the gl_DrawID builtin
// to uniform int angle_DrawID
//

#include "compiler/translator/tree_ops/EmulateGLDrawID.h"

#include "angle_gl.h"
#include "compiler/translator/InfoSink.h"
#include "compiler/translator/Symbol.h"
#include "compiler/translator/tree_util/FindMain.h"
#include "compiler/translator/tree_util/IntermNode_util.h"
#include "compiler/translator/tree_util/IntermTraverse.h"

namespace sh
{

namespace
{
constexpr const ImmutableString angleDrawIDString("angle_DrawID");

class EmulateGLDrawIDTraverser : public TIntermTraverser
{
  public:
    EmulateGLDrawIDTraverser(TSymbolTable *symbolTable);

    bool isGLDrawIDUsed() const { return angleDrawID != nullptr; }
    TVariable *getANGLEDrawIDVariable() { return angleDrawID; }

  protected:
    void visitSymbol(TIntermSymbol *node) override;

  private:
    TVariable *angleDrawID;
};

EmulateGLDrawIDTraverser::EmulateGLDrawIDTraverser(TSymbolTable *symbolTable)
    : TIntermTraverser(true, false, false, symbolTable), angleDrawID(nullptr)
{
}

void EmulateGLDrawIDTraverser::visitSymbol(TIntermSymbol *node)
{
    if (node->variable().symbolType() == SymbolType::BuiltIn && node->getName() == "gl_DrawID")
    {
        if (!angleDrawID)
        {
            TType *type = new TType(node->getType());
            type->setQualifier(EvqUniform);
            angleDrawID =
                new TVariable(mSymbolTable, angleDrawIDString, type, SymbolType::AngleInternal);
        }

        queueReplacement(new TIntermSymbol(angleDrawID), OriginalNode::IS_DROPPED);
    }
}

}  // namespace

void EmulateGLDrawID(TIntermBlock *root, TSymbolTable *symbolTable)
{
    ;
    EmulateGLDrawIDTraverser traverser(symbolTable);
    root->traverse(&traverser);
    if (traverser.isGLDrawIDUsed())
    {
        traverser.updateTree();

        TIntermDeclaration *uniformDeclaration = new TIntermDeclaration();
        uniformDeclaration->appendDeclarator(new TIntermSymbol(traverser.getANGLEDrawIDVariable()));

        size_t mainIndex = FindMainIndex(root);
        root->insertStatement(mainIndex, uniformDeclaration);
    }
}
}
