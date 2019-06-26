//
// Copyright 2019 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//
// Implementation of dFdy Y viewport transformation.
// See header for more info.

#include "compiler/translator/tree_ops/RewriteDfdy.h"

#include "common/angleutils.h"
#include "compiler/translator/SymbolTable.h"
#include "compiler/translator/tree_util/IntermNode_util.h"
#include "compiler/translator/tree_util/IntermTraverse.h"

namespace sh
{

namespace
{

class Traverser : public TIntermTraverser
{
  public:
    static void Apply(TIntermNode *root,
                      const TSymbolTable &symbolTable,
                      TIntermBinary *viewportYScale);

  private:
    Traverser(TIntermBinary *viewportYScale, TSymbolTable *symbolTable);
    bool visitUnary(Visit visit, TIntermUnary *node) override;

    TIntermBinary *mViewportYScale = nullptr;
};

Traverser::Traverser(TIntermBinary *viewportYScale, TSymbolTable *symbolTable)
    : TIntermTraverser(true, false, false, symbolTable), mViewportYScale(viewportYScale)
{}

// static
void Traverser::Apply(TIntermNode *root,
                      const TSymbolTable &symbolTable,
                      TIntermBinary *viewportYScale)
{
    TSymbolTable *pSymbolTable = const_cast<TSymbolTable *>(&symbolTable);
    Traverser traverser(viewportYScale, pSymbolTable);
    root->traverse(&traverser);
    traverser.updateTree();
}

bool Traverser::visitUnary(Visit visit, TIntermUnary *node)
{
    // Decide if the node represents a call to dFdy() with an output containing a Y component.
    if (node->getOp() != EOpDFdy || (node->getType().getObjectSize() < 2))
    {
        return true;
    }

    // Create the workaround
    // Copy the dFdy node so we can replace it
    TIntermUnary *newDfdy = node->deepCopy()->getAsUnaryNode();

    // Create the replacement sequence to perform the work to correct dFdy()'s Y value
    TIntermSequence *replacementSequence = new TIntermSequence();

    // Create a temp variable to store the result of dFdy():
    // vec2 s413 = dFdy(_uv_coord);
    TVariable *dfdyResultTemp          = CreateTempVariable(mSymbolTable, &newDfdy->getType());
    TIntermDeclaration *dfdyResultDecl = CreateTempInitDeclarationNode(dfdyResultTemp, newDfdy);
    TIntermSymbol *dfdyResult          = CreateTempSymbolNode(dfdyResultTemp);
    replacementSequence->push_back(dfdyResultDecl);

    // Create a swizzle to "dfdyResult.y"
    TVector<int> swizzleY;
    swizzleY.push_back(1);
    TIntermSwizzle *dfdyResultSwizzleY = new TIntermSwizzle(dfdyResult, swizzleY);

    // Correct dfdyResult's Y value:
    // (s413.y *= ANGLEUniforms.viewportYScale);
    TIntermBinary *correctedY =
        new TIntermBinary(EOpMulAssign, dfdyResultSwizzleY, mViewportYScale);
    replacementSequence->push_back(correctedY);

    // Insert the new sequence into the graph before dFdy is called
    insertStatementsInParentBlock(*replacementSequence);

    // Replace the old dFdy node with the new temp variable node that contains the corrected Y value
    queueReplacement(dfdyResult, OriginalNode::IS_DROPPED);

    return true;
}

}  // anonymous namespace

void RewriteDfdy(TIntermNode *root,
                 const TSymbolTable &symbolTable,
                 int shaderVersion,
                 TIntermBinary *viewportYScale)
{
    // dFdy is only valid in GLSL 3.0 and later.
    if (shaderVersion < 300)
        return;

    Traverser::Apply(root, symbolTable, viewportYScale);
}

}  // namespace sh