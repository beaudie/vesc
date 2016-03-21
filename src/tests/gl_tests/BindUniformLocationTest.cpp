//
// Copyright 2015 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//

#include "test_utils/ANGLETest.h"

#include <cmath>

using namespace angle;

namespace
{

class BindUniformLocationTest : public ANGLETest
{
  protected:
    BindUniformLocationTest()
    {
        setWindowWidth(128);
        setWindowHeight(128);
        setConfigRedBits(8);
        setConfigGreenBits(8);
        setConfigBlueBits(8);
        setConfigAlphaBits(8);
    }

    void SetUp() override
    {
        ANGLETest::SetUp();
        mBindUniformLocation = reinterpret_cast<PFNGLBINDUNIFORMLOCATIONCHROMIUMPROC>(
            eglGetProcAddress("glBindUniformLocationCHROMIUM"));
    }

    void TearDown() override { ANGLETest::TearDown(); }

    typedef void(GL_APIENTRYP PFNGLBINDUNIFORMLOCATIONCHROMIUMPROC)(GLuint program,
                                                                    GLint location,
                                                                    const GLchar *name);
    PFNGLBINDUNIFORMLOCATIONCHROMIUMPROC mBindUniformLocation;
};

TEST_P(BindUniformLocationTest, Basic)
{
    if (!extensionEnabled("GL_CHROMIUM_bind_uniform_location"))
    {
        std::cout << "Test skipped because GL_CHROMIUM_bind_uniform_location is not available."
                  << std::endl;
        return;
    }

    ASSERT_NE(mBindUniformLocation, nullptr);

    // clang-format off
    const std::string vsSource = SHADER_SOURCE
    (
        attribute vec4 a_position;
        void main()
        {
            gl_Position = a_position;
        }
    );

    const std::string fsSource = SHADER_SOURCE
    (
        precision mediump float;
        uniform vec4 u_colorC;
        uniform vec4 u_colorB[2];
        uniform vec4 u_colorA;
        void main()
        {
            gl_FragColor = u_colorA + u_colorB[0] + u_colorB[1] + u_colorC;
        }
    );
    // clang-format on

    GLint colorALocation = 3;
    GLint colorBLocation = 10;
    GLint colorCLocation = 5;

    GLuint vs = CompileShader(GL_VERTEX_SHADER, vsSource);
    GLuint fs = CompileShader(GL_FRAGMENT_SHADER, fsSource);

    GLuint program = glCreateProgram();

    mBindUniformLocation(program, colorALocation, "u_colorA");
    mBindUniformLocation(program, colorBLocation, "u_colorB[0]");
    mBindUniformLocation(program, colorCLocation, "u_colorC");

    glAttachShader(program, vs);
    glAttachShader(program, fs);
    // Link the program
    glLinkProgram(program);
    // Check the link status
    GLint linked = 0;
    glGetProgramiv(program, GL_LINK_STATUS, &linked);
    EXPECT_EQ(1, linked);

    glUseProgram(program);

    static const float colorB[] = {
        0.0f, 0.50f, 0.0f, 0.0f, 0.0f, 0.0f, 0.75f, 0.0f,
    };

    glUniform4f(colorALocation, 0.25f, 0.0f, 0.0f, 0.0f);
    glUniform4fv(colorBLocation, 2, colorB);
    glUniform4f(colorCLocation, 0.0f, 0.0f, 0.0f, 1.0f);

    drawQuad(program, "a_position", 0.5f);

    EXPECT_PIXEL_NEAR(0, 0, 64, 128, 192, 255, 1.0);
}

TEST_P(BindUniformLocationTest, ConflictsDetection)
{
    if (!extensionEnabled("GL_CHROMIUM_bind_uniform_location"))
    {
        std::cout << "Test skipped because GL_CHROMIUM_bind_uniform_location is not available."
                  << std::endl;
        return;
    }

    ASSERT_NE(mBindUniformLocation, nullptr);

    // clang-format off
    const std::string vsSource = SHADER_SOURCE
    (
        attribute vec4 a_position;
        void main()
        {
            gl_Position = a_position;
        }
    );

    const std::string fsSource = SHADER_SOURCE
    (
        precision mediump float;
        uniform vec4 u_colorA;
        uniform vec4 u_colorB;
        void main()
        {
            gl_FragColor = u_colorA + u_colorB;
        }
    );
    // clang-format on

    GLint colorALocation = 3;
    GLint colorBLocation = 4;

    GLuint vs = CompileShader(GL_VERTEX_SHADER, vsSource);
    GLuint fs = CompileShader(GL_FRAGMENT_SHADER, fsSource);

    GLuint program = glCreateProgram();
    glAttachShader(program, vs);
    glAttachShader(program, fs);

    mBindUniformLocation(program, colorALocation, "u_colorA");
    // Bind u_colorB to location a, causing conflicts, link should fail.
    mBindUniformLocation(program, colorALocation, "u_colorB");
    glLinkProgram(program);
    GLint linked = 0;
    glGetProgramiv(program, GL_LINK_STATUS, &linked);
    EXPECT_EQ(0, linked);

    // Bind u_colorB to location b, no conflicts, link should succeed.
    mBindUniformLocation(program, colorBLocation, "u_colorB");
    glLinkProgram(program);
    linked = 0;
    glGetProgramiv(program, GL_LINK_STATUS, &linked);
    EXPECT_EQ(1, linked);
}

TEST_P(BindUniformLocationTest, Compositor)
{
    if (!extensionEnabled("GL_CHROMIUM_bind_uniform_location"))
    {
        std::cout << "Test skipped because GL_CHROMIUM_bind_uniform_location is not available."
                  << std::endl;
        return;
    }

    ASSERT_NE(mBindUniformLocation, nullptr);

    // clang-format off
    const std::string vsSource = SHADER_SOURCE
    (
        attribute vec4 a_position;
        attribute vec2 a_texCoord;
        uniform mat4 matrix;
        uniform vec2 color_a[4];
        uniform vec4 color_b;
        varying vec4 v_color;
        void main()
        {
            v_color.xy = color_a[0] + color_a[1];
            v_color.zw = color_a[2] + color_a[3];
            v_color += color_b;
            gl_Position = matrix * a_position;
        }
    );

    const std::string fsSource = SHADER_SOURCE
    (
        precision mediump float;
        varying vec4 v_color;
        uniform float alpha;
        uniform vec4 multiplier;
        uniform vec3 color_c[8];
        void main()
        {
            vec4 color_c_sum = vec4(0.0);
            color_c_sum.xyz += color_c[0];
            color_c_sum.xyz += color_c[1];
            color_c_sum.xyz += color_c[2];
            color_c_sum.xyz += color_c[3];
            color_c_sum.xyz += color_c[4];
            color_c_sum.xyz += color_c[5];
            color_c_sum.xyz += color_c[6];
            color_c_sum.xyz += color_c[7];
            color_c_sum.w = alpha;
            color_c_sum *= multiplier;
            gl_FragColor = v_color + color_c_sum;
        }
    );
    // clang-format on

    int counter            = 6;
    int matrixLocation     = counter++;
    int colorALocation     = counter++;
    int colorBLocation     = counter++;
    int alphaLocation      = counter++;
    int multiplierLocation = counter++;
    int colorCLocation     = counter++;

    GLuint vs = CompileShader(GL_VERTEX_SHADER, vsSource);
    GLuint fs = CompileShader(GL_FRAGMENT_SHADER, fsSource);

    GLuint program = glCreateProgram();

    mBindUniformLocation(program, matrixLocation, "matrix");
    mBindUniformLocation(program, colorALocation, "color_a");
    mBindUniformLocation(program, colorBLocation, "color_b");
    mBindUniformLocation(program, alphaLocation, "alpha");
    mBindUniformLocation(program, multiplierLocation, "multiplier");
    mBindUniformLocation(program, colorCLocation, "color_c");

    glAttachShader(program, vs);
    glAttachShader(program, fs);
    // Link the program
    glLinkProgram(program);
    // Check the link status
    GLint linked = 0;
    glGetProgramiv(program, GL_LINK_STATUS, &linked);
    EXPECT_EQ(1, linked);

    glUseProgram(program);

    static const float colorA[] = {
        0.1f, 0.1f, 0.1f, 0.1f, 0.1f, 0.1f, 0.1f, 0.1f,
    };

    static const float colorC[] = {
        0.1f, 0.1f, 0.1f, 0.1f, 0.1f, 0.1f, 0.1f, 0.1f, 0.1f, 0.1f, 0.1f, 0.1f,
        0.1f, 0.1f, 0.1f, 0.1f, 0.1f, 0.1f, 0.1f, 0.1f, 0.1f, 0.1f, 0.1f, 0.1f,
    };

    static const float identity[] = {
        1, 0, 0, 0, 0, 1, 0, 0, 0, 0, 1, 0, 0, 0, 0, 1,
    };

    glUniformMatrix4fv(matrixLocation, 1, false, identity);
    glUniform2fv(colorALocation, 4, colorA);
    glUniform4f(colorBLocation, 0.2f, 0.2f, 0.2f, 0.2f);
    glUniform1f(alphaLocation, 0.8f);
    glUniform4f(multiplierLocation, 0.5f, 0.5f, 0.5f, 0.5f);
    glUniform3fv(colorCLocation, 8, colorC);

    glDrawArrays(GL_TRIANGLES, 0, 6);

    drawQuad(program, "a_position", 0.5f);

    EXPECT_PIXEL_EQ(0, 0, 204, 204, 204, 204);
}

TEST_P(BindUniformLocationTest, UnusedUniformUpdate)
{
    if (!extensionEnabled("GL_CHROMIUM_bind_uniform_location"))
    {
        std::cout << "Test skipped because GL_CHROMIUM_bind_uniform_location is not available."
                  << std::endl;
        return;
    }

    ASSERT_NE(mBindUniformLocation, nullptr);

    // clang-format off
    const std::string vsSource = SHADER_SOURCE
    (
        attribute vec4 a_position;
        void main()
        {
            gl_Position = a_position;
        }
    );

    const std::string fsSource = SHADER_SOURCE
    (
        precision mediump float;
        uniform vec4 u_colorA;
        uniform float u_colorU;
        uniform vec4 u_colorC;
        void main()
        {
            gl_FragColor = u_colorA + u_colorC;
        }
    );
    // clang-format on

    const GLint colorULocation      = 1;
    const GLint nonexistingLocation = 5;
    const GLint unboundLocation     = 6;

    GLuint vs = CompileShader(GL_VERTEX_SHADER, vsSource);
    GLuint fs = CompileShader(GL_FRAGMENT_SHADER, fsSource);

    GLuint program = glCreateProgram();
    mBindUniformLocation(program, colorULocation, "u_colorU");
    // The non-existing uniform should behave like existing, but optimized away
    // uniform.
    mBindUniformLocation(program, nonexistingLocation, "nonexisting");
    // Let A and C be assigned automatic locations.
    glAttachShader(program, vs);
    glAttachShader(program, fs);
    glLinkProgram(program);
    GLint linked = 0;
    glGetProgramiv(program, GL_LINK_STATUS, &linked);
    EXPECT_EQ(1, linked);
    glUseProgram(program);

    // No errors on bound locations, since caller does not know
    // if the driver optimizes them away or not.
    glUniform1f(colorULocation, 0.25f);
    EXPECT_GL_NO_ERROR();

    // No errors on bound locations of names that do not exist
    // in the shader. Otherwise it would be inconsistent wrt the
    // optimization case.
    glUniform1f(nonexistingLocation, 0.25f);
    EXPECT_GL_NO_ERROR();

    // The above are equal to updating -1.
    glUniform1f(-1, 0.25f);
    EXPECT_GL_NO_ERROR();

    // No errors when updating with other type either.
    // The type can not be known with the non-existing case.
    glUniform2f(colorULocation, 0.25f, 0.25f);
    EXPECT_GL_NO_ERROR();
    glUniform2f(nonexistingLocation, 0.25f, 0.25f);
    EXPECT_GL_NO_ERROR();
    glUniform2f(-1, 0.25f, 0.25f);
    EXPECT_GL_NO_ERROR();

    // Ensure that driver or ANGLE has optimized the variable
    // away and the test tests what it is supposed to.
    EXPECT_EQ(-1, glGetUniformLocation(program, "u_colorU"));

    // The bound location gets marked as used and the driver
    // does not allocate other variables to that location.
    EXPECT_NE(colorULocation, glGetUniformLocation(program, "u_colorA"));
    EXPECT_NE(colorULocation, glGetUniformLocation(program, "u_colorC"));
    EXPECT_NE(nonexistingLocation, glGetUniformLocation(program, "u_colorA"));
    EXPECT_NE(nonexistingLocation, glGetUniformLocation(program, "u_colorC"));

    // Unintuitive: while specifying value works, getting the value does not.
    GLfloat getResult = 0.0f;
    glGetUniformfv(program, colorULocation, &getResult);
    EXPECT_GL_ERROR(GL_INVALID_OPERATION);
    glGetUniformfv(program, nonexistingLocation, &getResult);
    EXPECT_GL_ERROR(GL_INVALID_OPERATION);
    glGetUniformfv(program, -1, &getResult);
    EXPECT_GL_ERROR(GL_INVALID_OPERATION);

    // Updating an unbound, non-existing location still causes
    // an error.
    glUniform1f(unboundLocation, 0.25f);
    EXPECT_GL_ERROR(GL_INVALID_OPERATION);
}

// Test for a bug where using a sampler caused GL error if the program had
// uniforms that were optimized away by the driver. This was only a problem with
// glBindUniformLocationCHROMIUM implementation. This could be reproed by
// binding the sampler to a location higher than the amount of active uniforms.
TEST_P(BindUniformLocationTest, UseSamplerWhenUnusedUniforms)
{
    if (!extensionEnabled("GL_CHROMIUM_bind_uniform_location"))
    {
        std::cout << "Test skipped because GL_CHROMIUM_bind_uniform_location is not available."
                  << std::endl;
        return;
    }

    ASSERT_NE(mBindUniformLocation, nullptr);

    // clang-format off
    const std::string vsSource = SHADER_SOURCE
    (
        void main()
        {
            gl_Position = vec4(0);
        }
    );

    const std::string fsSource = SHADER_SOURCE
    (
        uniform sampler2D tex;
        void main()
        {
            gl_FragColor = texture2D(tex, vec2(1));
        }
    );
    // clang-format on

    const GLuint texLocation = 54;

    GLuint vs = CompileShader(GL_VERTEX_SHADER, vsSource);
    GLuint fs = CompileShader(GL_FRAGMENT_SHADER, fsSource);

    GLuint program = glCreateProgram();
    mBindUniformLocation(program, texLocation, "tex");

    glAttachShader(program, vs);
    glAttachShader(program, fs);

    glLinkProgram(program);

    GLint linked = 0;
    glGetProgramiv(program, GL_LINK_STATUS, &linked);
    EXPECT_NE(0, linked);
    glUseProgram(program);
    glUniform1i(texLocation, 0);
    EXPECT_GL_NO_ERROR();
}

// Use this to select which configurations (e.g. which renderer, which GLES major version) these
// tests should be run against.
ANGLE_INSTANTIATE_TEST(BindUniformLocationTest,
                       ES2_D3D9(),
                       ES2_D3D11(),
                       ES2_D3D11_FL9_3(),
                       ES2_OPENGL(),
                       ES2_OPENGLES());

}  // namespace
