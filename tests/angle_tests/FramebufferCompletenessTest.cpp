#include "ANGLETest.h"

// Use this to select which configurations (e.g. which renderer, which GLES major version) these tests should be run against.
ANGLE_TYPED_TEST_CASE(FramebufferCompletenessTest, ES3_D3D11);

template<typename T>
class FramebufferCompletenessTest : public ANGLETest
{
protected:
    FramebufferCompletenessTest() : ANGLETest(T::GetGlesMajorVersion(), T::GetPlatform())
    {
        setWindowWidth(128);
        setWindowHeight(128);
        setConfigRedBits(8);
        setConfigGreenBits(8);
        setConfigBlueBits(8);
        setConfigAlphaBits(8);
    }

    virtual void SetUp()
    {
        ANGLETest::SetUp();
    }

    virtual void TearDown()
    {
        ANGLETest::TearDown();
    }
};

TYPED_TEST(FramebufferCompletenessTest, 2DArrayInvalidLayer)
{
    const size_t numLayers = 5;
    const size_t numMips = 5;

    GLuint tex = 0;
    glGenTextures(1, &tex);
    glBindTexture(GL_TEXTURE_2D_ARRAY, tex);
    glTexStorage3D(GL_TEXTURE_2D_ARRAY, numMips, GL_RGBA8, 1 << numMips, 1 << numMips, numLayers);
    ASSERT_GL_NO_ERROR();

    GLuint fbo = 0;
    glGenFramebuffers(1, &fbo);
    glBindFramebuffer(GL_FRAMEBUFFER, fbo);
    ASSERT_GL_NO_ERROR();

    glFramebufferTextureLayer(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, tex, 0, numLayers);
    EXPECT_NE(GLenum(GL_FRAMEBUFFER_COMPLETE), glCheckFramebufferStatus(GL_FRAMEBUFFER));

    glFramebufferTextureLayer(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, tex, numMips, 0);
    EXPECT_NE(GLenum(GL_FRAMEBUFFER_COMPLETE), glCheckFramebufferStatus(GL_FRAMEBUFFER));

    glFramebufferTextureLayer(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, tex, 0, 0);
    EXPECT_EQ(GLenum(GL_FRAMEBUFFER_COMPLETE), glCheckFramebufferStatus(GL_FRAMEBUFFER));

    glFramebufferTextureLayer(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, tex, 0, numLayers - 1);
    EXPECT_EQ(GLenum(GL_FRAMEBUFFER_COMPLETE), glCheckFramebufferStatus(GL_FRAMEBUFFER));

    glFramebufferTextureLayer(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, tex, numMips - 1, 0);
    EXPECT_EQ(GLenum(GL_FRAMEBUFFER_COMPLETE), glCheckFramebufferStatus(GL_FRAMEBUFFER));

    glFramebufferTextureLayer(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, tex, numMips - 1, numLayers - 1);
    EXPECT_EQ(GLenum(GL_FRAMEBUFFER_COMPLETE), glCheckFramebufferStatus(GL_FRAMEBUFFER));
}

TYPED_TEST(FramebufferCompletenessTest, 3DInvalidLayer)
{
    const size_t numMips = 5;
    const size_t size = 1 << numMips;

    GLuint tex = 0;
    glGenTextures(1, &tex);
    glBindTexture(GL_TEXTURE_3D, tex);
    glTexStorage3D(GL_TEXTURE_3D, numMips, GL_RGBA8, size, size, size);
    ASSERT_GL_NO_ERROR();

    GLuint fbo = 0;
    glGenFramebuffers(1, &fbo);
    glBindFramebuffer(GL_FRAMEBUFFER, fbo);
    ASSERT_GL_NO_ERROR();

    glFramebufferTextureLayer(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, tex, 0, size);
    EXPECT_NE(GLenum(GL_FRAMEBUFFER_COMPLETE), glCheckFramebufferStatus(GL_FRAMEBUFFER));

    glFramebufferTextureLayer(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, tex, numMips, 0);
    EXPECT_NE(GLenum(GL_FRAMEBUFFER_COMPLETE), glCheckFramebufferStatus(GL_FRAMEBUFFER));

    glFramebufferTextureLayer(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, tex, 0, 0);
    EXPECT_EQ(GLenum(GL_FRAMEBUFFER_COMPLETE), glCheckFramebufferStatus(GL_FRAMEBUFFER));

    glFramebufferTextureLayer(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, tex, 0, size - 1);
    EXPECT_EQ(GLenum(GL_FRAMEBUFFER_COMPLETE), glCheckFramebufferStatus(GL_FRAMEBUFFER));

    glFramebufferTextureLayer(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, tex, numMips - 1, 0);
    EXPECT_EQ(GLenum(GL_FRAMEBUFFER_COMPLETE), glCheckFramebufferStatus(GL_FRAMEBUFFER));

    glFramebufferTextureLayer(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, tex, 1, (size >> 1) - 1);
    EXPECT_EQ(GLenum(GL_FRAMEBUFFER_COMPLETE), glCheckFramebufferStatus(GL_FRAMEBUFFER));

    glFramebufferTextureLayer(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, tex, 1, size >> 1);
    EXPECT_NE(GLenum(GL_FRAMEBUFFER_COMPLETE), glCheckFramebufferStatus(GL_FRAMEBUFFER));
}
