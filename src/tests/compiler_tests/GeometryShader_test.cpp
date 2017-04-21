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

        sstream << "layout (";
        if (!primitive.empty())
        {
            sstream << primitive;
        }
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

    const std::string kHeader =
        "#version 310 es\n"
        "#extension GL_EXT_geometry_shader : require\n";
    const std::string kEmptyBody =
        "void main()\n"
        "{\n"
        "}\n";
    const std::string kInputLayout  = "layout (points) in;\n";
    const std::string kOutputLayout = "layout (points, max_vertices = 1) out;\n";

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
        "        gs_normal = vs_normal[gl_InvocationID];\n"
        "        gl_Layer   = gl_InvocationID;\n"
        "        gs_texcoord = vs_texcoord[gl_InvocationID];\n"
        "        EmitVertex();\n"
        "    }\n"
        "    EndPrimitive();\n"
        "}";

    if (!compile(shaderString))
    {
        FAIL() << "Shader compilation failed, expecting success:\n" << mInfoLog;
    }

    EXPECT_EQ(4, mTranslator->getGeometryShaderInvocations());
    EXPECT_EQ(3, mTranslator->getGeometryShaderMaxVertices());
    EXPECT_EQ(EgsTriangles, mTranslator->getGeometryShaderInputPrimitiveType());
    EXPECT_EQ(EgsTriangleStrip, mTranslator->getGeometryShaderOutputPrimitiveType());
}

// Geometry Shaders are not supported in GLSL ES shaders version lower than 310.
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

// Geometry Shaders are not supported in GLSL ES shaders version lower than 310.
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

// Geometry Shaders are not supported in GLSL ES shaders version 310 without extension
// EXT_geometry_shader enabled.
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

// Geometry Shaders are supported in GLSL ES shaders version 310 with EXT_geometry_shader enabled.
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

// It should be a linking error instead of compilation error to lack the declarations of input
// primitive in a geometry shader.
TEST_F(GeometryShaderTest, NoInputPrimitives)
{
    const std::string &shaderString =
        "#version 310 es\n"
        "#extension GL_EXT_geometry_shader : require\n"
        "layout(points, max_vertices = 1) out;\n"
        "void main()\n"
        "{\n"
        "}\n";
    if (!compile(shaderString))
    {
        FAIL() << "Shader compilation failed, expecting success:\n" << mInfoLog;
    }
}

// Geometry Shaders can only support 5 kinds of input primitives, which cannot be used as output
// primitives except 'points'.
// Skip testing "points" as it can be used as both input and output primitives.
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

// Geometry Shaders allow duplicated declaration of input primitive, but don't allow multiple
// different input primitives.
TEST_F(GeometryShaderTest, InputPrimitiveRedeclarations)
{
    const std::array<std::string, 5> kInputPrimitives = {
        {"points", "lines", "lines_adjacency", "triangles", "triangles_adjacency"}};

    for (GLuint i = 0; i < kInputPrimitives.size(); ++i)
    {
        const std::string &inputLayoutStr1 =
            GetGeometryShaderLayout("in", kInputPrimitives[i], -1, -1);
        if (!compileGeometryShader(inputLayoutStr1, inputLayoutStr1, kOutputLayout, ""))
        {
            FAIL() << "Shader compilation failed, expecting success:\n" << mInfoLog;
        }

        for (GLuint j = i + 1; j < kInputPrimitives.size(); ++j)
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

// Geometry Shaders don't allow declaring different input primitives.
TEST_F(GeometryShaderTest, DeclareDifferentInputPrimitivesInOneLayout)
{
    const std::string &shaderString =
        "#version 310 es\n"
        "#extension GL_EXT_geometry_shader : require\n"
        "layout (points, triangles) in;\n"
        "layout (points, max_vertices = 1) out;\n"
        "void main()\n"
        "{\n"
        "}\n";
    if (compile(shaderString))
    {
        FAIL() << "Shader compilation succeeded, expecting failure:\n" << mInfoLog;
    }
}

// Geometry Shaders don't allow 'invocations' < 1.
TEST_F(GeometryShaderTest, InvocationsLessThanOne)
{
    const std::string &shaderString =
        "#version 310 es\n"
        "#extension GL_EXT_geometry_shader : require\n"
        "layout (points, invocations = 0) in;\n"
        "layout (points, max_vertices = 1) out;\n"
        "void main()\n"
        "{\n"
        "}\n";
    if (compile(shaderString))
    {
        FAIL() << "Shader compilation succeeded, expecting failure:\n" << mInfoLog;
    }
}

// Geometry Shaders allow 'invocations' == 1.
TEST_F(GeometryShaderTest, InvocationsEqualsOne)
{
    const std::string &shaderString =
        "#version 310 es\n"
        "#extension GL_EXT_geometry_shader : require\n"
        "layout (points, invocations = 1) in;\n"
        "layout (points, max_vertices = 1) out;\n"
        "void main()\n"
        "{\n"
        "}\n";
    if (!compile(shaderString))
    {
        FAIL() << "Shader compilation failed, expecting success:\n" << mInfoLog;
    }
}

// Geometry Shaders allow 'invocations' == 1.
TEST_F(GeometryShaderTest, InvocationsEqualsOneSeparateLayout)
{
    const std::string &shaderString =
        "#version 310 es\n"
        "#extension GL_EXT_geometry_shader : require\n"
        "layout (points) in;\n"
        "layout (invocations = 1) in;\n"
        "layout (points, max_vertices = 1) out;\n"
        "void main()\n"
        "{\n"
        "}\n";
    if (!compile(shaderString))
    {
        FAIL() << "Shader compilation failed, expecting success:\n" << mInfoLog;
    }
}

// Geometry Shaders don't allow 'invocations' larger than the maximum.
TEST_F(GeometryShaderTest, TooLargeInvocations)
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

// Geometry Shaders allow 'invocations' declared in the same input layout with input primitives.
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
        FAIL() << "Shader compilation failed, expecting success:\n" << mInfoLog;
    }
}

// Geometry Shaders allow 'invocations' declared before input primitives in the same input layout.
TEST_F(GeometryShaderTest, InvocationsBeforeInputPrimitives)
{
    const std::string &shaderString =
        "#version 310 es\n"
        "#extension GL_EXT_geometry_shader : require\n"
        "layout (invocations = 3, points) in;\n"
        "layout (points, max_vertices = 1) out;\n"
        "void main()\n"
        "{\n"
        "}\n";
    if (!compile(shaderString))
    {
        FAIL() << "Shader compilation failed, expecting success:\n" << mInfoLog;
    }
}

// Geometry Shaders allow 'invocations' declared in an individual input layout expressions.
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
        FAIL() << "Shader compilation failed, expecting success:\n" << mInfoLog;
    }
}

// Geometry Shaders allow duplicated declaration of 'invocations'.
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
        FAIL() << "Shader compilation failed, expecting success:\n" << mInfoLog;
    }
}

// Geometry Shaders don't allow multiple different 'invocations'.
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

// Geometry Shaders don't allow multiple different 'invocations'.
TEST_F(GeometryShaderTest, RedeclareDifferentInvocationsAfterInvocationEqualsOne)
{
    const std::string &shaderString =
        "#version 310 es\n"
        "#extension GL_EXT_geometry_shader : require\n"
        "layout (points, invocations = 1) in;\n"
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

// Geometry Shader don't allow multiple different 'invocations'.
TEST_F(GeometryShaderTest, RedeclareDifferentInvocationsInOneLayout)
{
    const std::string &shaderString =
        "#version 310 es\n"
        "#extension GL_EXT_geometry_shader : require\n"
        "layout (points, invocations = 3, invocations = 5) in;\n"
        "layout (points, max_vertices = 1) out;\n"
        "void main()\n"
        "{\n"
        "}\n";
    if (compile(shaderString))
    {
        FAIL() << "Shader compilation succeeded, expecting failure:\n" << mInfoLog;
    }
}

// Geometry Shaders don't allow 'invocations' in out layouts.
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

// Geometry Shaders don't allow 'invocations' in layouts without qualifiers.
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

// An input of a Geometry Shader can be declared without an array size and will be sized by an
// earlier legal input primitive declaration when it is used.
TEST_F(GeometryShaderTest, UnsizedInputDeclarationBeforeInputPrimitive)
{
    const std::string &shaderString =
        "#version 310 es\n"
        "#extension GL_EXT_geometry_shader : require\n"
        "in vec4 texcoord1[];\n"
        "in vec4 color[1];\n"
        "in vec4 texcoord2[];\n"
        "layout (points) in;\n"
        "layout (points, max_vertices = 1) out;\n"
        "void main()\n"
        "{\n"
        "    vec4 coord1 = texcoord1[0];\n"
        "    int length1 = texcoord1.length();\n"
        "    vec4 coord2 = texcoord2[0];\n"
        "    int length2 = texcoord2.length();\n"
        "}\n";

    if (!compile(shaderString))
    {
        FAIL() << "Shader compilation failed, expecting success:\n" << mInfoLog;
    }
}

// An input of a Geometry Shader can be declared without an array size and will be sized by an
// earlier legal input primitive declaration when it is used.
TEST_F(GeometryShaderTest, UnsizedInputDeclarationAfterInputPrimitive)
{
    const std::string &shaderString =
        "#version 310 es\n"
        "#extension GL_EXT_geometry_shader : require\n"
        "layout (points) in;\n"
        "layout (points, max_vertices = 1) out;\n"
        "in vec4 texcoord[];\n"
        "void main()\n"
        "{\n"
        "    int len = texcoord.length();\n"
        "    vec4 coord = texcoord[0];\n"
        "}\n";
    if (!compile(shaderString))
    {
        FAIL() << "Shader compilation failed, expecting success:\n" << mInfoLog;
    }
}

// An input of a Geometry Shader can be declared without an array size but it won't
// be sized without any legal input primitive declarations.
TEST_F(GeometryShaderTest, UnsizedInputDeclarationWithoutInputPrimitive)
{
    const std::string &shaderString1 =
        "#version 310 es\n"
        "#extension GL_EXT_geometry_shader : require\n"
        "layout (points, max_vertices = 1) out;\n"
        "in vec4 texcoord[];\n"
        "void main()\n"
        "{\n"
        "    int len = texcoord.length();\n"
        "}\n";

    const std::string &shaderString2 =
        "#version 310 es\n"
        "#extension GL_EXT_geometry_shader : require\n"
        "layout (points, max_vertices = 1) out;\n"
        "in vec4 texcoord[];\n"
        "void main()\n"
        "{\n"
        "    vec4 coord = texcoord[0];\n"
        "}\n";

    if (compile(shaderString1) || compile(shaderString2))
    {
        FAIL() << "Shader compilation succeeded, expecting failure:\n" << mInfoLog;
    }
}

// Geometry Shaders allow inputs with location qualifiers.
TEST_F(GeometryShaderTest, InputWithLocations)
{
    const std::string &shaderString =
        "#version 310 es\n"
        "#extension GL_EXT_geometry_shader : require\n"
        "layout (points) in;\n"
        "layout (points, max_vertices = 1) out;\n"
        "layout (location = 0) in vec4 texcoord[];\n"
        "void main()\n"
        "{\n"
        "}\n";
    if (!compile(shaderString))
    {
        FAIL() << "Shader compilation failed, expecting success:\n" << mInfoLog;
    }
}

// Geometry Shaders allow inputs with explicit size declared before the declaration of the
// input primitive, but they should have same size and match the declaration of the
// following input primitive declaration.
TEST_F(GeometryShaderTest, InputWithSizeBeforeInputPrimitive)
{
    const std::array<std::string, 5> kInputPrimitives = {
        {"points", "lines", "lines_adjacency", "triangles", "triangles_adjacency"}};
    constexpr int kInputArraySize[] = {1, 2, 4, 3, 6};

    for (GLuint i = 0; i < kInputPrimitives.size(); i++)
    {
        const std::string &inputLayoutStr =
            GetGeometryShaderLayout("in", kInputPrimitives[i], -1, -1);
        const int inputSize = kInputArraySize[i];

        const std::string &inputDeclaration1 = GetInputDeclaration("var1", inputSize);
        if (!compileGeometryShader(inputDeclaration1, "", inputLayoutStr, kOutputLayout))
        {
            FAIL() << "Shader compilation failed, expecting success:\n" << mInfoLog;
        }

        const std::string &inputDeclaration2 = GetInputDeclaration("var2", inputSize);
        if (!compileGeometryShader(inputDeclaration1, inputDeclaration2, inputLayoutStr,
                                   kOutputLayout))
        {
            FAIL() << "Shader compilation failed, expecting success:\n" << mInfoLog;
        }

        const std::string &inputDeclaration3 = GetInputDeclaration("var3", inputSize + 1);
        if (compileGeometryShader(inputDeclaration3, "", inputLayoutStr, kOutputLayout) ||
            compileGeometryShader(inputDeclaration1, inputDeclaration3, inputLayoutStr,
                                  kOutputLayout))
        {
            FAIL() << "Shader compilation succeeded, expecting failure:\n" << mInfoLog;
        }
    }
}

// Geometry shaders allow inputs with explicit size declared after the declaration of the
// input primitive, but their sizes should match the previous input primitive declaration.
TEST_F(GeometryShaderTest, InputWithSizeAfterInputPrimitive)
{
    const std::array<std::string, 5> kInputPrimitives = {
        {"points", "lines", "lines_adjacency", "triangles", "triangles_adjacency"}};
    constexpr int kInputArraySize[] = {1, 2, 4, 3, 6};

    for (GLuint i = 0; i < kInputPrimitives.size(); i++)
    {
        const std::string &inputLayoutStr =
            GetGeometryShaderLayout("in", kInputPrimitives[i], -1, -1);
        const int inputSize = kInputArraySize[i];

        const std::string &inputDeclaration1 = GetInputDeclaration("var1", inputSize);
        if (!compileGeometryShader(inputLayoutStr, kOutputLayout, inputDeclaration1, ""))
        {
            FAIL() << "Shader compilation failed, expecting success:\n" << mInfoLog;
        }

        const std::string &inputDeclaration2 = GetInputDeclaration("var2", inputSize + 1);
        if (compileGeometryShader(inputLayoutStr, kOutputLayout, inputDeclaration2, ""))
        {
            FAIL() << "Shader compilation succeeded, expecting failure:\n" << mInfoLog;
        }
    }
}

// Geometry Shaders allow declaring output primitive before input primitive declaration.
TEST_F(GeometryShaderTest, DeclareOutputPrimitiveBeforeInputPrimitiveDeclare)
{
    const std::string &shaderString =
        "#version 310 es\n"
        "#extension GL_EXT_geometry_shader : require\n"
        "layout (points, max_vertices = 1) out;\n"
        "layout (points) in;\n"
        "void main()\n"
        "{\n"
        "}\n";
    if (!compile(shaderString))
    {
        FAIL() << "Shader compilation failed, expecting success:\n" << mInfoLog;
    }
}

// Geometry Shaders allow declaring 'max_vertices' before output primitive in the same output
// layout.
TEST_F(GeometryShaderTest, DeclareMaxVerticesBeforeOutputPrimitive)
{
    const std::string &shaderString =
        "#version 310 es\n"
        "#extension GL_EXT_geometry_shader : require\n"
        "layout (points) in;\n"
        "layout (max_vertices = 1, points) out;\n"
        "void main()\n"
        "{\n"
        "}\n";
    if (!compile(shaderString))
    {
        FAIL() << "Shader compilation failed, expecting success:\n" << mInfoLog;
    }
}

// It should be a linking error instead of a compilation error to lack the declarations of output
// primitive in a geometry shader.
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
    if (!compile(shaderString))
    {
        FAIL() << "Shader compilation failed, expecting success:\n" << mInfoLog;
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

// Geometry Shaders allow duplicated declaration of input primitive, but don't allow multiple
// different input primitives.
TEST_F(GeometryShaderTest, RedeclareOutputPrimitives)
{
    const std::array<std::string, 3> outPrimitives = {{"points", "line_strip", "triangle_strip"}};

    for (GLuint i = 0; i < outPrimitives.size(); i++)
    {
        constexpr int maxVertices = 1;
        const std::string &outputLayoutStr1 =
            GetGeometryShaderLayout("out", outPrimitives[i], -1, maxVertices);
        const std::string &outputLayoutStr2 =
            GetGeometryShaderLayout("out", outPrimitives[i], -1, -1);
        if (!compileGeometryShader(kInputLayout, outputLayoutStr1, outputLayoutStr2, ""))
        {
            FAIL() << "Shader compilation failure, expecting succeed:\n" << mInfoLog;
        }
        for (GLuint j = i + 1; j < outPrimitives.size(); j++)
        {
            const std::string &outputLayoutStr3 =
                GetGeometryShaderLayout("out", outPrimitives[j], -1, -1);
            if (compileGeometryShader(kInputLayout, outputLayoutStr1, outputLayoutStr3, ""))
            {
                FAIL() << "Shader compilation succeed, expecting failure:\n" << mInfoLog;
            }
        }
    }
}

// Geometry Shaders don't allow declaring different output primitives.
TEST_F(GeometryShaderTest, RedeclareDifferentOutputPrimitivesInOneLayout)
{
    const std::string &shaderString =
        "#version 310 es\n"
        "#extension GL_EXT_geometry_shader : require\n"
        "layout (points) in;\n"
        "layout (points, max_vertices = 3, line_strip) out;\n"
        "void main()\n"
        "{\n"
        "}\n";
    if (compile(shaderString))
    {
        FAIL() << "Shader compilation succeeded, expecting failure:\n" << mInfoLog;
    }
}

// It should be a linking error instead of a compilation error to lack the declarations of output
// primitives and max_vertices in a geometry shader.
TEST_F(GeometryShaderTest, NoOutLayouts)
{
    const std::string &shaderString =
        "#version 310 es\n"
        "#extension GL_EXT_geometry_shader : require\n"
        "layout (points) in;\n"
        "void main()\n"
        "{\n"
        "}\n";
    if (!compile(shaderString))
    {
        FAIL() << "Shader compilation failed, expecting success:\n" << mInfoLog;
    }
}

// It should be a linking error instead of a compilation error to lack the declarations of
// 'max_vertices' in a geometry shader.
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
    if (!compile(shaderString))
    {
        FAIL() << "Shader compilation failed, expecting success:\n" << mInfoLog;
    }
}

// Geometry Shaders cannot declare a negative 'max_vertices'.
TEST_F(GeometryShaderTest, NegativeMaxVertices)
{
    const std::string &shaderString =
        "#version 310 es\n"
        "#extension GL_EXT_geometry_shader : require\n"
        "layout (points) in;\n"
        "layout (points, max_vertices = -1) out;\n"
        "void main()\n"
        "{\n"
        "}\n";
    if (compile(shaderString))
    {
        FAIL() << "Shader compilation succeeded, expecting failure:\n" << mInfoLog;
    }
}

// Geometry Shaders allow max_vertices == 0.
TEST_F(GeometryShaderTest, ZeroMaxVertices)
{
    const std::string &shaderString =
        "#version 310 es\n"
        "#extension GL_EXT_geometry_shader : require\n"
        "layout (points) in;\n"
        "layout (points, max_vertices = 0) out;\n"
        "void main()\n"
        "{\n"
        "}\n";
    if (!compile(shaderString))
    {
        FAIL() << "Shader compilation failed, expecting success:\n" << mInfoLog;
    }
}

// Geometry Shaders cannot declare a 'max_vertices' that is greater than
// MAX_GEOMETRY_OUTPUT_VERTICES_EXT (256).
TEST_F(GeometryShaderTest, TooLargeMaxVertices)
{
    const std::string &shaderString =
        "#version 310 es\n"
        "#extension GL_EXT_geometry_shader : require\n"
        "layout (points) in;\n"
        "layout (points, max_vertices = 257) out;\n"
        "void main()\n"
        "{\n"
        "}\n";
    if (compile(shaderString))
    {
        FAIL() << "Shader compilation succeeded, expecting failure:\n" << mInfoLog;
    }
}

// Geometry Shaders can declare 'max_vertices' declared in an individual out layout.
TEST_F(GeometryShaderTest, MaxVerticesInIndividualLayout)
{
    const std::string &shaderString =
        "#version 310 es\n"
        "#extension GL_EXT_geometry_shader : require\n"
        "layout (points) in;\n"
        "layout (points) out;\n"
        "layout (max_vertices = 1) out;\n"
        "void main()\n"
        "{\n"
        "}\n";
    if (!compile(shaderString))
    {
        FAIL() << "Shader compilation failed, expecting success:\n" << mInfoLog;
    }
}

// Geometry Shaders allow duplicated declaration of 'max_vertices'.
TEST_F(GeometryShaderTest, DuplicatedMaxVertices)
{
    const std::string &shaderString =
        "#version 310 es\n"
        "#extension GL_EXT_geometry_shader : require\n"
        "layout (points) in;\n"
        "layout (points, max_vertices = 1) out;\n"
        "layout (max_vertices = 1) out;\n"
        "void main()\n"
        "{\n"
        "}\n";
    if (!compile(shaderString))
    {
        FAIL() << "Shader compilation failed, expecting success:\n" << mInfoLog;
    }
}

// Geometry Shaders don't allow declaring different 'max_vertices'.
TEST_F(GeometryShaderTest, RedeclareDifferentMaxVertices)
{
    const std::string &shaderString =
        "#version 310 es\n"
        "#extension GL_EXT_geometry_shader : require\n"
        "layout (points) in;\n"
        "layout (points, max_vertices = 1) out;\n"
        "layout (max_vertices = 2) out;\n"
        "void main()\n"
        "{\n"
        "}\n";
    if (compile(shaderString))
    {
        FAIL() << "Shader compilation succeeded, expecting failure:\n" << mInfoLog;
    }
}

// Geometry Shaders don't allow declaring different 'max_vertices'.
TEST_F(GeometryShaderTest, RedeclareDifferentMaxVerticesInOneLayout)
{
    const std::string &shaderString =
        "#version 310 es\n"
        "#extension GL_EXT_geometry_shader : require\n"
        "layout (points) in;\n"
        "layout (points, max_vertices = 2, max_vertices = 1) out;\n"
        "void main()\n"
        "{\n"
        "}\n";
    if (compile(shaderString))
    {
        FAIL() << "Shader compilation succeeded, expecting failure:\n" << mInfoLog;
    }
}

// Geometry Shaders allow declaring non-array outputs.
TEST_F(GeometryShaderTest, NonArrayOutputs)
{
    const std::string &shaderString =
        "#version 310 es\n"
        "#extension GL_EXT_geometry_shader : require\n"
        "layout (points) in;\n"
        "layout (points, max_vertices = 1) out;\n"
        "out vec4 o_texcoord;\n"
        "void main()\n"
        "{\n"
        "    o_texcoord = vec4(0, 0, 0, 1);\n"
        "}\n";
    if (!compile(shaderString))
    {
        FAIL() << "Shader compilation failed, expecting success:\n" << mInfoLog;
    }
}

// Geometry Shaders allow declaring non-array outputs with location layout qualifiers.
TEST_F(GeometryShaderTest, NonArrayOutputsWithLocation)
{
    const std::string &shaderString =
        "#version 310 es\n"
        "#extension GL_EXT_geometry_shader : require\n"
        "layout (points) in;\n"
        "layout (points, max_vertices = 1) out;\n"
        "layout (location = 0) out vec4 o_texcoord;\n"
        "void main()\n"
        "{\n"
        "    o_texcoord = vec4(0, 0, 0, 1);\n"
        "}\n";
    if (!compile(shaderString))
    {
        FAIL() << "Shader compilation failed, expecting success:\n" << mInfoLog;
    }
}

// It isn't allowed to declare unsized array outputs in Geometry Shaders.
TEST_F(GeometryShaderTest, UnsizedArrayOutputs)
{
    const std::string &shaderString =
        "#version 310 es\n"
        "#extension GL_EXT_geometry_shader : require\n"
        "layout (points) in;\n"
        "layout (points, max_vertices = 1) out;\n"
        "out vec4 o_texcoord[];\n"
        "void main()\n"
        "{\n"
        "}\n";
    if (compile(shaderString))
    {
        FAIL() << "Shader compilation failed, expecting success:\n" << mInfoLog;
    }
}

// Geometry Shaders allow declaring sized array outputs.
TEST_F(GeometryShaderTest, SizedArrayOutputs)
{
    const std::string &shaderString =
        "#version 310 es\n"
        "#extension GL_EXT_geometry_shader : require\n"
        "layout (points) in;\n"
        "layout (points, max_vertices = 1) out;\n"
        "out vec4 o_texcoord[2];\n"
        "void main()\n"
        "{\n"
        "    o_texcoord[0] = vec4(0, 1, 0, 1);\n"
        "    o_texcoord[1] = vec4(1, 0, 0, 1);\n"
        "}\n";
    if (!compile(shaderString))
    {
        FAIL() << "Shader compilation failed, expecting success:\n" << mInfoLog;
    }
}

// Verify the built-in variables that Geometry Shader should support
TEST_F(GeometryShaderTest, BuiltInVariables)
{
    const std::string &shaderString =
        "#version 310 es\n"
        "#extension GL_EXT_geometry_shader : require\n"
        "layout (points, invocations = 2) in;\n"
        "layout (points, max_vertices = 2) out;\n"
        "void main()\n"
        "{\n"
        "    gl_Position = gl_in[gl_InvocationID].gl_Position;\n"
        "    int invocation = gl_InvocationID;\n"
        "    gl_Layer = invocation;\n"
        "    int primitiveIn = gl_PrimitiveIDIn;\n"
        "    gl_PrimitiveID = primitiveIn;\n"
        "}\n";
    if (!compile(shaderString))
    {
        FAIL() << "Shader compilation failed, expecting success:\n" << mInfoLog;
    }
}

// Verify that gl_PrimitiveIDIn cannot be assigned a value.
TEST_F(GeometryShaderTest, AssignValueToGLPrimitiveIn)
{
    const std::string &shaderString =
        "#version 310 es\n"
        "#extension GL_EXT_geometry_shader : require\n"
        "layout (points, invocations = 2) in;\n"
        "layout (points, max_vertices = 2) out;\n"
        "void main()\n"
        "{\n"
        "    gl_PrimitiveIDIn = 1;\n"
        "}\n";
    if (compile(shaderString))
    {
        FAIL() << "Shader compilation succeeded, expecting failure:\n" << mInfoLog;
    }
}

// Verify that gl_InvocationID cannot be assigned a value.
TEST_F(GeometryShaderTest, AssignValueToGLInvocations)
{
    const std::string &shaderString =
        "#version 310 es\n"
        "#extension GL_EXT_geometry_shader : require\n"
        "layout (points, invocations = 2) in;\n"
        "layout (points, max_vertices = 2) out;\n"
        "void main()\n"
        "{\n"
        "    gl_InvocationID = 1;\n"
        "}\n";
    if (compile(shaderString))
    {
        FAIL() << "Shader compilation succeeded, expecting failure:\n" << mInfoLog;
    }
}

// Verify that gl_in[].gl_Position cannot be assigned a value.
TEST_F(GeometryShaderTest, AssignValueToGLIn)
{
    const std::string &shaderString =
        "#version 310 es\n"
        "#extension GL_EXT_geometry_shader : require\n"
        "layout (points) in;\n"
        "layout (points, max_vertices = 2) out;\n"
        "void main()\n"
        "{\n"
        "    gl_in[0].gl_Position = vec4(0, 0, 0, 1);\n"
        "}\n";
    if (compile(shaderString))
    {
        FAIL() << "Shader compilation succeeded, expecting failure:\n" << mInfoLog;
    }
}

// Verify that both EmitVertex() and EndPrimitive() are supported in Geometry Shader.
TEST_F(GeometryShaderTest, GeometryShaderBuiltInFunctions)
{
    const std::string &shaderString =
        "#version 310 es\n"
        "#extension GL_EXT_geometry_shader : require\n"
        "layout (points) in;\n"
        "layout (points, max_vertices = 2) out;\n"
        "void main()\n"
        "{\n"
        "    gl_Position = gl_in[0].gl_Position;\n"
        "    EmitVertex();\n"
        "    EndPrimitive();\n"
        "}\n";
    if (!compile(shaderString))
    {
        FAIL() << "Shader compilation failed, expecting success:\n" << mInfoLog;
    }
}

// Verify that all required built-in constant values are supported in Geometry Shaders
TEST_F(GeometryShaderTest, GeometryShaderBuiltInConstants)
{
    const std::string &shaderString =
        "#version 310 es\n"
        "#extension GL_EXT_geometry_shader : require\n"
        "layout (points) in;\n"
        "layout (points, max_vertices = 2) out;\n"
        "void main()\n"
        "{\n"
        "    int val1 = gl_MaxGeometryInputComponents;\n"
        "    int val2 = gl_MaxGeometryOutputComponents;\n"
        "    int val3 = gl_MaxGeometryImageUniforms;\n"
        "    int val4 = gl_MaxGeometryTextureImageUnits;\n"
        "    int val5 = gl_MaxGeometryOutputVertices;\n"
        "    int val6 = gl_MaxGeometryTotalOutputComponents;\n"
        "    int val7 = gl_MaxGeometryUniformComponents;\n"
        "    int val8 = gl_MaxGeometryAtomicCounters;\n"
        "    int val9 = gl_MaxGeometryAtomicCounterBuffers;\n"
        "}\n";
    if (!compile(shaderString))
    {
        FAIL() << "Shader compilation failed, expecting success: \n" << mInfoLog;
    }
}

// Verify that gl_in[].gl_PointSize isn't supported in Geometry Shaders without declaring
// GL_EXT_geometry_point_size.
TEST_F(GeometryShaderTest, GeometryShaderInputPointSizeNoExtension)
{
    const std::string &shaderString =
        "#version 310 es\n"
        "#extension GL_EXT_geometry_shader : require\n"
        "layout (points) in;\n"
        "layout (points, max_vertices = 2) out;\n"
        "void main()\n"
        "{\n"
        "    float pointSize;\n"
        "    pointSize = gl_in[0].gl_PointSize;\n"
        "}\n";
    if (compile(shaderString))
    {
        FAIL() << "Shader compilation succeeded, expecting failure:\n" << mInfoLog;
    }
}

// Verify that gl_PointSize isn't supported in Geometry Shaders without declaring
// GL_EXT_geometry_point_size.
TEST_F(GeometryShaderTest, GeometryShaderOutputPointSizeNoExtension)
{
    const std::string &shaderString =
        "#version 310 es\n"
        "#extension GL_EXT_geometry_shader : require\n"
        "layout (points) in;\n"
        "layout (points, max_vertices = 2) out;\n"
        "void main()\n"
        "{\n"
        "    gl_PointSize = 1.0;\n"
        "}\n";
    if (compile(shaderString))
    {
        FAIL() << "Shader compilation succeeded, expecting failure:\n" << mInfoLog;
    }
}

// Verify that Geometry Shaders allow 'flat' interpolation qualifier on input variables.
TEST_F(GeometryShaderTest, FlatInput)
{
    const std::string &shaderString =
        "#version 310 es\n"
        "#extension GL_EXT_geometry_shader : require\n"
        "layout (points) in;\n"
        "layout (points, max_vertices = 2) out;\n"
        "flat in vec4 color[];\n"
        "void main()\n"
        "{\n"
        "}\n";
    if (!compile(shaderString))
    {
        FAIL() << "Shader compilation failed, expecting success:\n" << mInfoLog;
    }
}

// Verify that Geometry Shaders allow 'flat' interpolation qualifier on output variables.
TEST_F(GeometryShaderTest, FlatOutput)
{
    const std::string &shaderString =
        "#version 310 es\n"
        "#extension GL_EXT_geometry_shader : require\n"
        "layout (points) in;\n"
        "layout (points, max_vertices = 2) out;\n"
        "flat out vec4 color;\n"
        "void main()\n"
        "{\n"
        "}\n";
    if (!compile(shaderString))
    {
        FAIL() << "Shader compilation failed, expecting success:\n" << mInfoLog;
    }
}

// Verify that Geometry Shaders allow 'smooth' interpolation qualifier on input variables.
TEST_F(GeometryShaderTest, SmoothInput)
{
    const std::string &shaderString =
        "#version 310 es\n"
        "#extension GL_EXT_geometry_shader : require\n"
        "layout (points) in;\n"
        "layout (points, max_vertices = 2) out;\n"
        "smooth in vec4 color[];\n"
        "void main()\n"
        "{\n"
        "}\n";
    if (!compile(shaderString))
    {
        FAIL() << "Shader compilation failed, expecting success:\n" << mInfoLog;
    }
}

// Verify that Geometry Shaders allow 'smooth' interpolation qualifier on output variables.
TEST_F(GeometryShaderTest, SmoothOutput)
{
    const std::string &shaderString =
        "#version 310 es\n"
        "#extension GL_EXT_geometry_shader : require\n"
        "layout (points) in;\n"
        "layout (points, max_vertices = 2) out;\n"
        "smooth out vec4 color;\n"
        "void main()\n"
        "{\n"
        "}\n";
    if (!compile(shaderString))
    {
        FAIL() << "Shader compilation failed, expecting success:\n" << mInfoLog;
    }
}

// Verify that Geometry Shaders allow 'centroid' qualifier on input variables.
TEST_F(GeometryShaderTest, CentroidInput)
{
    const std::string &shaderString =
        "#version 310 es\n"
        "#extension GL_EXT_geometry_shader : require\n"
        "layout (points) in;\n"
        "layout (points, max_vertices = 2) out;\n"
        "centroid in vec4 color[];\n"
        "void main()\n"
        "{\n"
        "}\n";
    if (!compile(shaderString))
    {
        FAIL() << "Shader compilation failed, expecting success:\n" << mInfoLog;
    }
}

// Verify that Geometry Shaders allow 'centroid' qualifier on output variables.
TEST_F(GeometryShaderTest, CentroidOutput)
{
    const std::string &shaderString =
        "#version 310 es\n"
        "#extension GL_EXT_geometry_shader : require\n"
        "layout (points) in;\n"
        "layout (points, max_vertices = 2) out;\n"
        "centroid out vec4 color;\n"
        "void main()\n"
        "{\n"
        "}\n";
    if (!compile(shaderString))
    {
        FAIL() << "Shader compilation failed, expecting success:\n" << mInfoLog;
    }
}

// Verify that Geometry Shaders doesn't allow 'location' defined with input/output primitives.
TEST_F(GeometryShaderTest, invalidLocation)
{
    const std::string &shaderString1 =
        "#version 310 es\n"
        "#extension GL_EXT_geometry_shader : require\n"
        "layout (points, location = 1) in;\n"
        "layout (points, max_vertices = 2) out;\n"
        "out vec4 color;\n"
        "void main()\n"
        "{\n"
        "}\n";

    const std::string &shaderString2 =
        "#version 310 es\n"
        "#extension GL_EXT_geometry_shader : require\n"
        "layout (points) in;\n"
        "layout (invocations = 2, location = 1) in;\n"
        "layout (points, max_vertices = 2) out;\n"
        "out vec4 color;\n"
        "void main()\n"
        "{\n"
        "}\n";

    const std::string &shaderString3 =
        "#version 310 es\n"
        "#extension GL_EXT_geometry_shader : require\n"
        "layout (points) in;\n"
        "layout (points, location = 3, max_vertices = 2) out;\n"
        "out vec4 color;\n"
        "void main()\n"
        "{\n"
        "}\n";

    const std::string &shaderString4 =
        "#version 310 es\n"
        "#extension GL_EXT_geometry_shader : require\n"
        "layout (points) in;\n"
        "layout (points) out;\n"
        "layout (max_vertices = 2, location = 3) out;\n"
        "out vec4 color;\n"
        "void main()\n"
        "{\n"
        "}\n";
    if (compile(shaderString1) || compile(shaderString2) || compile(shaderString3) ||
        compile(shaderString4))
    {
        FAIL() << "Shader compilation succeeded, expecting failure:\n" << mInfoLog;
    }
}

// Verify that Geometry Shaders doesn't allow invalid layout qualifier declarations.
TEST_F(GeometryShaderTest, invalidLayoutQualifiers)
{
    const std::string &shaderString1 =
        "#version 310 es\n"
        "#extension GL_EXT_geometry_shader : require\n"
        "layout (points, abc) in;\n"
        "layout (points, max_vertices = 2) out;\n"
        "out vec4 color;\n"
        "void main()\n"
        "{\n"
        "}\n";

    const std::string &shaderString2 =
        "#version 310 es\n"
        "#extension GL_EXT_geometry_shader : require\n"
        "layout (points) in;\n"
        "layout (points, abc, max_vertices = 2) out;\n"
        "out vec4 color;\n"
        "void main()\n"
        "{\n"
        "}\n";

    const std::string &shaderString3 =
        "#version 310 es\n"
        "#extension GL_EXT_geometry_shader : require\n"
        "layout (points, xyz = 2) in;\n"
        "layout (points, max_vertices = 2) out;\n"
        "out vec4 color;\n"
        "void main()\n"
        "{\n"
        "}\n";

    const std::string &shaderString4 =
        "#version 310 es\n"
        "#extension GL_EXT_geometry_shader : require\n"
        "layout (points) in;\n"
        "layout (points) out;\n"
        "layout (max_vertices = 2, xyz = 3) out;\n"
        "out vec4 color;\n"
        "void main()\n"
        "{\n"
        "}\n";
    if (compile(shaderString1) || compile(shaderString2) || compile(shaderString3) ||
        compile(shaderString4))
    {
        FAIL() << "Shader compilation succeeded, expecting failure:\n" << mInfoLog;
    }
}
