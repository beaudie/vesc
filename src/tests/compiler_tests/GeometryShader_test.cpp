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

// Missing the declarations of output primitives and 'max_vertices' in a geometry shader should
// be a link error instead of a compile error.
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

// Missing the declarations of 'max_vertices' in a geometry shader should be a link error
// instead of a compile error.
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

// Verify that Geometry Shaders doesn't allow 'location' defined with input/output primitives.
TEST_F(GeometryShaderTest, invalidLocation)
{
    const std::string &shaderString1 =
        "#version 310 es\n"
        "#extension GL_EXT_geometry_shader : require\n"
        "layout (points, location = 1) in;\n"
        "layout (points, max_vertices = 2) out;\n"
        "void main()\n"
        "{\n"
        "}\n";

    const std::string &shaderString2 =
        "#version 310 es\n"
        "#extension GL_EXT_geometry_shader : require\n"
        "layout (points) in;\n"
        "layout (invocations = 2, location = 1) in;\n"
        "layout (points, max_vertices = 2) out;\n"
        "void main()\n"
        "{\n"
        "}\n";

    const std::string &shaderString3 =
        "#version 310 es\n"
        "#extension GL_EXT_geometry_shader : require\n"
        "layout (points) in;\n"
        "layout (points, location = 3, max_vertices = 2) out;\n"
        "void main()\n"
        "{\n"
        "}\n";

    const std::string &shaderString4 =
        "#version 310 es\n"
        "#extension GL_EXT_geometry_shader : require\n"
        "layout (points) in;\n"
        "layout (points) out;\n"
        "layout (max_vertices = 2, location = 3) out;\n"
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
        "void main()\n"
        "{\n"
        "}\n";

    const std::string &shaderString2 =
        "#version 310 es\n"
        "#extension GL_EXT_geometry_shader : require\n"
        "layout (points) in;\n"
        "layout (points, abc, max_vertices = 2) out;\n"
        "void main()\n"
        "{\n"
        "}\n";

    const std::string &shaderString3 =
        "#version 310 es\n"
        "#extension GL_EXT_geometry_shader : require\n"
        "layout (points, xyz = 2) in;\n"
        "layout (points, max_vertices = 2) out;\n"
        "void main()\n"
        "{\n"
        "}\n";

    const std::string &shaderString4 =
        "#version 310 es\n"
        "#extension GL_EXT_geometry_shader : require\n"
        "layout (points) in;\n"
        "layout (points) out;\n"
        "layout (max_vertices = 2, xyz = 3) out;\n"
        "void main()\n"
        "{\n"
        "}\n";
    if (compile(shaderString1) || compile(shaderString2) || compile(shaderString3) ||
        compile(shaderString4))
    {
        FAIL() << "Shader compilation succeeded, expecting failure:\n" << mInfoLog;
    }
}
