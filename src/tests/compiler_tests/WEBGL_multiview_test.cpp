//
// Copyright (c) 2016 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//
// WEBGL_multiview_test.cpp:
//   Test that shaders with gl_ViewID_OVR are validated correctly.
//

#include "GLSLANG/ShaderLang.h"
#include "angle_gl.h"
#include "compiler/translator/FindMain.h"
#include "compiler/translator/IntermNode.h"
#include "gtest/gtest.h"
#include "tests/test_utils/ShaderCompileTreeTest.h"

using namespace sh;

class OccurrencesCounter : public TIntermTraverser
{
  public:
    OccurrencesCounter() : TIntermTraverser(true, false, false), mNumberOfOccurrences(0u) {}

    void visitSymbol(TIntermSymbol *node) override
    {
        if (shouldSymbolBeCounted(node))
        {
            ++mNumberOfOccurrences;
        }
    }

    virtual bool shouldSymbolBeCounted(const TIntermSymbol *node) const = 0;

    bool isAnythingFound() const { return mNumberOfOccurrences != 0; }
    unsigned getNumberOfOccurrences() const { return mNumberOfOccurrences; }

  private:
    unsigned mNumberOfOccurrences;
};

class OccurrencesCounterByQualifier : public OccurrencesCounter
{
  public:
    OccurrencesCounterByQualifier(TQualifier symbolQualifier) : mSymbolQualifier(symbolQualifier) {}

    bool shouldSymbolBeCounted(const TIntermSymbol *node) const override
    {
        return node->getQualifier() == mSymbolQualifier;
    }

  private:
    TQualifier mSymbolQualifier;
};

class OccurrencesCounterByName : public OccurrencesCounter
{
  public:
    OccurrencesCounterByName(const TString &symbolName) : mSymbolName(symbolName) {}

    bool shouldSymbolBeCounted(const TIntermSymbol *node) const override
    {
        return node->getName().getString() == mSymbolName;
    }

  private:
    TString mSymbolName;
};

class WEBGLMultiviewVertexShaderTest : public ShaderCompileTreeTest
{
  public:
    WEBGLMultiviewVertexShaderTest() {}
  protected:
    ::GLenum getShaderType() const override { return GL_VERTEX_SHADER; }
    ShShaderSpec getShaderSpec() const override { return SH_WEBGL3_SPEC; }
    void initResources(ShBuiltInResources *resources) override
    {
        resources->OVR_multiview = 1;
        resources->MaxViewsOVR   = 4;
    }
};

class WEBGLMultiviewFragmentShaderTest : public ShaderCompileTreeTest
{
  public:
    WEBGLMultiviewFragmentShaderTest() {}
  protected:
    ::GLenum getShaderType() const override { return GL_FRAGMENT_SHADER; }
    ShShaderSpec getShaderSpec() const override { return SH_WEBGL3_SPEC; }
    void initResources(ShBuiltInResources *resources) override
    {
        resources->OVR_multiview = 1;
        resources->MaxViewsOVR   = 4;
    }
};

// Invalid combination of extensions (restricted in the WEBGL_multiview spec).
TEST_F(WEBGLMultiviewVertexShaderTest, InvalidBothMultiviewAndMultiview2)
{
    const std::string &shaderString =
        "#version 300 es\n"
        "#extension GL_OVR_multiview : require\n"
        "#extension GL_OVR_multiview2 : enable\n"
        "layout(num_views = 2) in;\n"
        "void main()\n"
        "{\n"
        "    gl_Position.x = (gl_ViewID_OVR == 0u) ? 1.0 : 0.0;\n"
        "    gl_Position.yzw = vec3(0, 0, 1);\n"
        "}\n";
    if (compile(shaderString))
    {
        FAIL() << "Shader compilation succeeded, expecting failure:\n" << mInfoLog;
    }
}

// Invalid combination of non-matching num_views declarations.
TEST_F(WEBGLMultiviewVertexShaderTest, InvalidNumViewsMismatch)
{
    const std::string &shaderString =
        "#version 300 es\n"
        "#extension GL_OVR_multiview2 : require\n"
        "layout(num_views = 2) in;\n"
        "layout(num_views = 1) in;\n"
        "void main()\n"
        "{\n"
        "    gl_Position.x = (gl_ViewID_OVR == 0u) ? 1.0 : 0.0;\n"
        "    gl_Position.yzw = vec3(0, 0, 1);\n"
        "}\n";
    if (compile(shaderString))
    {
        FAIL() << "Shader compilation succeeded, expecting failure:\n" << mInfoLog;
    }
}

// Invalid value zero for num_views.
TEST_F(WEBGLMultiviewVertexShaderTest, InvalidNumViewsZero)
{
    const std::string &shaderString =
        "#version 300 es\n"
        "#extension GL_OVR_multiview2 : require\n"
        "layout(num_views = 0) in;\n"
        "void main()\n"
        "{\n"
        "    gl_Position.x = (gl_ViewID_OVR == 0u) ? 1.0 : 0.0;\n"
        "    gl_Position.yzw = vec3(0, 0, 1);\n"
        "}\n";
    if (compile(shaderString))
    {
        FAIL() << "Shader compilation succeeded, expecting failure:\n" << mInfoLog;
    }
}

// Too large value for num_views.
TEST_F(WEBGLMultiviewVertexShaderTest, InvalidNumViewsGreaterThanMax)
{
    const std::string &shaderString =
        "#version 300 es\n"
        "#extension GL_OVR_multiview2 : require\n"
        "layout(num_views = 5) in;\n"
        "void main()\n"
        "{\n"
        "    gl_Position.x = (gl_ViewID_OVR == 0u) ? 1.0 : 0.0;\n"
        "    gl_Position.yzw = vec3(0, 0, 1);\n"
        "}\n";
    if (compile(shaderString))
    {
        FAIL() << "Shader compilation succeeded, expecting failure:\n" << mInfoLog;
    }
}

// Valid use of gl_ViewID_OVR in a ternary operator.
TEST_F(WEBGLMultiviewVertexShaderTest, ValidViewIDInTernary)
{
    const std::string &shaderString =
        "#version 300 es\n"
        "#extension GL_OVR_multiview : require\n"
        "layout(num_views = 2) in;\n"
        "layout(num_views = 2) in;  // Duplicated on purpose\n"
        "void main()\n"
        "{\n"
        "    gl_Position.x = (gl_ViewID_OVR == 0u) ? 1.0 : 0.0;\n"
        "    gl_Position.yzw = vec3(0, 0, 1);\n"
        "}\n";
    if (!compile(shaderString))
    {
        FAIL() << "Shader compilation failed, expecting success:\n" << mInfoLog;
    }
}

// Valid use of gl_ViewID_OVR in an if statement.
TEST_F(WEBGLMultiviewVertexShaderTest, ValidViewIDInIf)
{
    const std::string &shaderString =
        "#version 300 es\n"
        "#extension GL_OVR_multiview : require\n"
        "layout(num_views = 2) in;\n"
        "precision highp float;\n"
        "in vec4 pos;\n"
        "void main()\n"
        "{\n"
        "    if (gl_ViewID_OVR == 0u)\n"
        "    {\n"
        "        gl_Position.x = pos.x;\n"
        "    }\n"
        "    else\n"
        "    {\n"
        "        gl_Position.x = 1.0;\n"
        "    }\n"
        "    gl_Position.yzw = pos.yzw;\n"
        "}\n";
    if (!compile(shaderString))
    {
        FAIL() << "Shader compilation failed, expecting success:\n" << mInfoLog;
    }
}

// Valid normal write of gl_Position in addition to the write that's dependent on gl_ViewID_OVR.
TEST_F(WEBGLMultiviewVertexShaderTest, ValidWriteOfGlPosition)
{
    const std::string &shaderString =
        "#version 300 es\n"
        "#extension GL_OVR_multiview : require\n"
        "layout(num_views = 2) in;\n"
        "void main()\n"
        "{\n"
        "    if (0u == gl_ViewID_OVR)\n"
        "    {\n"
        "        gl_Position.x = 1.0;\n"
        "    }\n"
        "    else\n"
        "    {\n"
        "        gl_Position.x = 1.0;\n"
        "    }\n"
        "    gl_Position = vec4(1, 1, 1, 1);\n"
        "}\n";
    if (!compile(shaderString))
    {
        FAIL() << "Shader compilation failed, expecting success:\n" << mInfoLog;
    }
}

// Invalid assignment to gl_Position.y inside if dependent on gl_ViewID_OVR.
TEST_F(WEBGLMultiviewVertexShaderTest, InvalidGlPositionAssignmentInIf)
{
    const std::string &shaderString =
        "#version 300 es\n"
        "#extension GL_OVR_multiview : require\n"
        "layout(num_views = 2) in;\n"
        "void main()\n"
        "{\n"
        "    if (gl_ViewID_OVR == 0u)\n"
        "    {\n"
        "        gl_Position.y = 1.0;\n"
        "    }\n"
        "    else\n"
        "    {\n"
        "        gl_Position.y = 1.0;\n"
        "    }\n"
        "    gl_Position.xzw = vec3(0, 0, 1);\n"
        "}\n";
    if (compile(shaderString))
    {
        FAIL() << "Shader compilation succeeded, expecting failure:\n" << mInfoLog;
    }
}

// Invalid multiple assignments inside if dependent on gl_ViewID_OVR.
TEST_F(WEBGLMultiviewVertexShaderTest, InvalidMultipleGlPositionXAssignmentsInIf)
{
    const std::string &shaderString =
        "#version 300 es\n"
        "#extension GL_OVR_multiview : require\n"
        "layout(num_views = 2) in;\n"
        "void main()\n"
        "{\n"
        "    if (gl_ViewID_OVR == 0u)\n"
        "    {\n"
        "        gl_Position.x = 1.0;\n"
        "        gl_Position.x = 2.0;\n"
        "    }\n"
        "    else\n"
        "    {\n"
        "        gl_Position.x = 1.0;\n"
        "    }\n"
        "    gl_Position.yzw = vec3(0, 0, 1);\n"
        "}\n";
    if (compile(shaderString))
    {
        FAIL() << "Shader compilation succeeded, expecting failure:\n" << mInfoLog;
    }
}

// Invalid read of gl_Position
TEST_F(WEBGLMultiviewVertexShaderTest, InvalidReadOfGlPosition)
{
    const std::string &shaderString =
        "#version 300 es\n"
        "#extension GL_OVR_multiview : require\n"
        "layout(num_views = 2) in;\n"
        "void main()\n"
        "{\n"
        "    if (gl_ViewID_OVR == 0u) {\n"
        "        gl_Position.x = 1.0;\n"
        "    } else {\n"
        "        gl_Position.x = 1.0;\n"
        "    }\n"
        "    gl_Position.yzw = vec3(0, 0, 1);\n"
        "    float f = gl_Position.y;\n"
        "}\n";
    if (compile(shaderString))
    {
        FAIL() << "Shader compilation succeeded, expecting failure:\n" << mInfoLog;
    }
}

// Read gl_Position when the shader does not refer to gl_ViewID_OVR.
TEST_F(WEBGLMultiviewVertexShaderTest, ValidReadOfGlPosition)
{
    const std::string &shaderString =
        "#version 300 es\n"
        "#extension GL_OVR_multiview : require\n"
        "layout(num_views = 2) in;\n"
        "uniform float u;\n"
        "void main()\n"
        "{\n"
        "    gl_Position = vec4(0, 0, 0, 1);\n"
        "    gl_Position.y = gl_Position.x * u;\n"
        "}\n";
    if (!compile(shaderString))
    {
        FAIL() << "Shader compilation failed, expecting success:\n" << mInfoLog;
    }
}

// Read gl_FragCoord in a OVR_multiview fragment shader.
TEST_F(WEBGLMultiviewFragmentShaderTest, InvalidReadOfFragCoord)
{
    const std::string &shaderString =
        "#version 300 es\n"
        "#extension GL_OVR_multiview : require\n"
        "precision highp float;\n"
        "out vec4 outColor;\n"
        "void main()\n"
        "{\n"
        "    outColor = vec4(gl_FragCoord.xy, 0, 1);\n"
        "}\n";
    if (compile(shaderString))
    {
        FAIL() << "Shader compilation succeeded, expecting failure:\n" << mInfoLog;
    }
}

// Read gl_ViewID_OVR in an OVR_multiview fragment shader.
TEST_F(WEBGLMultiviewFragmentShaderTest, InvalidReadOfViewID)
{
    const std::string &shaderString =
        "#version 300 es\n"
        "#extension GL_OVR_multiview : require\n"
        "precision highp float;\n"
        "out vec4 outColor;\n"
        "void main()\n"
        "{\n"
        "    outColor = vec4(gl_ViewID_OVR, 0, 0, 1);\n"
        "}\n";
    if (compile(shaderString))
    {
        FAIL() << "Shader compilation succeeded, expecting failure:\n" << mInfoLog;
    }
}

// Tricky invalid read of view ID.
TEST_F(WEBGLMultiviewVertexShaderTest, InvalidConsumingExpressionForAssignGLPositionX)
{
    const std::string &shaderString =
        "#version 300 es\n"
        "#extension GL_OVR_multiview : require\n"
        "layout(num_views = 2) in;\n"
        "void main()\n"
        "{\n"
        "    float f = (gl_Position.x = (gl_ViewID_OVR == 0u) ? 1.0 : 0.0);\n"
        "    gl_Position.yzw = vec3(f, f, f);\n"
        "}\n";
    if (compile(shaderString))
    {
        FAIL() << "Shader compilation succeeded, expecting failure:\n" << mInfoLog;
    }
}

// Using the OVR_multiview2 extension directive lifts restrictions of OVR_multiview.
TEST_F(WEBGLMultiviewVertexShaderTest, RestrictionsLiftedMultiview2)
{
    const std::string &shaderString =
        "#version 300 es\n"
        "#extension GL_OVR_multiview2 : require\n"
        "layout(num_views = 2) in;\n"
        "out float out_f;\n"
        "void main()\n"
        "{\n"
        "    if (gl_ViewID_OVR == 0u)\n"
        "    {\n"
        "        gl_Position.x = 1.0;\n"
        "        gl_Position.x = 2.0;\n"
        "    }\n"
        "    else\n"
        "    {\n"
        "        gl_Position.x = 1.0;\n"
        "    }\n"
        "    gl_Position.yzw = vec3(0, 0, 1);\n"
        "    gl_Position += vec4(1, 0, 0, 1);\n"
        "    out_f = float(gl_ViewID_OVR * 2u);\n"
        "}\n";
    if (!compile(shaderString))
    {
        FAIL() << "Shader compilation failed, expecting success:\n" << mInfoLog;
    }
}

// Correct use of GL_OVR_multiview macros.
TEST_F(WEBGLMultiviewVertexShaderTest, ValidUseOfExtensionMacros)
{
    const std::string &shaderString =
        "#version 300 es\n"
        "#ifdef GL_OVR_multiview\n"
        "#ifdef GL_OVR_multiview2\n"
        "#if (GL_OVR_multiview == 1) && (GL_OVR_multiview2 == 1)\n"
        "void main()\n"
        "{\n"
        "    gl_Position = vec4(0.0, 0.0, 0.0, 1.0);\n"
        "}\n"
        "#endif\n"
        "#endif\n"
        "#endif\n";
    if (!compile(shaderString))
    {
        FAIL() << "Shader compilation failed, expecting success:\n" << mInfoLog;
    }
}

// Test that the parent node is tracked correctly when validating assignment to gl_Position.
TEST_F(WEBGLMultiviewVertexShaderTest, AssignmentWithViewIDInsideAssignment)
{
    const std::string &shaderString =
        "#version 300 es\n"
        "#extension GL_OVR_multiview : require\n"
        "layout(num_views = 2) in;\n"
        "void main()\n"
        "{\n"
        "    gl_Position.y = (gl_Position.x = (gl_ViewID_OVR == 0u) ? 1.0 : 0.0);\n"
        "}\n";
    if (compile(shaderString))
    {
        FAIL() << "Shader compilation succeeded, expecting failure:\n" << mInfoLog;
    }
}

// Test that gl_ViewID_OVR can't be used as an l-value.
TEST_F(WEBGLMultiviewVertexShaderTest, ViewIdAsLValue)
{
    const std::string &shaderString =
        "#version 300 es\n"
        "#extension GL_OVR_multiview2 : require\n"
        "layout(num_views = 2) in;\n"
        "void foo(out uint u)\n"
        "{\n"
        "    u = 3u;\n"
        "}\n"
        "void main()\n"
        "{\n"
        "    foo(gl_ViewID_OVR);\n"
        "    gl_Position = vec4(0.0, 0.0, 0.0, 1.0);\n"
        "}\n";
    if (compile(shaderString))
    {
        FAIL() << "Shader compilation succeeded, expecting failure:\n" << mInfoLog;
    }
}

// Test that compiling an ESSL 1.00 shader with multiview support succeeds.
TEST_F(WEBGLMultiviewVertexShaderTest, ESSL1Shader)
{
    const std::string &shaderString =
        "#extension GL_OVR_multiview2 : require\n"
        "layout(num_views = 2) in;\n"
        "void main()\n"
        "{\n"
        "    if (gl_ViewID_OVR == 0)\n"
        "    {\n"
        "        gl_Position = vec4(-1.0, 0.0, 0.0, 1.0);\n"
        "    }\n"
        "    else\n"
        "    {\n"
        "        gl_Position = vec4(1.0, 0.0, 0.0, 1.0);\n"
        "    }\n"
        "}\n";
    if (!compile(shaderString))
    {
        FAIL() << "Shader compilation failed, expecting success:\n" << mInfoLog;
    }
}

// Test that compiling an ESSL 1.00 shader with an unsupported global layout qualifier fails.
TEST_F(WEBGLMultiviewVertexShaderTest, ESSL1ShaderUnsupportedGlobalLayoutQualifier)
{
    const std::string &shaderString =
        "#extension GL_OVR_multiview2 : require\n"
        "layout(num_views = 2) in;\n"
        "layout(std140) uniform;\n"
        "void main()\n"
        "{\n"
        "    if (gl_ViewID_OVR == 0)\n"
        "    {\n"
        "        gl_Position = vec4(-1.0, 0.0, 0.0, 1.0);\n"
        "    }\n"
        "    else\n"
        "    {\n"
        "        gl_Position = vec4(1.0, 0.0, 0.0, 1.0);\n"
        "    }\n"
        "}\n";
    if (compile(shaderString))
    {
        FAIL() << "Shader compilation succeeded, expecting failure:\n" << mInfoLog;
    }
}

// Test that compiling an ESSL 1.00 vertex shader with an unsupported input storage qualifier fails.
TEST_F(WEBGLMultiviewVertexShaderTest, ESSL1ShaderUnsupportedInputStorageQualifier)
{
    const std::string &shaderString =
        "#extension GL_OVR_multiview2 : require\n"
        "layout(num_views = 2) in;\n"
        "in vec4 pos;\n"
        "void main()\n"
        "{\n"
        "    if (gl_ViewID_OVR == 0)\n"
        "    {\n"
        "        gl_Position = vec4(-1.0, 0.0, 0.0, 1.0);\n"
        "    }\n"
        "    else\n"
        "    {\n"
        "        gl_Position = vec4(1.0, 0.0, 0.0, 1.0);\n"
        "    }\n"
        "}\n";
    if (compile(shaderString))
    {
        FAIL() << "Shader compilation succeeded, expecting failure:\n" << mInfoLog;
    }
}

// Test that compiling an ESSL 1.00 fragment shader with an unsupported input storage qualifier
// fails.
TEST_F(WEBGLMultiviewFragmentShaderTest, ESSL1ShaderUnsupportedInStorageQualifier)
{
    const std::string &shaderString =
        "#extension GL_OVR_multiview2 : require\n"
        "precision highp float;\n"
        "in vec4 color;\n"
        "void main()\n"
        "{\n"
        "    if (gl_ViewID_OVR == 0)\n"
        "    {\n"
        "        gl_FragColor = color;\n"
        "    }\n"
        "    else\n"
        "    {\n"
        "        gl_FragColor = color + vec4(1.0, 0.0, 0.0, 1.0);\n"
        "    }\n"
        "}\n";
    if (compile(shaderString))
    {
        FAIL() << "Shader compilation succeeded, expecting failure:\n" << mInfoLog;
    }
}

// Test that gl_InstanceID gets correctly replaced by gl_InstanceIDImpostor. gl_InstanceID should be
// only used only twice: once to initialize gl_ViewID_OVR and once for gl_InstanceIDImpostor. The
// number of occurrences of gl_InstanceIDImpostor in the AST should be the sum of one and the number
// of occurrences of gl_InstanceID before any renaming.
TEST_F(WEBGLMultiviewVertexShaderTest, InstanceIDIsRenamed)
{
    const std::string &shaderString =
        "#version 300 es\n"
        "#extension GL_OVR_multiview : require\n"
        "layout(num_views = 2) in;\n"
        "flat out int myInstance;\n"
        "out float myInstanceF;\n"
        "out float myInstanceF2;\n"
        "void main()\n"
        "{\n"
        "    gl_Position.x = gl_ViewID_OVR == 0u ? 0. : 1.;\n"
        "	 gl_Position.yzw = vec3(0., 0., 1.);\n"
        "	 myInstance = gl_InstanceID;\n"
        "	 myInstanceF = float(gl_InstanceID) + .5;\n"
        "	 myInstanceF2 = float(gl_InstanceID) + .1;\n"
        "}\n";
    mExtraCompileOptions |= SH_MAKE_MULTIVIEW_VARS_GLOBAL_VARS_AND_INITIALIZE;
    compileAssumeSuccess(shaderString);

    OccurrencesCounterByName instanceIDByName("gl_InstanceID");
    mASTRoot->traverse(&instanceIDByName);
    EXPECT_EQ(2, instanceIDByName.getNumberOfOccurrences());

    OccurrencesCounterByQualifier instanceIDByQualifier(EvqInstanceID);
    mASTRoot->traverse(&instanceIDByQualifier);
    EXPECT_EQ(2, instanceIDByQualifier.getNumberOfOccurrences());

    OccurrencesCounterByName instanceIDImpostorFinder("gl_InstanceIDImpostor");
    mASTRoot->traverse(&instanceIDImpostorFinder);
    EXPECT_EQ(5, instanceIDImpostorFinder.getNumberOfOccurrences());
}

// The test check that gl_ViewID_OVR and gl_InstanceIDImpostor are the first to be initialized
// inside the body of the main function.
TEST_F(WEBGLMultiviewVertexShaderTest, ViewIDAndInstanceIDDeferredFirst)
{
    const std::string &shaderString =
        "#version 300 es\n"
        "#extension GL_OVR_multiview : require\n"
        "layout(num_views = 2) in;\n"
        "flat out int myInstance;\n"
        "void main()\n"
        "{\n"
        "    gl_Position.x = gl_ViewID_OVR == 0u ? 0. : 1.;\n"
        "	 gl_Position.yzw = vec3(0., 0., 1.);\n"
        "	 myInstance = gl_InstanceID;\n"
        "}\n";
    mExtraCompileOptions |= SH_MAKE_MULTIVIEW_VARS_GLOBAL_VARS_AND_INITIALIZE;
    compileAssumeSuccess(shaderString);

    TIntermFunctionDefinition *main = FindMain(mASTRoot);
    ASSERT_TRUE(main != nullptr);
    TIntermBlock *mainBody = main->getBody();
    ASSERT_TRUE(mainBody != nullptr);

    const TIntermBlock *initializationBlock = (*mainBody->getSequence())[0]->getAsBlock();
    ASSERT_TRUE(initializationBlock != nullptr);

    const TIntermSequence &initializationSequence = *initializationBlock->getSequence();
    ASSERT_TRUE(initializationSequence.size() >= 2u);

    const TIntermBinary *viewIDInitialization = initializationSequence[0]->getAsBinaryNode();
    ASSERT_TRUE(viewIDInitialization != nullptr);

    const TIntermSymbol *viewIDInitializationLeft =
        viewIDInitialization->getLeft()->getAsSymbolNode();
    ASSERT_TRUE(viewIDInitializationLeft != nullptr);

    EXPECT_EQ(viewIDInitializationLeft->getName().getString(), "gl_ViewID_OVR");

    TIntermBinary *instanceIDImpostorInitialization = initializationSequence[1]->getAsBinaryNode();
    ASSERT_TRUE(instanceIDImpostorInitialization != nullptr);

    const TIntermSymbol *instanceIDImpostorInitializationLeft =
        instanceIDImpostorInitialization->getLeft()->getAsSymbolNode();
    ASSERT_TRUE(instanceIDImpostorInitializationLeft != nullptr);

    EXPECT_EQ(instanceIDImpostorInitializationLeft->getName().getString(), "gl_InstanceIDImpostor");
}

// The test check that gl_ViewID_OVR and gl_InstanceIDImpostor have the correct values based on the
// number of views.
TEST_F(WEBGLMultiviewVertexShaderTest, ViewIDAndInstanceIDHaveCorrectValues)
{
    const std::string &shaderString =
        "#version 300 es\n"
        "#extension GL_OVR_multiview : require\n"
        "layout(num_views = 3) in;\n"
        "flat out int myInstance;\n"
        "void main()\n"
        "{\n"
        "    gl_Position.x = gl_ViewID_OVR == 0u ? 0. : 1.;\n"
        "	 gl_Position.yzw = vec3(0., 0., 1.);\n"
        "	 myInstance = gl_InstanceID;\n"
        "}\n";
    mExtraCompileOptions |= SH_MAKE_MULTIVIEW_VARS_GLOBAL_VARS_AND_INITIALIZE;
    compileAssumeSuccess(shaderString);

    TIntermFunctionDefinition *main = FindMain(mASTRoot);
    ASSERT_TRUE(main != nullptr);
    TIntermBlock *mainBody = main->getBody();
    ASSERT_TRUE(mainBody != nullptr);

    const TIntermBlock *initializationBlock = (*mainBody->getSequence())[0]->getAsBlock();
    ASSERT_TRUE(initializationBlock != nullptr);

    // Should contain two expressions.
    const TIntermSequence &initializationSequence = *initializationBlock->getSequence();
    EXPECT_EQ(2u, initializationSequence.size());

    {
        // Check for the expression gl_ViewID_OVR = uint(gl_InstanceID) % 3u.
        const TIntermBinary *viewIDInitialization = initializationSequence[0]->getAsBinaryNode();
        ASSERT_TRUE(viewIDInitialization != nullptr);

        // Check that the binary operator is assignment.
        EXPECT_EQ(EOpAssign, viewIDInitialization->getOp());

        // Check name of lvalue on the left side.
        const TIntermSymbol *viewIDInitializationLeft =
            viewIDInitialization->getLeft()->getAsSymbolNode();
        ASSERT_TRUE(viewIDInitializationLeft != nullptr);
        EXPECT_EQ("gl_ViewID_OVR", viewIDInitializationLeft->getName().getString());

        // Check right side of the expression.
        const TIntermBinary *viewIDInitializationExpression =
            viewIDInitialization->getRight()->getAsBinaryNode();
        ASSERT_TRUE(viewIDInitializationExpression != nullptr);

        // Check that modulus is the applied operator.
        EXPECT_EQ(EOpIMod, viewIDInitializationExpression->getOp());

        // Check that the expression is uint(gl_InstanceID).
        const TIntermAggregate *leftArgumentInExpression =
            viewIDInitializationExpression->getLeft()->getAsAggregate();
        ASSERT_TRUE(leftArgumentInExpression != nullptr);
        EXPECT_EQ(EbtUInt, leftArgumentInExpression->getBasicType());
        EXPECT_EQ(1u, leftArgumentInExpression->getSequence()->size());
        const TIntermSymbol *uintArgument =
            (*leftArgumentInExpression->getSequence())[0]->getAsSymbolNode();
        EXPECT_EQ(EvqInstanceID, uintArgument->getQualifier());

        // Check that the expression is an unsigned integer with value 3u.
        const TIntermConstantUnion *rightArgumentInExpression =
            viewIDInitializationExpression->getRight()->getAsConstantUnion();
        ASSERT_TRUE(rightArgumentInExpression != nullptr);
        EXPECT_EQ(EbtUInt, rightArgumentInExpression->getBasicType());
        EXPECT_EQ(3u, rightArgumentInExpression->getUConst(0));
    }

    {
        // Check that the expression is gl_InstanceIDImpostor = gl_InstanceID / 3.
        TIntermBinary *instanceIDImpostorInitialization =
            initializationSequence[1]->getAsBinaryNode();
        ASSERT_TRUE(instanceIDImpostorInitialization != nullptr);

        // Check that the binary operator is assignment.
        EXPECT_EQ(EOpAssign, instanceIDImpostorInitialization->getOp());

        // Check name of lvalue on left side.
        const TIntermSymbol *instanceIDImpostorInitializationLeft =
            instanceIDImpostorInitialization->getLeft()->getAsSymbolNode();
        ASSERT_TRUE(instanceIDImpostorInitializationLeft != nullptr);
        EXPECT_EQ("gl_InstanceIDImpostor",
                  instanceIDImpostorInitializationLeft->getName().getString());

        const TIntermBinary *instanceIDImpostorInitializationRight =
            instanceIDImpostorInitialization->getRight()->getAsBinaryNode();
        ASSERT_TRUE(instanceIDImpostorInitializationRight != nullptr);

        // Check that the applied operator is /.
        EXPECT_EQ(EOpDiv, instanceIDImpostorInitializationRight->getOp());

        const TIntermSymbol *leftArgumentInExpression =
            instanceIDImpostorInitializationRight->getLeft()->getAsSymbolNode();
        ASSERT_TRUE(leftArgumentInExpression != nullptr);
        EXPECT_EQ(EvqInstanceID, leftArgumentInExpression->getQualifier());

        const TIntermConstantUnion *rightArgumentInExpression =
            instanceIDImpostorInitializationRight->getRight()->getAsConstantUnion();
        ASSERT_TRUE(rightArgumentInExpression != nullptr);
        EXPECT_EQ(3, rightArgumentInExpression->getIConst(0));
        EXPECT_EQ(EbtInt, rightArgumentInExpression->getBasicType());
    }
}