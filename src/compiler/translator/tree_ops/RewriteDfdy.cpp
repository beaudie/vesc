//
// Copyright (c) 2016 The ANGLE Project Authors. All rights reserved.
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
    TIntermUnary *newDfdy = node->deepCopy()->getAsUnaryNode();
    (void)newDfdy;

    // Create a const vec of 1.0's
    size_t objectSize = newDfdy->getType().getObjectSize();
    TConstantUnion *u = new TConstantUnion[objectSize];
    for (size_t i = 0; i < objectSize; i++)
    {
        u[i].setFConst(1.0f);
    }
    TType constType(newDfdy->getType());
    constType.setQualifier(EvqConst);
    TIntermConstantUnion *viewportScaleConst = new TIntermConstantUnion(u, constType);

    // Store the vec in a temp variable
    TVariable *viewportScaleTemp = CreateTempVariable(mSymbolTable, &newDfdy->getType());
    TIntermDeclaration *viewportScaleDecl =
        CreateTempInitDeclarationNode(viewportScaleTemp, viewportScaleConst);
    TIntermSymbol *viewportScale = CreateTempSymbolNode(viewportScaleTemp);

    // Create a swizzle to "viewportScale.y"
    TVector<int> swizzleY;
    swizzleY.push_back(1);

    // Create an assignment to the replaced variable's y.
    TIntermSequence *replacement = new TIntermSequence();
    replacement->push_back(viewportScaleDecl);
    TIntermSwizzle *correctedY = new TIntermSwizzle(viewportScale, swizzleY);
    TIntermBinary *assignToY   = new TIntermBinary(EOpAssign, correctedY, mViewportYScale);
    replacement->push_back(assignToY);

    // dFdy() * viewportScale
    TIntermBinary *scaleDfdy = new TIntermBinary(EOpMul, newDfdy, viewportScale);

    insertStatementsInParentBlock(*replacement);

    // Replace the old node by this new node.
    queueReplacement(scaleDfdy, OriginalNode::IS_DROPPED);

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