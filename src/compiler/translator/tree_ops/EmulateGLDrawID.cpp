//
// Copyright (c) 2002-2015 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//
// EmulateGLDrawID is an AST traverser to convert the gl_DrawID builtin
// to a uniform int
//

#include "compiler/translator/tree_ops/EmulateGLDrawID.h"

#include "angle_gl.h"
#include "compiler/translator/HashNames.h"
#include "compiler/translator/InfoSink.h"
#include "compiler/translator/Symbol.h"
#include "compiler/translator/SymbolTable.h"
#include "compiler/translator/tree_util/BuiltIn_autogen.h"
#include "compiler/translator/tree_util/FindMain.h"
#include "compiler/translator/tree_util/IntermNode_util.h"
#include "compiler/translator/tree_util/IntermTraverse.h"

namespace sh
{

namespace
{

constexpr const ImmutableString kEmulatedGLDrawIDName = ImmutableString("gl_DrawID");

class EmulateGLDrawIDTraverser : public TIntermTraverser
{
  public:
    EmulateGLDrawIDTraverser(TSymbolTable *symbolTable,
                             std::vector<sh::Uniform> *uniforms,
                             ShHashFunction64 hashFunction,
                             bool shouldCollect);

    bool isGLDrawIDUsed() const { return angleDrawID != nullptr; }
    TVariable *getANGLEDrawIDVariable() { return angleDrawID; }

  protected:
    void visitSymbol(TIntermSymbol *node) override;

  private:
    std::vector<sh::Uniform> *mUniforms;
    ShHashFunction64 mHashFunction;
    bool mShouldCollect;
    TVariable *angleDrawID;
};

EmulateGLDrawIDTraverser::EmulateGLDrawIDTraverser(TSymbolTable *symbolTable,
                                                   std::vector<sh::Uniform> *uniforms,
                                                   ShHashFunction64 hashFunction,
                                                   bool shouldCollect)
    : TIntermTraverser(true, false, false, symbolTable),
      mUniforms(uniforms),
      mHashFunction(hashFunction),
      mShouldCollect(shouldCollect),
      angleDrawID(nullptr)
{
}

void EmulateGLDrawIDTraverser::visitSymbol(TIntermSymbol *node)
{
    if (node->variable().symbolType() == SymbolType::BuiltIn &&
        node->getName() == BuiltInVariable::gl_DrawID()->name())
    {
        if (!angleDrawID)
        {
            const ImmutableString mappedName =
                HashName(kEmulatedGLDrawIDName, mHashFunction, nullptr);
            if (mShouldCollect)
            {
                Uniform info;
                info.name       = kEmulatedGLDrawIDName.data();
                info.mappedName = mappedName.data();
                info.type       = GL_INT;
                info.precision  = GL_HIGH_INT;
                info.staticUse  = true;
                info.active     = true;
                mUniforms->push_back(info);
            }

            TType *type = new TType(node->getType());
            type->setQualifier(EvqUniform);
            angleDrawID = new TVariable(mSymbolTable, mappedName, type, SymbolType::AngleInternal);
            ASSERT(mSymbolTable->declareInternal(angleDrawID));
        }

        queueReplacement(new TIntermSymbol(angleDrawID), OriginalNode::IS_DROPPED);
    }
}

}  // namespace

void EmulateGLDrawID(TIntermBlock *root,
                     TSymbolTable *symbolTable,
                     std::vector<sh::Uniform> *uniforms,
                     ShHashFunction64 hashFunction,
                     bool shouldCollect)
{
    EmulateGLDrawIDTraverser traverser(symbolTable, uniforms, hashFunction, shouldCollect);
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

const char *GetEmulatedGLDrawIDName()
{
    return kEmulatedGLDrawIDName.data();
}
}
