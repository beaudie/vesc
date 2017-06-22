//
// Copyright (c) 2017 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//
// InitOutputVariables_test.cpp: Tests correctness of the AST pass enabled through
// SH_INIT_OUTPUT_VARIABLES.
//

#include "common/angleutils.h"

#include "compiler/translator/FindMain.h"
#include "compiler/translator/IntermNode.h"
#include "tests/test_utils/ShaderCompileTreeTest.h"

#include <algorithm>

namespace sh
{

namespace
{

typedef std::vector<const TIntermBinary *> ExpectedAssignmentTargets;

bool AreTypesEqual(const TType &typeA, const TType &typeB)
{
    return typeA == typeB && typeA.getPrecision() == typeB.getPrecision() &&
           typeA.getQualifier() == typeB.getQualifier();
}

bool AreAssigmentsTheSame(const TIntermBinary *expected, const TIntermBinary *candidate)
{
    const TIntermSymbol *expectedLeftNode = expected->getLeft()->getAsSymbolNode();
    ASSERT(expectedLeftNode != nullptr);
    const TIntermSymbol *candidateLeftNode = candidate->getLeft()->getAsSymbolNode();
    ASSERT(candidateLeftNode != nullptr);

    bool areMatched = false;

    const bool equalOperators = expected->getOp() == candidate->getOp();
    const bool equalNames     = expectedLeftNode->getSymbol() == candidateLeftNode->getSymbol();
    const bool equalTypes =
        AreTypesEqual(expectedLeftNode->getType(), candidateLeftNode->getType());
    if (equalOperators && equalNames && equalTypes)
    {
        if (candidate->getOp() == EOpIndexDirect)
        {
            // The subscript has to be checked to guarantee that the expected element in the array
            // is initialized.
            const TIntermConstantUnion *candidateSubscript =
                candidate->getRight()->getAsConstantUnion();
            ASSERT(candidateSubscript != nullptr);

            const TIntermConstantUnion *expectedSubscript =
                expected->getRight()->getAsConstantUnion();
            ASSERT(expectedSubscript != nullptr);

            areMatched = (expectedSubscript->getIConst(0) == candidateSubscript->getIConst(0));
        }
        else
        {
            areMatched = true;
        }
    }
    return areMatched;
}

TType CreateType(TBasicType basicType,
                 TPrecision precision,
                 TQualifier qualifier,
                 unsigned char primarySize,
                 bool isArray,
                 unsigned arraySize)
{
    TType type = TType(basicType, precision, qualifier, primarySize, 1, isArray);
    if (isArray)
    {
        type.setArraySize(arraySize);
    }
    return type;
}

TType CreateType(TBasicType basicType,
                 TPrecision precision,
                 TQualifier qualifier,
                 unsigned char primarySize)
{
    return CreateType(basicType, precision, qualifier, primarySize, false, 0u);
}

TType CreateType(TBasicType basicType, TPrecision precision, TQualifier qualifier)
{
    return CreateType(basicType, precision, qualifier, 1u);
}

const TIntermBinary *CreateIndexingNode(const TString &lValueName, const TType &type, int index)
{
    return new TIntermBinary(EOpIndexDirect, new TIntermSymbol(0, lValueName, type),
                             TIntermTyped::CreateIndexNode(index));
}

const TIntermBinary *CreateAssignmentNode(const TString &lValueName, const TType &type)
{
    return new TIntermBinary(EOpAssign, new TIntermSymbol(0, lValueName, type),
                             TIntermTyped::CreateZero(type));
}

// VerifyOutputVariableInitializers traverses the AST and collects all assignments.
// For assignments to non-array types, the node which represents the assignment is collected (var =
// value). For assignments to elements in an array, the node which represents the indexing of the
// array is collected (var[index]).
class VerifyOutputVariableInitializers final : public TIntermTraverser
{
  public:
    VerifyOutputVariableInitializers() : TIntermTraverser(true, false, false) {}

    bool visitBinary(Visit visit, TIntermBinary *node) override
    {
        if (node->getOp() == EOpAssign)
        {
            {
                // Try to interpret as assignment to a non-array lvalue and collect.
                const TIntermSymbol *leftSymbol = node->getLeft()->getAsSymbolNode();
                if (leftSymbol)
                {
                    mCandidateAssignments.push_back(node);
                    return false;
                }
            }

            {
                // Try to interpret as assignment to an element in an array and collect.
                const TIntermBinary *leftBinary = node->getLeft()->getAsBinaryNode();
                if (leftBinary && leftBinary->getOp() == EOpIndexDirect)
                {
                    const TIntermSymbol *leftSymbol = leftBinary->getLeft()->getAsSymbolNode();
                    const TIntermConstantUnion *subscript =
                        leftBinary->getRight()->getAsConstantUnion();
                    ASSERT(subscript != nullptr);

                    if (leftSymbol)
                    {
                        mCandidateAssignments.push_back(leftBinary);
                        return false;
                    }
                }
            }
        }
        return true;
    }

    // The initialization of variables is considered valid if all of the following rules hold:
    // 1) All lvalues in the initializer have a type which matches the one specified in the
    // expectedAssignments container.
    // 2) All expected assignments are matched.
    bool isValid(const ExpectedAssignmentTargets &expectedAssignments) const
    {
        for (size_t i = 0u; i < expectedAssignments.size(); ++i)
        {
            bool isExpectedAssignmentFound = false;
            for (size_t j = 0; j < mCandidateAssignments.size() && !isExpectedAssignmentFound; ++j)
            {
                isExpectedAssignmentFound =
                    AreAssigmentsTheSame(expectedAssignments[i], mCandidateAssignments[j]);
            }

            if (!isExpectedAssignmentFound)
            {
                return false;
            }
        }
        return true;
    }

  private:
    std::vector<const TIntermBinary *> mCandidateAssignments;
};

// Traverses the AST and records a pointer to a structure with a given name.
class FindStructByName final : public TIntermTraverser
{
  public:
    FindStructByName(const TString &structName)
        : TIntermTraverser(true, false, false), mStructName(structName), mStructure(nullptr)
    {
    }

    void visitSymbol(TIntermSymbol *symbol) override
    {
        if (isStructureFound())
        {
            return;
        }

        TStructure *structure = symbol->getType().getStruct();

        if (structure != nullptr && structure->name() == mStructName)
        {
            mStructure = structure;
        }
    }

    bool isStructureFound() const { return mStructure != nullptr; };
    TStructure *getStructure() const { return mStructure; }

  private:
    TString mStructName;
    TStructure *mStructure;
};

}  // namespace

class InitOutputVariablesTest : public ShaderCompileTreeTest
{
  public:
    InitOutputVariablesTest()
    {
        mExtraCompileOptions |= SH_VARIABLES;
        mExtraCompileOptions |= SH_INIT_OUTPUT_VARIABLES;
    }

  protected:
    ShShaderSpec getShaderSpec() const override { return SH_GLES3_1_SPEC; }
    void checkInitialized()
    {
        VerifyOutputVariableInitializers verifier;

        // The traversal starts in the body of main because this is where the varyings are
        // initialized.
        sh::TIntermFunctionDefinition *main = FindMain(mASTRoot);
        ASSERT_NE(main, nullptr);

        main->traverse(&verifier);
        EXPECT_TRUE(verifier.isValid(mExpectedAssignmentTargets));
    }
    void TearDown() override
    {
        for (size_t i = 0u; i < mExpectedAssignmentTargets.size(); ++i)
        {
            delete mExpectedAssignmentTargets[i];
        }
        mExpectedAssignmentTargets.clear();
    }

    ExpectedAssignmentTargets mExpectedAssignmentTargets;
};

class InitOutputVariablesVertexShaderTest : public InitOutputVariablesTest
{
  protected:
    ::GLenum getShaderType() const override { return GL_VERTEX_SHADER; }
};

class InitOutputVariablesFragmentShaderTest : public InitOutputVariablesTest
{
  protected:
    ::GLenum getShaderType() const override { return GL_FRAGMENT_SHADER; }
};

// Test the initialization of output variables from the vertex shader with various qualifiers.
TEST_F(InitOutputVariablesVertexShaderTest, OutputAllQualifiers)
{
    const std::string &shaderString =
        "#version 300 es\n"
        "precision mediump float;\n"
        "precision lowp int;\n"
        "out vec4 out1;\n"
        "flat out int out2;\n"
        "centroid out float out3;\n"
        "smooth out float out4;\n"
        "void main() {\n"
        "}\n";
    compileAssumeSuccess(shaderString);

    mExpectedAssignmentTargets = {
        CreateAssignmentNode("out1", CreateType(EbtFloat, EbpMedium, EvqVertexOut, 4)),
        CreateAssignmentNode("out2", CreateType(EbtInt, EbpLow, EvqFlatOut)),
        CreateAssignmentNode("out3", CreateType(EbtFloat, EbpMedium, EvqCentroidOut)),
        CreateAssignmentNode("out4", CreateType(EbtFloat, EbpMedium, EvqSmoothOut))};
    checkInitialized();
}

// Test the initialization of an array output variable from the vertex shader.
TEST_F(InitOutputVariablesVertexShaderTest, OutputArray)
{
    const std::string &shaderString =
        "#version 300 es\n"
        "precision mediump float;\n"
        "out float out1[2];\n"
        "void main() {\n"
        "}\n";
    compileAssumeSuccess(shaderString);

    mExpectedAssignmentTargets = {
        CreateIndexingNode("out1", CreateType(EbtFloat, EbpMedium, EvqVertexOut, 1, true, 2), 0),
        CreateIndexingNode("out1", CreateType(EbtFloat, EbpMedium, EvqVertexOut, 1, true, 2), 1)};
    checkInitialized();
}

// Test the initialization of a struct output variable from the vertex shader.
TEST_F(InitOutputVariablesVertexShaderTest, OutputStruct)
{
    const std::string &shaderString =
        "#version 300 es\n"
        "precision mediump float;\n"
        "struct MyS{\n"
        "	float a;\n"
        "	float b;\n"
        "};\n"
        "out MyS out1;\n"
        "void main() {\n"
        "}\n";
    compileAssumeSuccess(shaderString);

    FindStructByName findStruct("MyS");
    mASTRoot->traverse(&findStruct);
    ASSERT(findStruct.isStructureFound());

    TType type = CreateType(EbtStruct, EbpUndefined, EvqVertexOut);
    type.setStruct(findStruct.getStructure());

    mExpectedAssignmentTargets = {CreateAssignmentNode("out1", type)};
    checkInitialized();
}

// Test the initialization of a varying output variable from an ESSL1 vertex shader.
TEST_F(InitOutputVariablesVertexShaderTest, OutputFromESSL1Shader)
{
    const std::string &shaderString =
        "precision mediump float;\n"
        "varying vec4 out1;\n"
        "void main() {\n"
        "}\n";
    compileAssumeSuccess(shaderString);

    mExpectedAssignmentTargets = {
        CreateAssignmentNode("out1", CreateType(EbtFloat, EbpMedium, EvqVaryingOut, 4))};
    checkInitialized();
}

// The the initialization of output varyiables from the fragment shader.
TEST_F(InitOutputVariablesFragmentShaderTest, Output)
{
    const std::string &shaderString =
        "#version 300 es\n"
        "precision mediump float;\n"
        "out vec4 out1;\n"
        "void main() {\n"
        "}\n";
    compileAssumeSuccess(shaderString);

    mExpectedAssignmentTargets = {
        CreateAssignmentNode("out1", CreateType(EbtFloat, EbpMedium, EvqFragmentOut, 4))};
    checkInitialized();
}

}  // namespace sh