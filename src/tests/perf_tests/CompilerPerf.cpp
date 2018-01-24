//
// Copyright 2018 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//
// CompilerPerfTest:
//   Performance test for the shader translator. The test initializes the compiler once and then
//   compiles the same shader repeatedly. There are different variations of the tests using
//   different shaders.
//

#include "ANGLEPerfTest.h"

#include "compiler/translator/Compiler.h"
#include "compiler/translator/InitializeGlobals.h"
#include "GLSLANG/ShaderLang.h"

using namespace testing;

namespace
{

const char *kSimpleESSL100FragSource = R"(
precision mediump float;
void main()
{
    gl_FragColor = vec4(0, 1, 0, 1);
}
)";

const char *kSimpleESSL300FragSource = R"(#version 300 es
precision highp float;
out vec4 outColor;
void main()
{
    outColor = vec4(0, 1, 0, 1);
}
)";

const char *kRealWorldESSL100FragSource = R"(precision highp float;
precision highp sampler2D;
precision highp int;
varying vec2 vPixelCoords; // in pixels
uniform int uCircleCount;
uniform sampler2D uCircleParameters;
uniform sampler2D uBrushTex;
void main(void)
{
    float destAlpha = 0.0;
    for (int i = 0; i < 32; ++i)
    {
        vec4 parameterColor = texture2D(uCircleParameters,vec2(0.25, (float(i) + 0.5) / 32.0));
        vec2 center = parameterColor.xy;
        float circleRadius = parameterColor.z;
        float circleFlowAlpha = parameterColor.w;
        vec4 parameterColor2 = texture2D(uCircleParameters,vec2(0.75, (float(i) + 0.5) / 32.0));
        float circleRotation = parameterColor2.x;
        vec2 centerDiff = vPixelCoords - center;
        float radius = max(circleRadius, 0.5);
        float flowAlpha = (circleRadius < 0.5) ? circleFlowAlpha * circleRadius * circleRadius * 4.0: circleFlowAlpha;
        float antialiasMult = clamp((radius + 1.0 - length(centerDiff)) * 0.5, 0.0, 1.0);
        mat2 texRotation = mat2(cos(circleRotation), -sin(circleRotation), sin(circleRotation), cos(circleRotation));
        vec2 texCoords = texRotation * centerDiff / radius * 0.5 + 0.5;
        float texValue = texture2D(uBrushTex, texCoords).r;
        float circleAlpha = flowAlpha * antialiasMult * texValue;
        if (i < uCircleCount)
        {
            destAlpha = clamp(circleAlpha + (1.0 - circleAlpha) * destAlpha, 0.0, 1.0);
        }
    }
    gl_FragColor = vec4(0.0, 0.0, 0.0, destAlpha);
})";

class CompilerPerfTest : public ANGLEPerfTest
{
  public:
    CompilerPerfTest();

    void step() override;

    void SetUp() override;
    void TearDown() override;

  protected:
    void setTestShader(const char *str) { mTestShader = str; }

  private:
    const char *mTestShader;

    ShBuiltInResources mResources;
    TPoolAllocator mAllocator;
    sh::TCompiler *mTranslator;
};

CompilerPerfTest::CompilerPerfTest() : ANGLEPerfTest("CompilerPerf", "")
{
    sh::InitBuiltInResources(&mResources);
    mResources.FragmentPrecisionHigh = true;
}

void CompilerPerfTest::SetUp()
{
    ANGLEPerfTest::SetUp();

    InitializePoolIndex();
    mAllocator.push();
    SetGlobalPoolAllocator(&mAllocator);

    mTranslator = sh::ConstructCompiler(GL_FRAGMENT_SHADER, SH_WEBGL2_SPEC, SH_HLSL_4_1_OUTPUT);
    if (!mTranslator->Init(mResources))
    {
        SafeDelete(mTranslator);
    }
}

void CompilerPerfTest::TearDown()
{
    SafeDelete(mTranslator);

    SetGlobalPoolAllocator(nullptr);
    mAllocator.pop();

    FreePoolIndex();

    ANGLEPerfTest::TearDown();
}

void CompilerPerfTest::step()
{
    const char *shaderStrings[] = {mTestShader};

    ShCompileOptions compileOptions = SH_OBJECT_CODE | SH_VARIABLES |
                                      SH_INITIALIZE_UNINITIALIZED_LOCALS | SH_INIT_OUTPUT_VARIABLES;

#if !defined(NDEBUG)
    if (!mTranslator->compile(shaderStrings, 1, compileOptions))
    {
        std::cout << mTranslator->getInfoSink().info.c_str();
    }
#endif

    for (unsigned int iteration = 0; iteration < 10; ++iteration)
    {
        mTranslator->compile(shaderStrings, 1, compileOptions);
    }
}

TEST_F(CompilerPerfTest, SimpleESSL100)
{
    setTestShader(kSimpleESSL100FragSource);
    run();
}

TEST_F(CompilerPerfTest, SimpleESSL300)
{
    setTestShader(kSimpleESSL300FragSource);
    run();
}

TEST_F(CompilerPerfTest, RealWorldESSL100)
{
    setTestShader(kRealWorldESSL100FragSource);
    run();
}

}  // anonymous namespace
