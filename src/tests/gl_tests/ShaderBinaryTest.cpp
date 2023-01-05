//
// Copyright 2022 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//

#include "test_utils/ANGLETest.h"

#include <vector>

#include "GLSLANG/ShaderLang.h"
#include "test_utils/gl_raii.h"

using namespace angle;

class ShaderBinaryTest : public ANGLETest<>
{
  protected:
    ShaderBinaryTest()
    {
        setWindowWidth(128);
        setWindowHeight(128);
        setConfigRedBits(8);
        setConfigGreenBits(8);
        setConfigBlueBits(8);
        setConfigAlphaBits(8);

        // Test flakiness was noticed when reusing displays.
        forceNewDisplay();
    }

    void testSetUp() override
    {
        ASSERT_EQ(sh::Initialize(), true);

        mCompileOptions.objectCode                    = true;
        mCompileOptions.variables                     = true;
        mCompileOptions.emulateGLDrawID               = true;
        mCompileOptions.initializeUninitializedLocals = true;

        sh::InitBuiltInResources(&mResources);

        // Generate a shader binary:
        ShShaderSpec spec     = SH_GLES2_SPEC;
        ShShaderOutput output = SH_SPIRV_VULKAN_OUTPUT;

        // Vertex shader:
        const char *source = essl1_shaders::vs::Simple();
        ShHandle vertexCompiler =
            sh::ConstructCompiler(GL_VERTEX_SHADER, spec, output, &mResources);
        sh::Compile(vertexCompiler, &source, 1, mCompileOptions);
        mVertexShaderBinary = sh::GetShaderBinary(vertexCompiler);

        if (mVertexShaderBinary.size() == 0)
        {
            FAIL() << "Creating vertex shader binary failed.";
        }

        // Fragment shader:
        source = essl1_shaders::fs::Red();
        ShHandle fragmentCompiler =
            sh::ConstructCompiler(GL_FRAGMENT_SHADER, spec, output, &mResources);
        sh::Compile(fragmentCompiler, &source, 1, mCompileOptions);
        mFragmentShaderBinary = sh::GetShaderBinary(fragmentCompiler);

        if (mFragmentShaderBinary.size() == 0)
        {
            FAIL() << "Creating fragment shader binary failed.";
        }
    }

    void testTearDown() override
    {
        sh::Finalize();
        glDeleteBuffers(1, &mBuffer);
    }

    bool supported() const
    {
        GLint formatCount;
        glGetIntegerv(GL_NUM_SHADER_BINARY_FORMATS, &formatCount);
        if (formatCount == 0)
        {
            std::cout << "Test skipped because no program binary formats are available."
                      << std::endl;
            return false;
        }
        std::vector<GLint> formats(formatCount);
        glGetIntegerv(GL_SHADER_BINARY_FORMATS, formats.data());

        ASSERT(formats[0] == GL_SHADER_BINARY_ANGLE);

        return true;
    }

    ShCompileOptions mCompileOptions = {};
    ShBuiltInResources mResources;
    GLuint mBuffer;
    sh::ShaderBinaryBlob mVertexShaderBinary;
    sh::ShaderBinaryBlob mFragmentShaderBinary;
};

// This tests the ability to successfully create and load a shader binary.
TEST_P(ShaderBinaryTest, CreateAndLoadBinary)
{
    if (!supported())
    {
        return;
    }

    GLint compileResult;
    // Create vertex shader and load binary
    GLuint vertShader = glCreateShader(GL_VERTEX_SHADER);
    glShaderBinary(1, &vertShader, GL_SHADER_BINARY_ANGLE, mVertexShaderBinary.data(),
                   mVertexShaderBinary.size());
    glGetShaderiv(vertShader, GL_COMPILE_STATUS, &compileResult);
    ASSERT_GL_TRUE(compileResult);

    // Create fragment shader and load binary
    GLuint fragShader = glCreateShader(GL_FRAGMENT_SHADER);
    glShaderBinary(1, &fragShader, GL_SHADER_BINARY_ANGLE, mFragmentShaderBinary.data(),
                   mFragmentShaderBinary.size());
    glGetShaderiv(fragShader, GL_COMPILE_STATUS, &compileResult);
    ASSERT_GL_TRUE(compileResult);

    // Create program from the shaders
    GLuint newProgram = glCreateProgram();
    glAttachShader(newProgram, vertShader);
    glAttachShader(newProgram, fragShader);
    glLinkProgram(newProgram);
    newProgram = CheckLinkStatusAndReturnProgram(newProgram, true);

    // Test with a basic draw
    drawQuad(newProgram, "a_position", 0.5f);
    ASSERT_GL_NO_ERROR();
    EXPECT_PIXEL_COLOR_EQ(0, 0, GLColor::red);
}

// Check invalid gl call parameters, such as providing a GL type when a shader handle is expected.
TEST_P(ShaderBinaryTest, InvalidCallParams)
{
    if (!supported())
    {
        return;
    }

    GLuint vertShader[2];
    vertShader[0]     = glCreateShader(GL_VERTEX_SHADER);
    GLuint fragShader = glCreateShader(GL_FRAGMENT_SHADER);

    // Invalid shader
    vertShader[1] = -1;
    glShaderBinary(1, &vertShader[1], GL_SHADER_BINARY_ANGLE, mVertexShaderBinary.data(),
                   mVertexShaderBinary.size());
    EXPECT_GL_ERROR(GL_INVALID_VALUE);

    // GL_INVALID_ENUM is generated if binaryFormat is not an accepted value.
    glShaderBinary(1, &vertShader[0], GL_INVALID_ENUM, mVertexShaderBinary.data(),
                   mVertexShaderBinary.size());
    EXPECT_GL_ERROR(GL_INVALID_ENUM);

    // GL_INVALID_VALUE is generated if n or length is negative
    glShaderBinary(-1, &vertShader[0], GL_SHADER_BINARY_ANGLE, mVertexShaderBinary.data(),
                   mVertexShaderBinary.size());
    EXPECT_GL_ERROR(GL_INVALID_VALUE);
    glShaderBinary(1, &vertShader[0], GL_SHADER_BINARY_ANGLE, mVertexShaderBinary.data(), -1);
    EXPECT_GL_ERROR(GL_INVALID_VALUE);

    // GL_INVALID_OPERATION is generated if any value in shaders is not a shader object.
    GLuint program = glCreateProgram();
    glShaderBinary(1, &program, GL_SHADER_BINARY_ANGLE, mVertexShaderBinary.data(),
                   mVertexShaderBinary.size());
    EXPECT_GL_ERROR(GL_INVALID_OPERATION);

    // GL_INVALID_OPERATION is generated if more than one of the handles in shaders refers to the
    // same shader object.
    vertShader[1] = vertShader[0];
    glShaderBinary(2, &vertShader[0], GL_SHADER_BINARY_ANGLE, mVertexShaderBinary.data(),
                   mVertexShaderBinary.size());
    EXPECT_GL_ERROR(GL_INVALID_OPERATION);

    // GL_INVALID_VALUE is generated if the data pointed to by binary does not match the format
    // specified by binaryFormat.
    std::string invalid("Invalid Shader Blob.");
    glShaderBinary(1, &vertShader[0], GL_SHADER_BINARY_ANGLE, invalid.data(), invalid.size());
    EXPECT_GL_ERROR(GL_INVALID_VALUE);

    // Try loading vertex shader binary into fragment shader
    glShaderBinary(1, &fragShader, GL_SHADER_BINARY_ANGLE, mVertexShaderBinary.data(),
                   mVertexShaderBinary.size());
    EXPECT_GL_ERROR(GL_INVALID_OPERATION);
}

// Check attempting to get source code from a shader that was loaded with glShaderBinary.
TEST_P(ShaderBinaryTest, GetSourceFromBinaryShader)
{
    if (!supported())
    {
        return;
    }

    GLint compileResult;
    // Create vertex shader and load binary
    GLuint vertShader = glCreateShader(GL_VERTEX_SHADER);
    glShaderBinary(1, &vertShader, GL_SHADER_BINARY_ANGLE, mVertexShaderBinary.data(),
                   mVertexShaderBinary.size());
    glGetShaderiv(vertShader, GL_COMPILE_STATUS, &compileResult);
    ASSERT_GL_TRUE(compileResult);

    GLsizei length = 0;
    glGetShaderSource(vertShader, 0, &length, nullptr);

    EXPECT_EQ(length, 0);
}

// Create a program from both shader source code and a binary blob.
TEST_P(ShaderBinaryTest, CombineSourceAndBinaryShaders)
{
    if (!supported())
    {
        return;
    }

    GLint compileResult;
    // Create vertex shader and load binary
    GLuint vertShader = glCreateShader(GL_VERTEX_SHADER);
    glShaderBinary(1, &vertShader, GL_SHADER_BINARY_ANGLE, mVertexShaderBinary.data(),
                   mVertexShaderBinary.size());
    glGetShaderiv(vertShader, GL_COMPILE_STATUS, &compileResult);
    ASSERT_GL_TRUE(compileResult);

    // Create fragment shader
    GLuint fragShader = CompileShader(GL_FRAGMENT_SHADER, essl1_shaders::fs::Red());

    GLuint newProgram = glCreateProgram();
    glAttachShader(newProgram, vertShader);
    glAttachShader(newProgram, fragShader);
    glLinkProgram(newProgram);
    newProgram = CheckLinkStatusAndReturnProgram(newProgram, true);

    // Test with a basic draw
    drawQuad(newProgram, "a_position", 0.5f);
    ASSERT_GL_NO_ERROR();
    EXPECT_PIXEL_COLOR_EQ(0, 0, GLColor::red);
}

// Use this to select which configurations (e.g. which renderer, which GLES major version) these
// tests should be run against.
ANGLE_INSTANTIATE_TEST_ES2_AND_ES3_AND_ES31(ShaderBinaryTest);
