//
// Copyright (c) 2014 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//
// TypeTracking_test.cpp:
//   Test for tracking types resulting from math operations, including their
//   precision.
//

#include "angle_gl.h"
#include "gtest/gtest.h"
#include "GLSLANG/ShaderLang.h"
#include "compiler/translator/TranslatorESSL.h"

class TypeTrackingTest : public testing::Test
{
  public:
    TypeTrackingTest() {}

  protected:
    virtual void SetUp()
    {
        ShBuiltInResources resources;
        ShInitBuiltInResources(&resources);

        mTranslator = new TranslatorESSL(GL_FRAGMENT_SHADER, SH_GLES2_SPEC);
        ASSERT_TRUE(mTranslator->Init(resources));
    }

    virtual void TearDown()
    {
        delete mTranslator;
    }

    void Compile(const std::string& shaderString)
    {
        const char *shaderStrings[] = { shaderString.c_str() };
        bool compilationSuccess = mTranslator->compile(shaderStrings, 1, SH_INTERMEDIATE_TREE);
        TInfoSink& infoSink = mTranslator->getInfoSink();
        mInfoLog = infoSink.info.c_str();
        if (!compilationSuccess)
            FAIL() << "Shader compilation failed " << mInfoLog;
    }

    bool FoundInIntermediateTree(const char* stringToFind)
    {
        mInfoLog.find(stringToFind) != std::string::npos;
    }

  private:
    TranslatorESSL *mTranslator;
    std::string mInfoLog;
};

TEST_F(TypeTrackingTest, BuiltInFunctionResultPrecision)
{
    const std::string &shaderString =
        "precision mediump float;\n"
        "uniform float f;\n"
        "void main() {\n"
        "   float ff = sin(f);\n"
        "   gl_FragColor = vec4(ff);\n"
        "}\n";
    Compile(shaderString);
    ASSERT_TRUE(FoundInIntermediateTree("sine (mediump float)"));
};

TEST_F(TypeTrackingTest, BinaryMathResultPrecision)
{
    const std::string &shaderString =
        "precision mediump float;\n"
        "uniform float f;\n"
        "void main() {\n"
        "   float ff = f * 0.5;\n"
        "   gl_FragColor = vec4(ff);\n"
        "}\n";
    Compile(shaderString);
    ASSERT_TRUE(FoundInIntermediateTree("multiply (mediump float)"));
};

TEST_F(TypeTrackingTest, BuiltInVecFunctionResultTypeAndPrecision)
{
    const std::string &shaderString =
        "precision mediump float;\n"
        "uniform vec2 a;\n"
        "void main() {\n"
        "   float b = length(a);\n"
        "   float c = dot(a, vec2(0.5));\n"
        "   float d = distance(vec2(0.5), a);\n"
        "   gl_FragColor = vec4(b, c, d, 1.0);\n"
        "}\n";
    Compile(shaderString);
    ASSERT_TRUE(FoundInIntermediateTree("length (mediump float)"));
    ASSERT_TRUE(FoundInIntermediateTree("dot-product (mediump float)"));
    ASSERT_TRUE(FoundInIntermediateTree("distance (mediump float)"));
};

TEST_F(TypeTrackingTest, BuiltInFunctionChoosesHigherPrecision)
{
    const std::string &shaderString =
        "precision lowp float;\n"
        "uniform mediump vec2 a;\n"
        "uniform lowp vec2 b;\n"
        "void main() {\n"
        "   float c = dot(a, b);\n"
        "   float d = distance(b, a);\n"
        "   gl_FragColor = vec4(c, d, 0.0, 1.0);\n"
        "}\n";
    Compile(shaderString);
    ASSERT_TRUE(FoundInIntermediateTree("dot-product (mediump float)"));
    ASSERT_TRUE(FoundInIntermediateTree("distance (mediump float)"));
};

TEST_F(TypeTrackingTest, BuiltInBoolFunctionResultType)
{
    const std::string &shaderString =
        "uniform bvec4 bees;\n"
        "void main() {\n"
        "   bool b = any(bees);\n"
        "   bool c = all(bees);\n"
        "   bvec4 d = not(bees);\n"
        "   gl_FragColor = vec4(b ? 1.0 : 0.0, c ? 1.0 : 0.0, d.x ? 1.0 : 0.0, 1.0);\n"
        "}\n";
    Compile(shaderString);
    ASSERT_TRUE(FoundInIntermediateTree("any (bool)"));
    ASSERT_TRUE(FoundInIntermediateTree("all (bool)"));
    ASSERT_TRUE(FoundInIntermediateTree("Negate conditional (4-component vector of bool)"));
};

TEST_F(TypeTrackingTest, BuiltInVecToBoolFunctionResultType)
{
    const std::string &shaderString =
        "precision mediump float;\n"
        "uniform vec2 apples;\n"
        "uniform vec2 oranges;\n"
        "uniform ivec2 foo;\n"
        "uniform ivec2 bar;\n"
        "void main() {\n"
        "   bvec2 a = lessThan(apples, oranges);\n"
        "   bvec2 b = greaterThan(foo, bar);\n"
        "   gl_FragColor = vec4(any(a) ? 1.0 : 0.0, any(b) ? 1.0 : 0.0, 0.0, 1.0);\n"
        "}\n";
    Compile(shaderString);
    ASSERT_TRUE(FoundInIntermediateTree("Less Than (2-component vector of bool)"));
    ASSERT_TRUE(FoundInIntermediateTree("Greater Than (2-component vector of bool)"));
};

TEST_F(TypeTrackingTest, Sampler2DResultTypeAndPrecision)
{
    // sampler2D is lowp by default
    const std::string &shaderString =
        "precision lowp float;\n"
        "uniform sampler2D s;\n"
        "uniform vec2 a;\n"
        "void main() {\n"
        "   vec4 c = texture2D(s, a);\n"
        "   gl_FragColor = c;\n"
        "}\n";
    Compile(shaderString);
    ASSERT_TRUE(FoundInIntermediateTree("texture2D (lowp 4-component vector of float)"));
};
