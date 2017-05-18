//
// Copyright (c) 2017 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//

#include "DeclareAndInitMultiviewVars.h"

#include "compiler/translator/FindMain.h"
#include "compiler/translator/InitializeVariables.h"
#include "compiler/translator/IntermNode.h"
#include "compiler/translator/SymbolTable.h"

namespace sh
{

namespace
{

TIntermSymbol *CreateInstanceIDImpostorSymbol()
{
    TIntermSymbol *instanceIDImpostorSymbol =
        new TIntermSymbol(0, "gl_InstanceIDImpostor", TType(EbtInt, EbpHigh, EvqGlobal));
    instanceIDImpostorSymbol->setInternal(true);
    return instanceIDImpostorSymbol;
}

TIntermSymbol *CreateViewIDOVRSymbol()
{
    TIntermSymbol *viewIDSymbol =
        new TIntermSymbol(0, "gl_ViewID_OVR", TType(EbtUInt, EbpHigh, EvqGlobal));
    viewIDSymbol->setInternal(true);
    return viewIDSymbol;
}

// Adds initializers for gl_InstanceIDImpostor and gl_ViewID_OVR at the beginning of main().
void InitializeMultiviewVariables(TIntermBlock *root, unsigned numberOfViews)
{
    TIntermSequence *deferredInitializers = new TIntermSequence();

    // Create a gl_ViewID_OVR node.
    TIntermSymbol *viewIDSymbol = CreateViewIDOVRSymbol();

    // Create a gl_InstanceIDImpostor node.
    TIntermSymbol *instanceIDImpostorSymbol = CreateInstanceIDImpostorSymbol();

    // Create a gl_InstanceID node.
    TIntermSymbol *instanceIDSymbol =
        new TIntermSymbol(0, "gl_InstanceID", TType(EbtInt, EbpHigh, EvqInstanceID));
    instanceIDSymbol->setInternal(false);
    TIntermSequence *instanceIDSymbolCastArguments = new TIntermSequence();
    instanceIDSymbolCastArguments->push_back(instanceIDSymbol);
    TIntermAggregate *instanceIDSymbolCast = TIntermAggregate::CreateConstructor(
        TType(EbtUInt, EbpHigh, EvqTemporary), instanceIDSymbolCastArguments);

    // Create a numberOfViews node.
    TConstantUnion *numberOfViewsConstant = new TConstantUnion();
    numberOfViewsConstant->setIConst(numberOfViews);
    TIntermConstantUnion *numberOfViewsUintSymbol =
        new TIntermConstantUnion(numberOfViewsConstant, TType(EbtUInt, EbpHigh, EvqTemporary));
    TIntermConstantUnion *numberOfViewsIntSymbol =
        new TIntermConstantUnion(numberOfViewsConstant, TType(EbtInt, EbpHigh, EvqTemporary));

    // Create a gl_InstanceID / numberOfViews node.
    TIntermBinary *normalizedInstanceID =
        new TIntermBinary(EOpDiv, instanceIDSymbol, numberOfViewsIntSymbol);

    // Create a gl_InstanceIDImpostor = gl_InstanceID / numberOfViews node.
    TIntermBinary *instanceIDImpostorInitializer =
        new TIntermBinary(EOpAssign, instanceIDImpostorSymbol, normalizedInstanceID);

    // Create a uint(gl_InstanceID) % numberOfViews node.
    TIntermBinary *normalizedViewID =
        new TIntermBinary(EOpIMod, instanceIDSymbolCast, numberOfViewsUintSymbol);

    // Create a gl_ViewID_OVR = uint(gl_InstanceID) % numberOfViews node.
    TIntermBinary *viewIDInitializer = new TIntermBinary(EOpAssign, viewIDSymbol, normalizedViewID);

    // Add nodes to sequence and insert into main.
    deferredInitializers->push_back(viewIDInitializer);
    deferredInitializers->push_back(instanceIDImpostorInitializer);

    // Insert init code as a block to the beginning of the main() function.
    TIntermBlock *initGlobalsBlock = new TIntermBlock();
    initGlobalsBlock->getSequence()->swap(*deferredInitializers);

    TIntermFunctionDefinition *main = FindMain(root);
    ASSERT(main != nullptr);
    TIntermBlock *mainBody = main->getBody();
    ASSERT(mainBody != nullptr);
    mainBody->getSequence()->insert(mainBody->getSequence()->begin(), initGlobalsBlock);
}

// Adds declarations for gl_ViewID_OVR and gl_InstanceIDImpostor in global scope at the beginning of
// the shader.
void DeclareMultiviewVariablesInGlobalScope(TIntermBlock *root)
{
    TIntermSequence *globalSequence = root->getSequence();

    TIntermDeclaration *viewIDOVRDeclaration = new TIntermDeclaration();
    viewIDOVRDeclaration->appendDeclarator(CreateViewIDOVRSymbol());

    TIntermDeclaration *instanceIDImpostorDeclaration = new TIntermDeclaration();
    instanceIDImpostorDeclaration->appendDeclarator(CreateInstanceIDImpostorSymbol());

    globalSequence->insert(globalSequence->begin(), viewIDOVRDeclaration);
    globalSequence->insert(globalSequence->begin(), instanceIDImpostorDeclaration);
}

class TurnInstanceIDIntoImpostorTraverser : public TIntermTraverser
{
  public:
    TurnInstanceIDIntoImpostorTraverser();

    void visitSymbol(TIntermSymbol *node) override;

  private:
};

TurnInstanceIDIntoImpostorTraverser::TurnInstanceIDIntoImpostorTraverser()
    : TIntermTraverser(true, false, false)
{
}

void TurnInstanceIDIntoImpostorTraverser::visitSymbol(TIntermSymbol *node)
{
    TName &name = node->getName();
    if (name.getString() == "gl_InstanceID")
    {
        node->getType().setQualifier(EvqTemporary);
        name.setInternal(true);
        name.setString("gl_InstanceIDImpostor");
    }
}

// Replaces every occurrence of gl_InstanceID with gl_InstanceIDImpostor, sets the name to internal
// and changes the qualifier from EvqInstanceID to EvqTemporary.
void TurnInstanceIDIntoImpostor(TIntermBlock *root)
{
    TurnInstanceIDIntoImpostorTraverser traverser;
    root->traverse(&traverser);
}

}  // namespace

void DeclareAndInitMultiviewVars(TIntermBlock *root, unsigned numberOfViews)
{
    TurnInstanceIDIntoImpostor(root);
    DeclareMultiviewVariablesInGlobalScope(root);
    InitializeMultiviewVariables(root, numberOfViews);
}

}  // namespace sh