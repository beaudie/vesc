//
// Copyright 2020 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//
// ReplaceClipDistanceVariable.cpp: Find any references to gl_ClipDistance or gl_CullDistance
// and replace it with ANGLEClipDistance or ANGLECullDistance.
//

#include "compiler/translator/tree_util/ReplaceClipDistanceVariable.h"

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

using ClipCullDistanceIdxSet = angle::BitSet<32>;

typedef TIntermNode *AssignFunc(const unsigned int index,
                                TIntermSymbol *left,
                                TIntermSymbol *right,
                                const TIntermTyped *enableFlags);

template <typename Variable>
const Variable *FindVariable(const std::vector<Variable> &mVars, const ImmutableString &name)
{
    for (const Variable &var : mVars)
    {
        if (name == var.instanceName)
        {
            return &var;
        }
    }

    return nullptr;
}

// Traverse the tree and collect the redeclaration and all constant index references of
// gl_ClipDistance/gl_CullDistance
class GLClipCullDistanceReferenceTraverser : public TIntermTraverser
{
  public:
    GLClipCullDistanceReferenceTraverser(const TIntermSymbol **redeclaredSymOut,
                                         bool *nonConstIdxUsedOut,
                                         unsigned int *maxConstIdxOut,
                                         ClipCullDistanceIdxSet *constIndicesOut,
                                         const ImmutableString &targetStr)
        : TIntermTraverser(true, false, false),
          mRedeclaredSym(redeclaredSymOut),
          mUseNonConstClipCullDistanceIndex(nonConstIdxUsedOut),
          mMaxConstClipCullDistanceIndex(maxConstIdxOut),
          mConstClipCullDistanceIndices(constIndicesOut),
          mTargetStr(targetStr)
    {
        *mRedeclaredSym                    = nullptr;
        *mUseNonConstClipCullDistanceIndex = false;
        *mMaxConstClipCullDistanceIndex    = 0;
        mConstClipCullDistanceIndices->reset();
    }

    bool visitDeclaration(Visit visit, TIntermDeclaration *node) override
    {
        // If gl_ClipDistance/gl_CullDistance is redeclared, we need to collect its information
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
        if (op != EOpIndexDirect && op != EOpIndexIndirect && op != EOpIndexDirectInterfaceBlock)
        {
            return true;
        }

        // gl_in[] / gl_out[] / gl_ClipDistance / gl_CullDistance
        TIntermTyped *left = node->getLeft()->getAsTyped();
        if (!left)
        {
            return true;
        }

        TIntermSymbol *clipCullDistance = nullptr;
        const TConstantUnion *constIdx  = nullptr;
        if (op == EOpIndexDirectInterfaceBlock)
        {
            // Need to implement it for gl_Clip/CullDistance in gl_PerVertex interfaceblock
            UNIMPLEMENTED();
            return true;
        }
        else
        {
            ASSERT(op == EOpIndexDirect || op == EOpIndexIndirect);
            clipCullDistance = left->getAsSymbolNode();
            if (!clipCullDistance)
            {
                return true;
            }
            if (clipCullDistance->getName() != mTargetStr)
            {
                return true;
            }

            constIdx = node->getRight()->getConstantValue();
        }

        if (!constIdx)
        {
            *mUseNonConstClipCullDistanceIndex = true;
        }
        else
        {
            unsigned int idx = 0;
            switch (constIdx->getType())
            {
                case EbtInt:
                    idx = constIdx->getIConst();
                    break;
                case EbtUInt:
                    idx = constIdx->getUConst();
                    break;
                case EbtFloat:
                    idx = static_cast<unsigned int>(constIdx->getFConst());
                    break;
                case EbtBool:
                    idx = constIdx->getBConst() ? 1 : 0;
                    break;
                default:
                    UNREACHABLE();
                    break;
            }
            ASSERT(idx < mConstClipCullDistanceIndices->size());
            mConstClipCullDistanceIndices->set(idx);

            *mMaxConstClipCullDistanceIndex = std::max(*mMaxConstClipCullDistanceIndex, idx);
        }

        return true;
    }

  private:
    const TIntermSymbol **mRedeclaredSym;
    // Flag indicating whether there is at least one reference of gl_ClipDistance with non-constant
    // index
    bool *mUseNonConstClipCullDistanceIndex;
    // Max constant index that is used to reference gl_ClipDistance
    unsigned int *mMaxConstClipCullDistanceIndex;
    // List of constant index reference of gl_ClipDistance
    ClipCullDistanceIdxSet *mConstClipCullDistanceIndices;
    // String for gl_ClipDistance/gl_CullDistance
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

unsigned int EnabledClipCullDistance(const bool useNonConstIndex,
                                     const unsigned int maxConstIndex,
                                     const unsigned int builtInVarArraySize,
                                     const TIntermSymbol *redeclaredGLClipCullDistance)
{
    unsigned enabledClipDistances = 0;

    if (redeclaredGLClipCullDistance)
    {
        // If array is redeclared by user, use that redeclared size.
        enabledClipDistances = redeclaredGLClipCullDistance->getType().getOutermostArraySize();
    }
    else if (!useNonConstIndex)
    {
        ASSERT(maxConstIndex < builtInVarArraySize);
        // Only use constant index, then use max array index used.
        enabledClipDistances = maxConstIndex + 1;
    }

    return enabledClipDistances;
}

TVariable *DeclareANGLEVariable(TIntermBlock *root,
                                TSymbolTable *symbolTable,
                                unsigned int arraySize,
                                const ImmutableString &varName)
{
    TType *clipCullDistanceType = new TType(EbtFloat, EbpMedium, EvqGlobal, 1);

    clipCullDistanceType->makeArray(arraySize);
    clipCullDistanceType->realize();

    TVariable *clipCullDistanceVar =
        new TVariable(symbolTable, varName, clipCullDistanceType, SymbolType::AngleInternal);

    TIntermSymbol *clipCullDistanceDeclarator = new TIntermSymbol(clipCullDistanceVar);
    TIntermDeclaration *clipCullDistanceDecl  = new TIntermDeclaration;
    clipCullDistanceDecl->appendDeclarator(clipCullDistanceDeclarator);

    // Must declare ANGLEClipdistance/ANGLECullDistance before any function, since
    // gl_ClipDistance/gl_CullDistance might be accessed within a function declared before main.
    root->insertStatement(0, clipCullDistanceDecl);

    return clipCullDistanceVar;
}

TIntermNode *simpleAssignFunc(const unsigned int index,
                              TIntermSymbol *leftSymbol,
                              TIntermSymbol *rightSymbol,
                              const TIntermTyped * /*enableFlags*/)
{
    // leftSymbol[index] = rightSymbol[index]
    // E.g., ANGLEClipDistance[index] = gl_ClipDistance[index]
    TIntermBinary *left =
        new TIntermBinary(EOpIndexDirect, leftSymbol->deepCopy(), CreateIndexNode(index));
    TIntermBinary *right =
        new TIntermBinary(EOpIndexDirect, rightSymbol->deepCopy(), CreateIndexNode(index));

    return new TIntermBinary(EOpAssign, left, right);
}

// This is only used for gl_Clipdistance
TIntermNode *assignFuncWithEnableFlags(const unsigned int index,
                                       TIntermSymbol *leftSymbol,
                                       TIntermSymbol *rightSymbol,
                                       const TIntermTyped *enableFlags)
{
    //  if (ANGLEUniforms.clipDistancesEnabled & (0x1 << index))
    //      gl_ClipDistance[index] = ANGLEClipDistance[index];
    //  else
    //      gl_ClipDistance[index] = 0;
    TIntermConstantUnion *bitMask = CreateUIntNode(0x1 << index);
    TIntermBinary *bitwiseAnd = new TIntermBinary(EOpBitwiseAnd, enableFlags->deepCopy(), bitMask);
    TIntermBinary *nonZero    = new TIntermBinary(EOpNotEqual, bitwiseAnd, CreateUIntNode(0));

    TIntermBinary *left =
        new TIntermBinary(EOpIndexDirect, leftSymbol->deepCopy(), CreateIndexNode(index));
    TIntermBinary *right =
        new TIntermBinary(EOpIndexDirect, rightSymbol->deepCopy(), CreateIndexNode(index));
    TIntermBinary *assignment = new TIntermBinary(EOpAssign, left, right);
    TIntermBlock *trueBlock   = new TIntermBlock();
    trueBlock->appendStatement(assignment);

    TIntermBinary *zeroAssignment =
        new TIntermBinary(EOpAssign, left->deepCopy(), CreateFloatNode(0));
    TIntermBlock *falseBlock = new TIntermBlock();
    falseBlock->appendStatement(zeroAssignment);

    return new TIntermIfElse(nonZero, trueBlock, falseBlock);
}

ANGLE_NO_DISCARD bool ReadOriginalValueToANGLEVariable(TCompiler *compiler,
                                                       TIntermBlock *root,
                                                       const TVariable *clipCullDistanceVar,
                                                       const TVariable *glClipCullDistanceVar,
                                                       const unsigned int enabledClipCullDistances)
{
    TIntermBlock *readBlock                 = new TIntermBlock;
    TIntermSymbol *glClipCullDistanceSymbol = new TIntermSymbol(glClipCullDistanceVar);
    TIntermSymbol *clipCullDistanceSymbol   = new TIntermSymbol(clipCullDistanceVar);

    for (unsigned int i = 0; i < enabledClipCullDistances; i++)
    {
        readBlock->appendStatement(
            simpleAssignFunc(i, glClipCullDistanceSymbol, clipCullDistanceSymbol, nullptr));
    }

    return RunAtTheBeginningOfShader(compiler, root, readBlock);
}

ANGLE_NO_DISCARD bool ReassignANGLEValueToOriginalVariable(
    TCompiler *compiler,
    TIntermBlock *root,
    TSymbolTable *symbolTable,
    const TIntermTyped *enableFlags,
    const bool isRedeclared,
    const TVariable *clipCullDistanceVar,
    const TVariable *glClipCullDistanceVar,
    const unsigned int enabledClipCullDistances,
    const ClipCullDistanceIdxSet *constIndices,
    AssignFunc assignFunc)
{
    TIntermBlock *reassignBlock             = new TIntermBlock;
    TIntermSymbol *glClipCullDistanceSymbol = new TIntermSymbol(glClipCullDistanceVar);
    TIntermSymbol *clipCullDistanceSymbol   = new TIntermSymbol(clipCullDistanceVar);

    // The array size is decided by either redeclaring the variable or accessing the variable with a
    // integral constant index. And this size is the count of the enabled value. So, if the index
    // which is greater than the array size, is used to access the variable, this access will be
    // ignored.
    if (isRedeclared || !constIndices)
    {
        for (unsigned int i = 0; i < enabledClipCullDistances; ++i)
        {
            reassignBlock->appendStatement(
                assignFunc(i, glClipCullDistanceSymbol, clipCullDistanceSymbol, enableFlags));
        }
    }
    else
    {
        // Assign ANGLEClip/CullDistance[i]'s value to gl_Clip/CullDistance[i] if i is in the
        // constant indices list. Those elements whose index is not in the constant index list will
        // be zeroise for initialization.
        for (unsigned int i = 0; i < enabledClipCullDistances; ++i)
        {
            if (constIndices->test(i))
            {
                reassignBlock->appendStatement(
                    assignFunc(i, glClipCullDistanceSymbol, clipCullDistanceSymbol, enableFlags));
            }
            else
            {
                // gl_Clip/CullDistance[i] = 0;
                TIntermBinary *left = new TIntermBinary(
                    EOpIndexDirect, glClipCullDistanceSymbol->deepCopy(), CreateIndexNode(i));
                TIntermBinary *zeroAssignment =
                    new TIntermBinary(EOpAssign, left, CreateFloatNode(0));
                reassignBlock->appendStatement(zeroAssignment);
            }
        }
    }

    return RunAtTheEndOfShader(compiler, root, reassignBlock, symbolTable);
}

}  // anonymous namespace

ANGLE_NO_DISCARD bool ReplaceClipDistanceAssignments(
    TCompiler *compiler,
    TIntermBlock *root,
    TSymbolTable *symbolTable,
    const GLenum shaderType,
    const TIntermTyped *clipDistanceEnableFlags,
    const std::vector<sh::InterfaceBlock> &interfaceBlocks)
{
    // Collect all constant index references of gl_ClipDistance
    ImmutableString glClipDistanceName("gl_ClipDistance");
    ClipCullDistanceIdxSet constIndices;
    bool useNonConstIndex                         = false;
    const TIntermSymbol *redeclaredGLClipDistance = nullptr;
    unsigned int maxConstIndex                    = 0;
    GLClipCullDistanceReferenceTraverser indexTraverser(&redeclaredGLClipDistance,
                                                        &useNonConstIndex, &maxConstIndex,
                                                        &constIndices, glClipDistanceName);
    root->traverse(&indexTraverser);
    if (!useNonConstIndex && constIndices.none())
    {
        // No references of gl_ClipDistance
        return true;
    }

    // Retrieve gl_ClipDistance variable reference
    // Search user redeclared gl_ClipDistance first
    const TVariable *glClipDistanceVar = nullptr;
    if (redeclaredGLClipDistance)
    {
        glClipDistanceVar = &redeclaredGLClipDistance->variable();
    }
    else
    {
        // User defined not found, find in built-in table
        glClipDistanceVar =
            static_cast<const TVariable *>(symbolTable->findBuiltIn(glClipDistanceName, 300));
    }
    if (!glClipDistanceVar)
    {
        return false;
    }

    // Declare a global variable substituting gl_ClipDistance
    unsigned int enabledClipDistances = EnabledClipCullDistance(
        useNonConstIndex, maxConstIndex, glClipDistanceVar->getType().getOutermostArraySize(),
        redeclaredGLClipDistance);
    if (!enabledClipDistances)
    {
        // Spec :
        // The gl_ClipDistance array is predeclared as unsized and must be explicitly sized by the
        // shader either redeclaring it with a size or implicitly sized by indexing it only with
        // integral constant expressions.
        return false;
    }

    const TVariable *clipDistanceVar = DeclareANGLEVariable(root, symbolTable, enabledClipDistances,
                                                            ImmutableString("ANGLEClipDistance"));

    // Replace gl_ClipDistance reference with ANGLEClipDistance, except the declaration
    ReplaceVariableExceptOneTraverser replaceTraverser(glClipDistanceVar,
                                                       new TIntermSymbol(clipDistanceVar),
                                                       /** exception */ redeclaredGLClipDistance);
    root->traverse(&replaceTraverser);
    if (!replaceTraverser.updateTree(compiler, root))
    {
        return false;
    }

    // Read gl_ClipDistance to ANGLEClipDistance for getting a original data
    switch (shaderType)
    {
        case GL_VERTEX_SHADER:
            // Vertex shader can use gl_ClipDistance as a output only
            break;
        case GL_TESS_CONTROL_SHADER:
        case GL_TESS_EVALUATION_SHADER:
        case GL_GEOMETRY_SHADER:
        case GL_FRAGMENT_SHADER:
        {
            // These shader types can use gl_ClipDistance as input
            if (!ReadOriginalValueToANGLEVariable(compiler, root, glClipDistanceVar,
                                                  clipDistanceVar, enabledClipDistances))
            {
                return false;
            }
            break;
        }
        default:
        {
            UNREACHABLE();
            return false;
        }
    }

    // Reassign ANGLEClipDistance to gl_ClipDistance but ignore those that are disabled
    const bool isRedeclared = (redeclaredGLClipDistance != nullptr);
    switch (shaderType)
    {
        case GL_VERTEX_SHADER:
        {
            // Vertex shader can use gl_ClipDistance as output
            if (!ReassignANGLEValueToOriginalVariable(
                    compiler, root, symbolTable, clipDistanceEnableFlags, isRedeclared,
                    clipDistanceVar, glClipDistanceVar, enabledClipDistances, &constIndices,
                    assignFuncWithEnableFlags))
            {
                return false;
            }
            break;
        }
        case GL_TESS_CONTROL_SHADER:
        case GL_TESS_EVALUATION_SHADER:
        case GL_GEOMETRY_SHADER:
        {
            // These shader types can use gl_ClipDistance as output
            // If gl_ClipDistance variable isn't used even if it is enabled, the value of that is
            // inherited from the previous shader stage.
            if (!ReassignANGLEValueToOriginalVariable(
                    compiler, root, symbolTable, clipDistanceEnableFlags, isRedeclared,
                    clipDistanceVar, glClipDistanceVar, enabledClipDistances, nullptr,
                    assignFuncWithEnableFlags))
            {
                return false;
            }
            break;
        }
        case GL_FRAGMENT_SHADER:
            // Fragment shader can use gl_ClipDistance as input only
            break;
        default:
        {
            UNREACHABLE();
            return false;
        }
    }

    return true;
}

ANGLE_NO_DISCARD bool ReplaceCullDistanceAssignments(
    TCompiler *compiler,
    TIntermBlock *root,
    TSymbolTable *symbolTable,
    const GLenum shaderType,
    const std::vector<sh::InterfaceBlock> &interfaceBlocks)
{
    // Collect all constant index references of gl_CullDistance
    ImmutableString glCullDistanceName("gl_CullDistance");
    ClipCullDistanceIdxSet constIndices;
    bool useNonConstIndex                         = false;
    const TIntermSymbol *redeclaredGLCullDistance = nullptr;
    unsigned int maxConstIndex                    = 0;
    GLClipCullDistanceReferenceTraverser indexTraverser(&redeclaredGLCullDistance,
                                                        &useNonConstIndex, &maxConstIndex,
                                                        &constIndices, glCullDistanceName);
    root->traverse(&indexTraverser);
    if (!useNonConstIndex)
    {
        // Nothing to do
        return true;
    }

    // Retrieve gl_CullDistance variable reference
    // Search user redeclared gl_CullDistance first
    const TVariable *glCullDistanceVar = nullptr;
    if (redeclaredGLCullDistance)
    {
        glCullDistanceVar = &redeclaredGLCullDistance->variable();
    }
    else
    {
        // User defined not found, find in built-in table
        glCullDistanceVar =
            static_cast<const TVariable *>(symbolTable->findBuiltIn(glCullDistanceName, 300));
    }
    if (!glCullDistanceVar)
    {
        return false;
    }

    // Declare a global variable substituting gl_CullDistance
    unsigned int enabledCullDistances = EnabledClipCullDistance(
        useNonConstIndex, maxConstIndex, glCullDistanceVar->getType().getOutermostArraySize(),
        redeclaredGLCullDistance);
    if (!enabledCullDistances)
    {
        // Spec :
        // The gl_CullDistance array is predeclared as unsized and must be sized by the shader
        // either redeclaring it with a size or indexing it only with integral constant expressions.
        return false;
    }

    const TVariable *cullDistanceVar = DeclareANGLEVariable(root, symbolTable, enabledCullDistances,
                                                            ImmutableString("ANGLECullDistance"));

    // Replace gl_CullDistance reference with ANGLECullDistance, except the declaration
    ReplaceVariableExceptOneTraverser replaceTraverser(glCullDistanceVar,
                                                       new TIntermSymbol(cullDistanceVar),
                                                       /** exception */ redeclaredGLCullDistance);
    root->traverse(&replaceTraverser);
    if (!replaceTraverser.updateTree(compiler, root))
    {
        return false;
    }

    // Read gl_CullDistance to ANGLECullDistance for getting a original data
    switch (shaderType)
    {
        case GL_VERTEX_SHADER:
            // Vertex shader can use gl_CullDistance as a output only
            break;
        case GL_TESS_CONTROL_SHADER:
        case GL_TESS_EVALUATION_SHADER:
        case GL_GEOMETRY_SHADER:
        case GL_FRAGMENT_SHADER:
        {
            // These shader types can use gl_CullDistance as input
            if (!ReadOriginalValueToANGLEVariable(compiler, root, glCullDistanceVar,
                                                  cullDistanceVar, enabledCullDistances))
            {
                return false;
            }
            break;
        }
        default:
        {
            UNREACHABLE();
            return false;
        }
    }

    // Reassign ANGLECullDistance to gl_CullDistance but ignore those that are disabled
    const bool isRedeclared = (redeclaredGLCullDistance != nullptr);
    switch (shaderType)
    {
        case GL_VERTEX_SHADER:
        {
            // Vertex shader can use gl_CullDistance as output
            if (!ReassignANGLEValueToOriginalVariable(
                    compiler, root, symbolTable, nullptr, isRedeclared, cullDistanceVar,
                    glCullDistanceVar, enabledCullDistances, &constIndices, simpleAssignFunc))
            {
                return false;
            }
            break;
        }
        case GL_TESS_CONTROL_SHADER:
        case GL_TESS_EVALUATION_SHADER:
        case GL_GEOMETRY_SHADER:
        {
            // These shader types can use gl_ClipDistance as output
            // If gl_ClipDistance variable isn't used even if it is enabled, the value of that is
            // inherited from the previous shader stage.
            if (!ReassignANGLEValueToOriginalVariable(
                    compiler, root, symbolTable, nullptr, isRedeclared, cullDistanceVar,
                    glCullDistanceVar, enabledCullDistances, nullptr, simpleAssignFunc))
            {
                return false;
            }
            break;
        }
        case GL_FRAGMENT_SHADER:
            // Fragment shader can use gl_ClipDistance as input only
            break;
        default:
        {
            UNREACHABLE();
            return false;
        }
    }

    return true;
}

}  // namespace sh
