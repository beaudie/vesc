//
// Copyright 2020 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//
// ReplaceSampleMaskVariable.cpp: Find any references to gl_SampleMask and gl_SampleMaskIn, and
// replace it with ANGLESampleMask or ANGLESampleMaskIn.
//

#include "compiler/translator/tree_util/ReplaceSampleMaskVariables.h"

#include "common/bitset_utils.h"
#include "common/debug.h"
#include "common/utilities.h"
#include "compiler/translator/SymbolTable.h"
#include "compiler/translator/tree_util/BuiltIn.h"
#include "compiler/translator/tree_util/IntermNode_util.h"
#include "compiler/translator/tree_util/IntermTraverse.h"
#include "compiler/translator/tree_util/RunAtTheBeginOfShader.h"
#include "compiler/translator/tree_util/RunAtTheEndOfShader.h"

namespace sh
{
namespace
{

static constexpr int kFullSampleMask = 0xFFFFFFFF;

// Traverse the tree and collect the redeclaration and all constant index references of
// gl_SampleMask or gl_SampleMaskIn
class GLSampleMaskRelatedReferenceTraverser : public TIntermTraverser
{
  public:
    GLSampleMaskRelatedReferenceTraverser(const TIntermSymbol **redeclaredSymOut,
                                          bool *nonConstIdxUsedOut,
                                          const ImmutableString &targetStr)
        : TIntermTraverser(true, false, false),
          mRedeclaredSym(redeclaredSymOut),
          mUseNonConstSampleMaskIndex(nonConstIdxUsedOut),
          mTargetStr(targetStr)
    {
        *mRedeclaredSym              = nullptr;
        *mUseNonConstSampleMaskIndex = false;
    }

    bool visitDeclaration(Visit visit, TIntermDeclaration *node) override
    {
        // If gl_SampleMask is redeclared, we need to collect its information
        const TIntermSequence &sequence = *(node->getSequence());

        if (sequence.size() != 1)
        {
            return true;
        }

        TIntermTyped *variable = sequence.front()->getAsTyped();
        if (!variable->getAsSymbolNode() || variable->getAsSymbolNode()->getName() != mTargetStr)
        {
            return true;
        }

        *mRedeclaredSym = variable->getAsSymbolNode();

        return true;
    }

    bool visitBinary(Visit visit, TIntermBinary *node) override
    {
        TOperator op = node->getOp();
        if (op != EOpIndexDirect && op != EOpIndexIndirect)
        {
            return true;
        }
        TIntermSymbol *left = node->getLeft()->getAsSymbolNode();
        if (!left)
        {
            return true;
        }
        if (left->getName() != mTargetStr)
        {
            return true;
        }
        const TConstantUnion *constIdx = node->getRight()->getConstantValue();
        if (!constIdx)
        {
            *mUseNonConstSampleMaskIndex = true;
        }

        return true;
    }

  private:
    const TIntermSymbol **mRedeclaredSym;
    // Flag indicating whether there is at least one reference of gl_SampleMask or gl_SampleMaskIn
    // with non-constant index
    bool *mUseNonConstSampleMaskIndex;

    const ImmutableString mTargetStr;
};

// Replace all symbolic occurrences of given variables except one symbol.
class ReplaceVariableExceptOneTraverser : public TIntermTraverser
{
  public:
    ReplaceVariableExceptOneTraverser(const TVariable *toBeReplaced,
                                      const TIntermTyped *replacement,
                                      const TIntermSymbol *exception)
        : TIntermTraverser(true, false, false),
          mToBeReplaced(toBeReplaced),
          mException(exception),
          mReplacement(replacement)
    {}

    void visitSymbol(TIntermSymbol *node) override
    {
        if (&node->variable() == mToBeReplaced && node != mException)
        {
            queueReplacement(mReplacement->deepCopy(), OriginalNode::IS_DROPPED);
        }
    }

  private:
    const TVariable *const mToBeReplaced;
    const TIntermSymbol *const mException;
    const TIntermTyped *const mReplacement;
};

// Replace all gl_SampleMask except redeclaration with if-else block contains the original
// gl_SampleMask sentence
class WrapUpSampleMaskWithIfElseTraverser : public TIntermTraverser
{
  public:
    WrapUpSampleMaskWithIfElseTraverser(const TIntermBinary *condition,
                                        const TVariable *sampleMask,
                                        const TIntermSymbol *redeclaration)
        : TIntermTraverser(true, false, false),
          mToBeReplaced(sampleMask),
          mException(redeclaration),
          mCondition(condition)
    {}

    void visitSymbol(TIntermSymbol *node) override
    {
        // Only if the node which we want to replace is on left, we wrap up it with if-else block
        if (&node->variable() == mToBeReplaced && node != mException &&
            getParentNode()->getChildNode(0) == node)
        {
            // Example : gl_SampleMask[0] = int(0xAAAAAAAA);
            // node : gl_SampleMask
            // leftOfSentence : gl_SampleMask[0]
            // fullSentence : gl_SampleMask[0] = int(0xAAAAAAAA)
            TIntermNode *leftOfSentence       = getParentNode();
            TIntermNode *fullSentence         = getAncestorNode(1);
            TIntermNode *parentOfFullSentence = getAncestorNode(2);
            ASSERT(fullSentence && parentOfFullSentence);

            TIntermBlock *trueBlock = new TIntermBlock();
            trueBlock->appendStatement(fullSentence->deepCopy());

            TIntermBlock *falseBlock = new TIntermBlock();
            TIntermBinary *left      = leftOfSentence->deepCopy()->getAsBinaryNode();
            ASSERT(left);
            TIntermConstantUnion *fullSampleMask = CreateIndexNode(kFullSampleMask);
            TIntermBinary *assignment = new TIntermBinary(EOpAssign, left, fullSampleMask);
            falseBlock->appendStatement(assignment);

            TIntermIfElse *multiSampleOfNot =
                new TIntermIfElse(mCondition->deepCopy(), trueBlock, falseBlock);

            queueReplacementWithParent(parentOfFullSentence, fullSentence, multiSampleOfNot,
                                       OriginalNode::IS_DROPPED);
        }
    }

  private:
    const TVariable *const mToBeReplaced;
    const TIntermSymbol *const mException;
    const TIntermTyped *mCondition;
};

}  // anonymous namespace

ANGLE_NO_DISCARD bool ReplaceSampleMaskAssignments(TCompiler *compiler,
                                                   TIntermBlock *root,
                                                   TSymbolTable *symbolTable,
                                                   const TIntermTyped *numSamplesFlags)
{
    // Collect all constant index references of gl_SampleMask
    bool useNonConstIndex                       = false;
    const TIntermSymbol *redeclaredGLSampleMask = nullptr;
    GLSampleMaskRelatedReferenceTraverser indexTraverser(&redeclaredGLSampleMask, &useNonConstIndex,
                                                         ImmutableString("gl_SampleMask"));
    root->traverse(&indexTraverser);

    // Retrieve gl_SampleMask variable reference
    // Search user redeclared it first
    const TVariable *glSampleMaskVar = nullptr;
    if (redeclaredGLSampleMask)
    {
        glSampleMaskVar = &redeclaredGLSampleMask->variable();
    }
    else
    {
        // User defined not found, find in built-in table
        glSampleMaskVar = static_cast<const TVariable *>(
            symbolTable->findBuiltIn(ImmutableString("gl_SampleMask"), 320));
    }
    if (!glSampleMaskVar)
    {
        return false;
    }

    // Even if no need to replace gl_SampleMask with ANGLESampleMask, there is need to add
    // code block if the target is singlesample target to ignore multisample operation.
    TIntermConstantUnion *singleSampleCount = CreateUIntNode(1);
    TIntermBinary *greaterThan =
        new TIntermBinary(EOpGreaterThan, numSamplesFlags->deepCopy(), singleSampleCount);

    if (!useNonConstIndex)
    {
        WrapUpSampleMaskWithIfElseTraverser wrapUpTraverser(
            greaterThan, glSampleMaskVar, /** exception */ redeclaredGLSampleMask);
        root->traverse(&wrapUpTraverser);
        if (!wrapUpTraverser.updateTree(compiler, root))
        {
            return false;
        }

        return true;
    }

    // Declare a global variable substituting gl_SampleMask
    TType *sampleMaskType = new TType(EbtInt, EbpHigh, EvqGlobal, 1);
    sampleMaskType->makeArray(glSampleMaskVar->getType().getOutermostArraySize());
    sampleMaskType->realize();

    TVariable *sampleMaskVar = new TVariable(symbolTable, ImmutableString("ANGLESampleMask"),
                                             sampleMaskType, SymbolType::AngleInternal);

    TIntermSymbol *sampleMaskDeclarator = new TIntermSymbol(sampleMaskVar);
    TIntermDeclaration *sampleMaskDecl  = new TIntermDeclaration;
    sampleMaskDecl->appendDeclarator(sampleMaskDeclarator);

    // Must declare ANGLESampleMask before any function, since gl_SampleMask might be accessed
    // within a function declared before main.
    root->insertStatement(0, sampleMaskDecl);

    // Replace gl_SampleMask reference with ANGLESampleMask, except the declaration
    ReplaceVariableExceptOneTraverser replaceTraverser(
        glSampleMaskVar, new TIntermSymbol(sampleMaskVar), /** exception */ redeclaredGLSampleMask);
    root->traverse(&replaceTraverser);
    if (!replaceTraverser.updateTree(compiler, root))
    {
        return false;
    }

    TIntermSymbol *glSampleMaskSymbol = new TIntermSymbol(glSampleMaskVar);
    TIntermSymbol *sampleMaskSymbol   = new TIntermSymbol(sampleMaskVar);

    // if (ANGLEUniforms.numSamples > 1)
    // {
    //     gl_SampleMask[0] = ANGLESampleMask[0];
    //     gl_SampleMask[1] = ANGLESampleMask[1];
    // }
    // else
    // {
    //     gl_SampleMask[0] = int(0xFFFFFFFF);
    //     gl_SampleMask[1] = int(0xFFFFFFFF);
    // }
    TIntermBlock *trueBlock  = new TIntermBlock();
    TIntermBlock *falseBlock = new TIntermBlock();

    for (unsigned int i = 0; i < sampleMaskType->getOutermostArraySize(); ++i)
    {
        TIntermBinary *left =
            new TIntermBinary(EOpIndexDirect, glSampleMaskSymbol->deepCopy(), CreateIndexNode(i));

        // For true block of if statement
        TIntermBinary *right =
            new TIntermBinary(EOpIndexDirect, sampleMaskSymbol->deepCopy(), CreateIndexNode(i));
        TIntermBinary *assignmentTrue = new TIntermBinary(EOpAssign, left->deepCopy(), right);
        trueBlock->appendStatement(assignmentTrue);

        // For false block of if statement
        TIntermConstantUnion *fullSampleMask = CreateIndexNode(kFullSampleMask);
        TIntermBinary *assignmentFalse =
            new TIntermBinary(EOpAssign, left->deepCopy(), fullSampleMask);
        falseBlock->appendStatement(assignmentFalse);
    }

    TIntermIfElse *multiSampleOrNot = new TIntermIfElse(greaterThan, trueBlock, falseBlock);

    return RunAtTheEndOfShader(compiler, root, multiSampleOrNot, symbolTable);
}

ANGLE_NO_DISCARD bool ReplaceSampleMaskInAssignments(TCompiler *compiler,
                                                     TIntermBlock *root,
                                                     TSymbolTable *symbolTable)
{
    // Collect all constant index references of gl_SampleMaskIn
    bool useNonConstIndex                         = false;
    const TIntermSymbol *redeclaredGLSampleMaskIn = nullptr;
    GLSampleMaskRelatedReferenceTraverser indexTraverser(
        &redeclaredGLSampleMaskIn, &useNonConstIndex, ImmutableString("gl_SampleMaskIn"));
    root->traverse(&indexTraverser);

    if (!useNonConstIndex)
    {
        // No references of the target variable
        return true;
    }

    // Retrieve gl_SampleMaskIn variable reference
    // Search user redeclared it first
    const TVariable *glSampleMaskInVar = nullptr;
    glSampleMaskInVar                  = static_cast<const TVariable *>(
        symbolTable->findBuiltIn(ImmutableString("gl_SampleMaskIn"), 320));
    if (!glSampleMaskInVar)
    {
        return false;
    }

    // Declare a global variable substituting gl_SampleMaskIn
    TType *sampleMaskInType = new TType(EbtInt, EbpHigh, EvqGlobal, 1);
    sampleMaskInType->makeArray(glSampleMaskInVar->getType().getOutermostArraySize());
    sampleMaskInType->realize();

    TVariable *sampleMaskInVar = new TVariable(symbolTable, ImmutableString("ANGLESampleMaskIn"),
                                               sampleMaskInType, SymbolType::AngleInternal);

    TIntermSymbol *sampleMaskInDeclarator = new TIntermSymbol(sampleMaskInVar);
    TIntermDeclaration *sampleMaskInDecl  = new TIntermDeclaration;
    sampleMaskInDecl->appendDeclarator(sampleMaskInDeclarator);

    // Must declare ANGLESampleMaskIn before any function, since gl_SampleMaskIn might be accessed
    // within a function declared before main.
    root->insertStatement(0, sampleMaskInDecl);

    // Replace gl_SampleMaskIn reference with ANGLESampleMaskIn
    ReplaceVariableExceptOneTraverser replaceTraverser(
        glSampleMaskInVar, new TIntermSymbol(sampleMaskInVar), /** exception */ nullptr);
    root->traverse(&replaceTraverser);
    if (!replaceTraverser.updateTree(compiler, root))
    {
        return false;
    }

    TIntermBlock *reassignBlock         = new TIntermBlock;
    TIntermSymbol *glSampleMaskInSymbol = new TIntermSymbol(glSampleMaskInVar);
    TIntermSymbol *sampleMaskInSymbol   = new TIntermSymbol(sampleMaskInVar);

    // {
    //     ANGLESampleMaskIn[0] = gl_SampleMaskIn[0];
    //     ANGLESampleMaskIn[1] = gl_SampleMaskIn[1];
    // }
    for (unsigned int i = 0; i < sampleMaskInType->getOutermostArraySize(); ++i)
    {
        TIntermBinary *left =
            new TIntermBinary(EOpIndexDirect, sampleMaskInSymbol->deepCopy(), CreateIndexNode(i));
        TIntermBinary *right =
            new TIntermBinary(EOpIndexDirect, glSampleMaskInSymbol->deepCopy(), CreateIndexNode(i));
        TIntermBinary *assignment = new TIntermBinary(EOpAssign, left, right);
        reassignBlock->appendStatement(assignment);
    }

    return RunAtTheBeginOfShader(compiler, root, reassignBlock);
}

}  // namespace sh
