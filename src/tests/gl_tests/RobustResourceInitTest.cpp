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
    }

    bool hasEGLExtension()
    {
        return eglClientExtensionEnabled("EGL_ANGLE_display_robust_resource_initialization");
    }

    bool hasGLExtension() { return extensionEnabled("GL_ANGLE_robust_resource_initialization"); }

    bool setup()
    {
        if (!hasEGLExtension())
        {
            return false;
        }

        TearDown();
        setRobustResourceInit(true);
        SetUp();

        return true;
    }

    void setupTexture(GLTexture *tex);
    void setup3DTexture(GLTexture *tex);

    // Checks for uninitialized (non-zero pixels) in a Texture.
    void checkNonZeroPixels(GLTexture *texture,
                            int skipX,
                            int skipY,
                            int skipWidth,
                            int skipHeight,
                            const GLColor &skip);
    void checkNonZeroPixels3D(GLTexture *texture,
                              int skipX,
                              int skipY,
                              int skipWidth,
                              int skipHeight,
                              const GLColor &skip);
    void checkFramebufferNonZeroPixels(int skipX,
                                       int skipY,
                                       int skipWidth,
                                       int skipHeight,
                                       const GLColor &skip);

    void checkCustomFramebufferNonZeroPixels(int fboWidth,
                                             int fboHeight,
                                             int skipX,
                                             int skipY,
                                             int skipWidth,
                                             int skipHeight,
                                             const GLColor &skip);

    template <typename PixelT>
    void testIntegerTextureInit(const char *samplerType,
                                GLenum internalFormatRGBA,
                                GLenum internalFormatRGB,
                                GLenum type);

    const std::string kSimpleTextureVertexShader =
        "#version 300 es\n"
        "in vec4 position;\n"
        "out vec2 texcoord;\n"
        "void main()\n"
        "{\n"
        "    gl_Position = position;\n"
        "    texcoord = vec2(position.xy * 0.5 - 0.5);\n"
        "}";

    static std::string GetSimpleTextureFragmentShader(const char *samplerType)
    {
        std::stringstream fragmentStream;
        fragmentStream << "#version 300 es\n"
                          "precision mediump "
                       << samplerType
                       << "sampler2D;\n"
                          "precision mediump float;\n"
                          "out "
                       << samplerType
                       << "vec4 color;\n"
                          "in vec2 texcoord;\n"
                          "uniform "
                       << samplerType
                       << "sampler2D tex;\n"
                          "void main()\n"
                          "{\n"
                          "    color = texture(tex, texcoord);\n"
                          "}";
        return fragmentStream.str();
    }
};

// Display creation should fail if EGL_ANGLE_display_robust_resource_initialization
// is not available, and succeed otherwise.
TEST_P(RobustResourceInitTest, ExtensionInit)
{
    if (setup())
    {
        // Robust resource init extension should be available.
        EXPECT_TRUE(hasGLExtension());

        // Querying the state value should return true.
        GLboolean enabled = 0;
        glGetBooleanv(GL_CONTEXT_ROBUST_RESOURCE_INITIALIZATION_ANGLE, &enabled);
        EXPECT_GL_NO_ERROR();
        EXPECT_GL_TRUE(enabled);

        EXPECT_GL_TRUE(glIsEnabled(GL_CONTEXT_ROBUST_RESOURCE_INITIALIZATION_ANGLE));
    }
    else
    {
        // If context extension string exposed, check queries.
        if (hasGLExtension())
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

    // If context extension string exposed, check queries.
    ASSERT_TRUE(hasGLExtension());

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

// Regression test for passing a zero size init buffer with the extension.
TEST_P(RobustResourceInitTest, BufferDataZeroSize)
{
    if (!setup())
    {
        return;
    }

    GLBuffer buffer;
    glBindBuffer(GL_ARRAY_BUFFER, buffer);
    glBufferData(GL_ARRAY_BUFFER, 0, nullptr, GL_STATIC_DRAW);
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
    for (size_t i = 0; i < badData.size(); ++i)
    {
        badData[i] = static_cast<uint8_t>(i % 255);
    }

    glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, kWidth, kHeight, GL_RGBA, GL_UNSIGNED_BYTE,
                    badData.data());
    glDeleteTextures(1, &tempTexture);

    // This will create the GLTexture.
    glBindTexture(GL_TEXTURE_2D, *tex);
}

void RobustResourceInitTest::setup3DTexture(GLTexture *tex)
{
    GLuint tempTexture;
    glGenTextures(1, &tempTexture);
    glBindTexture(GL_TEXTURE_3D, tempTexture);
    glTexImage3D(GL_TEXTURE_3D, 0, GL_RGBA, kWidth, kHeight, 2, 0, GL_RGBA, GL_UNSIGNED_BYTE,
                 nullptr);

    // this can be quite undeterministic so to improve odds of seeing uninitialized data write bits
    // into tex then delete texture then re-create one with same characteristics (driver will likely
    // reuse mem) with this trick on r59046 WebKit/OSX I get FAIL 100% of the time instead of ~15%
    // of the time.

    std::array<uint8_t, kWidth * kHeight * 2 * 4> badData;
    for (size_t i = 0; i < badData.size(); ++i)
    {
        badData[i] = static_cast<uint8_t>(i % 255);
    }

    glTexSubImage3D(GL_TEXTURE_3D, 0, 0, 0, 0, kWidth, kHeight, 2, GL_RGBA, GL_UNSIGNED_BYTE,
                    badData.data());
    glDeleteTextures(1, &tempTexture);

    // This will create the GLTexture.
    glBindTexture(GL_TEXTURE_3D, *tex);
}

void RobustResourceInitTest::checkNonZeroPixels(GLTexture *texture,
                                                int skipX,
                                                int skipY,
                                                int skipWidth,
                                                int skipHeight,
                                                const GLColor &skip)
{
    glBindTexture(GL_TEXTURE_2D, 0);
    GLFramebuffer fb;
    glBindFramebuffer(GL_FRAMEBUFFER, fb);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, texture->get(), 0);
    EXPECT_GLENUM_EQ(GL_FRAMEBUFFER_COMPLETE, glCheckFramebufferStatus(GL_FRAMEBUFFER));

    checkFramebufferNonZeroPixels(skipX, skipY, skipWidth, skipHeight, skip);
}

void RobustResourceInitTest::checkNonZeroPixels3D(GLTexture *texture,
                                                  int skipX,
                                                  int skipY,
                                                  int skipWidth,
                                                  int skipHeight,
                                                  const GLColor &skip)
{
    glBindTexture(GL_TEXTURE_3D, 0);
    GLFramebuffer fb;
    glBindFramebuffer(GL_FRAMEBUFFER, fb);
    glFramebufferTextureLayer(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, texture->get(), 0, 0);
    EXPECT_GLENUM_EQ(GL_FRAMEBUFFER_COMPLETE, glCheckFramebufferStatus(GL_FRAMEBUFFER));

    checkFramebufferNonZeroPixels(skipX, skipY, skipWidth, skipHeight, skip);
}

void RobustResourceInitTest::checkFramebufferNonZeroPixels(int skipX,
                                                           int skipY,
                                                           int skipWidth,
                                                           int skipHeight,
                                                           const GLColor &skip)
{
    checkCustomFramebufferNonZeroPixels(kWidth, kHeight, skipX, skipY, skipWidth, skipHeight, skip);
}

void RobustResourceInitTest::checkCustomFramebufferNonZeroPixels(int fboWidth,
                                                                 int fboHeight,
                                                                 int skipX,
                                                                 int skipY,
                                                                 int skipWidth,
                                                                 int skipHeight,
                                                                 const GLColor &skip)
{
    std::vector<GLColor> data(fboWidth * fboHeight);
    glReadPixels(0, 0, fboWidth, fboHeight, GL_RGBA, GL_UNSIGNED_BYTE, data.data());

    int k = 0;
    for (int y = 0; y < fboHeight; ++y)
    {
        for (int x = 0; x < fboWidth; ++x)
        {
            int index = (y * fboWidth + x);
            if (x >= skipX && x < skipX + skipWidth && y >= skipY && y < skipY + skipHeight)
            {
                ASSERT_EQ(skip, data[index]);
            }
            else
            {
                k += (data[index] != GLColor::transparentBlack) ? 1 : 0;
            }
        }
    }

    EXPECT_EQ(0, k);
}

// Reading an uninitialized texture (texImage2D) should succeed with all bytes set to 0.
TEST_P(RobustResourceInitTest, ReadingUninitializedTexture)
{
    if (!setup())
    {
        return;
    }

    if (IsOpenGL() || IsD3D9())
    {
        std::cout << "Robust resource init is not yet fully implemented. (" << GetParam() << ")"
                  << std::endl;
        return;
    }

    GLTexture tex;
    setupTexture(&tex);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, kWidth, kHeight, 0, GL_RGBA, GL_UNSIGNED_BYTE, nullptr);
    checkNonZeroPixels(&tex, 0, 0, 0, 0, GLColor::transparentBlack);
    EXPECT_GL_NO_ERROR();
}

// Test that calling glTexImage2D multiple times with the same size and no data resets all texture
// data
TEST_P(RobustResourceInitTest, ReuploadingClearsTexture)
{
    if (!setup())
    {
        return;
    }

    if (IsOpenGL() || IsD3D9())
    {
        std::cout << "Robust resource init is not yet fully implemented. (" << GetParam() << ")"
                  << std::endl;
        return;
    }

    // Put some data into the texture
    std::array<GLColor, kWidth * kHeight> data;
    data.fill(GLColor::white);

    GLTexture tex;
    glBindTexture(GL_TEXTURE_2D, tex);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, kWidth, kHeight, 0, GL_RGBA, GL_UNSIGNED_BYTE,
                 data.data());

    // Reset the texture
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, kWidth, kHeight, 0, GL_RGBA, GL_UNSIGNED_BYTE, nullptr);
    checkNonZeroPixels(&tex, 0, 0, 0, 0, GLColor::transparentBlack);
    EXPECT_GL_NO_ERROR();
}

// Cover the case where null pixel data is uploaded to a texture and then sub image is used to
// upload partial data
TEST_P(RobustResourceInitTest, TexImageThenSubImage)
{
    if (!setup())
    {
        return;
    }

    if (IsOpenGL() || IsD3D9())
    {
        std::cout << "Robust resource init is not yet fully implemented. (" << GetParam() << ")"
                  << std::endl;
        return;
    }

    // Put some data into the texture

    GLTexture tex;
    glBindTexture(GL_TEXTURE_2D, tex);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, kWidth, kHeight, 0, GL_RGBA, GL_UNSIGNED_BYTE, nullptr);

    // Force the D3D texture to create a storage
    checkNonZeroPixels(&tex, 0, 0, 0, 0, GLColor::transparentBlack);

    glBindTexture(GL_TEXTURE_2D, tex);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, kWidth, kHeight, 0, GL_RGBA, GL_UNSIGNED_BYTE, nullptr);

    std::array<GLColor, kWidth * kHeight> data;
    data.fill(GLColor::white);

    glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, kWidth / 2, kHeight / 2, GL_RGBA, GL_UNSIGNED_BYTE,
                    data.data());
    checkNonZeroPixels(&tex, 0, 0, kWidth / 2, kHeight / 2, GLColor::white);
    EXPECT_GL_NO_ERROR();
}

// Reading an uninitialized texture (texImage2D) should succeed with all bytes set to 0.
TEST_P(RobustResourceInitTest, ReadingUninitialized3DTexture)
{
    if (!setup() || getClientMajorVersion() < 3)
    {
        return;
    }

    if (IsOpenGL())
    {
        std::cout << "Robust resource init is not yet fully implemented. (" << GetParam() << ")"
                  << std::endl;
        return;
    }

    GLTexture tex;
    setup3DTexture(&tex);
    glTexImage3D(GL_TEXTURE_3D, 0, GL_RGBA, kWidth, kHeight, 2, 0, GL_RGBA, GL_UNSIGNED_BYTE,
                 nullptr);
    checkNonZeroPixels3D(&tex, 0, 0, 0, 0, GLColor::transparentBlack);
    EXPECT_GL_NO_ERROR();
}

// Copy of the copytexsubimage3d_texture_wrongly_initialized test that is part of the WebGL2
// conformance suite: copy-texture-image-webgl-specific.html
TEST_P(RobustResourceInitTest, CopyTexSubImage3DTextureWronglyInitialized)
{
    if (!setup() || getClientMajorVersion() < 3)
    {
        return;
    }

    if (IsOpenGL())
    {
        std::cout << "Robust resource init is not yet fully implemented. (" << GetParam() << ")"
                  << std::endl;
        return;
    }

    constexpr GLint kTextureLayer     = 0;
    constexpr GLint kTextureWidth     = 2;
    constexpr GLint kTextureHeight    = 2;
    constexpr GLint kTextureDepth     = 2;
    constexpr size_t kTextureDataSize = kTextureWidth * kTextureHeight * 4;

    GLTexture texture2D;
    glBindTexture(GL_TEXTURE_2D, texture2D);
    constexpr std::array<uint8_t, kTextureDataSize> data = {{0x01, 0x02, 0x03, 0x04, 0x05, 0x06,
                                                             0x07, 0x08, 0x09, 0x0A, 0x0B, 0x0C,
                                                             0x0D, 0x0E, 0x0F, 0x10}};
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, kTextureWidth, kTextureHeight, 0, GL_RGBA,
                 GL_UNSIGNED_BYTE, data.data());

    GLFramebuffer fbo;
    glBindFramebuffer(GL_FRAMEBUFFER, fbo);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, texture2D, 0);
    ASSERT_GLENUM_EQ(GL_FRAMEBUFFER_COMPLETE, glCheckFramebufferStatus(GL_FRAMEBUFFER));

    GLTexture texture3D;
    glBindTexture(GL_TEXTURE_3D, texture3D);
    glTexStorage3D(GL_TEXTURE_3D, 1, GL_RGBA8, kTextureWidth, kTextureHeight, kTextureDepth);
    glCopyTexSubImage3D(GL_TEXTURE_3D, 0, 0, 0, kTextureLayer, 0, 0, kTextureWidth, kTextureHeight);

    glFramebufferTextureLayer(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, texture3D, 0, kTextureLayer);
    std::array<uint8_t, kTextureDataSize> pixels;
    glReadPixels(0, 0, kTextureWidth, kTextureHeight, GL_RGBA, GL_UNSIGNED_BYTE, pixels.data());
    ASSERT_GL_NO_ERROR();
    EXPECT_EQ(data, pixels);
}

// Test that binding an EGL surface to a texture does not cause it to be cleared.
TEST_P(RobustResourceInitTest, BindTexImage)
{
    if (!setup() || getClientMajorVersion() < 3)
    {
        return;
    }

    if (IsOpenGL())
    {
        std::cout << "Robust resource init is not yet fully implemented. (" << GetParam() << ")"
                  << std::endl;
        return;
    }

    EGLWindow *window  = getEGLWindow();
    EGLSurface surface = window->getSurface();
    EGLDisplay display = window->getDisplay();
    EGLConfig config   = window->getConfig();
    EGLContext context = window->getContext();

    EGLint surfaceType = 0;
    eglGetConfigAttrib(display, config, EGL_SURFACE_TYPE, &surfaceType);
    if ((surfaceType & EGL_PBUFFER_BIT) == 0)
    {
        std::cout << "Test skipped because EGL config cannot be used to create pbuffers."
                  << std::endl;
        return;
    }

    EGLint attribs[] = {
        EGL_WIDTH,          32,
        EGL_HEIGHT,         32,
        EGL_TEXTURE_FORMAT, EGL_TEXTURE_RGBA,
        EGL_TEXTURE_TARGET, EGL_TEXTURE_2D,
        EGL_NONE,
    };

    EGLSurface pbuffer = eglCreatePbufferSurface(display, config, attribs);
    ASSERT_NE(EGL_NO_SURFACE, pbuffer);

    // Clear the pbuffer
    eglMakeCurrent(display, pbuffer, pbuffer, context);
    GLColor clearColor = GLColor::magenta;
    glClearColor(clearColor.R, clearColor.G, clearColor.B, clearColor.A);
    glClear(GL_COLOR_BUFFER_BIT);
    EXPECT_PIXEL_COLOR_EQ(0, 0, clearColor);

    // Bind the pbuffer to a texture and read its color
    eglMakeCurrent(display, surface, surface, context);

    GLTexture texture;
    glBindTexture(GL_TEXTURE_2D, texture);
    eglBindTexImage(display, pbuffer, EGL_BACK_BUFFER);

    GLFramebuffer fbo;
    glBindFramebuffer(GL_FRAMEBUFFER, fbo);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, texture, 0);
    EXPECT_PIXEL_COLOR_EQ(0, 0, clearColor);

    eglDestroySurface(display, pbuffer);
}

// Tests that drawing with an uninitialized Texture works as expected.
TEST_P(RobustResourceInitTest, DrawWithTexture)
{
    if (!setup())
    {
        return;
    }

    GLTexture texture;
    glBindTexture(GL_TEXTURE_2D, texture);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, kWidth, kHeight, 0, GL_RGBA, GL_UNSIGNED_BYTE, nullptr);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

    const std::string &vertexShader =
        "attribute vec2 position;\n"
        "varying vec2 texCoord;\n"
        "void main() {\n"
        "    gl_Position = vec4(position, 0, 1);\n"
        "    texCoord = (position * 0.5) + 0.5;\n"
        "}";
    const std::string &fragmentShader =
        "precision mediump float;\n"
        "varying vec2 texCoord;\n"
        "uniform sampler2D tex;\n"
        "void main() {\n"
        "    gl_FragColor = texture2D(tex, texCoord);\n"
        "}";

    ANGLE_GL_PROGRAM(program, vertexShader, fragmentShader);
    drawQuad(program, "position", 0.5f);

    checkFramebufferNonZeroPixels(0, 0, 0, 0, GLColor::black);
}

// Reading a partially initialized texture (texImage2D) should succeed with all uninitialized bytes
// set to 0 and initialized bytes untouched.
TEST_P(RobustResourceInitTest, ReadingPartiallyInitializedTexture)
{
    if (!setup())
    {
        return;
    }

    if (IsOpenGL() || IsD3D11() || IsD3D9() || IsD3D11_FL93())
    {
        std::cout << "Robust resource init is not yet fully implemented. (" << GetParam() << ")"
                  << std::endl;
        return;
    }

    GLTexture tex;
    setupTexture(&tex);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, kWidth, kHeight, 0, GL_RGBA, GL_UNSIGNED_BYTE, nullptr);
    GLColor data(108, 72, 36, 9);
    glTexSubImage2D(GL_TEXTURE_2D, 0, kWidth / 2, kHeight / 2, 1, 1, GL_RGBA, GL_UNSIGNED_BYTE,
                    &data.R);
    checkNonZeroPixels(&tex, kWidth / 2, kHeight / 2, 1, 1, data);
    EXPECT_GL_NO_ERROR();
}

// Uninitialized parts of textures initialized via copyTexImage2D should have all bytes set to 0.
TEST_P(RobustResourceInitTest, UninitializedPartsOfCopied2DTexturesAreBlack)
{
    if (!setup())
    {
        return;
    }

    if (IsOpenGL() || IsD3D9() || IsD3D11_FL93())
    {
        std::cout << "Robust resource init is not yet fully implemented. (" << GetParam() << ")"
                  << std::endl;
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
    checkNonZeroPixels(&tex, 0, 0, fboWidth, fboHeight, GLColor::red);
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

    if (IsOpenGL() || IsD3D9() || IsD3D11_FL93())
    {
        std::cout << "Robust resource init is not yet fully implemented. (" << GetParam() << ")"
                  << std::endl;
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
    checkNonZeroPixels(&tex, -x, -y, fboWidth, fboHeight, GLColor::red);
    EXPECT_GL_NO_ERROR();
}

// Tests resources are initialized properly with multisample resolve.
TEST_P(RobustResourceInitTest, MultisampledDepthInitializedCorrectly)
{
    ANGLE_SKIP_TEST_IF(!setup() || getClientMajorVersion() < 3);
    ANGLE_SKIP_TEST_IF(!IsD3D11());

    const std::string vs = "attribute vec4 position; void main() { gl_Position = position; }";
    const std::string fs = "void main() { gl_FragColor = vec4(1, 0, 0, 1); }";
    ANGLE_GL_PROGRAM(program, vs, fs);

    // Make the destination non-multisampled depth FBO.
    GLTexture color;
    glBindTexture(GL_TEXTURE_2D, color);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, kWidth, kHeight, 0, GL_RGBA, GL_UNSIGNED_BYTE, nullptr);

    GLRenderbuffer depth;
    glBindRenderbuffer(GL_RENDERBUFFER, depth);
    glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT16, kWidth, kHeight);

    GLFramebuffer fbo;
    glBindFramebuffer(GL_FRAMEBUFFER, fbo);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, color, 0);
    glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, depth);
    ASSERT_GLENUM_EQ(GL_FRAMEBUFFER_COMPLETE, glCheckFramebufferStatus(GL_FRAMEBUFFER));

    glClearColor(0, 1, 0, 1);
    glClearDepthf(0);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    ASSERT_GL_NO_ERROR();
    EXPECT_PIXEL_COLOR_EQ(0, 0, GLColor::green);

    // Make the multisampled depth FBO.
    GLRenderbuffer msDepth;
    glBindRenderbuffer(GL_RENDERBUFFER, msDepth);
    glRenderbufferStorageMultisample(GL_RENDERBUFFER, 4, GL_DEPTH_COMPONENT16, kWidth, kHeight);

    GLFramebuffer msFBO;
    glBindFramebuffer(GL_READ_FRAMEBUFFER, msFBO);
    glFramebufferRenderbuffer(GL_READ_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, msDepth);
    ASSERT_GLENUM_EQ(GL_FRAMEBUFFER_COMPLETE, glCheckFramebufferStatus(GL_READ_FRAMEBUFFER));

    // Multisample resolve.
    glBlitFramebuffer(0, 0, kWidth, kHeight, 0, 0, kWidth, kHeight, GL_DEPTH_BUFFER_BIT,
                      GL_NEAREST);
    ASSERT_GL_NO_ERROR();

    // Test drawing with the resolved depth buffer.
    glBindFramebuffer(GL_FRAMEBUFFER, fbo);
    glDepthMask(GL_FALSE);
    glEnable(GL_DEPTH_TEST);
    glDepthFunc(GL_EQUAL);
    drawQuad(program, "position", 1.0f);
    ASSERT_GL_NO_ERROR();
    EXPECT_PIXEL_COLOR_EQ(0, 0, GLColor::red);
}

// Tests CompressedTexSubImage2D with S3TC works correctly.
// FIXME: jmadill
TEST_P(RobustResourceInitTest, DISABLED_CompressedTexSubImage2DInitialization)
{
    ANGLE_SKIP_TEST_IF(!setup() || getClientMajorVersion() < 3);
    ANGLE_SKIP_TEST_IF(!IsD3D11());

    constexpr int kSize     = 8;
    constexpr int kHalfSize = 4;

    GLFramebuffer smallFBO;
    glBindFramebuffer(GL_FRAMEBUFFER, smallFBO);

    GLTexture smallTexture;
    glBindTexture(GL_TEXTURE_2D, smallTexture);
    glTexStorage2D(GL_TEXTURE_2D, 1, GL_RGBA8, kSize, kSize);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, smallTexture, 0);

    glViewport(0, 0, kSize, kSize);

    constexpr uint8_t kImg4x4RGBADXT1[] = {
        0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
    };

    GLTexture tex;
    glBindTexture(GL_TEXTURE_2D, tex);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexStorage2D(GL_TEXTURE_2D, 1, GL_COMPRESSED_RGBA_S3TC_DXT1_EXT, kSize, kSize);

    const std::string vs =
        "attribute vec4 position;\n"
        "varying vec2 texCoord;\n"
        "void main()\n"
        "{\n"
        "    gl_Position = position;\n"
        "    texCoord = ((position.xy * 0.5) + 0.5);\n"
        "    texCoord.y = 1.0 - texCoord.y;\n"
        "}";

    const std::string fs =
        "varying mediump vec2 texCoord;\n"
        "uniform sampler2D tex;\n"
        "void main()\n"
        "{\n"
        "    gl_FragColor = texture2D(tex, texCoord);\n"
        "}";

    ANGLE_GL_PROGRAM(program, vs, fs);

    glCompressedTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, kHalfSize, kHalfSize,
                              GL_COMPRESSED_RGBA_S3TC_DXT1_EXT,
                              static_cast<GLsizei>(ArraySize(kImg4x4RGBADXT1)), kImg4x4RGBADXT1);

    drawQuad(program, "position", 0.5f);
    ASSERT_GL_NO_ERROR();

    checkCustomFramebufferNonZeroPixels(kSize, kSize, 0, 0, kHalfSize, kHalfSize, GLColor::black);
    ASSERT_GL_NO_ERROR();
}

// Basic test that textures are initialized correctly.
TEST_P(RobustResourceInitTest, Texture)
{
    if (!setup())
    {
        return;
    }

    GLTexture texture;
    glBindTexture(GL_TEXTURE_2D, texture);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, kWidth, kHeight, 0, GL_RGBA, GL_UNSIGNED_BYTE, nullptr);

    GLFramebuffer framebuffer;
    glBindFramebuffer(GL_FRAMEBUFFER, framebuffer);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, texture, 0);
    checkFramebufferNonZeroPixels(0, 0, 0, 0, GLColor::black);
}

template <typename PixelT>
void RobustResourceInitTest::testIntegerTextureInit(const char *samplerType,
                                                    GLenum internalFormatRGBA,
                                                    GLenum internalFormatRGB,
                                                    GLenum type)
{
    if (!setup() || getClientMajorVersion() < 3)
    {
        return;
    }

    ANGLE_GL_PROGRAM(program, kSimpleTextureVertexShader,
                     GetSimpleTextureFragmentShader(samplerType));

    // Make an RGBA framebuffer.
    GLTexture framebufferTexture;
    glBindTexture(GL_TEXTURE_2D, framebufferTexture);
    glTexImage2D(GL_TEXTURE_2D, 0, internalFormatRGBA, kWidth, kHeight, 0, GL_RGBA_INTEGER, type,
                 nullptr);
    ASSERT_GL_NO_ERROR();

    GLFramebuffer framebuffer;
    glBindFramebuffer(GL_FRAMEBUFFER, framebuffer);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, framebufferTexture,
                           0);

    // Make an RGB texture.
    GLTexture texture;
    glBindTexture(GL_TEXTURE_2D, texture);
    glTexImage2D(GL_TEXTURE_2D, 0, internalFormatRGB, kWidth, kHeight, 0, GL_RGB_INTEGER, type,
                 nullptr);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    ASSERT_GL_NO_ERROR();

    // Blit from the texture to the framebuffer.
    drawQuad(program, "position", 0.5f);

    std::array<PixelT, kWidth * kHeight * 4> data;
    glReadPixels(0, 0, kWidth, kHeight, GL_RGBA_INTEGER, type, data.data());

    // Check the color channels are zero and the alpha channel is 1.
    int incorrectPixels = 0;
    for (int y = 0; y < kHeight; ++y)
    {
        for (int x = 0; x < kWidth; ++x)
        {
            int index    = (y * kWidth + x) * 4;
            bool correct = (data[index] == 0 && data[index + 1] == 0 && data[index + 2] == 0 &&
                            data[index + 3] == 1);
            incorrectPixels += (!correct ? 1 : 0);
        }
    }

    ASSERT_GL_NO_ERROR();
    EXPECT_EQ(0, incorrectPixels);
}

// Simple tests for integer formats that ANGLE must emulate on D3D11.
TEST_P(RobustResourceInitTest, TextureInit_UIntRGB8)
{
    testIntegerTextureInit<uint8_t>("u", GL_RGBA8UI, GL_RGB8UI, GL_UNSIGNED_BYTE);
}

TEST_P(RobustResourceInitTest, TextureInit_UIntRGB32)
{
    testIntegerTextureInit<uint32_t>("u", GL_RGBA32UI, GL_RGB32UI, GL_UNSIGNED_INT);
}

TEST_P(RobustResourceInitTest, TextureInit_IntRGB8)
{
    testIntegerTextureInit<int8_t>("i", GL_RGBA8I, GL_RGB8I, GL_BYTE);
}

TEST_P(RobustResourceInitTest, TextureInit_IntRGB32)
{
    testIntegerTextureInit<int32_t>("i", GL_RGBA32I, GL_RGB32I, GL_INT);
}

// Basic test that renderbuffers are initialized correctly.
TEST_P(RobustResourceInitTest, Renderbuffer)
{
    if (!setup())
    {
        return;
    }

    GLRenderbuffer renderbuffer;
    glBindRenderbuffer(GL_RENDERBUFFER, renderbuffer);
    glRenderbufferStorage(GL_RENDERBUFFER, GL_RGBA8, kWidth, kHeight);

    GLFramebuffer framebuffer;
    glBindFramebuffer(GL_FRAMEBUFFER, framebuffer);
    glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_RENDERBUFFER, renderbuffer);

    checkFramebufferNonZeroPixels(0, 0, 0, 0, GLColor::black);
}

// Tests creating mipmaps with robust resource init.
TEST_P(RobustResourceInitTest, GenerateMipmap)
{
    ANGLE_SKIP_TEST_IF(getClientMajorVersion() < 3);

    constexpr GLint kTextureSize = 16;

    // Initialize a 16x16 RGBA8 texture with no data.
    GLTexture tex;
    glBindTexture(GL_TEXTURE_2D, tex);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, kTextureSize, kTextureSize, 0, GL_RGBA,
                 GL_UNSIGNED_BYTE, nullptr);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST_MIPMAP_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

    ANGLE_GL_PROGRAM(program, kSimpleTextureVertexShader, GetSimpleTextureFragmentShader(""));

    // Generate mipmaps and verify all the mips.
    glGenerateMipmap(GL_TEXTURE_2D);
    ASSERT_GL_NO_ERROR();

    // Validate a small texture.
    glClearColor(1, 0, 0, 1);
    glClear(GL_COLOR_BUFFER_BIT);
    EXPECT_PIXEL_COLOR_EQ(0, 0, GLColor::red);

    // Set viewport to resize the texture and draw.
    glViewport(0, 0, 2, 2);
    drawQuad(program, "position", 0.5f);
    ASSERT_GL_NO_ERROR();
    EXPECT_PIXEL_COLOR_EQ(0, 0, GLColor::transparentBlack);
}

// ----

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
