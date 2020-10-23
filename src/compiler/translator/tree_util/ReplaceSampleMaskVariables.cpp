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
#include "compiler/translator/tree_util/RunAtTheBeginningOfShader.h"
#include "compiler/translator/tree_util/RunAtTheEndOfShader.h"

namespace sh
{
namespace
{
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

}  // anonymous namespace

ANGLE_NO_DISCARD bool ReplaceSampleMaskToANGLESampleMask(TCompiler *compiler,
                                                         TIntermBlock *root,
                                                         TSymbolTable *symbolTable)
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

    TIntermBlock *reassignBlock       = new TIntermBlock;
    TIntermSymbol *glSampleMaskSymbol = new TIntermSymbol(glSampleMaskVar);
    TIntermSymbol *sampleMaskSymbol   = new TIntermSymbol(sampleMaskVar);

    // {
    //     gl_SampleMask[0] = ANGLESampleMask[0];
    //     gl_SampleMask[1] = ANGLESampleMask[1];
    // }
    for (unsigned int i = 0; i < sampleMaskType->getOutermostArraySize(); ++i)
    {
        TIntermBinary *left =
            new TIntermBinary(EOpIndexDirect, glSampleMaskSymbol->deepCopy(), CreateIndexNode(i));
        TIntermBinary *right =
            new TIntermBinary(EOpIndexDirect, sampleMaskSymbol->deepCopy(), CreateIndexNode(i));
        TIntermBinary *assignment = new TIntermBinary(EOpAssign, left, right);
        reassignBlock->appendStatement(assignment);
    }

    return RunAtTheEndOfShader(compiler, root, reassignBlock, symbolTable);
}

ANGLE_NO_DISCARD bool ReplaceSampleMaskInToANGLESampleMaskIn(TCompiler *compiler,
                                                             TIntermBlock *root,
                                                             TSymbolTable *symbolTable)
{
    // Collect all constant index references of gl_SampleMaskIn
    bool useNonConstIndex                         = false;
    const TIntermSymbol *redeclaredGLSampleMaskIn = nullptr;
    GLSampleMaskRelatedReferenceTraverser indexTraverser(
        &redeclaredGLSampleMaskIn, &useNonConstIndex, ImmutableString("gl_SampleMaskIn"));
    root->traverse(&indexTraverser);

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

    return RunAtTheBeginningOfShader(compiler, root, reassignBlock);
}

}  // namespace sh
