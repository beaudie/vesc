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

typedef std::vector<TIntermTyped *> ExpectedLValues;

bool AreTypesEqual(const TType &typeA, const TType &typeB)
{
    return typeA == typeB && typeA.getPrecision() == typeB.getPrecision() &&
           typeA.getQualifier() == typeB.getQualifier();
}

bool AreSymbolsTheSame(const TIntermSymbol *expected, const TIntermSymbol *candidate)
{
    return expected != nullptr && candidate != nullptr &&
           expected->getSymbol() == candidate->getSymbol();
}

bool AreLValuesTheSame(TIntermTyped *expected, TIntermTyped *candidate)
{
    const TIntermBinary *expectedBinary  = expected->getAsBinaryNode();
    const TIntermBinary *candidateBinary = candidate->getAsBinaryNode();
    if (expectedBinary && candidateBinary)
    {
        ASSERT(expectedBinary->getOp() == EOpIndexDirect);
        ASSERT(candidateBinary->getOp() == EOpIndexDirect);
        if (expectedBinary->getRight()->getAsConstantUnion()->getIConst(0) !=
            candidateBinary->getRight()->getAsConstantUnion()->getIConst(0))
        {
            return false;
        }
        return AreSymbolsTheSame(expectedBinary->getLeft()->getAsSymbolNode(),
                                 candidateBinary->getLeft()->getAsSymbolNode());
    }
    return AreSymbolsTheSame(expected->getAsSymbolNode(), candidate->getAsSymbolNode());
}

TIntermTyped *CreateLValueNode(const TString &lValueName, const TType &type)
{
    return new TIntermSymbol(0, lValueName, type);
}

ExpectedLValues CreateIndexedLValueNodeList(const TString &lValueName,
                                            TType elementType,
                                            unsigned arraySize)
{
    elementType.setArraySize(arraySize);
    ExpectedLValues expected(arraySize);
    for (unsigned index = 0u; index < arraySize; ++index)
    {
        expected[index] =
            new TIntermBinary(EOpIndexDirect, new TIntermSymbol(0, lValueName, elementType),
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
    bool isValid(const ExpectedLValues &expectedLValues) const
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
    ExpectedLValues mCandidateLValues;
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
        EXPECT_TRUE(verifier.isValid(mExpectedLValues));
    }
    void TearDown() override
    {
        for (size_t i = 0u; i < mExpectedLValues.size(); ++i)
        {
            delete mExpectedLValues[i];
        }
        mExpectedLValues.clear();
    }

    ExpectedLValues mExpectedLValues;
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

// Test the initialization of output variables with various qualifiers in a vertex shader.
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

    mExpectedLValues = {CreateLValueNode("out1", TType(EbtFloat, EbpMedium, EvqVertexOut, 4)),
                        CreateLValueNode("out2", TType(EbtInt, EbpLow, EvqFlatOut)),
                        CreateLValueNode("out3", TType(EbtFloat, EbpMedium, EvqCentroidOut)),
                        CreateLValueNode("out4", TType(EbtFloat, EbpMedium, EvqSmoothOut))};
    checkInitialized();
}

// Test the initialization of an output array in a vertex shader.
TEST_F(InitOutputVariablesVertexShaderTest, OutputArray)
{
    const std::string &shaderString =
        "#version 300 es\n"
        "precision mediump float;\n"
        "out float out1[2];\n"
        "void main() {\n"
        "}\n";
    compileAssumeSuccess(shaderString);

    mExpectedLValues =
        CreateIndexedLValueNodeList("out1", TType(EbtFloat, EbpMedium, EvqVertexOut), 2);
    checkInitialized();
}

// Test the initialization of a struct output variable in a vertex shader.
TEST_F(InitOutputVariablesVertexShaderTest, OutputStruct)
{
    const std::string &shaderString =
        "#version 300 es\n"
        "precision mediump float;\n"
        "struct MyS{\n"
        "   float a;\n"
        "   float b;\n"
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

    mExpectedLValues = {CreateLValueNode("out1", type)};
    checkInitialized();
}

// Test the initialization of a varying variable in an ESSL1 vertex shader.
TEST_F(InitOutputVariablesVertexShaderTest, OutputFromESSL1Shader)
{
    const std::string &shaderString =
        "precision mediump float;\n"
        "varying vec4 out1;\n"
        "void main() {\n"
        "}\n";
    compileAssumeSuccess(shaderString);

    mExpectedLValues = {CreateLValueNode("out1", TType(EbtFloat, EbpMedium, EvqVaryingOut, 4))};
    checkInitialized();
}

// Test the initialization of output varyiables in a fragment shader.
TEST_F(InitOutputVariablesFragmentShaderTest, Output)
{
    const std::string &shaderString =
        "#version 300 es\n"
        "precision mediump float;\n"
        "out vec4 out1;\n"
        "void main() {\n"
        "}\n";
    compileAssumeSuccess(shaderString);

    mExpectedLValues = {CreateLValueNode("out1", TType(EbtFloat, EbpMedium, EvqFragmentOut, 4))};
    checkInitialized();
}

}  // namespace sh