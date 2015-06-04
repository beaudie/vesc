//
// Copyright (c) 2002-2015 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//
// During parsing, all constant expressions are folded to constant union nodes. The expressions that have been
// folded may have had precision qualifiers, which should affect the precision of the consuming operation.
// If the folded constant union nodes are written to output as such they won't have any precision qualifiers,
// and their effect on the precision of the consuming operation is lost.
//
// RecordConstantPrecision is an AST traverser that inspects the precision qualifiers of constants and hoists
// the constants outside the containing expression as named and precision qualified variables in case that is
// required for correct precision propagation.
//

#include "compiler/translator/RecordConstantPrecision.h"

#include "compiler/translator/InfoSink.h"
#include "compiler/translator/IntermNode.h"

namespace
{

class RecordConstantPrecisionTraverser : public TIntermTraverser
{
  public:
    RecordConstantPrecisionTraverser();

    void visitConstantUnion(TIntermConstantUnion *node) override;

    void nextIteration();

    bool foundHigherPrecisionConstant() const { return mFoundHigherPrecisionConstant; }
  protected:
    bool mFoundHigherPrecisionConstant;
};

RecordConstantPrecisionTraverser::RecordConstantPrecisionTraverser()
    : TIntermTraverser(true, false, true),
      mFoundHigherPrecisionConstant(false)
{
}

void RecordConstantPrecisionTraverser::visitConstantUnion(TIntermConstantUnion *node)
{
    if (mFoundHigherPrecisionConstant)
        return;

    // If the constant has lowp or undefined precision, it can't increase the precision of consuming operations.
    if (node->getPrecision() < EbpMedium)
        return;
    TIntermBinary *parentAsBinary = getParentNode()->getAsBinaryNode();
    if (parentAsBinary != nullptr)
    {
        // If the constant is assigned or is used to initialize a variable, or if it's an index,
        // its precision has no effect.
        switch (parentAsBinary->getOp())
        {
            case EOpInitialize:
            case EOpAssign:
            case EOpIndexDirect:
            case EOpIndexDirectStruct:
            case EOpIndexDirectInterfaceBlock:
            case EOpIndexIndirect:
              return;
            default:
              break;
        }
        return;
    }

    // If the precisions of other operands in the expression are lower than the precision of the constant variable,
    // make the constant a precision-qualified named variable to make sure it affects the precision of the consuming
    // expression. For now, doing this conservatively by always processing constants with precision >= mediump.
    // TODO: Traverse the consuming expression to find out if this really is necessary.
    TIntermSequence insertions;
    insertions.push_back(createTempInitDeclaration(node, EvqConst));
    insertStatementsInParentBlock(insertions);
    mReplacements.push_back(NodeUpdateEntry(getParentNode(), node, createTempSymbol(node->getType()), false));
    mFoundHigherPrecisionConstant = true;
}

void RecordConstantPrecisionTraverser::nextIteration()
{
    nextTemporaryIndex();
    mFoundHigherPrecisionConstant = false;
}

} // namespace

void RecordConstantPrecision(TIntermNode *root, unsigned int *temporaryIndex)
{
    RecordConstantPrecisionTraverser traverser;
    ASSERT(temporaryIndex != nullptr);
    traverser.useTemporaryIndex(temporaryIndex);
    // Iterate as necessary, and reset the traverser between iterations.
    do
    {
        traverser.nextIteration();
        root->traverse(&traverser);
        if (traverser.foundHigherPrecisionConstant())
            traverser.updateTree();
    }
    while (traverser.foundHigherPrecisionConstant());
}
