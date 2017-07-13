//
// Copyright 2017 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//
// Framebuffer multiview tests:
// The tests modify and examine the multiview state.
//

#include "test_utils/ANGLETest.h"

using namespace angle;

class FramebufferMultiviewTest : public ANGLETest
{
  protected:
    FramebufferMultiviewTest() : mFramebuffer(0), mTexture2D(0), mTexture2DArray(0)
    {
        setWindowWidth(128);
        setWindowHeight(128);
        setWebGLCompatibilityEnabled(true);
    }

    void SetUp() override
    {
        ANGLETest::SetUp();

        glGenFramebuffers(1, &mFramebuffer);
        glBindFramebuffer(GL_FRAMEBUFFER, mFramebuffer);

        glRequestExtensionANGLE = reinterpret_cast<PFNGLREQUESTEXTENSIONANGLEPROC>(
            eglGetProcAddress("glRequestExtensionANGLE"));
    }

    void TearDown() override
    {
        if (mTexture2D != 0)
        {
            glDeleteTextures(1, &mTexture2D);
            mTexture2D = 0;
        }

        if (mTexture2DArray != 0)
        {
            glDeleteTextures(1, &mTexture2DArray);
            mTexture2DArray = 0;
        }

        if (mFramebuffer != 0)
        {
            glDeleteFramebuffers(1, &mFramebuffer);
            mFramebuffer = 0;
        }

        ANGLETest::TearDown();
    }

    void createTexture2D()
    {
        glGenTextures(1, &mTexture2D);
        glBindTexture(GL_TEXTURE_2D, mTexture2D);
        glTexStorage2D(GL_TEXTURE_2D, 1, GL_RGBA16F, 1, 1);
        ASSERT_GL_NO_ERROR();
    }

    void createTexture2DArray()
    {
        glGenTextures(1, &mTexture2DArray);
        glBindTexture(GL_TEXTURE_2D_ARRAY, mTexture2DArray);
        glTexStorage3D(GL_TEXTURE_2D_ARRAY, 1, GL_RGBA16F, 1, 1, 2);
        ASSERT_GL_NO_ERROR();
    }

    // Requests the ANGLE_multiview extension and returns true if the operation succeeds.
    bool requestMultiviewExtension()
    {
        if (extensionRequestable("GL_ANGLE_multiview"))
        {
            glRequestExtensionANGLE("GL_ANGLE_multiview");
        }

        if (!extensionEnabled("GL_ANGLE_multiview"))
        {
            std::cout << "Test skipped due to missing GL_ANGLE_multiview." << std::endl;
            return false;
        }
        return true;
    }

    GLuint mFramebuffer;
    GLuint mTexture2D;
    GLuint mTexture2DArray;
    PFNGLREQUESTEXTENSIONANGLEPROC glRequestExtensionANGLE = nullptr;
};

// Test that the correct errors are generated whenever glFramebufferTextureMultiviewSideBySideANGLE
// is called with invalid arguments.
TEST_P(FramebufferMultiviewTest, NegativeSideBySideChecks)
{
    if (!requestMultiviewExtension())
    {
        return;
    }

    createTexture2D();
    // Negative offsets.
    int viewportOffsets[2] = {-1};
    glFramebufferTextureMultiviewSideBySideANGLE(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, mTexture2D,
                                                 0, 1, &viewportOffsets[0]);
    EXPECT_GL_ERROR(GL_INVALID_VALUE);

    // Negative number of views.
    viewportOffsets[0] = 0;
    viewportOffsets[1] = 0;
    glFramebufferTextureMultiviewSideBySideANGLE(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, mTexture2D,
                                                 0, -1, &viewportOffsets[0]);
    EXPECT_GL_ERROR(GL_INVALID_VALUE);
}

// Test that the correct errors are generated whenever glFramebufferTextureMultiviewLayeredANGLE is
// called with invalid arguments.
TEST_P(FramebufferMultiviewTest, NegativeLayeredChecks)
{
    if (!requestMultiviewExtension())
    {
        return;
    }

    createTexture2DArray();
    // Negative base view index.
    glFramebufferTextureMultiviewLayeredANGLE(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, mTexture2DArray,
                                              0, -1, 1);
    EXPECT_GL_ERROR(GL_INVALID_VALUE);

    // baseViewIndex + numViews is greater than MAX_TEXTURE_LAYERS.
    GLint maxTextureLayers = 0;
    glGetIntegerv(GL_MAX_ARRAY_TEXTURE_LAYERS, &maxTextureLayers);
    ASSERT_GL_NO_ERROR();
    glFramebufferTextureMultiviewLayeredANGLE(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, mTexture2DArray,
                                              0, maxTextureLayers, 1);
    EXPECT_GL_ERROR(GL_INVALID_VALUE);
}

// Test that an INVALID_OPERATION error is generated whenever the ANGLE_multiview extension is not
// available.
TEST_P(FramebufferMultiviewTest, ExtensionNotAvailableCheck)
{
    createTexture2D();
    int viewportOffsets[2] = {0};
    glFramebufferTextureMultiviewSideBySideANGLE(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, mTexture2D,
                                                 0, 1, &viewportOffsets[0]);
    EXPECT_GL_ERROR(GL_INVALID_OPERATION);
}

ANGLE_INSTANTIATE_TEST(FramebufferMultiviewTest, ES3_OPENGL());