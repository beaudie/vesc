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
#include "compiler/translator/IntermTraverse.h"
#include "compiler/translator/SymbolTable.h"
#include "compiler/translator/util.h"

#include <algorithm>

namespace sh
{

namespace
{

class ReplaceVariableTraverser : public TIntermTraverser
{
  public:
    ReplaceVariableTraverser(const TString &symbolName, TIntermSymbol *newSymbol)
        : TIntermTraverser(true, false, false), mSymbolName(symbolName), mNewSymbol(newSymbol)
    {
    }

    void visitSymbol(TIntermSymbol *node) override
    {
        TName &name = node->getName();
        if (name.getString() == mSymbolName)
        {
            queueReplacement(node, mNewSymbol->deepCopy(), OriginalNode::IS_DROPPED);
        }
    }

  private:
    TString mSymbolName;
    TIntermSymbol *mNewSymbol;
};

TIntermSymbol *CreateGLInstanceIDSymbol()
{
    return new TIntermSymbol(0, "gl_InstanceID", TType(EbtInt, EbpHigh, EvqInstanceID));
}

void InitializeViewIDAndInstanceID(TIntermBlock *root,
                                   TIntermTyped *viewIDSymbol,
                                   TIntermTyped *instanceIDSymbol,
                                   unsigned numberOfViews)
{
    // Create a signed numberOfViews node.
    TConstantUnion *numberOfViewsConstant = new TConstantUnion();
    numberOfViewsConstant->setIConst(static_cast<int>(numberOfViews));
    TIntermConstantUnion *numberOfViewsIntSymbol =
        new TIntermConstantUnion(numberOfViewsConstant, TType(EbtInt, EbpHigh, EvqConst));

    // Create a gl_InstanceID / numberOfViews node.
    TIntermBinary *normalizedInstanceID =
        new TIntermBinary(EOpDiv, CreateGLInstanceIDSymbol(), numberOfViewsIntSymbol);

    // Create a InstanceID = gl_InstanceID / numberOfViews node.
    TIntermBinary *instanceIDInitializer =
        new TIntermBinary(EOpAssign, instanceIDSymbol->deepCopy(), normalizedInstanceID);

    // Create a uint(gl_InstanceID) node.
    TIntermSequence *glInstanceIDSymbolCastArguments = new TIntermSequence();
    glInstanceIDSymbolCastArguments->push_back(CreateGLInstanceIDSymbol());
    TIntermAggregate *glInstanceIDAsUint = TIntermAggregate::CreateConstructor(
        TType(EbtUInt, EbpHigh, EvqTemporary), glInstanceIDSymbolCastArguments);

    // Create an unsigned numberOfViews node.
    TConstantUnion *numberOfViewsUnsignedConstant = new TConstantUnion();
    numberOfViewsUnsignedConstant->setUConst(numberOfViews);
    TIntermConstantUnion *numberOfViewsUintSymbol =
        new TIntermConstantUnion(numberOfViewsUnsignedConstant, TType(EbtUInt, EbpHigh, EvqConst));

    // Create a uint(gl_InstanceID) % numberOfViews node.
    TIntermBinary *normalizedViewID =
        new TIntermBinary(EOpIMod, glInstanceIDAsUint, numberOfViewsUintSymbol);

    // Create a ViewID_OVR = uint(gl_InstanceID) % numberOfViews node.
    TIntermBinary *viewIDInitializer =
        new TIntermBinary(EOpAssign, viewIDSymbol->deepCopy(), normalizedViewID);

    // Add initializers at the beginning of main().
    TIntermBlock *mainBody = FindMainBody(root);
    mainBody->getSequence()->insert(mainBody->getSequence()->begin(), instanceIDInitializer);
    mainBody->getSequence()->insert(mainBody->getSequence()->begin(), viewIDInitializer);
}

// Replaces every occurrence of a symbol with the name specified in symbolName with newSymbolNode.
void ReplaceSymbol(TIntermBlock *root, const TString &symbolName, TIntermSymbol *newSymbolNode)
{
    ReplaceVariableTraverser traverser(symbolName, newSymbolNode);
    root->traverse(&traverser);
    traverser.updateTree();
}

void DeclareGlobalVariable(TIntermBlock *root, TIntermTyped *typedNode)
{
    TIntermSequence *globalSequence = root->getSequence();
    TIntermDeclaration *declaration = new TIntermDeclaration();
    declaration->appendDeclarator(typedNode->deepCopy());
    globalSequence->insert(globalSequence->begin(), declaration);
}

// Adds the expression gl_ViewportIndex = int(ViewID_OVR) into the AST.
void SelectViewportIndexInVertexShader(TIntermBlock *root, TIntermTyped *viewIDSymbol)
{
    // Create a gl_ViewportIndex node.
    TIntermSymbol *viewportIndexSymbol =
        new TIntermSymbol(0, "gl_ViewportIndex", TType(EbtInt, EbpHigh, EvqViewportIndex));

    // Create an int(ViewID_OVR) node.
    TIntermSequence *viewIDSymbolCastArguments = new TIntermSequence();
    viewIDSymbolCastArguments->push_back(viewIDSymbol);
    TIntermAggregate *viewIDAsInt = TIntermAggregate::CreateConstructor(
        TType(EbtInt, EbpHigh, EvqTemporary), viewIDSymbolCastArguments);

    // Create a gl_ViewportIndex = int(ViewID_OVR) node.
    TIntermBinary *viewIDInitializer =
        new TIntermBinary(EOpAssign, viewportIndexSymbol, viewIDAsInt);

    // Insert just after ViewID and InstanceID are initialized.
    const size_t viewportIndexOffset           = 2u;
    TIntermBlock *mainBody                     = FindMainBody(root);
    TIntermSequence::iterator sequenceIterator = mainBody->getSequence()->begin();
    std::advance(sequenceIterator, viewportIndexOffset);
    mainBody->getSequence()->insert(sequenceIterator, viewIDInitializer);
}

}  // namespace

void DeclareAndInitBuiltinsForInstancedMultiview(TIntermBlock *root,
                                                 unsigned numberOfViews,
                                                 GLenum shaderType,
                                                 ShCompileOptions compileOptions,
                                                 ShShaderOutput shaderOutput)
{
    ASSERT(shaderType == GL_VERTEX_SHADER || shaderType == GL_FRAGMENT_SHADER);

    TQualifier viewIDQualifier  = (shaderType == GL_VERTEX_SHADER) ? EvqFlatOut : EvqFlatIn;
    TIntermSymbol *viewIDSymbol = new TIntermSymbol(TSymbolTable::nextUniqueId(), "ViewID_OVR",
                                                    TType(EbtUInt, EbpHigh, viewIDQualifier));
    viewIDSymbol->setInternal(true);

    DeclareGlobalVariable(root, viewIDSymbol);
    ReplaceSymbol(root, "gl_ViewID_OVR", viewIDSymbol);
    if (shaderType == GL_VERTEX_SHADER)
    {
        // Replacing gl_InstanceID with InstanceID should happen before adding the initializers of
        // InstanceID and ViewID.
        TIntermSymbol *instanceIDSymbol = new TIntermSymbol(
            TSymbolTable::nextUniqueId(), "InstanceID", TType(EbtInt, EbpHigh, EvqGlobal));
        instanceIDSymbol->setInternal(true);
        DeclareGlobalVariable(root, instanceIDSymbol);
        ReplaceSymbol(root, "gl_InstanceID", instanceIDSymbol);
        InitializeViewIDAndInstanceID(root, viewIDSymbol, instanceIDSymbol, numberOfViews);

        // The AST transformation which adds the expression to select the viewport index should
        // be done only for the GLSL and ESSL output.
        const bool selectViewport =
            (compileOptions & SH_SELECT_VIEW_IN_NV_GLSL_VERTEX_SHADER) != 0u;
        // Assert that if the viewport is selected in the vertex shader, then the output is
        // either GLSL or ESSL.
        ASSERT(!selectViewport || IsOutputGLSL(shaderOutput) || IsOutputESSL(shaderOutput));
        if (selectViewport)
        {
            SelectViewportIndexInVertexShader(root, viewIDSymbol->deepCopy());
        }
    }
}

}  // namespace sh