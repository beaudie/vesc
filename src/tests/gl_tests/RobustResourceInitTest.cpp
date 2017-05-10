//
// Copyright 2017 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//
// RobustResourceInitTest: Tests for GL_ANGLE_robust_resource_initialization.

#include "test_utils/ANGLETest.h"

#include "test_utils/gl_raii.h"

namespace angle
{

class RobustResourceInitTest : public ANGLETest
{
  protected:
    constexpr static int kWidth  = 128;
    constexpr static int kHeight = 128;

    RobustResourceInitTest()
    {
        setWindowWidth(kWidth);
        setWindowHeight(kHeight);
        setConfigRedBits(8);
        setConfigGreenBits(8);
        setConfigBlueBits(8);
        setConfigAlphaBits(8);

        // Defer context init until the test body.
        setDeferContextInit(true);
        setRobustResourceInit(true);
    }

    bool hasEGLExtension()
    {
        EGLDisplay display = getEGLWindow()->getDisplay();
        ASSERT(display != EGL_NO_DISPLAY);

        return (eglDisplayExtensionEnabled(
            display, "EGL_ANGLE_create_context_robust_resource_initialization"));
    }

    bool setup()
    {
        if (!hasEGLExtension())
        {
            return false;
        }

        if (!getEGLWindow()->initializeContext())
        {
            EXPECT_TRUE(false);
            return false;
        }

        return true;
    }

    void setupTexture(GLTexture *tex);
    void checkNonZeroPixels(GLTexture *texture,
                            int skipX,
                            int skipY,
                            int skipWidth,
                            int skipHeight,
                            int skipR,
                            int skipG,
                            int skipB,
                            int skipA);
};

// Context creation should fail if EGL_ANGLE_create_context_robust_resource_initialization
// is not available, and succeed otherwise.
TEST_P(RobustResourceInitTest, ExtensionInit)
{
    if (hasEGLExtension())
    {
        // Context creation shold succeed with robust resource init enabled.
        EXPECT_TRUE(getEGLWindow()->initializeContext());

        // Robust resource init extension should be available.
        EXPECT_TRUE(extensionEnabled("GL_ANGLE_robust_resource_initialization"));

        // Querying the state value should return true.
        GLboolean enabled = 0;
        glGetBooleanv(GL_CONTEXT_ROBUST_RESOURCE_INITIALIZATION_ANGLE, &enabled);
        EXPECT_GL_NO_ERROR();
        EXPECT_GL_TRUE(enabled);

        EXPECT_GL_TRUE(glIsEnabled(GL_CONTEXT_ROBUST_RESOURCE_INITIALIZATION_ANGLE));
    }
    else
    {
        // Context creation should fail with robust resource init enabled.
        EXPECT_FALSE(getEGLWindow()->initializeContext());

        // Context creation should succeed with robust resource init disabled.
        setRobustResourceInit(false);
        ASSERT_TRUE(getEGLWindow()->initializeGL(GetOSWindow()));

        // If context extension string exposed, check queries.
        if (extensionEnabled("GL_ANGLE_robust_resource_initialization"))
        {
            GLboolean enabled = 0;
            glGetBooleanv(GL_CONTEXT_ROBUST_RESOURCE_INITIALIZATION_ANGLE, &enabled);
            EXPECT_GL_FALSE(enabled);

            EXPECT_GL_FALSE(glIsEnabled(GL_CONTEXT_ROBUST_RESOURCE_INITIALIZATION_ANGLE));
            EXPECT_GL_NO_ERROR();
        }
        else
        {
            // Querying robust resource init should return INVALID_ENUM.
            GLboolean enabled = 0;
            glGetBooleanv(GL_CONTEXT_ROBUST_RESOURCE_INITIALIZATION_ANGLE, &enabled);
            EXPECT_GL_ERROR(GL_INVALID_ENUM);
        }
    }
}

// Test queries on a normal, non-robust enabled context.
TEST_P(RobustResourceInitTest, QueriesOnNonRobustContext)
{
    EGLDisplay display = getEGLWindow()->getDisplay();
    ASSERT_TRUE(display != EGL_NO_DISPLAY);

    if (!hasEGLExtension())
    {
        return;
    }

    setRobustResourceInit(false);
    EXPECT_TRUE(getEGLWindow()->initializeContext());

    // If context extension string exposed, check queries.
    ASSERT_TRUE(extensionEnabled("GL_ANGLE_robust_resource_initialization"));

    // Querying robust resource init should return INVALID_ENUM.
    GLboolean enabled = 0;
    glGetBooleanv(GL_CONTEXT_ROBUST_RESOURCE_INITIALIZATION_ANGLE, &enabled);
    EXPECT_GL_FALSE(enabled);

    EXPECT_GL_FALSE(glIsEnabled(GL_CONTEXT_ROBUST_RESOURCE_INITIALIZATION_ANGLE));
    EXPECT_GL_NO_ERROR();
}

// Tests that buffers start zero-filled if the data pointer is null.
TEST_P(RobustResourceInitTest, BufferData)
{
    if (!setup())
    {
        return;
    }

    GLBuffer buffer;
    glBindBuffer(GL_ARRAY_BUFFER, buffer);
    glBufferData(GL_ARRAY_BUFFER, getWindowWidth() * getWindowHeight() * sizeof(GLfloat), nullptr,
                 GL_STATIC_DRAW);

    const std::string &vertexShader =
        "attribute vec2 position;\n"
        "attribute float testValue;\n"
        "varying vec4 colorOut;\n"
        "void main() {\n"
        "    gl_Position = vec4(position, 0, 1);\n"
        "    colorOut = testValue == 0.0 ? vec4(0, 1, 0, 1) : vec4(1, 0, 0, 1);\n"
        "}";
    const std::string &fragmentShader =
        "varying mediump vec4 colorOut;\n"
        "void main() {\n"
        "    gl_FragColor = colorOut;\n"
        "}";

    ANGLE_GL_PROGRAM(program, vertexShader, fragmentShader);

    GLint testValueLoc = glGetAttribLocation(program.get(), "testValue");
    ASSERT_NE(-1, testValueLoc);

    glBindBuffer(GL_ARRAY_BUFFER, buffer);
    glVertexAttribPointer(testValueLoc, 1, GL_FLOAT, GL_FALSE, 4, nullptr);
    glEnableVertexAttribArray(testValueLoc);
    glBindBuffer(GL_ARRAY_BUFFER, 0);

    drawQuad(program.get(), "position", 0.5f);

    ASSERT_GL_NO_ERROR();

    std::vector<GLColor> expected(getWindowWidth() * getWindowHeight(), GLColor::green);
    std::vector<GLColor> actual(getWindowWidth() * getWindowHeight());
    glReadPixels(0, 0, getWindowWidth(), getWindowHeight(), GL_RGBA, GL_UNSIGNED_BYTE,
                 actual.data());
    EXPECT_EQ(expected, actual);
}

// The following test code translated from WebGL 1 test:
// https://www.khronos.org/registry/webgl/sdk/tests/conformance/misc/uninitialized-test.html
void RobustResourceInitTest::setupTexture(GLTexture *tex)
{
    GLuint tempTexture;
    glGenTextures(1, &tempTexture);
    glBindTexture(GL_TEXTURE_2D, tempTexture);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, kWidth, kHeight, 0, GL_RGBA, GL_UNSIGNED_BYTE, nullptr);

    // this can be quite undeterministic so to improve odds of seeing uninitialized data write bits
    // into tex then delete texture then re-create one with same characteristics (driver will likely
    // reuse mem) with this trick on r59046 WebKit/OSX I get FAIL 100% of the time instead of ~15%
    // of the time.

    std::array<uint8_t, kWidth * kHeight * 4> badData;
    for (int i     = 0; i < badData.size(); ++i)
        badData[i] = i % 255;

    glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, kWidth, kHeight, GL_RGBA, GL_UNSIGNED_BYTE,
                    badData.data());
    glDeleteTextures(1, &tempTexture);

    // This will create the GLTexture.
    glBindTexture(GL_TEXTURE_2D, *tex);
}

void RobustResourceInitTest::checkNonZeroPixels(GLTexture *texture,
                                                int skipX,
                                                int skipY,
                                                int skipWidth,
                                                int skipHeight,
                                                int skipR,
                                                int skipG,
                                                int skipB,
                                                int skipA)
{
    glBindTexture(GL_TEXTURE_2D, 0);
    GLFramebuffer fb;
    glBindFramebuffer(GL_FRAMEBUFFER, fb);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, texture->get(), 0);
    EXPECT_GLENUM_EQ(GL_FRAMEBUFFER_COMPLETE, glCheckFramebufferStatus(GL_FRAMEBUFFER));

    std::array<uint8_t, kWidth * kHeight * 4> data;
    glReadPixels(0, 0, kWidth, kHeight, GL_RGBA, GL_UNSIGNED_BYTE, data.data());

    int k = 0;
    for (int y = 0; y < kHeight; ++y)
    {
        for (int x = 0; x < kWidth; ++x)
        {
            int index = (y * kWidth + x) * 4;
            if (x >= skipX && x < skipX + skipWidth && y >= skipY && y < skipY + skipHeight)
            {
                ASSERT_FALSE(data[index] != skipR || data[index + 1] != skipG ||
                             data[index + 2] != skipB || data[index + 3] != skipA);
            }
            else
            {
                for (int i = 0; i < 4; ++i)
                {
                    if (data[index + i] != 0)
                        k++;
                }
            }
        }
    }

    EXPECT_EQ(0, k);
}

// Reading an uninitialized texture (texImage2D) should succeed with all bytes set to 0.");
TEST_P(RobustResourceInitTest, ReadingUninitializedTexture)
{
    if (!setup())
    {
        return;
    }

    GLTexture tex;
    setupTexture(&tex);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, kWidth, kHeight, 0, GL_RGBA, GL_UNSIGNED_BYTE, nullptr);
    checkNonZeroPixels(&tex, 0, 0, 0, 0, 0, 0, 0, 0);
    EXPECT_GL_NO_ERROR();
}

// Reading a partially initialized texture (texImage2D) should succeed with all uninitialized bytes
// set to 0 and initialized bytes untouched.
TEST_P(RobustResourceInitTest, ReadingPartiallyInitializedTexture)
{
    if (!setup())
    {
        return;
    }

    GLTexture tex;
    setupTexture(&tex);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, kWidth, kHeight, 0, GL_RGBA, GL_UNSIGNED_BYTE, nullptr);
    std::array<uint8_t, 4> data = {{108, 72, 36, 9}};
    glTexSubImage2D(GL_TEXTURE_2D, 0, kWidth / 2, kHeight / 2, 1, 1, GL_RGBA, GL_UNSIGNED_BYTE,
                    data.data());
    checkNonZeroPixels(&tex, kWidth / 2, kHeight / 2, 1, 1, data[0], data[1], data[2], data[3]);
    EXPECT_GL_NO_ERROR();
}

// Reading an uninitialized portion of a texture(copyTexImage2D) should succeed with all bytes set
// to 0.
TEST_P(RobustResourceInitTest, ReadingUninitializedCopiedTexture)
{
    if (!setup())
    {
        return;
    }

    GLTexture tex;
    setupTexture(&tex);
    GLFramebuffer fbo;
    glBindFramebuffer(GL_FRAMEBUFFER, fbo);
    GLRenderbuffer rbo;
    glBindRenderbuffer(GL_RENDERBUFFER, rbo);
    constexpr int fboWidth  = 16;
    constexpr int fboHeight = 16;
    glRenderbufferStorage(GL_RENDERBUFFER, GL_RGBA4, fboWidth, fboHeight);
    glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_RENDERBUFFER, rbo);
    EXPECT_GLENUM_EQ(GL_FRAMEBUFFER_COMPLETE, glCheckFramebufferStatus(GL_FRAMEBUFFER));
    glClearColor(1.0, 0.0, 0.0, 1.0);
    glClear(GL_COLOR_BUFFER_BIT);
    EXPECT_GL_NO_ERROR();
    glCopyTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, 0, 0, kWidth, kHeight, 0);
    checkNonZeroPixels(&tex, 0, 0, fboWidth, fboHeight, 255, 0, 0, 255);
    EXPECT_GL_NO_ERROR();
}

// Reading an uninitialized portion of a texture (copyTexImage2D with negative x and y) should
// succeed with all bytes set to 0.
TEST_P(RobustResourceInitTest, ReadingOutOfboundsCopiedTexture)
{
    if (!setup())
    {
        return;
    }

    GLTexture tex;
    setupTexture(&tex);
    GLFramebuffer fbo;
    glBindFramebuffer(GL_FRAMEBUFFER, fbo);
    GLRenderbuffer rbo;
    glBindRenderbuffer(GL_RENDERBUFFER, rbo);
    constexpr int fboWidth  = 16;
    constexpr int fboHeight = 16;
    glRenderbufferStorage(GL_RENDERBUFFER, GL_RGBA4, fboWidth, fboHeight);
    glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_RENDERBUFFER, rbo);
    EXPECT_GLENUM_EQ(GL_FRAMEBUFFER_COMPLETE, glCheckFramebufferStatus(GL_FRAMEBUFFER));
    glClearColor(1.0, 0.0, 0.0, 1.0);
    glClear(GL_COLOR_BUFFER_BIT);
    EXPECT_GL_NO_ERROR();
    constexpr int x = -8;
    constexpr int y = -8;
    glCopyTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, x, y, kWidth, kHeight, 0);
    checkNonZeroPixels(&tex, -x, -y, fboWidth, fboHeight, 255, 0, 0, 255);
    EXPECT_GL_NO_ERROR();
}

// Reading an uninitialized portion of a texture (copyTexImage2D from internal fbo) should succeed
// with all bytes set to 0.
TEST_P(RobustResourceInitTest, ReadingUninitializedSurface)
{
    if (!setup())
    {
        return;
    }

    GLTexture tex;
    setupTexture(&tex);
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    glClearColor(0.0, 1.0, 0.0, 0.0);
    glClear(GL_COLOR_BUFFER_BIT);
    EXPECT_GL_NO_ERROR();
    glCopyTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, 0, 0, kWidth, kHeight, 0);
    checkNonZeroPixels(&tex, 0, 0, kWidth, kHeight, 0, 255, 0, 0);
    EXPECT_GL_NO_ERROR();
}

ANGLE_INSTANTIATE_TEST(RobustResourceInitTest,
                       ES2_D3D9(),
                       ES2_D3D11(),
                       ES3_D3D11(),
                       ES2_D3D11_FL9_3(),
                       ES2_OPENGL(),
                       ES3_OPENGL(),
                       ES2_OPENGLES(),
                       ES3_OPENGLES());
}  // namespace
