//
// Copyright 2024 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//
// These tests verify shadow sampler functions and their options.

#include "common/gl_enum_utils.h"
#include "test_utils/ANGLETest.h"
#include "test_utils/gl_raii.h"

using namespace angle;

namespace
{

enum class FunctionType
{
    Texture,
    TextureBias,
    TextureOffset,
    TextureOffsetBias,
    TextureLod,
    TextureLodOffset,
    TextureGrad,
    TextureGradOffset,
    TextureProj,
    TextureProjBias,
    TextureProjOffset,
    TextureProjOffsetBias,
    TextureProjLod,
    TextureProjLodOffset,
    TextureProjGrad,
    TextureProjGradOffset,
};

const char *FunctionName(FunctionType function)
{
    switch (function)
    {
        case FunctionType::Texture:
        case FunctionType::TextureBias:
            return "texture";
        case FunctionType::TextureOffset:
        case FunctionType::TextureOffsetBias:
            return "textureOffset";
        case FunctionType::TextureLod:
            return "textureLod";
        case FunctionType::TextureLodOffset:
            return "textureLodOffset";
        case FunctionType::TextureGrad:
            return "textureGrad";
        case FunctionType::TextureGradOffset:
            return "textureGradOffset";
        case FunctionType::TextureProj:
        case FunctionType::TextureProjBias:
            return "textureProj";
        case FunctionType::TextureProjOffset:
        case FunctionType::TextureProjOffsetBias:
            return "textureProjOffset";
        case FunctionType::TextureProjLod:
            return "textureProjLod";
        case FunctionType::TextureProjLodOffset:
            return "textureProjLodOffset";
        case FunctionType::TextureProjGrad:
            return "textureProjGrad";
        case FunctionType::TextureProjGradOffset:
            return "textureProjGradOffset";
    }
}

constexpr bool IsProj(FunctionType function)
{
    switch (function)
    {
        case FunctionType::TextureProj:
        case FunctionType::TextureProjBias:
        case FunctionType::TextureProjOffset:
        case FunctionType::TextureProjOffsetBias:
        case FunctionType::TextureProjLod:
        case FunctionType::TextureProjLodOffset:
        case FunctionType::TextureProjGrad:
        case FunctionType::TextureProjGradOffset:
            return true;
        default:
            return false;
    }
}

constexpr bool HasBias(FunctionType function)
{
    switch (function)
    {
        case FunctionType::TextureBias:
        case FunctionType::TextureOffsetBias:
        case FunctionType::TextureProjBias:
        case FunctionType::TextureProjOffsetBias:
            return true;
        default:
            return false;
    }
}

constexpr bool HasLOD(FunctionType function)
{
    switch (function)
    {
        case FunctionType::TextureLod:
        case FunctionType::TextureLodOffset:
        case FunctionType::TextureProjLod:
        case FunctionType::TextureProjLodOffset:
            return true;
        default:
            return false;
    }
}

constexpr bool HasGrad(FunctionType function)
{
    switch (function)
    {
        case FunctionType::TextureGrad:
        case FunctionType::TextureGradOffset:
        case FunctionType::TextureProjGrad:
        case FunctionType::TextureProjGradOffset:
            return true;
        default:
            return false;
    }
}

constexpr bool HasOffset(FunctionType function)
{
    switch (function)
    {
        case FunctionType::TextureOffset:
        case FunctionType::TextureOffsetBias:
        case FunctionType::TextureLodOffset:
        case FunctionType::TextureGradOffset:
        case FunctionType::TextureProjOffset:
        case FunctionType::TextureProjOffsetBias:
        case FunctionType::TextureProjLodOffset:
        case FunctionType::TextureProjGradOffset:
            return true;
        default:
            return false;
    }
}

enum class RefValueType
{
    Zero,
    Quarter,
    Half,
    ThreeQuarters,
    One,
};

float RefValueFloat(RefValueType refValue)
{
    switch (refValue)
    {
        case RefValueType::Zero:
            return 0.0;
        case RefValueType::Quarter:
            return 0.25;
        case RefValueType::Half:
            return 0.5;
        case RefValueType::ThreeQuarters:
            return 0.75;
        case RefValueType::One:
            return 1.0;
    }
}

bool Compare(float reference, float sampled, GLenum op)
{
    switch (op)
    {
        case GL_NEVER:
            return false;
        case GL_LESS:
            return reference < sampled;
        case GL_EQUAL:
            return reference == sampled;
        case GL_LEQUAL:
            return reference <= sampled;
        case GL_GREATER:
            return reference > sampled;
        case GL_NOTEQUAL:
            return reference != sampled;
        case GL_GEQUAL:
            return reference >= sampled;
        case GL_ALWAYS:
            return true;
        default:
            UNREACHABLE();
            return false;
    }
}

// Variations corresponding to enums above.
using ShadowSamplerFunctionVariationsTestParams =
    std::tuple<angle::PlatformParameters, FunctionType, GLenum, RefValueType, bool>;

std::ostream &operator<<(std::ostream &out, FunctionType function)
{
    switch (function)
    {
        case FunctionType::Texture:
            out << "Texture";
            break;
        case FunctionType::TextureBias:
            out << "TextureBias";
            break;
        case FunctionType::TextureOffset:
            out << "TextureOffset";
            break;
        case FunctionType::TextureOffsetBias:
            out << "TextureOffsetBias";
            break;
        case FunctionType::TextureLod:
            out << "TextureLod";
            break;
        case FunctionType::TextureLodOffset:
            out << "TextureLodOffset";
            break;
        case FunctionType::TextureGrad:
            out << "TextureGrad";
            break;
        case FunctionType::TextureGradOffset:
            out << "TextureGradOffset";
            break;
        case FunctionType::TextureProj:
            out << "TextureProj";
            break;
        case FunctionType::TextureProjBias:
            out << "TextureProjBias";
            break;
        case FunctionType::TextureProjOffset:
            out << "TextureProjOffset";
            break;
        case FunctionType::TextureProjOffsetBias:
            out << "TextureProjOffsetBias";
            break;
        case FunctionType::TextureProjLod:
            out << "TextureProjLod";
            break;
        case FunctionType::TextureProjLodOffset:
            out << "TextureProjLodOffset";
            break;
        case FunctionType::TextureProjGrad:
            out << "TextureProjGrad";
            break;
        case FunctionType::TextureProjGradOffset:
            out << "TextureProjGradOffset";
            break;
    }

    return out;
}

std::ostream &operator<<(std::ostream &out, RefValueType refValue)
{
    switch (refValue)
    {
        case RefValueType::Zero:
            out << "Zero";
            break;
        case RefValueType::Quarter:
            out << "Quarter";
            break;
        case RefValueType::Half:
            out << "Half";
            break;
        case RefValueType::ThreeQuarters:
            out << "ThreeQuarters";
            break;
        case RefValueType::One:
            out << "One";
            break;
    }

    return out;
}

void ParseShadowSamplerFunctionVariationsTestParams(
    const ShadowSamplerFunctionVariationsTestParams &params,
    FunctionType *functionOut,
    GLenum *compareFuncOut,
    RefValueType *refValueOut,
    bool *mipmappedOut)
{
    *functionOut    = std::get<1>(params);
    *compareFuncOut = std::get<2>(params);
    *refValueOut    = std::get<3>(params);
    *mipmappedOut   = std::get<4>(params);
}

std::string ShadowSamplerFunctionVariationsTestPrint(
    const ::testing::TestParamInfo<ShadowSamplerFunctionVariationsTestParams> &paramsInfo)
{
    const ShadowSamplerFunctionVariationsTestParams &params = paramsInfo.param;
    std::ostringstream out;

    out << std::get<0>(params);

    FunctionType function;
    GLenum compareFunc;
    RefValueType refValue;
    bool mipmapped;
    ParseShadowSamplerFunctionVariationsTestParams(params, &function, &compareFunc, &refValue,
                                                   &mipmapped);

    out << "__" << function << "_" << gl::GLenumToString(gl::GLESEnum::DepthFunction, compareFunc)
        << "_" << refValue << "_" << (mipmapped ? "Mipmapped" : "NonMipmapped");
    return out.str();
}

class ShadowSamplerFunctionTest : public ANGLETest<ShadowSamplerFunctionVariationsTestParams>
{
  protected:
    ShadowSamplerFunctionTest()
    {
        setWindowWidth(16);
        setWindowHeight(16);
        setConfigRedBits(8);
        setConfigGreenBits(8);
        setConfigBlueBits(8);
        setConfigAlphaBits(8);
    }

    void setupProgram2D(FunctionType function, bool useShadowSampler, float dRef)
    {
        std::stringstream fragmentSource;
        fragmentSource << "#version 300 es\n"
                       << "precision mediump float;\n"
                       << "precision mediump sampler2D;\n"
                       << "precision mediump sampler2DShadow;\n"
                       << "uniform sampler2D" << (useShadowSampler ? "Shadow" : "") << " tex;\n"
                       << "in vec4 v_position;\n"
                       << "out vec4 my_FragColor;\n"
                       << "void main()\n"
                       << "{\n"
                       << "    vec2 texcoord = v_position.xy * 0.5 + 0.5;\n"
                       << "    float r = " << FunctionName(function) << "(tex, ";
        if (IsProj(function))
        {
            if (useShadowSampler)
            {
                fragmentSource << "vec4(texcoord * 2.0, float(" << dRef << ") * 2.0, 2.0)";
            }
            else
            {
                fragmentSource << "vec3(texcoord * 2.0, 2.0)";
            }
        }
        else
        {
            if (useShadowSampler)
            {
                fragmentSource << "vec3(texcoord, float(" << dRef << "))";
            }
            else
            {
                fragmentSource << "vec2(texcoord)";
            }
        }

        if (HasLOD(function))
        {
            fragmentSource << ", 2.0";
        }
        else if (HasGrad(function))
        {
            fragmentSource << ", vec2(0.17), vec2(0.17)";
        }

        if (HasOffset(function))
        {
            // Does not affect LOD selection, added to try all overloads.
            fragmentSource << ", ivec2(1, 1)";
        }

        if (HasBias(function))
        {
            fragmentSource << ", 3.0";
        }

        fragmentSource << ")" << (useShadowSampler ? "" : ".r") << ";\n";
        if (useShadowSampler)
        {
            fragmentSource << "    my_FragColor = vec4(0.0, r, 1.0 - r, 1.0);\n";
        }
        else
        {
            fragmentSource << "    my_FragColor = vec4(r, 0.0, 0.0, 1.0);\n";
        }
        fragmentSource << "}";

        ANGLE_GL_PROGRAM(program, essl3_shaders::vs::Passthrough(), fragmentSource.str().c_str());
        glUseProgram(program);
        mPrg = program;
    }

    GLuint mPrg = 0;
};

// Test TEXTURE_2D with shadow samplers
TEST_P(ShadowSamplerFunctionTest, Texture2D)
{
    FunctionType function;
    GLenum compareFunc;
    RefValueType refValue;
    bool mipmapped;
    ParseShadowSamplerFunctionVariationsTestParams(GetParam(), &function, &compareFunc, &refValue,
                                                   &mipmapped);

    GLTexture tex;
    const std::vector<GLfloat> level0(64, 0.125f);
    const std::vector<GLfloat> level1(16, 0.5f);
    const std::vector<GLfloat> level2(4, 0.25f);
    const std::vector<GLfloat> level3(1, 0.75f);

    glBindTexture(GL_TEXTURE_2D, tex);
    glTexStorage2D(GL_TEXTURE_2D, 4, GL_DEPTH_COMPONENT32F, 8, 8);
    glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, 8, 8, GL_DEPTH_COMPONENT, GL_FLOAT, level0.data());
    glTexSubImage2D(GL_TEXTURE_2D, 1, 0, 0, 4, 4, GL_DEPTH_COMPONENT, GL_FLOAT, level1.data());
    glTexSubImage2D(GL_TEXTURE_2D, 2, 0, 0, 2, 2, GL_DEPTH_COMPONENT, GL_FLOAT, level2.data());
    glTexSubImage2D(GL_TEXTURE_2D, 3, 0, 0, 1, 1, GL_DEPTH_COMPONENT, GL_FLOAT, level3.data());
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER,
                    mipmapped ? GL_NEAREST_MIPMAP_NEAREST : GL_NEAREST);
    ASSERT_GL_NO_ERROR();

    float expectedSample;
    if (mipmapped)
    {
        if (HasBias(function))
        {
            // Base level 8x8, viewport 8x8, bias 3.0
            expectedSample = level3[0];
        }
        else if (HasLOD(function))
        {
            // Explicitly requested level 2
            expectedSample = level2[0];
        }
        else if (HasGrad(function))
        {
            // Screen space derivatives of 0.17 for a 8x8 texture should resolve to level 1
            expectedSample = level1[0];
        }
        else  // implicit LOD
        {
            // Base level 8x8, viewport 8x8, no bias
            expectedSample = level0[0];
        }
    }
    else
    {
        // LOD options must have no effect when the texture is not mipmapped.
        expectedSample = level0[0];
    }

    glViewport(0, 0, 8, 8);
    glClearColor(1.0, 0.0, 1.0, 1.0);

    // First sample the texture directly for easier debugging
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_COMPARE_MODE, GL_NONE);
    setupProgram2D(function, false, 0.0);
    ASSERT_GL_NO_ERROR();

    glClear(GL_COLOR_BUFFER_BIT);
    drawQuad(mPrg, essl3_shaders::PositionAttrib(), 0.0f);
    EXPECT_PIXEL_COLOR_NEAR(0, 0, GLColor(expectedSample * 255.0, 0, 0, 255), 1);

    // Try shadow samplers
    setupProgram2D(function, true, RefValueFloat(refValue));
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_COMPARE_MODE, GL_COMPARE_REF_TO_TEXTURE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_COMPARE_FUNC, compareFunc);
    ASSERT_GL_NO_ERROR();

    glClear(GL_COLOR_BUFFER_BIT);
    drawQuad(mPrg, essl3_shaders::PositionAttrib(), 0.0f);
    if (Compare(RefValueFloat(refValue), expectedSample, compareFunc))
    {
        EXPECT_PIXEL_RECT_EQ(0, 0, 8, 8, GLColor::green);
    }
    else
    {
        EXPECT_PIXEL_RECT_EQ(0, 0, 8, 8, GLColor::blue);
    }
}

constexpr FunctionType kFunctionTypes[] = {
    FunctionType::Texture,           FunctionType::TextureBias,
    FunctionType::TextureOffset,     FunctionType::TextureOffsetBias,
    FunctionType::TextureLod,        FunctionType::TextureLodOffset,
    FunctionType::TextureGrad,       FunctionType::TextureGradOffset,
    FunctionType::TextureProj,       FunctionType::TextureProjBias,
    FunctionType::TextureProjOffset, FunctionType::TextureProjOffsetBias,
    FunctionType::TextureProjLod,    FunctionType::TextureProjLodOffset,
    FunctionType::TextureProjGrad,   FunctionType::TextureProjGradOffset,
};
constexpr GLenum kCompareFuncs[] = {
    GL_LEQUAL, GL_GEQUAL, GL_LESS, GL_GREATER, GL_EQUAL, GL_NOTEQUAL, GL_ALWAYS, GL_NEVER,
};
constexpr RefValueType kRefValueTypes[] = {
    RefValueType::Zero,          RefValueType::Quarter, RefValueType::Half,
    RefValueType::ThreeQuarters, RefValueType::One,
};

GTEST_ALLOW_UNINSTANTIATED_PARAMETERIZED_TEST(ShadowSamplerFunctionTest);
ANGLE_INSTANTIATE_TEST_COMBINE_4(ShadowSamplerFunctionTest,
                                 ShadowSamplerFunctionVariationsTestPrint,
                                 testing::ValuesIn(kFunctionTypes),
                                 testing::ValuesIn(kCompareFuncs),
                                 testing::ValuesIn(kRefValueTypes),
                                 testing::Bool(),
                                 ANGLE_ALL_TEST_PLATFORMS_ES3);

}  // anonymous namespace
