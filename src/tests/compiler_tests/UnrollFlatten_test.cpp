//
// Copyright (c) 2015 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//
// UnrollFlatten_test.cpp:
//   Test for the outputting of [[unroll]] and [[flatten]] for the D3D compiler
//

#include "angle_gl.h"
#include "gtest/gtest.h"
#include "GLSLANG/ShaderLang.h"
#include "compiler/translator/TranslatorHLSL.h"

class UnrollFlattenTest : public testing::Test
{
  public:
    UnrollFlattenTest() {}

  protected:
    virtual void SetUp()
    {
        ShBuiltInResources resources;
        ShInitBuiltInResources(&resources);
        resources.FragmentPrecisionHigh = 1;

        mTranslator = new TranslatorHLSL(GL_FRAGMENT_SHADER, SH_GLES2_SPEC, SH_HLSL11_OUTPUT);
        ASSERT_TRUE(mTranslator->Init(resources));
    }

    virtual void TearDown()
    {
        delete mTranslator;
    }

    void compile(const std::string& shaderString)
    {
        const char *shaderStrings[] = { shaderString.c_str() };
        bool compilationSuccess = mTranslator->compile(shaderStrings, 1, SH_VARIABLES | SH_OBJECT_CODE);
        TInfoSink &infoSink = mTranslator->getInfoSink();
        if (!compilationSuccess)
            FAIL() << "Shader compilation failed " << infoSink.info.str();
        mTranslatedSource = infoSink.obj.str();

        // Ignore the beginning of the shader to avoid the definitions of LOOP and FLATTEN
        mCurrentPosition = mTranslatedSource.find("GL_USES_FRAG_COLOR");
    }

    void expect(std::initializer_list<std::string> patterns) {
        for (auto &pattern : patterns)
        {
            auto position = mTranslatedSource.find(pattern, mCurrentPosition);
            if (position == std::string::npos)
            {
                FAIL() << "Couldn't find '" << pattern << "' after expectations '"
                    << mExpectationList << "' in translated source:\n" << mTranslatedSource;
            }

            for (auto &badPattern : { UNROLL, FLATTEN })
            {
                if (pattern != badPattern &&
                    mTranslatedSource.find(badPattern, mCurrentPosition) < position)
                {
                    FAIL() << "Found '" << badPattern << "' before '" << pattern << "' after expectations '"
                        << mExpectationList << "' in translated source:\n" << mTranslatedSource;
                }
            }
            mExpectationList += " - " + pattern;
            mCurrentPosition = position + 1;
        }
    }

    static std::string UNROLL;
    static std::string FLATTEN;

  private:
    TranslatorHLSL *mTranslator;
    std::string mTranslatedSource;

    int mCurrentPosition;
    std::string mExpectationList;
};

std::string UnrollFlattenTest::UNROLL = "LOOP";
std::string UnrollFlattenTest::FLATTEN = "FLATTEN";

// Check that the nothing is added if there is no gradient operation
// even when there is ifs and discontinuous loops
TEST_F(UnrollFlattenTest, NoGradient)
{
    const std::string &shaderString =
        "precision mediump float;\n"
        "uniform float f;\n"
        "float fun(float a){\n" // 1
        "    if (a > 1.0) {return f;}\n" // 2
        "    else {return a + 1.0;}\n"
        "}\n"
        "float fun2(float a){\n" // 3
        "    for (int i = 0; i < 10; i++) {\n" // 4
        "        if (a > 1.0) {break;}\n" // 5
        "        a = fun(a);\n" // 6
        "    }\n"
        "    return a;\n"
        "}\n"
        "void main() {\n"
        "    float accum = 0.0;\n"
        "    if (f < 5.0) {accum = fun2(accum);}\n" // 7
        "    gl_FragColor = vec4(accum);\n"
        "}\n";
    compile(shaderString);
    // 1 - shouldn't get a Lod0 version generated
    // 2 - no FLATTEN because does not contain discont loop
    // 3 - shouldn't get a Lod0 version generated
    // 4 - no LOOP because discont, and also no gradient
    // 5 - no FLATTEN because does not contain discont loop
    // 6 - call non-Lod0 version
    // 7 - no FLATTEN
    expect({"fun(", "if",
            "fun2(", "for", "if", "break", "fun(",
            "main(", "if", "fun2("});
}

// Check that when we have a gradient in a non-discontinuous loop
// we use the regular version of the functions. Also checks that
// UNROLL is generated for the loop containing the gradient.
TEST_F(UnrollFlattenTest, GradientNotInDiscont)
{
    const std::string &shaderString =
        "precision mediump float;\n"
        "uniform float f;\n"
        "uniform sampler2D tex;"
        "float fun(float a){\n" // 1
        "    return texture2D(tex, vec2(0.5, f)).x;\n" // 2
        "}\n"
        "float fun2(float a){\n" // 3
        "    for (int i = 0; i < 10; i++) {\n" // 4
        "        if (a > 1.0) {}\n" // 5
        "        a = fun(a);\n" // 6
        "    }\n"
        "    return a;\n"
        "}\n"
        "void main() {\n"
        "    float accum = 0.0;\n"
        "    if (f < 5.0) {accum = fun2(accum);}\n" // 7
        "    gl_FragColor = vec4(accum);\n"
        "}\n";
    // 1 - shouldn't get a Lod0 version generated
    // 2 - no Lod0 version generated
    // 3 - shouldn't get a Lod0 version generated (not in discont loop)
    // 4 - should have LOOP because it contains a gradient operation (even if Lod0)
    // 5 - no FLATTEN because doesn't contain discont loop
    // 6 - call Lod0 version
    // 7 - no FLATTEN
    compile(shaderString);
    expect({"fun(", "texture2D(",
            "fun2(", "LOOP", "for", "if", "fun(",
            "main(", "if", "fun2(" });
}

// Check that when we have a gradient in a discontinuous loop
// we use the Lod0 version of the functions.
TEST_F(UnrollFlattenTest, GradientInDiscont)
{
    const std::string &shaderString =
        "precision mediump float;\n"
        "uniform float f;\n"
        "uniform sampler2D tex;"
        "float fun(float a){\n" // 1
        "    return texture2D(tex, vec2(0.5, f)).x;\n" // 2
        "}\n"
        "float fun2(float a){\n" // 3
        "    for (int i = 0; i < 10; i++) {\n" // 4
        "        if (a > 1.0) {break;}\n" // 5
        "        a = fun(a);\n" // 6
        "    }\n"
        "    return a;\n"
        "}\n"
        "void main() {\n"
        "    float accum = 0.0;\n"
        "    if (f < 5.0) {accum = fun2(accum);}\n" // 7
        "    gl_FragColor = vec4(accum);\n"
        "}\n";
    // 1 - should get a Lod0 version generated (gradient + discont loop)
    // 2 - will get the Lod0 if in funLod0
    // 3 - shouldn't get a Lod0 version generated (not in discont loop)
    // 4 - should have LOOP because it contains a gradient operation (even if Lod0)
    // 5 - no FLATTEN because doesn't contain discont loop
    // 6 - call Lod0 version
    // 7 - should have a FLATTEN because has a discont loop and gradient
    compile(shaderString);
    expect({"fun(", "texture2D(",
            "funLod0(", "texture2DLod0(",
            "fun2(", "LOOP", "for", "if", "break", "funLod0(",
            "main(", "FLATTEN", "if", "fun2(" });
}

