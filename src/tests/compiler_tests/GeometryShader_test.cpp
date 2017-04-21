//
// Copyright (c) 2017 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//
// GeometryShader_test.cpp:
// tests for compiling a Geometry Shader
//

#include "GLSLANG/ShaderLang.h"
#include "angle_gl.h"
#include "compiler/translator/BaseTypes.h"
#include "compiler/translator/TranslatorESSL.h"
#include "gtest/gtest.h"
#include "tests/test_utils/compiler_test.h"

using namespace sh;

class GeometryShaderTest : public testing::Test
{
  public:
    GeometryShaderTest() {}

  protected:
    void SetUp() override
    {
        ShBuiltInResources resources;
        InitBuiltInResources(&resources);
        resources.EXT_geometry_shader = 1;

        mTranslator = new TranslatorESSL(GL_GEOMETRY_SHADER_EXT, SH_GLES3_1_SPEC);
        ASSERT_TRUE(mTranslator->Init(resources));
    }

    void TearDown() override { SafeDelete(mTranslator); }

    // Return true when compilation succeeds
    bool compile(const std::string &shaderString)
    {
        const char *shaderStrings[] = {shaderString.c_str()};

        bool status         = mTranslator->compile(shaderStrings, 1, SH_OBJECT_CODE | SH_VARIABLES);
        TInfoSink &infoSink = mTranslator->getInfoSink();
        mInfoLog            = infoSink.info.c_str();
        // std::cout << mInfoLog << std::endl;
        // std::cout << infoSink.obj.c_str() << std::endl;
        return status;
    }

    bool compileGeometryShader(const std::string &statement1, const std::string &statement2)
    {
        std::ostringstream sstream;
        sstream << kHeader << statement1 << statement2 << kEmptyBody;
        return compile(sstream.str());
    }

    bool compileGeometryShader(const std::string &statement1,
                               const std::string &statement2,
                               const std::string &statement3,
                               const std::string &statement4)
    {
        std::ostringstream sstream;
        sstream << kHeader << statement1 << statement2 << statement3 << statement4 << kEmptyBody;
        return compile(sstream.str());
    }

    static std::string GetGeometryShaderLayout(const std::string &layoutType,
                                               const std::string &primitive,
                                               int invocations,
                                               int maxVertices)
    {
        std::ostringstream sstream;
        sstream << "layout (" << primitive;
        if (invocations > 0)
        {
            sstream << ", invocations = " << invocations;
        }
        if (maxVertices >= 0)
        {
            sstream << ", max_vertices = " << maxVertices;
        }
        sstream << ") " << layoutType << ";" << std::endl;

        return sstream.str();
    }

    static std::string GetInputDeclaration(const std::string &var, int size)
    {
        std::ostringstream sstream;
        sstream << "in vec4 ";
        if (size < 0)
        {
            sstream << var << "[];\n";
        }
        else
        {
            sstream << var << "[" << size << "];\n";
        }

        return sstream.str();
    }

    const std::string &kHeader =
        "#version 310 es\n"
        "#extension GL_EXT_geometry_shader : require\n";
    const std::string &kEmptyBody =
        "void main()\n"
        "{\n"
        "}\n";
    const std::string &kInputLayout  = "layout (points) in;\n";
    const std::string &kOutputLayout = "layout (points, max_vertices = 1) out;\n";

    std::string mInfoLog;
    TranslatorESSL *mTranslator = nullptr;
};

// Compile a basic Geometry Shader.
TEST_F(GeometryShaderTest, BasicGeometryShader)
{
    const std::string &shaderString =
        "#version 310 es\n"
        "#extension GL_EXT_geometry_shader : require\n"
        "layout (triangles, invocations = 4) in;\n"
        "layout (triangle_strip, max_vertices = 3) out;\n"
        "in vec3 vs_normal[];\n"
        "in vec2 vs_texcoord[3];\n"
        "out vec3 gs_normal;\n"
        "out vec2 gs_texcoord;\n"
        "void main()\n"
        "{\n"
        "    int n;\n"
        "    for (n = 0; n < gl_in.length(); n++)\n"
        "    {\n"
        "        gl_Position = gl_in[n].gl_Position;\n"
        "        gl_PointSize = gl_in[n].gl_PointSize;\n"
        "        gs_normal = vs_normal[gl_InvocationID];\n"
        "        gl_Layer   = gl_InvocationID;\n"
        "        gs_texcoord = vs_texcoord[gl_InvocationID];\n"
        "        EmitVertex();\n"
        "    }\n"
        "    EndPrimitive();\n"
        "}";

    compile(shaderString);

    EXPECT_EQ(4, mTranslator->getGeometryShaderInvocations());
    EXPECT_EQ(3, mTranslator->getGeometryShaderMaxVertices());
    EXPECT_EQ(EgsTriangles, mTranslator->getGeometryShaderInputPrimitives());
    EXPECT_EQ(EgsTriangleStrip, mTranslator->getGeometryShaderOutputPrimitives());
}

// Geometry shaders are not supported in versions lower than 310.
TEST_F(GeometryShaderTest, Version100)
{
    const std::string &shaderString =
        "layout(points) in;\n"
        "layout(points, max_vertices = 1) out;\n"
        "void main()\n"
        "{\n"
        "}\n";
    if (compile(shaderString))
    {
        FAIL() << "Shader compilation succeeded, expecting failure:\n" << mInfoLog;
    }
}

// Geometry shaders are not supported in versions lower than 310.
TEST_F(GeometryShaderTest, Version300)
{
    const std::string &shaderString =
        "#version 300 es\n"
        "layout(points) in;\n"
        "layout(points, max_vertices = 1) out;\n"
        "void main()\n"
        "{\n"
        "}\n";
    if (compile(shaderString))
    {
        FAIL() << "Shader compilation succeeded, expecting failure:\n" << mInfoLog;
    }
}

// Geometry shaders are not supported in version 310 without EXT_geometry_shader enabled.
TEST_F(GeometryShaderTest, Version310WithoutExtension)
{
    const std::string &shaderString =
        "#version 310 es\n"
        "layout(points) in;\n"
        "layout(points, max_vertices = 1) out;\n"
        "void main()\n"
        "{\n"
        "}\n";
    if (compile(shaderString))
    {
        FAIL() << "Shader compilation succeeded, expecting failure:\n" << mInfoLog;
    }
}

// Geometry shaders are supported in version 310 with EXT_geometry_shader enabled.
TEST_F(GeometryShaderTest, Version310WithExtension)
{
    const std::string &shaderString =
        "#version 310 es\n"
        "#extension GL_EXT_geometry_shader : require\n"
        "layout(points) in;\n"
        "layout(points, max_vertices = 1) out;\n"
        "void main()\n"
        "{\n"
        "}\n";
    if (!compile(shaderString))
    {
        FAIL() << "Shader compilation failure, expecting succeed:\n" << mInfoLog;
    }
}

// Geometry shaders cannot lack the declarations of input primitive.
TEST_F(GeometryShaderTest, NoInputPrimitives)
{
    const std::string &shaderString =
        "#version 310 es\n"
        "#extension GL_EXT_geometry_shader : require\n"
        "layout(points, max_vertices = 1) out;\n"
        "void main()\n"
        "{\n"
        "}\n";
    if (compile(shaderString))
    {
        FAIL() << "Shader compilation succeeded, expecting failure:\n" << mInfoLog;
    }
}

// Geometry Shaders can only support 5 kinds of input primitives, which cannot be used as output
// primitives except 'points'. Skip testing "points" as it can be used as both input and output
// primitives.
TEST_F(GeometryShaderTest, ValidateInputPrimitives)
{
    const std::array<std::string, 4> kInputPrimitives = {
        {"lines", "lines_adjacency", "triangles", "triangles_adjacency"}};

    for (const std::string &inputPrimitive : kInputPrimitives)
    {
        if (!compileGeometryShader(GetGeometryShaderLayout("in", inputPrimitive, -1, -1),
                                   kOutputLayout))
        {
            FAIL() << "Shader compilation failure, expecting succeed:\n" << mInfoLog;
        }
        if (compileGeometryShader(kInputLayout,
                                  GetGeometryShaderLayout("out", inputPrimitive, -1, 6)))
        {
            FAIL() << "Shader compilation succeed, expecting failure:\n" << mInfoLog;
        }
    }
}

// Geometry Shaders	allow duplicated declaration of input primitive, but don't allow multiple
// different input primitives.
TEST_F(GeometryShaderTest, InputPrimitiveRedeclarations)
{
    const std::array<std::string, 5> kInputPrimitives = {
        {"points", "lines", "lines_adjacency", "triangles", "triangles_adjacency"}};

    for (int i = 0; i < kInputPrimitives.size(); ++i)
    {
        const std::string &inputLayoutStr1 =
            GetGeometryShaderLayout("in", kInputPrimitives[i], -1, -1);
        if (!compileGeometryShader(inputLayoutStr1, inputLayoutStr1, kOutputLayout, ""))
        {
            FAIL() << "Shader compilation failure, expecting succeeded:\n" << mInfoLog;
        }

        for (int j = i + 1; j < kInputPrimitives.size(); ++j)
        {
            const std::string &inputLayoutStr2 =
                GetGeometryShaderLayout("in", kInputPrimitives[j], -1, -1);
            if (compileGeometryShader(inputLayoutStr1, inputLayoutStr2, kOutputLayout, ""))
            {
                FAIL() << "Shader compilation succeeded, expecting failure:\n" << mInfoLog;
            }
        }
    }
}

// Geometry Shaders don't allow negative invocations.
TEST_F(GeometryShaderTest, NegativeInvocations)
{
    const std::string &shaderString =
        "#version 310 es\n"
        "#extension GL_EXT_geometry_shader : require\n"
        "layout (points, invocations = -1) in;\n"
        "layout (points, max_vertices = 1) out;\n"
        "void main()\n"
        "{\n"
        "}\n";
    if (compile(shaderString))
    {
        FAIL() << "Shader compilation succeeded, expecting failure:\n" << mInfoLog;
    }
}

// Geometry Shaders don't allow an invocation bigger than the maximum.
TEST_F(GeometryShaderTest, TooBigInvocations)
{
    const std::string &shaderString =
        "#version 310 es\n"
        "#extension GL_EXT_geometry_shader : require\n"
        "layout (points, invocations = 9989899) in;\n"
        "layout (points, max_vertices = 1) out;\n"
        "void main()\n"
        "{\n"
        "}\n";
    if (compile(shaderString))
    {
        FAIL() << "Shader compilation succeeded, expecting failure:\n" << mInfoLog;
    }
}

// Geometry Shaders allow invocations declared in the same input layout with input primitives.
TEST_F(GeometryShaderTest, InvocationsWithInputPrimitives)
{
    const std::string &shaderString =
        "#version 310 es\n"
        "#extension GL_EXT_geometry_shader : require\n"
        "layout (points, invocations = 3) in;\n"
        "layout (points, max_vertices = 1) out;\n"
        "void main()\n"
        "{\n"
        "}\n";
    if (!compile(shaderString))
    {
        FAIL() << "Shader compilation failure, expecting succeeded:\n" << mInfoLog;
    }
}

// Geometry Shaders allow invocations declared in an individual input layout expressions.
TEST_F(GeometryShaderTest, InvocationsInIndividualLayout)
{
    const std::string &shaderString =
        "#version 310 es\n"
        "#extension GL_EXT_geometry_shader : require\n"
        "layout (points) in;\n"
        "layout (invocations = 3) in;\n"
        "layout (points, max_vertices = 1) out;\n"
        "void main()\n"
        "{\n"
        "}\n";
    if (!compile(shaderString))
    {
        FAIL() << "Shader compilation failure, expecting succeeded:\n" << mInfoLog;
    }
}

// Geometry Shaders	allow duplicated declaration of invocations.
TEST_F(GeometryShaderTest, DuplicatedInvocations)
{
    const std::string &shaderString =
        "#version 310 es\n"
        "#extension GL_EXT_geometry_shader : require\n"
        "layout (points, invocations = 3) in;\n"
        "layout (invocations = 3) in;\n"
        "layout (points, max_vertices = 1) out;\n"
        "void main()\n"
        "{\n"
        "}\n";
    if (!compile(shaderString))
    {
        FAIL() << "Shader compilation failure, expecting succeeded:\n" << mInfoLog;
    }
}

// Geometry Shaders	don't allow multiple different invocations.
TEST_F(GeometryShaderTest, RedeclareDifferentInvocations)
{
    const std::string &shaderString =
        "#version 310 es\n"
        "#extension GL_EXT_geometry_shader : require\n"
        "layout (points, invocations = 3) in;\n"
        "layout (invocations = 5) in;\n"
        "layout (points, max_vertices = 1) out;\n"
        "void main()\n"
        "{\n"
        "}\n";
    if (compile(shaderString))
    {
        FAIL() << "Shader compilation succeeded, expecting failure:\n" << mInfoLog;
    }
}

// Geometry Shaders don't allow invocations in out layouts.
TEST_F(GeometryShaderTest, DeclareInvocationsInOutLayout)
{
    const std::string &shaderString =
        "#version 310 es\n"
        "#extension GL_EXT_geometry_shader : require\n"
        "layout (points) in;\n"
        "layout (points, invocations = 3, max_vertices = 1) out;\n"
        "void main()\n"
        "{\n"
        "}\n";
    if (compile(shaderString))
    {
        FAIL() << "Shader compilation succeeded, expecting failure:\n" << mInfoLog;
    }
}

// Geometry Shaders don't allow invocations in layouts without qualifiers.
TEST_F(GeometryShaderTest, DeclareInvocationsInLayoutNoQualifier)
{
    const std::string &shaderString =
        "#version 310 es\n"
        "#extension GL_EXT_geometry_shader : require\n"
        "layout (points) in;\n"
        "layout (invocations = 3);\n"
        "layout (points, max_vertices = 1) out;\n"
        "void main()\n"
        "{\n"
        "}\n";
    if (compile(shaderString))
    {
        FAIL() << "Shader compilation succeeded, expecting failure:\n" << mInfoLog;
    }
}

// Geometry Shaders can only accept arrays as their inputs.
TEST_F(GeometryShaderTest, InputNotArray)
{
    const std::string &shaderString =
        "#version 310 es\n"
        "#extension GL_EXT_geometry_shader : require\n"
        "layout (points) in;\n"
        "layout (points, max_vertices = 1) out;\n"
        "in vec4 texcoord;\n"
        "void main()\n"
        "{\n"
        "}\n";
    if (compile(shaderString))
    {
        FAIL() << "Shader compilation succeeded, expecting failure:\n" << mInfoLog;
    }
}

// Geometry shaders allow an input without explicitly declaring its size after the declaration of
// the input primitive. Its size should be set by the type of the input primitive.
TEST_F(GeometryShaderTest, InputWithoutSizeAfterInputPrimitiveDeclaration)
{
    const std::string &shaderString =
        "#version 310 es\n"
        "#extension GL_EXT_geometry_shader : require\n"
        "layout (points) in;\n"
        "layout (points, max_vertices = 1) out;\n"
        "in vec4 texcoord[];\n"
        "void main()\n"
        "{\n"
        "}\n";
    if (!compile(shaderString))
    {
        FAIL() << "Shader compilation failure, expecting succeeded:\n" << mInfoLog;
    }
}

// Geometry shaders allow an input with explicit size declared, but its size should match the type
// of the input primitive.
TEST_F(GeometryShaderTest, InputWithExplicitSize)
{
    const std::array<std::string, 5> kInputPrimitives = {
        {"points", "lines", "lines_adjacency", "triangles", "triangles_adjacency"}};
    constexpr int kInputArraySize[] = {1, 2, 4, 3, 6};

    for (int i = 0; i < kInputPrimitives.size(); i++)
    {
        const std::string &inputLayoutStr =
            GetGeometryShaderLayout("in", kInputPrimitives[i], -1, -1);
        const int inputSize = kInputArraySize[i];

        const std::string &inputDeclaration1 = GetInputDeclaration("var1", inputSize);
        if (!compileGeometryShader(inputLayoutStr, kOutputLayout, inputDeclaration1, ""))
        {
            FAIL() << "Shader compilation failure, expecting succeeded:\n" << mInfoLog;
        }

        const std::string &inputDeclaration2 = GetInputDeclaration("var2", inputSize + 1);
        if (compileGeometryShader(inputLayoutStr, kOutputLayout, inputDeclaration2, ""))
        {
            FAIL() << "Shader compilation succeeded, expecting failure:\n" << mInfoLog;
        }
    }
}

// Geometry shaders cannot lack the declarations of output primitive.
TEST_F(GeometryShaderTest, NoOutputPrimitives)
{
    const std::string &shaderString =
        "#version 310 es\n"
        "#extension GL_EXT_geometry_shader : require\n"
        "layout (points) in;\n"
        "layout (max_vertices = 1) out;\n"
        "void main()\n"
        "{\n"
        "}\n";
    if (compile(shaderString))
    {
        FAIL() << "Shader compilation succeeded, expecting failure:\n" << mInfoLog;
    }
}

// Geometry shaders cannot lack the declarations of max_vertices.
TEST_F(GeometryShaderTest, NoMaxVertices)
{
    const std::string &shaderString =
        "#version 310 es\n"
        "#extension GL_EXT_geometry_shader : require\n"
        "layout (points) in;\n"
        "layout (points) out;\n"
        "void main()\n"
        "{\n"
        "}\n";
    if (compile(shaderString))
    {
        FAIL() << "Shader compilation succeeded, expecting failure:\n" << mInfoLog;
    }
}

// Geometry Shaders can only support 3 kinds of output primitives, which cannot be used as input
// primitives except 'points'. Skip testing "points" as it can be used as both input and output
// primitives.
TEST_F(GeometryShaderTest, ValidateOutputPrimitives)
{
    const std::string outputPrimitives[] = {"line_strip", "triangle_strip"};

    for (const std::string &outPrimitive : outputPrimitives)
    {
        if (!compileGeometryShader(kInputLayout,
                                   GetGeometryShaderLayout("out", outPrimitive, -1, 6)))
        {
            FAIL() << "Shader compilation failure, expecting succeed:\n" << mInfoLog;
        }

        if (compileGeometryShader(GetGeometryShaderLayout("in", outPrimitive, -1, -1),
                                  kOutputLayout))
        {
            FAIL() << "Shader compilation succeed, expecting failure:\n" << mInfoLog;
        }
    }
}