#include "ANGLETest.h"

class DrawBuffersTest : public ANGLETest
{
  protected:
    DrawBuffersTest()
    {
        setWindowWidth(128);
        setWindowHeight(128);
        setConfigRedBits(8);
        setConfigGreenBits(8);
        setConfigBlueBits(8);
        setConfigAlphaBits(8);
        setConfigDepthBits(24);
        setClientVersion(3);
    }

    virtual void SetUp()
    {
        ANGLETest::SetUp();

        const std::string vertexShaderSource =
            "#version 300 es\n"
            "in vec4 position;\n"
            "void main() {\n"
            "    gl_Position = position;\n"
            "}\n";

        const std::string fragmentShaderSource =
            "#version 300 es\n"
            "precision highp float;\n"
            "layout(location = 0) out vec4 nothing;\n"
            "layout(location = 1) out vec4 color;\n"
            "void main() {\n"
            "    nothing = vec4(1.0, 0.0, 0.0, 1.0);\n"
            "    color = vec4(1.0, 0.0, 0.0, 1.0);\n"
            "}\n";

        mProgram = CompileProgram(vertexShaderSource, fragmentShaderSource);
        if (mProgram == 0)
        {
            FAIL() << "shader compilation failed.";
        }

        glGenFramebuffers(1, &mFBO);
        glBindFramebuffer(GL_FRAMEBUFFER, mFBO);

        glGenTextures(1, &mTexture);
        glBindTexture(GL_TEXTURE_2D, mTexture);
        glTexStorage2DEXT(GL_TEXTURE_2D, 1, GL_RGBA8, getWindowWidth(), getWindowHeight());
        glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT1, GL_TEXTURE_2D, mTexture, 0);

        GLfloat data[] =
        {
            -1.0f, 1.0f,
            -1.0f, -2.0f,
            2.0f, 1.0f
        };

        glGenBuffers(1, &mBuffer);
        glBindBuffer(GL_ARRAY_BUFFER, mBuffer);
        glBufferData(GL_ARRAY_BUFFER, sizeof(GLfloat) * 6, data, GL_STATIC_DRAW);

        GLint mAttribLocation = glGetAttribLocation(mProgram, "position");
        ASSERT_NE(mAttribLocation, -1);
        glBindBuffer(GL_ARRAY_BUFFER, mBuffer);
        glVertexAttribPointer(mAttribLocation, 2, GL_FLOAT, GL_FALSE, 8, NULL);
        glEnableVertexAttribArray(mAttribLocation);

        ASSERT_GL_NO_ERROR();
    }

    virtual void TearDown()
    {
        glDeleteProgram(mProgram);
        glDeleteFramebuffers(1, &mFBO);
        glDeleteTextures(1, &mTexture);
        glDeleteBuffers(1, &mBuffer);

        ANGLETest::TearDown();
    }

    GLuint mProgram;
    GLuint mFBO;
    GLuint mTexture;
    GLuint mBuffer;
    GLint mAttribLocation;
};

// An nVidia driver bug in versions prior to 337.88 caused problems with
// ANGLE's MRT implementation on D3D11 when using NULL values between
// non-NULL values in OMSetRenderTargets.
TEST_F(DrawBuffersTest, Gaps)
{
    const GLenum bufs[] =
    {
        GL_NONE,
        GL_COLOR_ATTACHMENT1
    };
    glUseProgram(mProgram);
    glDrawBuffers(2, bufs);
    glDrawArrays(GL_TRIANGLES, 0, 3);

    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT1, GL_TEXTURE_2D, 0, 0);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, mTexture, 0);
    EXPECT_PIXEL_EQ(getWindowWidth() / 2, getWindowHeight() / 2, 255, 0, 0, 255);
}
