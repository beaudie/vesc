//
// Copyright (c) 2014 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//
// DebugShaderPrecision_test.cpp:
//   Tests for writing the code for shader precision emulation.
//

#include "angle_gl.h"
#include "gtest/gtest.h"
#include "GLSLANG/ShaderLang.h"
#include "compiler/translator/TranslatorGLSL.h"

class DebugShaderPrecisionTest : public testing::Test
{
  public:
    DebugShaderPrecisionTest() {}

  protected:
    virtual void SetUp()
    {
        ShBuiltInResources resources;
        ShInitBuiltInResources(&resources);
        resources.WEBGL_debug_shader_precision = 1;

        mTranslator = new TranslatorGLSL(GL_FRAGMENT_SHADER, SH_GLES2_SPEC);
        ASSERT_TRUE(mTranslator->Init(resources));
    }

    virtual void TearDown()
    {
        delete mTranslator;
    }

    void compile(const std::string& shaderString)
    {
        const char *shaderStrings[] = { shaderString.c_str() };
        bool compilationSuccess = mTranslator->compile(shaderStrings, 1, SH_OBJECT_CODE);
        TInfoSink &infoSink = mTranslator->getInfoSink();
        mCode = infoSink.obj.c_str();
        if (!compilationSuccess)
            FAIL() << "Shader compilation failed " << infoSink.info.c_str();
    }

    bool foundInCode(const char* stringToFind)
    {
        return mCode.find(stringToFind) != std::string::npos;
    }

  private:
    TranslatorGLSL *mTranslator;
    std::string mCode;
};

class NoDebugShaderPrecisionTest : public testing::Test
{
  public:
    NoDebugShaderPrecisionTest() {}

  protected:
    virtual void SetUp()
    {
        ShBuiltInResources resources;
        ShInitBuiltInResources(&resources);

        mTranslator = new TranslatorGLSL(GL_FRAGMENT_SHADER, SH_GLES2_SPEC);
        ASSERT_TRUE(mTranslator->Init(resources));
    }

    virtual void TearDown()
    {
        delete mTranslator;
    }

    bool compile(const std::string& shaderString)
    {
        const char *shaderStrings[] = { shaderString.c_str() };
        bool compilationSuccess = mTranslator->compile(shaderStrings, 1, SH_OBJECT_CODE);
        TInfoSink &infoSink = mTranslator->getInfoSink();
        mCode = infoSink.obj.c_str();
        return compilationSuccess;
    }

    bool foundInCode(const char* stringToFind)
    {
        return mCode.find(stringToFind) != std::string::npos;
    }

  private:
    TranslatorGLSL *mTranslator;
    std::string mCode;
};

TEST_F(DebugShaderPrecisionTest, RoundingFunctionsDefined)
{
    const std::string &shaderString =
        "precision mediump float;\n"
        "uniform float u;\n"
        "void main() {\n"
        "   gl_FragColor = vec4(u);\n"
        "}\n";
    compile(shaderString);
    ASSERT_TRUE(foundInCode("float webgl_frm(in float"));
    ASSERT_TRUE(foundInCode("vec2 webgl_frm(in vec2"));
    ASSERT_TRUE(foundInCode("vec3 webgl_frm(in vec3"));
    ASSERT_TRUE(foundInCode("vec4 webgl_frm(in vec4"));
    ASSERT_TRUE(foundInCode("mat2 webgl_frm(in mat2"));
    ASSERT_TRUE(foundInCode("mat3 webgl_frm(in mat3"));
    ASSERT_TRUE(foundInCode("mat4 webgl_frm(in mat4"));

    ASSERT_TRUE(foundInCode("float webgl_frl(in float"));
    ASSERT_TRUE(foundInCode("vec2 webgl_frl(in vec2"));
    ASSERT_TRUE(foundInCode("vec3 webgl_frl(in vec3"));
    ASSERT_TRUE(foundInCode("vec4 webgl_frl(in vec4"));
    ASSERT_TRUE(foundInCode("mat2 webgl_frl(in mat2"));
    ASSERT_TRUE(foundInCode("mat3 webgl_frl(in mat3"));
    ASSERT_TRUE(foundInCode("mat4 webgl_frl(in mat4"));
};

TEST_F(DebugShaderPrecisionTest, PragmaDisablesEmulation)
{
    const std::string &shaderString =
        "#pragma webgl_debug_shader_precision(off)\n"
        "precision mediump float;\n"
        "uniform float u;\n"
        "void main() {\n"
        "   gl_FragColor = vec4(u);\n"
        "}\n";
    compile(shaderString);
    ASSERT_FALSE(foundInCode("webgl_frm"));
    const std::string &shaderStringPragmaOn =
        "#pragma webgl_debug_shader_precision(on)\n"
        "precision mediump float;\n"
        "uniform float u;\n"
        "void main() {\n"
        "   gl_FragColor = vec4(u);\n"
        "}\n";
    compile(shaderStringPragmaOn);
    ASSERT_TRUE(foundInCode("webgl_frm"));
};

TEST_F(NoDebugShaderPrecisionTest, HelpersWrittenOnlyWithExtension)
{
    const std::string &shaderString =
        "precision mediump float;\n"
        "uniform float u;\n"
        "void main() {\n"
        "   gl_FragColor = vec4(u);\n"
        "}\n";
    ASSERT_TRUE(compile(shaderString));
    ASSERT_FALSE(foundInCode("webgl_frm"));
};

TEST_F(DebugShaderPrecisionTest, DeclarationsAndConstants)
{
    const std::string &shaderString =
        "precision mediump float;\n"
        "uniform vec4 f;\n"
        "uniform float uu, uu2;\n"
        "varying float vv, vv2;\n"
        "float gg = 0.0, gg2;\n"
        "void main() {\n"
        "   float aa = 0.0, aa2;\n"
        "   gl_FragColor = f;\n"
        "}\n";
    compile(shaderString);
    // Declarations or constants should not have rounding inserted around them
    ASSERT_FALSE(foundInCode("webgl_frm(0"));
    ASSERT_FALSE(foundInCode("webgl_frm(uu"));
    ASSERT_FALSE(foundInCode("webgl_frm(vv"));
    ASSERT_FALSE(foundInCode("webgl_frm(gg"));
    ASSERT_FALSE(foundInCode("webgl_frm(aa"));
};

TEST_F(DebugShaderPrecisionTest, InitializerRounding)
{
    const std::string &shaderString =
        "precision mediump float;\n"
        "uniform float u;\n"
        "void main() {\n"
        "   float a = u;\n"
        "   gl_FragColor = vec4(a);\n"
        "}\n";
    compile(shaderString);
    // An expression that's part of initialization should have rounding
    ASSERT_TRUE(foundInCode("webgl_frm(u)"));
};

TEST_F(DebugShaderPrecisionTest, CompoundAddFunction)
{
    const std::string &shaderString =
        "precision mediump float;\n"
        "uniform vec4 u;\n"
        "uniform vec4 u2;\n"
        "void main() {\n"
        "   vec4 v = u;\n"
        "   v += u2;\n"
        "   gl_FragColor = v;\n"
        "}\n";
    compile(shaderString);
    ASSERT_TRUE(foundInCode("vec4 webgl_compound_add_frm(inout vec4 x, in vec4 y) {\n"
        "    x = webgl_frm(webgl_frm(x) + y);"
    ));
    ASSERT_TRUE(foundInCode("webgl_compound_add_frm(v, webgl_frm(u2));"));
    ASSERT_FALSE(foundInCode("+="));
};

TEST_F(DebugShaderPrecisionTest, CompoundSubFunction)
{
    const std::string &shaderString =
        "precision mediump float;\n"
        "uniform vec4 u;\n"
        "uniform vec4 u2;\n"
        "void main() {\n"
        "   vec4 v = u;\n"
        "   v -= u2;\n"
        "   gl_FragColor = v;\n"
        "}\n";
    compile(shaderString);
    ASSERT_TRUE(foundInCode("vec4 webgl_compound_sub_frm(inout vec4 x, in vec4 y) {\n"
        "    x = webgl_frm(webgl_frm(x) - y);"
    ));
    ASSERT_TRUE(foundInCode("webgl_compound_sub_frm(v, webgl_frm(u2));"));
    ASSERT_FALSE(foundInCode("-="));
};

TEST_F(DebugShaderPrecisionTest, CompoundDivFunction)
{
    const std::string &shaderString =
        "precision mediump float;\n"
        "uniform vec4 u;\n"
        "uniform vec4 u2;\n"
        "void main() {\n"
        "   vec4 v = u;\n"
        "   v /= u2;\n"
        "   gl_FragColor = v;\n"
        "}\n";
    compile(shaderString);
    ASSERT_TRUE(foundInCode("vec4 webgl_compound_div_frm(inout vec4 x, in vec4 y) {\n"
        "    x = webgl_frm(webgl_frm(x) / y);"
    ));
    ASSERT_TRUE(foundInCode("webgl_compound_div_frm(v, webgl_frm(u2));"));
    ASSERT_FALSE(foundInCode("/="));
};

TEST_F(DebugShaderPrecisionTest, CompoundMulFunction)
{
    const std::string &shaderString =
        "precision mediump float;\n"
        "uniform vec4 u;\n"
        "uniform vec4 u2;\n"
        "void main() {\n"
        "   vec4 v = u;\n"
        "   v *= u2;\n"
        "   gl_FragColor = v;\n"
        "}\n";
    compile(shaderString);
    ASSERT_TRUE(foundInCode("vec4 webgl_compound_mul_frm(inout vec4 x, in vec4 y) {\n"
        "    x = webgl_frm(webgl_frm(x) * y);"
    ));
    ASSERT_TRUE(foundInCode("webgl_compound_mul_frm(v, webgl_frm(u2));"));
    ASSERT_FALSE(foundInCode("*="));
};

TEST_F(DebugShaderPrecisionTest, CompoundAddVectorPlusScalarFunction)
{
    const std::string &shaderString =
        "precision mediump float;\n"
        "uniform vec4 u;\n"
        "uniform float u2;\n"
        "void main() {\n"
        "   vec4 v = u;\n"
        "   v += u2;\n"
        "   gl_FragColor = v;\n"
        "}\n";
    compile(shaderString);
    ASSERT_TRUE(foundInCode("vec4 webgl_compound_add_frm(inout vec4 x, in float y) {\n"
        "    x = webgl_frm(webgl_frm(x) + y);"
    ));
    ASSERT_TRUE(foundInCode("webgl_compound_add_frm(v, webgl_frm(u2));"));
    ASSERT_FALSE(foundInCode("+="));
};

TEST_F(DebugShaderPrecisionTest, CompoundMatrixTimesMatrixFunction)
{
    const std::string &shaderString =
        "precision mediump float;\n"
        "uniform mat4 u;\n"
        "uniform mat4 u2;\n"
        "void main() {\n"
        "   mat4 m = u;\n"
        "   m *= u2;\n"
        "   gl_FragColor = m[0];\n"
        "}\n";
    compile(shaderString);
    ASSERT_TRUE(foundInCode(
        "mat4 webgl_compound_mul_frm(inout mat4 x, in mat4 y) {\n"
        "    x = webgl_frm(webgl_frm(x) * y);"
    ));
    ASSERT_TRUE(foundInCode("webgl_compound_mul_frm(m, webgl_frm(u2));"));
    ASSERT_FALSE(foundInCode("*="));
};

TEST_F(DebugShaderPrecisionTest, CompoundMatrixTimesScalarFunction)
{
    const std::string &shaderString =
        "precision mediump float;\n"
        "uniform mat4 u;\n"
        "uniform float u2;\n"
        "void main() {\n"
        "   mat4 m = u;\n"
        "   m *= u2;\n"
        "   gl_FragColor = m[0];\n"
        "}\n";
    compile(shaderString);
    ASSERT_TRUE(foundInCode(
        "mat4 webgl_compound_mul_frm(inout mat4 x, in float y) {\n"
        "    x = webgl_frm(webgl_frm(x) * y);"
    ));
    ASSERT_TRUE(foundInCode("webgl_compound_mul_frm(m, webgl_frm(u2));"));
    ASSERT_FALSE(foundInCode("*="));
};

TEST_F(DebugShaderPrecisionTest, CompoundVectorTimesMatrixFunction)
{
    const std::string &shaderString =
        "precision mediump float;\n"
        "uniform vec4 u;\n"
        "uniform mat4 u2;\n"
        "void main() {\n"
        "   vec4 v = u;\n"
        "   v *= u2;\n"
        "   gl_FragColor = v;\n"
        "}\n";
    compile(shaderString);
    ASSERT_TRUE(foundInCode("vec4 webgl_compound_mul_frm(inout vec4 x, in mat4 y) {\n"
        "    x = webgl_frm(webgl_frm(x) * y);"
    ));
    ASSERT_TRUE(foundInCode("webgl_compound_mul_frm(v, webgl_frm(u2));"));
    ASSERT_FALSE(foundInCode("*="));
};

TEST_F(DebugShaderPrecisionTest, CompoundVectorTimesScalarFunction)
{
    const std::string &shaderString =
        "precision mediump float;\n"
        "uniform vec4 u;\n"
        "uniform float u2;\n"
        "void main() {\n"
        "   vec4 v = u;\n"
        "   v *= u2;\n"
        "   gl_FragColor = v;\n"
        "}\n";
    compile(shaderString);
    ASSERT_TRUE(foundInCode("vec4 webgl_compound_mul_frm(inout vec4 x, in float y) {\n"
        "    x = webgl_frm(webgl_frm(x) * y);"
    ));
    ASSERT_TRUE(foundInCode("webgl_compound_mul_frm(v, webgl_frm(u2));"));
    ASSERT_FALSE(foundInCode("*="));
};

TEST_F(DebugShaderPrecisionTest, BinaryMathRounding)
{
    const std::string &shaderString =
        "precision mediump float;\n"
        "uniform vec4 u1;\n"
        "uniform vec4 u2;\n"
        "uniform vec4 u3;\n"
        "uniform vec4 u4;\n"
        "uniform vec4 u5;\n"
        "void main() {\n"
        "   vec4 v1 = u1 + u2;\n"
        "   vec4 v2 = u2 - u3;\n"
        "   vec4 v3 = u3 * u4;\n"
        "   vec4 v4 = u4 / u5;\n"
        "   vec4 v5;\n"
        "   vec4 v6 = (v5 = u5);\n"
        "   gl_FragColor = v1 + v2 + v3 + v4;\n"
        "}\n";
    compile(shaderString);
    ASSERT_TRUE(foundInCode("v1 = webgl_frm((webgl_frm(u1) + webgl_frm(u2)))"));
    ASSERT_TRUE(foundInCode("v2 = webgl_frm((webgl_frm(u2) - webgl_frm(u3)))"));
    ASSERT_TRUE(foundInCode("v3 = webgl_frm((webgl_frm(u3) * webgl_frm(u4)))"));
    ASSERT_TRUE(foundInCode("v4 = webgl_frm((webgl_frm(u4) / webgl_frm(u5)))"));
    ASSERT_TRUE(foundInCode("v6 = webgl_frm((v5 = webgl_frm(u5)))"));
};

TEST_F(DebugShaderPrecisionTest, BuiltInMathFunctionRounding)
{
    const std::string &shaderString =
        "precision mediump float;\n"
        "uniform vec4 u1;\n"
        "uniform vec4 u2;\n"
        "uniform vec4 u3;\n"
        "uniform float uf;\n"
        "uniform float uf2;\n"
        "uniform vec3 uf31;\n"
        "uniform vec3 uf32;\n"
        "uniform mat4 um1;\n"
        "uniform mat4 um2;\n"
        "void main() {\n"
        "   vec4 v1 = radians(u1);\n"
        "   vec4 v2 = degrees(u1);\n"
        "   vec4 v3 = sin(u1);\n"
        "   vec4 v4 = cos(u1);\n"
        "   vec4 v5 = tan(u1);\n"
        "   vec4 v6 = asin(u1);\n"
        "   vec4 v7 = acos(u1);\n"
        "   vec4 v8 = atan(u1);\n"
        "   vec4 v9 = atan(u1, u2);\n"
        "   vec4 v10 = pow(u1, u2);\n"
        "   vec4 v11 = exp(u1);\n"
        "   vec4 v12 = log(u1);\n"
        "   vec4 v13 = exp2(u1);\n"
        "   vec4 v14 = log2(u1);\n"
        "   vec4 v15 = sqrt(u1);\n"
        "   vec4 v16 = inversesqrt(u1);\n"
        "   vec4 v17 = abs(u1);\n"
        "   vec4 v18 = sign(u1);\n"
        "   vec4 v19 = floor(u1);\n"
        "   vec4 v20 = ceil(u1);\n"
        "   vec4 v21 = fract(u1);\n"
        "   vec4 v22 = mod(u1, uf);\n"
        "   vec4 v23 = mod(u1, u2);\n"
        "   vec4 v24 = min(u1, uf);\n"
        "   vec4 v25 = min(u1, u2);\n"
        "   vec4 v26 = max(u1, uf);\n"
        "   vec4 v27 = max(u1, u2);\n"
        "   vec4 v28 = clamp(u1, u2, u3);\n"
        "   vec4 v29 = clamp(u1, uf, uf2);\n"
        "   vec4 v30 = mix(u1, u2, u3);\n"
        "   vec4 v31 = mix(u1, u2, uf);\n"
        "   vec4 v32 = step(u1, u2);\n"
        "   vec4 v33 = step(uf, u1);\n"
        "   vec4 v34 = smoothstep(u1, u2, u3);\n"
        "   vec4 v35 = smoothstep(uf, uf2, u1);\n"
        "   vec4 v36 = normalize(u1);\n"
        "   vec4 v37 = faceforward(u1, u2, u3);\n"
        "   vec4 v38 = reflect(u1, u2);\n"
        "   vec4 v39 = refract(u1, u2, uf);\n"

        "   float f1 = length(u1);\n"
        "   float f2 = distance(u1, u2);\n"
        "   float f3 = dot(u1, u2);\n"
        "   vec3 vf31 = cross(uf31, uf32);\n"
        "   mat4 m1 = matrixCompMult(um1, um2);\n"

        "   gl_FragColor = v1 + v2 + v3 + v4 + v5 + v6 + v7 + v8 + v9 + v10 +"
            "v11 + v12 + v13 + v14 + v15 + v16 + v17 + v18 + v19 + v20 +"
            "v21 + v22 + v23 + v24 + v25 + v26 + v27 + v28 + v29 + v30 +"
            "v31 + v32 + v33 + v34 + v35 + v36 + v37 + v38 + v39 +"
            "vec4(f1, f2, f3, 0.0) + vec4(vf31, 0.0) + m1[0];\n"
        "}\n";
    compile(shaderString);
    ASSERT_TRUE(foundInCode("v1 = webgl_frm(radians(webgl_frm(u1)))"));
    ASSERT_TRUE(foundInCode("v2 = webgl_frm(degrees(webgl_frm(u1)))"));
    ASSERT_TRUE(foundInCode("v3 = webgl_frm(sin(webgl_frm(u1)))"));
    ASSERT_TRUE(foundInCode("v4 = webgl_frm(cos(webgl_frm(u1)))"));
    ASSERT_TRUE(foundInCode("v5 = webgl_frm(tan(webgl_frm(u1)))"));
    ASSERT_TRUE(foundInCode("v6 = webgl_frm(asin(webgl_frm(u1)))"));
    ASSERT_TRUE(foundInCode("v7 = webgl_frm(acos(webgl_frm(u1)))"));
    ASSERT_TRUE(foundInCode("v8 = webgl_frm(atan(webgl_frm(u1)))"));
    ASSERT_TRUE(foundInCode("v9 = webgl_frm(atan(webgl_frm(u1), webgl_frm(u2)))"));
    ASSERT_TRUE(foundInCode("v10 = webgl_frm(pow(webgl_frm(u1), webgl_frm(u2)))"));
    ASSERT_TRUE(foundInCode("v11 = webgl_frm(exp(webgl_frm(u1)))"));
    ASSERT_TRUE(foundInCode("v12 = webgl_frm(log(webgl_frm(u1)))"));
    ASSERT_TRUE(foundInCode("v13 = webgl_frm(exp2(webgl_frm(u1)))"));
    ASSERT_TRUE(foundInCode("v14 = webgl_frm(log2(webgl_frm(u1)))"));
    ASSERT_TRUE(foundInCode("v15 = webgl_frm(sqrt(webgl_frm(u1)))"));
    ASSERT_TRUE(foundInCode("v16 = webgl_frm(inversesqrt(webgl_frm(u1)))"));
    ASSERT_TRUE(foundInCode("v17 = webgl_frm(abs(webgl_frm(u1)))"));
    ASSERT_TRUE(foundInCode("v18 = webgl_frm(sign(webgl_frm(u1)))"));
    ASSERT_TRUE(foundInCode("v19 = webgl_frm(floor(webgl_frm(u1)))"));
    ASSERT_TRUE(foundInCode("v20 = webgl_frm(ceil(webgl_frm(u1)))"));
    ASSERT_TRUE(foundInCode("v21 = webgl_frm(fract(webgl_frm(u1)))"));
    ASSERT_TRUE(foundInCode("v22 = webgl_frm(mod(webgl_frm(u1), webgl_frm(uf)))"));
    ASSERT_TRUE(foundInCode("v23 = webgl_frm(mod(webgl_frm(u1), webgl_frm(u2)))"));
    ASSERT_TRUE(foundInCode("v24 = webgl_frm(min(webgl_frm(u1), webgl_frm(uf)))"));
    ASSERT_TRUE(foundInCode("v25 = webgl_frm(min(webgl_frm(u1), webgl_frm(u2)))"));
    ASSERT_TRUE(foundInCode("v26 = webgl_frm(max(webgl_frm(u1), webgl_frm(uf)))"));
    ASSERT_TRUE(foundInCode("v27 = webgl_frm(max(webgl_frm(u1), webgl_frm(u2)))"));
    ASSERT_TRUE(foundInCode("v28 = webgl_frm(clamp(webgl_frm(u1), webgl_frm(u2), webgl_frm(u3)))"));
    ASSERT_TRUE(foundInCode("v29 = webgl_frm(clamp(webgl_frm(u1), webgl_frm(uf), webgl_frm(uf2)))"));
    ASSERT_TRUE(foundInCode("v30 = webgl_frm(mix(webgl_frm(u1), webgl_frm(u2), webgl_frm(u3)))"));
    ASSERT_TRUE(foundInCode("v31 = webgl_frm(mix(webgl_frm(u1), webgl_frm(u2), webgl_frm(uf)))"));
    ASSERT_TRUE(foundInCode("v32 = webgl_frm(step(webgl_frm(u1), webgl_frm(u2)))"));
    ASSERT_TRUE(foundInCode("v33 = webgl_frm(step(webgl_frm(uf), webgl_frm(u1)))"));
    ASSERT_TRUE(foundInCode("v34 = webgl_frm(smoothstep(webgl_frm(u1), webgl_frm(u2), webgl_frm(u3)))"));
    ASSERT_TRUE(foundInCode("v35 = webgl_frm(smoothstep(webgl_frm(uf), webgl_frm(uf2), webgl_frm(u1)))"));
    ASSERT_TRUE(foundInCode("v36 = webgl_frm(normalize(webgl_frm(u1)))"));
    ASSERT_TRUE(foundInCode("v37 = webgl_frm(faceforward(webgl_frm(u1), webgl_frm(u2), webgl_frm(u3)))"));
    ASSERT_TRUE(foundInCode("v38 = webgl_frm(reflect(webgl_frm(u1), webgl_frm(u2)))"));
    ASSERT_TRUE(foundInCode("v39 = webgl_frm(refract(webgl_frm(u1), webgl_frm(u2), webgl_frm(uf)))"));

    ASSERT_TRUE(foundInCode("f1 = webgl_frm(length(webgl_frm(u1)))"));
    ASSERT_TRUE(foundInCode("f2 = webgl_frm(distance(webgl_frm(u1), webgl_frm(u2)))"));
    ASSERT_TRUE(foundInCode("f3 = webgl_frm(dot(webgl_frm(u1), webgl_frm(u2)))"));
    ASSERT_TRUE(foundInCode("vf31 = webgl_frm(cross(webgl_frm(uf31), webgl_frm(uf32)))"));
    ASSERT_TRUE(foundInCode("m1 = webgl_frm(matrixCompMult(webgl_frm(um1), webgl_frm(um2)))"));
};

TEST_F(DebugShaderPrecisionTest, BuiltInRelationalFunctionRounding)
{
    const std::string &shaderString =
        "precision mediump float;\n"
        "uniform vec4 u1;\n"
        "uniform vec4 u2;\n"
        "void main() {\n"
        "   bvec4 bv1 = lessThan(u1, u2);\n"
        "   bvec4 bv2 = lessThanEqual(u1, u2);\n"
        "   bvec4 bv3 = greaterThan(u1, u2);\n"
        "   bvec4 bv4 = greaterThanEqual(u1, u2);\n"
        "   bvec4 bv5 = equal(u1, u2);\n"
        "   bvec4 bv6 = notEqual(u1, u2);\n"
        "   gl_FragColor = vec4(bv1) + vec4(bv2) + vec4(bv3) + vec4(bv4) + vec4(bv5) + vec4(bv6);\n"
        "}\n";
    compile(shaderString);
    ASSERT_TRUE(foundInCode("bv1 = lessThan(webgl_frm(u1), webgl_frm(u2))"));
    ASSERT_TRUE(foundInCode("bv2 = lessThanEqual(webgl_frm(u1), webgl_frm(u2))"));
    ASSERT_TRUE(foundInCode("bv3 = greaterThan(webgl_frm(u1), webgl_frm(u2))"));
    ASSERT_TRUE(foundInCode("bv4 = greaterThanEqual(webgl_frm(u1), webgl_frm(u2))"));
    ASSERT_TRUE(foundInCode("bv5 = equal(webgl_frm(u1), webgl_frm(u2))"));
    ASSERT_TRUE(foundInCode("bv6 = notEqual(webgl_frm(u1), webgl_frm(u2))"));
};

TEST_F(DebugShaderPrecisionTest, ConstructorRounding)
{
    const std::string &shaderString =
        "precision mediump float;\n"
        "precision mediump int;\n"
        "uniform float u1;\n"
        "uniform float u2;\n"
        "uniform float u3;\n"
        "uniform float u4;\n"
        "uniform ivec4 uiv;\n"
        "void main() {\n"
        "   vec4 v1 = vec4(u1, u2, u3, u4);\n"
        "   vec4 v2 = vec4(uiv);\n"
        "   gl_FragColor = v1 + v2;\n"
        "}\n";
    compile(shaderString);
    // Note: this is suboptimal for the case taking four floats, but optimizing would be tricky.
    ASSERT_TRUE(foundInCode("v1 = webgl_frm(vec4(webgl_frm(u1), webgl_frm(u2), webgl_frm(u3), webgl_frm(u4)))"));
    ASSERT_TRUE(foundInCode("v2 = webgl_frm(vec4(uiv))"));
};

TEST_F(DebugShaderPrecisionTest, StructConstructorNoRounding)
{
    const std::string &shaderString =
        "precision mediump float;\n"
        "struct S { mediump vec4 a; };\n"
        "uniform vec4 u;\n"
        "void main() {\n"
        "   S s = S(u);\n"
        "   gl_FragColor = s.a;\n"
        "}\n";
    compile(shaderString);
    ASSERT_TRUE(foundInCode("s = S(webgl_frm(u))"));
    ASSERT_FALSE(foundInCode("webgl_frm(S"));
};

TEST_F(DebugShaderPrecisionTest, SwizzleRounding)
{
    const std::string &shaderString =
        "precision mediump float;\n"
        "uniform vec4 u;\n"
        "void main() {\n"
        "   vec4 v = u.xyxy;"
        "   gl_FragColor = v;\n"
        "}\n";
    compile(shaderString);
    ASSERT_TRUE(foundInCode("v = webgl_frm(u).xyxy"));
};

TEST_F(DebugShaderPrecisionTest, BuiltInTexFunctionRounding)
{
    const std::string &shaderString =
        "precision mediump float;\n"
        "precision lowp sampler2D;\n"
        "uniform vec2 u;\n"
        "uniform sampler2D s;\n"
        "void main() {\n"
        "   lowp vec4 v = texture2D(s, u);\n"
        "   gl_FragColor = v;\n"
        "}\n";
    compile(shaderString);
    ASSERT_TRUE(foundInCode("v = webgl_frl(texture2D(s, webgl_frm(u)))"));
};

TEST_F(DebugShaderPrecisionTest, FunctionCallParameterQualifiersFromDefinition)
{
    const std::string &shaderString =
        "precision mediump float;\n"
        "uniform vec4 u1;\n"
        "uniform vec4 u2;\n"
        "uniform vec4 u3;\n"
        "uniform vec4 u4;\n"
        "uniform vec4 u5;\n"
        "vec4 add(in vec4 x, in vec4 y) {\n"
        "   return x + y;\n"
        "}\n"
        "void compound_add(inout vec4 x, in vec4 y) {\n"
        "   x = x + y;\n"
        "}\n"
        "void add_to_last(in vec4 x, in vec4 y, out vec4 z) {\n"
        "   z = x + y;\n"
        "}\n"
        "void main() {\n"
        "   vec4 v = add(u1, u2);\n"
        "   compound_add(v, u3);\n"
        "   vec4 v2;\n"
        "   add_to_last(u4, u5, v2);\n"
        "   gl_FragColor = v + v2;\n"
        "}\n";
    compile(shaderString);
    // Note that this is not optimal code, there are redundant frm calls.
    // However, getting the implementation working when other operations
    // are nested within function calls would be tricky if to get right
    // otherwise.
    // Test in parameters
    ASSERT_TRUE(foundInCode("v = add(webgl_frm(u1), webgl_frm(u2))"));
    // Test inout parameter
    ASSERT_TRUE(foundInCode("compound_add(v, webgl_frm(u3))"));
    // Test out parameter
    ASSERT_TRUE(foundInCode("add_to_last(webgl_frm(u4), webgl_frm(u5), v2)"));
};

TEST_F(DebugShaderPrecisionTest, FunctionCallParameterQualifiersFromPrototype)
{
    const std::string &shaderString =
        "precision mediump float;\n"
        "uniform vec4 u1;\n"
        "uniform vec4 u2;\n"
        "uniform vec4 u3;\n"
        "uniform vec4 u4;\n"
        "uniform vec4 u5;\n"
        "vec4 add(in vec4 x, in vec4 y);\n"
        "void compound_add(inout vec4 x, in vec4 y);\n"
        "void add_to_last(in vec4 x, in vec4 y, out vec4 z);\n"
        "void main() {\n"
        "   vec4 v = add(u1, u2);\n"
        "   compound_add(v, u3);\n"
        "   vec4 v2;\n"
        "   add_to_last(u4, u5, v2);\n"
        "   gl_FragColor = v + v2;\n"
        "}\n"
        "vec4 add(in vec4 x, in vec4 y) {\n"
        "   return x + y;\n"
        "}\n"
        "void compound_add(inout vec4 x, in vec4 y) {\n"
        "   x = x + y;\n"
        "}\n"
        "void add_to_last(in vec4 x, in vec4 y, out vec4 z) {\n"
        "   z = x + y;\n"
        "}\n";
    compile(shaderString);
    // Test in parameters
    ASSERT_TRUE(foundInCode("v = add(webgl_frm(u1), webgl_frm(u2))"));
    // Test inout parameter
    ASSERT_TRUE(foundInCode("compound_add(v, webgl_frm(u3))"));
    // Test out parameter
    ASSERT_TRUE(foundInCode("add_to_last(webgl_frm(u4), webgl_frm(u5), v2)"));
};

TEST_F(DebugShaderPrecisionTest, NestedFunctionCalls)
{
    const std::string &shaderString =
        "precision mediump float;\n"
        "uniform vec4 u1;\n"
        "uniform vec4 u2;\n"
        "uniform vec4 u3;\n"
        "vec4 add(in vec4 x, in vec4 y) {\n"
        "   return x + y;\n"
        "}\n"
        "vec4 compound_add(inout vec4 x, in vec4 y) {\n"
        "   x = x + y;\n"
        "   return x;\n"
        "}\n"
        "void main() {\n"
        "   vec4 v = u1;\n"
        "   vec4 v2 = add(compound_add(v, u2), fract(u3));\n"
        "   gl_FragColor = v + v2;\n"
        "}\n";
    compile(shaderString);
    // Test nested calls
    ASSERT_TRUE(foundInCode("v2 = add(compound_add(v, webgl_frm(u2)), webgl_frm(fract(webgl_frm(u3))))"));
};
