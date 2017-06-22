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

typedef std::vector<const TIntermSymbol *> ExpectedSymbols;

bool AreTypesEqual(const TType &typeA, const TType &typeB)
{
    return typeA.getBasicType() == typeB.getBasicType() &&
           typeA.getPrecision() == typeB.getPrecision() &&
           typeA.getQualifier() == typeB.getQualifier() && typeA.isArray() == typeB.isArray() &&
           typeA.getArraySize() == typeB.getArraySize() &&
           typeA.getNominalSize() == typeB.getNominalSize() &&
           typeA.getSecondarySize() == typeB.getSecondarySize();
}

const TIntermSymbol *CreateExpectedSymbol(const TString &name,
                                          TBasicType basicType,
                                          TPrecision precision,
                                          TQualifier qualifier,
                                          unsigned char primarySize,
                                          unsigned char secondarySize,
                                          bool isArray,
                                          unsigned arraySize)
{
    TType type = TType(basicType, precision, qualifier, primarySize, secondarySize, isArray);
    if (isArray)
    {
        type.setArraySize(arraySize);
    }
    return new TIntermSymbol(0, name, type);
}

const TIntermSymbol *CreateExpectedSymbol(const TString &name,
                                          TBasicType basicType,
                                          TPrecision precision,
                                          TQualifier qualifier,
                                          unsigned char primarySize)
{
    return CreateExpectedSymbol(name, basicType, precision, qualifier, primarySize, 1, false, 0u);
}

const TIntermSymbol *CreateExpectedSymbol(const TString &name,
                                          TBasicType basicType,
                                          TPrecision precision,
                                          TQualifier qualifier)
{
    return CreateExpectedSymbol(name, basicType, precision, qualifier, 1u);
}

// VerifyOutputVariableInitializers traverses the AST and examines binary nodes for initialization
// of any of the output variables in the provided list. The initialization of variables is
// considered valid if all of the following rules hold: 1) All lvalues in the initializer have a
// qualifier which matches the one specified in the ExpectedTypes container. 2) There is a
// one-to-one mapping between the variables specified in ExpectedTypes and shader varyings.
class VerifyOutputVariableInitializers : public TIntermTraverser
{
  public:
    VerifyOutputVariableInitializers(const ExpectedSymbols &expectedSymbols)
        : TIntermTraverser(true, false, false),
          mExpectedSymbols(expectedSymbols),
          mMatchedSymbols(mExpectedSymbols.size(), false),
          mValid(true),
          mNumMatched(0u)
    {
    }

    bool visitBinary(Visit visit, TIntermBinary *node)
    {
        bool canBeHandled = false;
        TString name;
        TType type;

        if (node->getOp() == EOpAssign)
        {
            {
                // Check whether we can interpret the expression as variable = initialValue.
                // Structures are also handled by this case.
                const TIntermSymbol *leftSymbol = node->getLeft()->getAsSymbolNode();
                if (leftSymbol)
                {
                    canBeHandled = true;
                    name         = leftSymbol->getSymbol();
                    type         = leftSymbol->getType();
                }
            }

            {
                // Check whether we can interpret the expression as variable[index] = initialValue.
                const TIntermBinary *leftBinary = node->getLeft()->getAsBinaryNode();
                if (leftBinary && leftBinary->getOp() == EOpIndexDirect)
                {
                    const TIntermSymbol *leftSymbol = leftBinary->getLeft()->getAsSymbolNode();
                    const TIntermConstantUnion *subscript =
                        leftBinary->getRight()->getAsConstantUnion();
                    if (leftSymbol && subscript)
                    {
                        canBeHandled                   = true;
                        std::string subscriptStdString = ToString(subscript->getUConst(0));
                        name = leftSymbol->getSymbol() + "[" + TString(subscriptStdString.c_str()) +
                               "]";
                        type = leftSymbol->getType();
                    }
                }
            }
        }

        if (canBeHandled)
        {
            size_t symbolIndex = 0u;
            while (symbolIndex < mExpectedSymbols.size())
            {
                if (name == mExpectedSymbols[symbolIndex]->getSymbol())
                {
                    break;
                }
                ++symbolIndex;
            }

            if (symbolIndex != mExpectedSymbols.size() &&
                AreTypesEqual(mExpectedSymbols[symbolIndex]->getType(), type))
            {
                if (!mMatchedSymbols[symbolIndex])
                {
                    // Keep track of the unique number of occurrences.
                    mMatchedSymbols[symbolIndex] = true;
                    ++mNumMatched;
                }
            }
            else
            {
                mValid = false;
            }
        }

        return mValid;
    }

    bool isValid() const { return mValid && mNumMatched == mExpectedSymbols.size(); }

  private:
    ExpectedSymbols mExpectedSymbols;
    std::vector<bool> mMatchedSymbols;
    bool mValid;
    size_t mNumMatched;
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
    void checkInitialized(const ExpectedSymbols &expectedTypes)
    {
        VerifyOutputVariableInitializers verifier(expectedTypes);

        // The traversal starts in the body of main because this is where the varyings are
        // initialized.
        sh::TIntermFunctionDefinition *main = FindMain(mASTRoot);
        ASSERT_NE(main, nullptr);

        main->traverse(&verifier);
        EXPECT_TRUE(verifier.isValid());
    }
    void TearDown() override
    {
        for (size_t i = 0u; i < mExpectedSymbols.size(); ++i)
        {
            delete mExpectedSymbols[i];
        }
        mExpectedSymbols.clear();
    }

    ExpectedSymbols mExpectedSymbols;
};

class InitOutputVariablesVertexShaderTest : public InitOutputVariablesTest
{
  protected:
    ::GLenum getShaderType() const override { return GL_VERTEX_SHADER; }
};

class InitOutputVariablesFragmentShaderTest : public InitOutputVariablesTest
{
  public:
    void initResources(ShBuiltInResources *resources)
    {
        InitOutputVariablesTest::initResources(resources);
        resources->EXT_draw_buffers = 1;
        resources->MaxDrawBuffers   = 3;
    }

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
    mExtraCompileOptions = (SH_VARIABLES | SH_INIT_OUTPUT_VARIABLES);
    compileAssumeSuccess(shaderString);

    mExpectedSymbols = {CreateExpectedSymbol("out1", EbtFloat, EbpMedium, EvqVertexOut, 4),
                        CreateExpectedSymbol("out2", EbtInt, EbpLow, EvqFlatOut),
                        CreateExpectedSymbol("out3", EbtFloat, EbpMedium, EvqCentroidOut),
                        CreateExpectedSymbol("out4", EbtFloat, EbpMedium, EvqSmoothOut)};
    checkInitialized(mExpectedSymbols);
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
    mExtraCompileOptions = (SH_VARIABLES | SH_INIT_OUTPUT_VARIABLES);
    compileAssumeSuccess(shaderString);

    mExpectedSymbols = {
        CreateExpectedSymbol("out1[0]", EbtFloat, EbpMedium, EvqVertexOut, 1, 1, true, 2),
        CreateExpectedSymbol("out1[1]", EbtFloat, EbpMedium, EvqVertexOut, 1, 1, true, 2)};
    checkInitialized(mExpectedSymbols);
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
    mExtraCompileOptions = (SH_VARIABLES | SH_INIT_OUTPUT_VARIABLES);
    compileAssumeSuccess(shaderString);

    mExpectedSymbols = {CreateExpectedSymbol("out1", EbtStruct, EbpUndefined, EvqVertexOut)};
    checkInitialized(mExpectedSymbols);
}

// Test the initialization of a varying output variable from the vertex shader.
TEST_F(InitOutputVariablesVertexShaderTest, OutputFromESSL1Shader)
{
    const std::string &shaderString =
        "precision mediump float;\n"
        "varying vec4 out1;\n"
        "void main() {\n"
        "}\n";
    mExtraCompileOptions = (SH_VARIABLES | SH_INIT_OUTPUT_VARIABLES);
    compileAssumeSuccess(shaderString);

    mExpectedSymbols = {CreateExpectedSymbol("out1", EbtFloat, EbpMedium, EvqVaryingOut, 4)};
    checkInitialized(mExpectedSymbols);
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
    mExtraCompileOptions = (SH_VARIABLES | SH_INIT_OUTPUT_VARIABLES);
    compileAssumeSuccess(shaderString);

    mExpectedSymbols = {CreateExpectedSymbol("out1", EbtFloat, EbpMedium, EvqFragmentOut, 4)};
    checkInitialized(mExpectedSymbols);
}

// Test the initialization of gl_FragData.
TEST_F(InitOutputVariablesFragmentShaderTest, InitializeFragData)
{
    const std::string &shaderString =
        "#extension GL_EXT_draw_buffers : require\n"
        "precision mediump float;\n"
        "void main() {\n"
        "   gl_FragColor = vec4(1, 0, 0, 0);\n"
        "}\n";
    mExtraCompileOptions = (SH_VARIABLES | SH_INIT_OUTPUT_VARIABLES);
    compileAssumeSuccess(shaderString);

    mExpectedSymbols = {
        CreateExpectedSymbol("gl_FragData[0]", EbtFloat, EbpMedium, EvqFragData, 4, 1, true, 3),
        CreateExpectedSymbol("gl_FragData[1]", EbtFloat, EbpMedium, EvqFragData, 4, 1, true, 3),
        CreateExpectedSymbol("gl_FragData[2]", EbtFloat, EbpMedium, EvqFragData, 4, 1, true, 3)};
    checkInitialized(mExpectedSymbols);
}

}  // namespace sh