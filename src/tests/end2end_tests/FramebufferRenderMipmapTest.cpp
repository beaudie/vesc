#include "ANGLETest.h"

// Use this to select which configurations (e.g. which renderer, which GLES major version) these tests should be run against.
ANGLE_TYPED_TEST_CASE(FramebufferRenderMipmapTest, ES2_D3D9, ES2_D3D11, ES3_D3D11, ES2_OPENGL, ES3_OPENGL);

template<typename T>
class FramebufferRenderMipmapTest : public ANGLETest
{
protected:
    FramebufferRenderMipmapTest() : ANGLETest(T::GetGlesMajorVersion(), T::GetPlatform())
    {
        setWindowWidth(256);
        setWindowHeight(256);
        setConfigRedBits(8);
        setConfigGreenBits(8);
        setConfigBlueBits(8);
        setConfigAlphaBits(8);
    }

    virtual void SetUp()
    {
        ANGLETest::SetUp();

        const std::string vsSource = SHADER_SOURCE
        (
            attribute highp vec4 position;
            void main(void)
            {
                gl_Position = position;
            }
        );

        const std::string fsSource = SHADER_SOURCE
        (
            uniform highp vec4 color;
            void main(void)
            {
                gl_FragColor = color;
            }
        );

        mProgram = CompileProgram(vsSource, fsSource);
        if (mProgram == 0)
        {
            FAIL() << "shader compilation failed.";
        }

        mColorLocation = glGetUniformLocation(mProgram, "color");

        glUseProgram(mProgram);

        glClearColor(0, 0, 0, 0);
        glClearDepthf(0.0);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        glEnable(GL_BLEND);
        glDisable(GL_DEPTH_TEST);

        ASSERT_GL_NO_ERROR();
    }

    virtual void TearDown()
    {
        glDeleteProgram(mProgram);

        ANGLETest::TearDown();
    }

    GLuint mProgram;
    GLint mColorLocation;
};

// Validate that if we are in ES3 or GL_OES_fbo_render_mipmap exists, there are no validation errors
// when using a non-zero level in glFramebufferTexture2D.
TYPED_TEST(FramebufferRenderMipmapTest, Validation)
{
    bool renderToMipmapSupported = extensionEnabled("GL_OES_fbo_render_mipmap") || getClientVersion() > 2;

    GLuint tex = 0;
    glGenTextures(1, &tex);
    glBindTexture(GL_TEXTURE_2D, tex);

    const size_t levels = 5;
    for (size_t i = 0; i < levels; i++)
    {
        size_t size = 1 << ((levels - 1) - i);
        glTexImage2D(GL_TEXTURE_2D, i, GL_RGBA, size, size, 0, GL_RGBA, GL_UNSIGNED_BYTE, NULL);
    }

    EXPECT_GL_NO_ERROR();

    GLuint fbo = 0;
    glGenFramebuffers(1, &fbo);
    glBindFramebuffer(GL_FRAMEBUFFER, fbo);
    EXPECT_GL_NO_ERROR();

    for (size_t i = 0; i < levels; i++)
    {
        glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, tex, i);

        if (i > 0 && !renderToMipmapSupported)
        {
            EXPECT_GL_ERROR(GL_INVALID_VALUE);
        }
        else
        {
            EXPECT_GL_NO_ERROR();
            EXPECT_EQ(glCheckFramebufferStatus(GL_FRAMEBUFFER), GLenum(GL_FRAMEBUFFER_COMPLETE));
        }
    }

    glDeleteFramebuffers(1, &fbo);
    glDeleteTextures(1, &tex);
}

// Render to various levels of a texture and check that they have the correct color data via ReadPixels
TYPED_TEST(FramebufferRenderMipmapTest, RenderToMipmap)
{
    // TODO(geofflang): Figure out why this is broken on Intel OpenGL
    if (isIntel() && getPlatformRenderer() == EGL_PLATFORM_ANGLE_TYPE_OPENGL_ANGLE)
    {
        std::cout << "Test skipped on Intel OpenGL." << std::endl;
        return;
    }

    bool renderToMipmapSupported = extensionEnabled("GL_OES_fbo_render_mipmap") || getClientVersion() > 2;
    if (!renderToMipmapSupported)
    {
        std::cout << "Test skipped because GL_OES_fbo_render_mipmap or ES3 is not available." << std::endl;
        return;
    }

    const GLfloat levelColors[] =
    {
        1.0f, 0.0f, 0.0f, 1.0f,
        0.0f, 1.0f, 0.0f, 1.0f,
        0.0f, 0.0f, 1.0f, 1.0f,
        1.0f, 1.0f, 0.0f, 1.0f,
        1.0f, 0.0f, 1.0f, 1.0f,
        0.0f, 1.0f, 1.0f, 1.0f,
    };
    const size_t testLevels = ArraySize(levelColors) / 4;

    GLuint tex = 0;
    glGenTextures(1, &tex);
    glBindTexture(GL_TEXTURE_2D, tex);

    for (size_t i = 0; i < testLevels; i++)
    {
        size_t size = 1 << ((testLevels - 1) - i);
        glTexImage2D(GL_TEXTURE_2D, i, GL_RGBA, size, size, 0, GL_RGBA, GL_UNSIGNED_BYTE, NULL);
    }

    EXPECT_GL_NO_ERROR();

    GLuint fbo = 0;
    glGenFramebuffers(1, &fbo);
    glBindFramebuffer(GL_FRAMEBUFFER, fbo);
    EXPECT_GL_NO_ERROR();

    // Render to the levels of the texture with different colors
    for (size_t i = 0; i < testLevels; i++)
    {
        glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, tex, i);
        EXPECT_GL_NO_ERROR();

        glUseProgram(mProgram);
        glUniform4fv(mColorLocation, 1, levelColors + (i * 4));

        drawQuad(mProgram, "position", 0.5f);
        EXPECT_GL_NO_ERROR();
    }

    // Test that the levels of the texture are correct
    for (size_t i = 0; i < testLevels; i++)
    {
        glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, tex, i);
        EXPECT_GL_NO_ERROR();

        const GLfloat *color = levelColors + (i * 4);
        EXPECT_PIXEL_EQ(0, 0, color[0] * 255, color[1] * 255, color[2] * 255, color[3] * 255);
    }

    glDeleteFramebuffers(1, &fbo);
    glDeleteTextures(1, &tex);

    EXPECT_GL_NO_ERROR();
}

// Test for a bug where TextureStorage fails assertions when the texture is not mip complete in D3D9 and D3D11.
TYPED_TEST(FramebufferRenderMipmapTest, RenderToIncompleteTextureBug)
{
    bool renderToMipmapSupported = extensionEnabled("GL_OES_fbo_render_mipmap") || getClientVersion() > 2;
    if (!renderToMipmapSupported)
    {
        return;
    }

    GLuint fbo = 0;
    glGenFramebuffers(1, &fbo);
    glBindFramebuffer(GL_FRAMEBUFFER, fbo);
    EXPECT_GL_NO_ERROR();

    GLuint tex = 0;
    glGenTextures(1, &tex);
    glBindTexture(GL_TEXTURE_2D, tex);

    const size_t levels = 5;
    for (size_t i = 0; i < levels; i++)
    {
        glTexImage2D(GL_TEXTURE_2D, i, GL_RGBA, 1, 1, 0, GL_RGBA, GL_UNSIGNED_BYTE, NULL);
    }
    EXPECT_GL_NO_ERROR();

    for (size_t i = 0; i < levels; i++)
    {
        glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, tex, i);
        EXPECT_GL_NO_ERROR();

        // The GL renderer does not always support this type of framebuffer
        if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_UNSUPPORTED)
        {
            glClear(GL_COLOR_BUFFER_BIT);
            EXPECT_GL_NO_ERROR();
        }
    }

    glDeleteFramebuffers(1, &fbo);
    glDeleteTextures(1, &tex);
}

// Try resizing a cube map between renders and verify that it renders correctly.
TYPED_TEST(FramebufferRenderMipmapTest, MissmatchedCubeFaceSizesBug)
{
    GLuint fbo = 0;
    glGenFramebuffers(1, &fbo);
    glBindFramebuffer(GL_FRAMEBUFFER, fbo);
    EXPECT_GL_NO_ERROR();

    GLuint tex = 0;
    glGenTextures(1, &tex);
    glBindTexture(GL_TEXTURE_CUBE_MAP, tex);

    for (GLenum face = GL_TEXTURE_CUBE_MAP_POSITIVE_X; face <= GL_TEXTURE_CUBE_MAP_NEGATIVE_Z; face++)
    {
        glTexImage2D(face, 0, GL_RGBA, 256, 256, 0, GL_RGBA, GL_UNSIGNED_BYTE, NULL);
    }
    EXPECT_GL_NO_ERROR();

    // Ensure a texture storage is created
    glClearColor(0, 0, 1.0f, 1.0f);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_CUBE_MAP_POSITIVE_X, tex, 0);
    glClear(GL_COLOR_BUFFER_BIT);
    EXPECT_GL_NO_ERROR();
    EXPECT_PIXEL_EQ(0, 0, 0, 0, 255, 255);

    // Resize a face
    glTexImage2D(GL_TEXTURE_CUBE_MAP_NEGATIVE_Y, 0, GL_RGBA, 1, 1, 0, GL_RGBA, GL_UNSIGNED_BYTE, NULL);
    EXPECT_GL_NO_ERROR();

    // The GL renderer does not always support this type of framebuffer
    if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_UNSUPPORTED)
    {
        // Clear again
        glClearColor(0, 1.0f, 0.0f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT);
        EXPECT_GL_NO_ERROR();
        EXPECT_PIXEL_EQ(0, 0, 0, 255, 0, 255);
    }
}
