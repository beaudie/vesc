//
// Copyright (c) 2017 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//
// Applies the necessary AST transformations to support multiview rendering through instancing.
// Check the header file For more information.
//

#include "compiler/translator/DeclareAndInitBuiltinsForInstancedMultiview.h"

#include "compiler/translator/FindMain.h"
#include "compiler/translator/InitializeVariables.h"
#include "compiler/translator/IntermNode.h"
#include "compiler/translator/SymbolTable.h"

namespace sh
{

namespace
{

class AlterVariableTraverser : public TIntermTraverser
{
  public:
    AlterVariableTraverser(const TString &oldName, const TString &newName, TQualifier newQualifier)
        : TIntermTraverser(true, false, false),
          mOldName(oldName),
          mNewName(newName),
          mNewQualifier(newQualifier)
    {
    }

    void visitSymbol(TIntermSymbol *node) override
    {
        TName &name = node->getName();
        if (name.getString() == mOldName)
        {
            node->getTypePointer()->setQualifier(mNewQualifier);
            name.setInternal(true);
            name.setString(mNewName);
        }
    }

  private:
    TString mOldName;
    TString mNewName;
    TQualifier mNewQualifier;
};

TIntermSymbol *CreateGLInstanceIDSymbol()
{
    return new TIntermSymbol(0, "gl_InstanceID", TType(EbtInt, EbpHigh, EvqInstanceID));
}

TIntermBlock *FindMainBody(TIntermBlock *root)
{
    TIntermFunctionDefinition *main = FindMain(root);
    ASSERT(main != nullptr);
    TIntermBlock *mainBody = main->getBody();
    ASSERT(mainBody != nullptr);
    return mainBody;
}

void DeclareViewID(TIntermBlock *root, TIntermTyped *viewIDSymbol)
{
    TIntermDeclaration *viewIDOVRDeclaration = new TIntermDeclaration();
    viewIDOVRDeclaration->appendDeclarator(viewIDSymbol);
    TIntermSequence *globalSequence = root->getSequence();
    globalSequence->insert(globalSequence->begin(), viewIDOVRDeclaration);
}

void InitializeViewID(TIntermBlock *root, TIntermTyped *viewIDSymbol, unsigned numberOfViews)
{
    // Create a uint(gl_InstanceID) node.
    TIntermSequence *instanceIDSymbolCastArguments = new TIntermSequence();
    instanceIDSymbolCastArguments->push_back(CreateGLInstanceIDSymbol());
    TIntermAggregate *instanceIDAsUint = TIntermAggregate::CreateConstructor(
        TType(EbtUInt, EbpHigh, EvqTemporary), instanceIDSymbolCastArguments);

    // Create a unsigned numberOfViews node.
    TConstantUnion *numberOfViewsUnsignedConstant = new TConstantUnion();
    numberOfViewsUnsignedConstant->setUConst(numberOfViews);
    TIntermConstantUnion *numberOfViewsUintSymbol =
        new TIntermConstantUnion(numberOfViewsUnsignedConstant, TType(EbtUInt, EbpHigh, EvqConst));

    // Create a uint(gl_InstanceID) % numberOfViews node.
    TIntermBinary *normalizedViewID =
        new TIntermBinary(EOpIMod, instanceIDAsUint, numberOfViewsUintSymbol);

    // Create a ViewID_OVR = uint(gl_InstanceID) % numberOfViews node.
    TIntermBinary *viewIDInitializer = new TIntermBinary(EOpAssign, viewIDSymbol, normalizedViewID);

    TIntermBlock *mainBody = FindMainBody(root);
    mainBody->getSequence()->insert(mainBody->getSequence()->begin(), viewIDInitializer);
}

void DeclareAndInitializeInstanceID(TIntermBlock *root, unsigned numberOfViews)
{
    TIntermSymbol *instanceIDSymbol = new TIntermSymbol(TSymbolTable::nextUniqueId(), "InstanceID",
                                                        TType(EbtInt, EbpHigh, EvqGlobal));
    instanceIDSymbol->setInternal(true);

    // Add declaration in global scope.
    TIntermDeclaration *instanceIDDeclaration = new TIntermDeclaration();
    TIntermSequence *globalSequence           = root->getSequence();
    instanceIDDeclaration->appendDeclarator(instanceIDSymbol);
    globalSequence->insert(globalSequence->begin(), instanceIDDeclaration);

    // Add initializer at the beginning of main().
    // Create a signed numberOfViews node.
    TConstantUnion *numberOfViewsConstant = new TConstantUnion();
    numberOfViewsConstant->setIConst(numberOfViews);
    TIntermConstantUnion *numberOfViewsIntSymbol =
        new TIntermConstantUnion(numberOfViewsConstant, TType(EbtInt, EbpHigh, EvqConst));

    // Create a gl_InstanceID / numberOfViews node.
    TIntermBinary *normalizedInstanceID =
        new TIntermBinary(EOpDiv, CreateGLInstanceIDSymbol(), numberOfViewsIntSymbol);

    // Create a InstanceID = gl_InstanceID / numberOfViews node.
    TIntermBinary *instanceIDInitializer =
        new TIntermBinary(EOpAssign, instanceIDSymbol->deepCopy(), normalizedInstanceID);

    TIntermBlock *mainBody = FindMainBody(root);
    mainBody->getSequence()->insert(mainBody->getSequence()->begin(), instanceIDInitializer);
}

// Replaces every occurrence of gl_InstanceID with InstanceID, sets the name to internal
// and changes the qualifier from EvqInstanceID to EvqTemporary.
void AlterGLInstanceID(TIntermBlock *root)
{
    AlterVariableTraverser traverser("gl_InstanceID", "InstanceID", EvqTemporary);
    root->traverse(&traverser);
}

// Replaces every occurrence of gl_ViewID_OVR with ViewID_OVR, sets the name to internal
// and changes the qualifier from EvqViewIDOVR to viewIDQualifier.
void AlterGLViewID(TIntermBlock *root, TQualifier viewIDQualifier)
{
    AlterVariableTraverser traverser("gl_ViewID_OVR", "ViewID_OVR", viewIDQualifier);
    root->traverse(&traverser);
}

}  // namespace

void DeclareAndInitBuiltinsForInstancedMultiview(TIntermBlock *root,
                                                 unsigned numberOfViews,
                                                 GLenum shaderType,
                                                 int shaderVersion)
{
    ASSERT(shaderType == GL_VERTEX_SHADER || shaderType == GL_FRAGMENT_SHADER);

    TQualifier viewIDQualifier;
    if (shaderType == GL_VERTEX_SHADER)
    {
        viewIDQualifier = shaderVersion >= 300 ? EvqVertexOut : EvqVaryingOut;
    }
    else
    {
        viewIDQualifier = shaderVersion >= 300 ? EvqFragmentIn : EvqVaryingIn;
    }
    TIntermSymbol *viewIDSymbol = new TIntermSymbol(TSymbolTable::nextUniqueId(), "ViewID_OVR",
                                                    TType(EbtUInt, EbpHigh, viewIDQualifier));
    viewIDSymbol->setInternal(true);

    DeclareViewID(root, viewIDSymbol);
    if (shaderType == GL_VERTEX_SHADER)
    {
        // Altering gl_InstanceID to InstanceID should happen before adding the initializers of
        // InstanceID and ViewID.
        AlterGLInstanceID(root);
        // Declaration of InstanceID and initialization of ViewID and InstanceID should happen only
        // in the vertex shader.
        DeclareAndInitializeInstanceID(root, numberOfViews);
        InitializeViewID(root, viewIDSymbol->deepCopy(), numberOfViews);
    }
    AlterGLViewID(root, viewIDQualifier);
}

}  // namespace sh