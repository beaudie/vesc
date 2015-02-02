//
// Copyright (c) 2015 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//
// Extension_test.cpp
//   Test that shaders need various extensions to be compiled.
//

#include "angle_gl.h"
#include "gtest/gtest.h"
#include "GLSLANG/ShaderLang.h"

class ExtensionTest : public testing::Test
{
  public:
    ExtensionTest() {}

  protected:
    virtual void SetUp()
    {
        ShInitBuiltInResources(&mResources);
        mCompiler = NULL;
    }

    virtual void TearDown()
    {
        DestroyCompiler();
    }

    void DestroyCompiler()
    {
        if (mCompiler)
        {
            ShDestruct(mCompiler);
            mCompiler = NULL;
        }
    }

    void InitializeCompiler()
    {
        DestroyCompiler();
        mCompiler = ShConstructCompiler(GL_FRAGMENT_SHADER, SH_WEBGL_SPEC, SH_GLSL_OUTPUT, &mResources);
        ASSERT_TRUE(mCompiler) << "Compiler could not be constructed.";
    }

    void TestShaderExtension(const char** shaderStrings, int stringCount, bool expectation)
    {
        bool success = ShCompile(mCompiler, shaderStrings, stringCount, 0);
        const std::string& compileLog = ShGetInfoLog(mCompiler);
        EXPECT_EQ(success, expectation) << compileLog;
    }

  protected:
    ShBuiltInResources mResources;
    ShHandle mCompiler;
};

static const char* fragDepthExtShdr[] = {
  "#extension GL_EXT_frag_depth : enable\n",
  "void main() { gl_FragDepthEXT = 1.0; }\n"
};

TEST_F(ExtensionTest, FragDepthExtRequiresExt)
{
    // Extension is required to compile properly.
    mResources.EXT_frag_depth = 0;
    InitializeCompiler();
    TestShaderExtension(fragDepthExtShdr, 2, false);
}

TEST_F(ExtensionTest, FragDepthExtRequiresPragma)
{
    // Pragma is required to compile properly.
    mResources.EXT_frag_depth = 1;
    InitializeCompiler();
    TestShaderExtension(&fragDepthExtShdr[1], 1, false);
}

TEST_F(ExtensionTest, FragDepthExtCompiles)
{
    // Test that it compiles properly with extension enabled.
    mResources.EXT_frag_depth = 1;
    InitializeCompiler();
    TestShaderExtension(fragDepthExtShdr, 2, true);
}

TEST_F(ExtensionTest, FragDepthExtResetsInternalStates)
{
    // Test that extension internal states are reset properly between compiles.
    mResources.EXT_frag_depth = 1;
    InitializeCompiler();

    TestShaderExtension(fragDepthExtShdr, 2, true);
    TestShaderExtension(&fragDepthExtShdr[1], 1, false);
    TestShaderExtension(fragDepthExtShdr, 2, true);
};


static const char* standDerivExtShdr[] = {
  "#extension GL_OES_standard_derivatives : enable\n",

  "precision mediump float;\n"
  "varying vec2 texCoord;\n"
  "void main() { gl_FragColor = vec4(dFdx(texCoord.x), dFdy(texCoord.y), fwidth(texCoord.x), 1.0); }\n"
};

TEST_F(ExtensionTest, StandDerivExtRequiresExt)
{
    // Extension is required to compile properly.
    mResources.OES_standard_derivatives = 0;
    InitializeCompiler();
    TestShaderExtension(standDerivExtShdr, 2, false);
}

TEST_F(ExtensionTest, StandDerivExtRequiresPragma)
{
    // Pragma is required to compile properly.
    mResources.OES_standard_derivatives = 1;
    InitializeCompiler();
    TestShaderExtension(&standDerivExtShdr[1], 1, false);
}

TEST_F(ExtensionTest, StandDerivExtCompiles)
{
    // Test that it compiles properly with extension enabled.
    mResources.OES_standard_derivatives = 1;
    InitializeCompiler();
    TestShaderExtension(standDerivExtShdr, 2, true);
}

TEST_F(ExtensionTest, StandDerivExtResetsInternalStates)
{
    // Test that extension internal states are reset properly between compiles.
    mResources.OES_standard_derivatives = 1;
    InitializeCompiler();

    TestShaderExtension(standDerivExtShdr, 2, true);
    TestShaderExtension(&standDerivExtShdr[1], 1, false);
    TestShaderExtension(standDerivExtShdr, 2, true);
    TestShaderExtension(&standDerivExtShdr[1], 1, false);
};

static const char* textureLODShdr[] = {
  "#extension GL_EXT_shader_texture_lod : enable\n",

  "precision mediump float;\n"
  "varying vec2 texCoord0v;\n"
  "uniform float lod;\n"
  "uniform sampler2D tex;\n"
  "void main() { vec4 color = texture2DLodEXT(tex, texCoord0v, lod); }\n"
};

TEST_F(ExtensionTest, TextureLODExtRequiresExt)
{
    // Extension is required to compile properly.
    mResources.EXT_shader_texture_lod = 0;
    InitializeCompiler();
    TestShaderExtension(textureLODShdr, 2, false);
}

TEST_F(ExtensionTest, TextureLODExtRequiresPragma)
{
    // Pragma is required to compile properly.
    mResources.EXT_shader_texture_lod = 1;
    InitializeCompiler();
    TestShaderExtension(&textureLODShdr[1], 1, false);
}

TEST_F(ExtensionTest, TextureLODExtCompiles)
{
    // Test that it compiles properly with extension enabled.
    mResources.EXT_shader_texture_lod = 1;
    InitializeCompiler();
    TestShaderExtension(textureLODShdr, 2, true);
}

TEST_F(ExtensionTest, TextureLODExtResetsInternalStates)
{
    // Test that extension internal states are reset properly between compiles.
    mResources.EXT_shader_texture_lod = 1;
    InitializeCompiler();

    TestShaderExtension(&textureLODShdr[1], 1, false);
    TestShaderExtension(textureLODShdr, 2, true);
    TestShaderExtension(&textureLODShdr[1], 1, false);
    TestShaderExtension(textureLODShdr, 2, true);
};
