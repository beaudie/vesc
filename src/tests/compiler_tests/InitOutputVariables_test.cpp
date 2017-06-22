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

struct ExpectedTypeForSymbol
{
    ExpectedTypeForSymbol(const TString &nameIn, TQualifier qualifierIn)
        : name(nameIn), qualifier(qualifierIn), isMatched(false)
    {
    }

    bool operator==(const TString &symbolName) { return name == symbolName; }

    TString name;
    TQualifier qualifier;
    bool isMatched;
};

typedef std::vector<ExpectedTypeForSymbol> ExpectedTypes;

// VerifyOutputVariableInitializers traverses the AST and examines binary nodes for initialization
// of any of the output variables in the provided list. The initialization of variables is
// considered valid if all of the following rules hold: 1) All lvalues in the initializer have a
// qualifier which matches the one specified in the ExpectedTypes container. 2) There is a
// one-to-one mapping between the variables specified in ExpectedTypes and shader varyings.
class VerifyOutputVariableInitializers : public TIntermTraverser
{
  public:
    VerifyOutputVariableInitializers(const ExpectedTypes &expectedTypes)
        : TIntermTraverser(true, false, false),
          mExpectedTypes(expectedTypes),
          mValid(true),
          mNumMatched(0u)
    {
    }

    bool visitBinary(Visit visit, TIntermBinary *node)
    {
        bool canBeHandled = false;
        TString name;
        TQualifier qualifier = EvqLast;

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
                    qualifier    = leftSymbol->getType().getQualifier();
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
                        qualifier = leftSymbol->getType().getQualifier();
                    }
                }
            }
        }

        if (canBeHandled)
        {
            ExpectedTypes::iterator it =
                std::find(mExpectedTypes.begin(), mExpectedTypes.end(), name);

            if (it != mExpectedTypes.end() && it->qualifier == qualifier)
            {
                if (!it->isMatched)
                {
                    // Keep track of the unique number of occurrences.
                    it->isMatched = true;
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

    bool isValid() const { return mValid && mNumMatched == mExpectedTypes.size(); }

  private:
    ExpectedTypes mExpectedTypes;
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
    void checkInitialized(const ExpectedTypes &expectedTypes)
    {
        VerifyOutputVariableInitializers verifier(expectedTypes);

        // The traversal starts in the body of main because this is where the varyings are
        // initialized.
        sh::TIntermFunctionDefinition *main = FindMain(mASTRoot);
        ASSERT_NE(main, nullptr);

        main->traverse(&verifier);
        EXPECT_TRUE(verifier.isValid());
    }
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
        "out vec4 out1;\n"
        "flat out int out2;\n"
        "centroid out float out3;\n"
        "smooth out float out4;\n"
        "void main() {\n"
        "}\n";
    mExtraCompileOptions = (SH_VARIABLES | SH_INIT_OUTPUT_VARIABLES);
    compileAssumeSuccess(shaderString);

    const ExpectedTypes &expected = {
        ExpectedTypeForSymbol("out1", EvqVertexOut), ExpectedTypeForSymbol("out2", EvqFlatOut),
        ExpectedTypeForSymbol("out3", EvqCentroidOut), ExpectedTypeForSymbol("out4", EvqSmoothOut)};
    checkInitialized(expected);
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

    const ExpectedTypes &expected = {ExpectedTypeForSymbol("out1[0]", EvqVertexOut),
                                     ExpectedTypeForSymbol("out1[1]", EvqVertexOut)};
    checkInitialized(expected);
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

    const ExpectedTypes &expected = {ExpectedTypeForSymbol("out1", EvqVertexOut)};
    checkInitialized(expected);
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

    const ExpectedTypes &expected = {ExpectedTypeForSymbol("out1", EvqVaryingOut)};
    checkInitialized(expected);
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

    const ExpectedTypes &expected = {ExpectedTypeForSymbol("out1", EvqFragmentOut)};
    checkInitialized(expected);
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

    const ExpectedTypes &expected = {ExpectedTypeForSymbol("gl_FragData[0]", EvqFragData),
                                     ExpectedTypeForSymbol("gl_FragData[1]", EvqFragData),
                                     ExpectedTypeForSymbol("gl_FragData[2]", EvqFragData)};
    checkInitialized(expected);
}

}  // namespace sh