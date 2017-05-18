//
// Copyright (c) 2017 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//

#include "DeclareAndInitBuiltinsForInstancedMultiview.h"

#include "compiler/translator/FindMain.h"
#include "compiler/translator/InitializeVariables.h"
#include "compiler/translator/IntermNode.h"
#include "compiler/translator/SymbolTable.h"

namespace sh
{

namespace
{

class RenameVariableAndMarkAsInternalTraverser : public TIntermTraverser
{
  public:
    RenameVariableAndMarkAsInternalTraverser(const TString &oldName, const TString &newName)
        : TIntermTraverser(true, false, false), mOldName(oldName), mNewName(newName)
    {
    }

    void visitSymbol(TIntermSymbol *node) override
    {
        TName &name = node->getName();
        if (name.getString() == mOldName)
        {
            node->getTypePointer()->setQualifier(EvqTemporary);
            name.setInternal(true);
            name.setString(mNewName);
        }
    }

  private:
    TString mOldName;
    TString mNewName;
};

TIntermSymbol *CreateInstanceIDSymbol()
{
    TIntermSymbol *instanceIDSymbol = new TIntermSymbol(TSymbolTable::nextUniqueId(), "InstanceID",
                                                        TType(EbtInt, EbpHigh, EvqGlobal));
    instanceIDSymbol->setInternal(true);
    return instanceIDSymbol;
}

TIntermSymbol *CreateViewIDOVRSymbol()
{
    TIntermSymbol *viewIDSymbol = new TIntermSymbol(TSymbolTable::nextUniqueId(), "ViewID_OVR",
                                                    TType(EbtUInt, EbpHigh, EvqGlobal));
    viewIDSymbol->setInternal(true);
    return viewIDSymbol;
}

// Adds initializers for InstanceID and ViewID_OVR at the beginning of main().
void InitializeBuiltinsInMain(TIntermBlock *root, unsigned numberOfViews)
{
    // Create a gl_InstanceID node.
    TIntermSymbol *glInstanceIDSymbol =
        new TIntermSymbol(0, "gl_InstanceID", TType(EbtInt, EbpHigh, EvqInstanceID));

    // Create a signed numberOfViews node.
    TConstantUnion *numberOfViewsConstant = new TConstantUnion();
    numberOfViewsConstant->setIConst(numberOfViews);
    TIntermConstantUnion *numberOfViewsIntSymbol =
        new TIntermConstantUnion(numberOfViewsConstant, TType(EbtInt, EbpHigh, EvqTemporary));

    // Create a gl_InstanceID / numberOfViews node.
    TIntermBinary *normalizedInstanceID =
        new TIntermBinary(EOpDiv, glInstanceIDSymbol, numberOfViewsIntSymbol);

    // Create a InstanceID node.
    TIntermSymbol *instanceIDSymbol = CreateInstanceIDSymbol();

    // Create a InstanceID = gl_InstanceID / numberOfViews node.
    TIntermBinary *instanceIDInitializer =
        new TIntermBinary(EOpAssign, instanceIDSymbol, normalizedInstanceID);

    // Create a uint(gl_InstanceID) node.
    TIntermSequence *instanceIDSymbolCastArguments = new TIntermSequence();
    instanceIDSymbolCastArguments->push_back(glInstanceIDSymbol);
    TIntermAggregate *instanceIDAsUint = TIntermAggregate::CreateConstructor(
        TType(EbtUInt, EbpHigh, EvqTemporary), instanceIDSymbolCastArguments);

    // Create a unsigned numberOfViews node.
    TIntermConstantUnion *numberOfViewsUintSymbol =
        new TIntermConstantUnion(numberOfViewsConstant, TType(EbtUInt, EbpHigh, EvqTemporary));

    // Create a uint(gl_InstanceID) % numberOfViews node.
    TIntermBinary *normalizedViewID =
        new TIntermBinary(EOpIMod, instanceIDAsUint, numberOfViewsUintSymbol);

    // Create a ViewID_OVR node.
    TIntermSymbol *viewIDSymbol = CreateViewIDOVRSymbol();

    // Create a ViewID_OVR = uint(gl_InstanceID) % numberOfViews node.
    TIntermBinary *viewIDInitializer = new TIntermBinary(EOpAssign, viewIDSymbol, normalizedViewID);

    // Add nodes to sequence and insert into main.
    TIntermSequence *initializers = new TIntermSequence();
    initializers->push_back(viewIDInitializer);
    initializers->push_back(instanceIDInitializer);

    // Insert init code as a block at the beginning of the main() function.
    TIntermBlock *initGlobalsBlock = new TIntermBlock();
    initGlobalsBlock->getSequence()->swap(*initializers);

    TIntermFunctionDefinition *main = FindMain(root);
    ASSERT(main != nullptr);
    TIntermBlock *mainBody = main->getBody();
    ASSERT(mainBody != nullptr);
    mainBody->getSequence()->insert(mainBody->getSequence()->begin(), initGlobalsBlock);
}

// Adds declarations for ViewID_OVR and InstanceID in global scope at the beginning of
// the shader.
void DeclareBuiltinsInGlobalScope(TIntermBlock *root)
{
    TIntermSequence *globalSequence = root->getSequence();

    TIntermDeclaration *viewIDOVRDeclaration = new TIntermDeclaration();
    viewIDOVRDeclaration->appendDeclarator(CreateViewIDOVRSymbol());

    TIntermDeclaration *instanceIDDeclaration = new TIntermDeclaration();
    instanceIDDeclaration->appendDeclarator(CreateInstanceIDSymbol());

    globalSequence->insert(globalSequence->begin(), viewIDOVRDeclaration);
    globalSequence->insert(globalSequence->begin(), instanceIDDeclaration);
}

// Replaces every occurrence of gl_InstanceID with InstanceID, sets the name to internal
// and changes the qualifier from EvqInstanceID to EvqTemporary.
void RenameGLInstanceIDAndMarkAsInternal(TIntermBlock *root)
{
    RenameVariableAndMarkAsInternalTraverser traverser("gl_InstanceID", "InstanceID");
    root->traverse(&traverser);
}

// Replaces every occurrence of gl_ViewID_OVR with ViewID_OVR, sets the name to internal
// and changes the qualifier from EvqViewIDOVR to EvqTemporary.
void RenameGLViewIDAndMarkAsInternal(TIntermBlock *root)
{
    RenameVariableAndMarkAsInternalTraverser traverser("gl_ViewID_OVR", "ViewID_OVR");
    root->traverse(&traverser);
}

}  // namespace

void DeclareAndInitBuiltinsForInstancedMultiview(TIntermBlock *root, unsigned numberOfViews)
{
    // Renaming the variables should happen before adding the initializers.
    RenameGLInstanceIDAndMarkAsInternal(root);
    DeclareBuiltinsInGlobalScope(root);
    InitializeBuiltinsInMain(root, numberOfViews);
    RenameGLViewIDAndMarkAsInternal(root);
}

}  // namespace sh