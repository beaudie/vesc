//
// Copyright (c) 2015 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//
// Pack_Unpack_test.cpp:
//   Tests for the emulating pack_unpack functions for GLSL 4.1.
//

#include "angle_gl.h"
#include "gtest/gtest.h"
#include "GLSLANG/ShaderLang.h"
#include "compiler/translator/TranslatorGLSL.h"

class PackUnpackTest : public testing::Test
{
public:
    PackUnpackTest() {}

protected:
    virtual void SetUp()
    {
        ShBuiltInResources resources;
        ShInitBuiltInResources(&resources);

        mTranslator = new TranslatorGLSL(
        GL_FRAGMENT_SHADER, SH_GLES3_SPEC, SH_GLSL_410_CORE_OUTPUT);
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
        mGLSLCode = infoSink.obj.c_str();
        if (!compilationSuccess)
            FAIL() << "Shader compilation into GLSL 4.1 failed " << infoSink.info.c_str();
    }

    bool foundInGLSLCode(const char* stringToFind)
    {
        return mGLSLCode.find(stringToFind) != std::string::npos;
    }

private:
    TranslatorGLSL *mTranslator;
    std::string mGLSLCode;
};

TEST_F(PackUnpackTest, PackSnorm2x16Emulation)
{
    const std::string &shaderString =
        "#version 300 es\n"
        "precision mediump float;\n"
        "layout(location = 0) out mediump vec4 fragColor;"
        "void main() {\n"
        "   vec2 v;\n"
        "   uint u = packSnorm2x16(v);\n"
        "   fragColor = vec4(0.0);\n"
        "}\n";
    compile(shaderString);
    ASSERT_TRUE(foundInGLSLCode("uint webgl_packSnorm2x16_emu(vec2 v)"));
    ASSERT_TRUE(foundInGLSLCode("int x = int(round(clamp(v.x, -1.0, 1.0) * 32767.0));"));
    ASSERT_TRUE(foundInGLSLCode("int y = int(round(clamp(v.y, -1.0, 1.0) * 32767.0));"));
    ASSERT_TRUE(foundInGLSLCode("return uint((y << 16) | (x & 0xffff));"));
}

TEST_F(PackUnpackTest, UnpackSnorm2x16Emulation)
{
    const std::string &shaderString =
        "#version 300 es\n"
        "precision mediump float;\n"
        "layout(location = 0) out mediump vec4 fragColor;"
        "void main() {\n"
        "   uint u;\n"
        "   vec2 v=unpackSnorm2x16(u);\n"
        "   fragColor = vec4(0.0);\n"
        "}\n";
    compile(shaderString);
    ASSERT_TRUE(foundInGLSLCode("float webgl_fromSnorm(uint x)"));
    ASSERT_TRUE(foundInGLSLCode("int xi = (int(x) & 0x7fff) - (int(x) & 0x8000);"));
    ASSERT_TRUE(foundInGLSLCode("return clamp(float(xi) / 32767.0, -1.0, 1.0);"));
    ASSERT_TRUE(foundInGLSLCode("float webgl_fromSnorm(uint x)"));
    ASSERT_TRUE(foundInGLSLCode("vec2 webgl_unpackSnorm2x16_emu(uint u)"));
    ASSERT_TRUE(foundInGLSLCode("uint y = (u >> 16);"));
    ASSERT_TRUE(foundInGLSLCode("uint x = u;"));
    ASSERT_TRUE(foundInGLSLCode("return vec2(webgl_fromSnorm(x), webgl_fromSnorm(y));"));
}

TEST_F(PackUnpackTest, PackHalf2x16Emulation)
{
    const std::string &shaderString =
        "#version 300 es\n"
        "precision mediump float;\n"
        "layout(location = 0) out mediump vec4 fragColor;"
        "void main() {\n"
        "   vec2 v;\n"
        "   uint u=packHalf2x16(v);\n"
        "   fragColor = vec4(0.0);\n"
        "}\n";
    compile(shaderString);
    ASSERT_TRUE(foundInGLSLCode("uint webgl_f32tof16(float val)"));
    ASSERT_TRUE(foundInGLSLCode("uint f32 = floatBitsToInt(val);"));
    ASSERT_TRUE(foundInGLSLCode("uint f16 = 0;"));
    ASSERT_TRUE(foundInGLSLCode("uint sign = (f32 >> 16) & 0x8000u;"));
    ASSERT_TRUE(foundInGLSLCode("int exponent = int((f32 >> 23) & 0xff) - 127;"));
    ASSERT_TRUE(foundInGLSLCode("uint mantissa = f32 & 0x007fffffu;"));
    ASSERT_TRUE(foundInGLSLCode("if (exponent == 128)"));
    ASSERT_TRUE(foundInGLSLCode("f16 = sign | (0x1F << 10);"));
    ASSERT_TRUE(foundInGLSLCode("f16 |= (mantissa & 0x3ff);"));
    ASSERT_TRUE(foundInGLSLCode("else if (exponent > 15)"));
    ASSERT_TRUE(foundInGLSLCode("f16 = sign | (0x1F << 10);"));
    ASSERT_TRUE(foundInGLSLCode("else if (exponent > -15)"));
    ASSERT_TRUE(foundInGLSLCode("exponent += 15;"));
    ASSERT_TRUE(foundInGLSLCode("mantissa >>= 13;"));
    ASSERT_TRUE(foundInGLSLCode("f16 = sign | exponent << 10 | mantissa;"));
    ASSERT_TRUE(foundInGLSLCode("f16 = sign;"));
    ASSERT_TRUE(foundInGLSLCode("return f16;"));
    ASSERT_TRUE(foundInGLSLCode("uint webgl_packHalf2x16_emu(vec2 v)"));
    ASSERT_TRUE(foundInGLSLCode("uint x = webgl_f32tof16(v.x);"));
    ASSERT_TRUE(foundInGLSLCode("uint y = webgl_f32tof16(v.y);"));
    ASSERT_TRUE(foundInGLSLCode("return (y << 16) | x;"));
}

TEST_F(PackUnpackTest, UnpackHalf2x16Emulation)
{
    const std::string &shaderString =
        "#version 300 es\n"
        "precision mediump float;\n"
        "layout(location = 0) out mediump vec4 fragColor;"
        "void main() {\n"
        "   uint u;\n"
        "   vec2 v=unpackHalf2x16(u);\n"
        "   fragColor = vec4(0.0);\n"
        "}\n";
    compile(shaderString);
    ASSERT_TRUE(foundInGLSLCode("float webgl_f16tof32(uint val)"));
    ASSERT_TRUE(foundInGLSLCode("uint sign = (val & 0x8000u) << 16;"));
    ASSERT_TRUE(foundInGLSLCode("int exponent = int((val & 0x7c00) >> 10);"));
    ASSERT_TRUE(foundInGLSLCode("uint mantissa = val & 0x03ffu;"));
    ASSERT_TRUE(foundInGLSLCode("float f32 = 0.0;"));
    ASSERT_TRUE(foundInGLSLCode("if(exponent == 0)"));
    ASSERT_TRUE(foundInGLSLCode("if (mantissa != 0)"));
    ASSERT_TRUE(foundInGLSLCode("const float scale = 1.0 / (1 << 24);"));
    ASSERT_TRUE(foundInGLSLCode("f32 = scale * mantissa;"));
    ASSERT_TRUE(foundInGLSLCode("else if (exponent == 31)"));
    ASSERT_TRUE(foundInGLSLCode("return uintBitsToFloat(sign | 0x7f800000 | mantissa);"));
    ASSERT_TRUE(foundInGLSLCode("float scale, decimal; exponent -= 15;"));
    ASSERT_TRUE(foundInGLSLCode("if(exponent < 0)"));
    ASSERT_TRUE(foundInGLSLCode("scale = 1.0 / (1 << -exponent);"));
    ASSERT_TRUE(foundInGLSLCode("scale = 1 << exponent;"));
    ASSERT_TRUE(foundInGLSLCode("decimal = 1.0 + float(mantissa) / float(1 << 10);"));
    ASSERT_TRUE(foundInGLSLCode("f32 = scale * decimal;"));
    ASSERT_TRUE(foundInGLSLCode("if (sign != 0) f32 = -f32;"));
    ASSERT_TRUE(foundInGLSLCode("return f32;"));
    ASSERT_TRUE(foundInGLSLCode("vec2 webgl_unpackHalf2x16_emu(uint u)"));
    ASSERT_TRUE(foundInGLSLCode("uint y = (u >> 16);"));
    ASSERT_TRUE(foundInGLSLCode("uint x = u & 0xffffu;"));
    ASSERT_TRUE(foundInGLSLCode("return vec2(webgl_f16tof32(x), webgl_f16tof32(y));"));
}