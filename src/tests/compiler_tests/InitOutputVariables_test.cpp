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

typedef std::vector<TIntermNode *> ExpectedLValueTargets;

bool AreTypesEqual(const TType &typeA, const TType &typeB)
{
    return typeA == typeB && typeA.getPrecision() == typeB.getPrecision() &&
           typeA.getQualifier() == typeB.getQualifier();
}

bool AreLValuesTheSame(TIntermNode *expected, TIntermNode *candidate)
{
    // Try to interpret the nodes as symbols.
    const TIntermSymbol *expectedSymbol  = expected->getAsSymbolNode();
    const TIntermSymbol *candidateSymbol = candidate->getAsSymbolNode();

    int expectedSubscriptValue  = -1;
    int candidateSubscriptValue = -1;

    if (!expectedSymbol && !candidateSymbol)
    {
        // Try to interpret the nodes as indexing binary nodes.
        const TIntermBinary *expectedBinary  = expected->getAsBinaryNode();
        const TIntermBinary *candidateBinary = candidate->getAsBinaryNode();

        if (expectedBinary && candidateBinary)
        {
            expectedSymbol  = expectedBinary->getLeft()->getAsSymbolNode();
            candidateSymbol = candidateBinary->getLeft()->getAsSymbolNode();

            // The left child can only be a symbol.
            ASSERT(expectedSymbol != nullptr);
            ASSERT(candidateSymbol != nullptr);

            // It must be an indexing node.
            ASSERT(expectedBinary->getOp() == EOpIndexDirect);
            ASSERT(candidateBinary->getOp() == EOpIndexDirect);

            const TIntermConstantUnion *expectedSubscript =
                expectedBinary->getRight()->getAsConstantUnion();
            ASSERT(expectedSubscript != nullptr);

            const TIntermConstantUnion *candidateSubscript =
                candidateBinary->getRight()->getAsConstantUnion();
            ASSERT(candidateSubscript != nullptr);

            expectedSubscriptValue  = expectedSubscript->getIConst(0);
            candidateSubscriptValue = candidateSubscript->getIConst(0);
        }
    }

    if (expectedSymbol == nullptr || candidateSymbol == nullptr)
    {
        // The nodes are not of the same type and cannot be matched.
        return false;
    }

    return (expectedSymbol->getSymbol() == candidateSymbol->getSymbol()) &&
           (expectedSubscriptValue == candidateSubscriptValue);
}

TIntermNode *CreateLValueNode(const TString &lValueName, const TType &type)
{
    return new TIntermSymbol(0, lValueName, type);
}

ExpectedLValueTargets CreateIndexedLValueNodeList(const TString &lValueName,
                                                  TType type,
                                                  unsigned arraySize)
{
    type.setArraySize(arraySize);
    ExpectedLValueTargets expected(arraySize);
    for (unsigned index = 0u; index < arraySize; ++index)
    {
        expected[index] = new TIntermBinary(EOpIndexDirect, new TIntermSymbol(0, lValueName, type),
                                            TIntermTyped::CreateIndexNode(static_cast<int>(index)));
    }
    return expected;
}

// VerifyOutputVariableInitializers traverses the AST and collects the lvalues in all assignments.
class VerifyOutputVariableInitializers final : public TIntermTraverser
{
  public:
    VerifyOutputVariableInitializers() : TIntermTraverser(true, false, false) {}

    bool visitBinary(Visit visit, TIntermBinary *node) override
    {
        if (node->getOp() == EOpAssign)
        {
            mCandidateLValues.push_back(node->getLeft());
            return false;
        }
        return true;
    }

    // The collected lvalues are considered valid if every expected lvalue in expectedLValues is
    // matched by name and type with lvalues in mCandidateLValues.
    bool isValid(const ExpectedLValueTargets &expectedLValues) const
    {
        for (size_t i = 0u; i < expectedLValues.size(); ++i)
        {
            bool isExpectedLValueFound = false;
            for (size_t j = 0; j < mCandidateLValues.size() && !isExpectedLValueFound; ++j)
            {
                isExpectedLValueFound = AreLValuesTheSame(expectedLValues[i], mCandidateLValues[j]);
            }

            if (!isExpectedLValueFound)
            {
                return false;
            }
        }
        return true;
    }

  private:
    ExpectedLValueTargets mCandidateLValues;
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
        EXPECT_TRUE(verifier.isValid(mExpectedLValueTargets));
    }
    void TearDown() override
    {
        for (size_t i = 0u; i < mExpectedLValueTargets.size(); ++i)
        {
            delete mExpectedLValueTargets[i];
        }
        mExpectedLValueTargets.clear();
    }

    ExpectedLValueTargets mExpectedLValueTargets;
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

    mExpectedLValueTargets = {CreateLValueNode("out1", TType(EbtFloat, EbpMedium, EvqVertexOut, 4)),
                              CreateLValueNode("out2", TType(EbtInt, EbpLow, EvqFlatOut)),
                              CreateLValueNode("out3", TType(EbtFloat, EbpMedium, EvqCentroidOut)),
                              CreateLValueNode("out4", TType(EbtFloat, EbpMedium, EvqSmoothOut))};
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

    mExpectedLValueTargets =
        CreateIndexedLValueNodeList("out1", TType(EbtFloat, EbpMedium, EvqVertexOut), 2);
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

    TType type(EbtStruct, EbpUndefined, EvqVertexOut);
    type.setStruct(findStruct.getStructure());

    mExpectedLValueTargets = {CreateLValueNode("out1", type)};
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

    mExpectedLValueTargets = {
        CreateLValueNode("out1", TType(EbtFloat, EbpMedium, EvqVaryingOut, 4))};
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

    mExpectedLValueTargets = {
        CreateLValueNode("out1", TType(EbtFloat, EbpMedium, EvqFragmentOut, 4))};
    checkInitialized();
}

}  // namespace sh