//
// Copyright 2017 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//
// Multiview buffer tests:
// Test issuing multiview buffer commands.
//

#include "platform/WorkaroundsD3D.h"
#include "test_utils/ANGLETest.h"
#include "test_utils/gl_raii.h"

using namespace angle;

class MultiviewDrawBuffersTest : public ANGLETest
{
  protected:
    MultiviewDrawBuffersTest()
        : ANGLETest(),
          mHasMultiviewDrawBuffersExtension(true),
          mFramebuffer(0),
          mColorBuffer0(0),
          mColorBuffer1(0),
          mNumBuffers(-1),
          mMaxColorAttachments(-1)
    {
        DestroyTestWindow();
        InitTestWindow();
        setWindowWidth(128);
        setWindowHeight(128);
        setWebGLCompatibilityEnabled(true);
        setMultiviewWindowViewCount(2);
    }
    virtual ~MultiviewDrawBuffersTest() {}

    void SetUp() override
    {
        ANGLETest::SetUp();
        if (checkMultiviewDrawBuffersExtension())
        {
            getBufferCounts();
            createFBO();
        }
        else
        {
            mHasMultiviewDrawBuffersExtension = false;
        }
    }

    void TearDown() override
    {
        ANGLETest::TearDown();
        DestroyTestWindow();
        InitTestWindow();
    }

    // Checks for the GL_EXT_multiview_draw_buffers extension and returns true if the
    // extension is enabled.
    bool checkMultiviewDrawBuffersExtension()
    {
        if (extensionRequestable("GL_EXT_multiview_draw_buffers"))
        {
            glRequestExtensionANGLE("GL_EXT_multiview_draw_buffers");
        }

        if (!extensionEnabled("GL_EXT_multiview_draw_buffers"))
        {
            std::cout << "Test skipped due to missing GL_EXT_multiview_draw_buffers." << std::endl;
            return false;
        }
        return true;
    }

    void createFBO()
    {
        glGenFramebuffers(1, &mFramebuffer);
        glBindFramebuffer(GL_FRAMEBUFFER, mFramebuffer);

        glGenTextures(1, &mColorBuffer0);
        glBindTexture(GL_TEXTURE_2D, mColorBuffer0);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, 128, 128, 0, GL_RGBA, GL_UNSIGNED_BYTE, nullptr);
        glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, mColorBuffer0,
                               0);

        glGenTextures(1, &mColorBuffer1);
        glBindTexture(GL_TEXTURE_2D, mColorBuffer1);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, 128, 128, 0, GL_RGBA, GL_UNSIGNED_BYTE, nullptr);
        glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT1, GL_TEXTURE_2D, mColorBuffer1,
                               0);
        glBindFramebuffer(GL_FRAMEBUFFER, 0);
        ASSERT_GL_NO_ERROR();
    }

    void getBufferCounts()
    {
        // First check EGLqueries
        EGLint value   = -1;
        EGLBoolean err = EGL_FALSE;
        err            = eglQuerySurface(getEGLWindow()->getDisplay(), getEGLWindow()->getSurface(),
                              EGL_MULTIVIEW_VIEW_COUNT_EXT, &value);
        EXPECT_EQ(err, (EGLBoolean)EGL_TRUE);

        err = EGL_FALSE;
        err = eglQueryContext(getEGLWindow()->getDisplay(), getEGLWindow()->getContext(),
                              EGL_MULTIVIEW_VIEW_COUNT_EXT, &value);
        EXPECT_EQ(err, (EGLBoolean)EGL_TRUE);

        glGetIntegerv(GL_MAX_MULTIVIEW_BUFFERS_EXT, &mNumBuffers);
        ASSERT_GL_NO_ERROR();

        glGetIntegerv(GL_MAX_COLOR_ATTACHMENTS, &mMaxColorAttachments);
        ASSERT_GL_NO_ERROR();
    }

    bool mHasMultiviewDrawBuffersExtension;
    GLuint mFramebuffer;
    GLuint mColorBuffer0;
    GLuint mColorBuffer1;
    GLint mNumBuffers;
    GLint mMaxColorAttachments;
};

// The test verifies Valid API usage and error conditions for the default framebuffer.
TEST_P(MultiviewDrawBuffersTest, DefaultFramebuffer)
{
    if (!mHasMultiviewDrawBuffersExtension)
    {
        return;
    }

    // Check glDrawBuffersIndexedEXT with valid data
    GLenum locations[1] = {GL_MULTIVIEW_EXT};
    GLint indices[1];
    GLint drawBuffer[2];

    drawBuffer[0] = -1;
    drawBuffer[1] = -1;
    for (int i = 0; i < mNumBuffers; ++i)
    {
        indices[0] = i;
        glDrawBuffersIndexedEXT(1, locations, indices);
        EXPECT_GL_NO_ERROR();

        drawBuffer[0] = -1;
        drawBuffer[1] = -1;
        glGetIntegeri_vEXT(GL_DRAW_BUFFER_EXT, 0, &drawBuffer[0]);
        EXPECT_GL_NO_ERROR();
        EXPECT_EQ(drawBuffer[1], i);
    }

    GLenum bothLocations[] = {GL_MULTIVIEW_EXT, GL_MULTIVIEW_EXT};
    GLint bothIndices[]    = {0, 1};
    if (mNumBuffers > 1)
    {

        glDrawBuffersIndexedEXT(2, bothLocations, bothIndices);
        EXPECT_GL_NO_ERROR();

        glGetIntegeri_vEXT(GL_DRAW_BUFFER_EXT, 0, &drawBuffer[0]);
        EXPECT_GL_NO_ERROR();
        EXPECT_EQ(drawBuffer[1], 0);

        glGetIntegeri_vEXT(GL_DRAW_BUFFER_EXT, 1, &drawBuffer[0]);
        EXPECT_GL_NO_ERROR();
        EXPECT_EQ(drawBuffer[1], 1);
    }

    // Check glReadBufferIndexedEXT with valid data
    GLint readBuffer[2];
    readBuffer[0] = -1;
    readBuffer[1] = -1;
    for (int i = 0; i < mNumBuffers; ++i)
    {
        glReadBufferIndexedEXT(GL_MULTIVIEW_EXT, i);
        EXPECT_GL_NO_ERROR();

        readBuffer[0] = -1;
        readBuffer[1] = -1;
        glGetIntegeri_vEXT(GL_READ_BUFFER_EXT, 0, &readBuffer[0]);
        EXPECT_GL_NO_ERROR();
        EXPECT_EQ(readBuffer[0], GL_MULTIVIEW_EXT);
        EXPECT_EQ(readBuffer[1], i);
    }

    // Check glDrawBuffersIndexedEXT with invalid data
    indices[0] = mNumBuffers;
    glDrawBuffersIndexedEXT(1, locations, indices);
    EXPECT_GL_ERROR(GL_INVALID_OPERATION);

    indices[0]   = 0;
    locations[0] = GL_COLOR_ATTACHMENT_EXT;
    glDrawBuffersIndexedEXT(1, locations, indices);
    EXPECT_GL_ERROR(GL_INVALID_ENUM);

    if (mNumBuffers > 1)
    {
        bothIndices[1] = mNumBuffers;
        glDrawBuffersIndexedEXT(2, bothLocations, bothIndices);
        EXPECT_GL_ERROR(GL_INVALID_OPERATION);

        bothLocations[1] = GL_COLOR_ATTACHMENT_EXT;
        bothIndices[1]   = 1;
        glDrawBuffersIndexedEXT(2, bothLocations, bothIndices);
        EXPECT_GL_ERROR(GL_INVALID_ENUM);
    }

    // Check glReadBufferIndexedEXT with valid data
    glReadBufferIndexedEXT(GL_MULTIVIEW_EXT, mNumBuffers);
    EXPECT_GL_ERROR(GL_INVALID_OPERATION);

    glReadBufferIndexedEXT(GL_COLOR_ATTACHMENT_EXT, mNumBuffers);
    EXPECT_GL_ERROR(GL_INVALID_ENUM);
}

// The test verifies Valid API usage and error conditions for the default framebuffer.
TEST_P(MultiviewDrawBuffersTest, FramebufferObject)
{
    if (!mHasMultiviewDrawBuffersExtension)
    {
        return;
    }

    glBindFramebuffer(GL_FRAMEBUFFER, mFramebuffer);
    EXPECT_GL_NO_ERROR();

    // Check glDrawBuffersIndexedEXT with valid data
    GLenum locations[1] = {GL_COLOR_ATTACHMENT_EXT};
    GLint indices[1];
    GLint drawBuffer[2];

    drawBuffer[0] = -1;
    drawBuffer[1] = -1;
    for (int i = 0; i < mMaxColorAttachments; ++i)
    {
        indices[0] = i;
        glDrawBuffersIndexedEXT(1, locations, indices);
        EXPECT_GL_NO_ERROR();

        drawBuffer[0] = -1;
        drawBuffer[1] = -1;
        glGetIntegeri_vEXT(GL_DRAW_BUFFER_EXT, 0, &drawBuffer[0]);
        EXPECT_GL_NO_ERROR();
        EXPECT_EQ(drawBuffer[1], i);
    }

    GLenum bothLocations[] = {GL_COLOR_ATTACHMENT_EXT, GL_COLOR_ATTACHMENT_EXT};
    GLint bothIndices[]    = {0, 1};
    glDrawBuffersIndexedEXT(2, bothLocations, bothIndices);
    EXPECT_GL_NO_ERROR();

    glGetIntegeri_vEXT(GL_DRAW_BUFFER_EXT, 0, &drawBuffer[0]);
    EXPECT_GL_NO_ERROR();
    EXPECT_EQ(drawBuffer[1], 0);

    glGetIntegeri_vEXT(GL_DRAW_BUFFER_EXT, 1, &drawBuffer[0]);
    EXPECT_GL_NO_ERROR();
    EXPECT_EQ(drawBuffer[1], 1);

    // Check glReadBufferIndexedEXT with valid data
    GLint readBuffer[2];
    readBuffer[0] = -1;
    readBuffer[1] = -1;
    for (int i = 0; i < mMaxColorAttachments; ++i)
    {
        glReadBufferIndexedEXT(GL_COLOR_ATTACHMENT_EXT, i);
        EXPECT_GL_NO_ERROR();

        readBuffer[0] = -1;
        readBuffer[1] = -1;
        glGetIntegeri_vEXT(GL_READ_BUFFER_EXT, 0, &readBuffer[0]);
        EXPECT_GL_NO_ERROR();
        EXPECT_EQ(readBuffer[0], GL_COLOR_ATTACHMENT_EXT);
        EXPECT_EQ(readBuffer[1], i);
    }

    // Check glDrawBuffersIndexedEXT with invalid data
    indices[0] = mMaxColorAttachments;
    glDrawBuffersIndexedEXT(1, locations, indices);
    EXPECT_GL_ERROR(GL_INVALID_OPERATION);

    indices[0]   = 0;
    locations[0] = GL_MULTIVIEW_EXT;
    glDrawBuffersIndexedEXT(1, locations, indices);
    EXPECT_GL_ERROR(GL_INVALID_ENUM);

    bothIndices[1] = mMaxColorAttachments;
    glDrawBuffersIndexedEXT(2, bothLocations, bothIndices);
    EXPECT_GL_ERROR(GL_INVALID_OPERATION);

    bothLocations[1] = GL_MULTIVIEW_EXT;
    bothIndices[1]   = 1;
    glDrawBuffersIndexedEXT(2, bothLocations, bothIndices);
    EXPECT_GL_ERROR(GL_INVALID_ENUM);

    // Check glReadBufferIndexedEXT with valid data
    glReadBufferIndexedEXT(GL_COLOR_ATTACHMENT_EXT, mMaxColorAttachments);
    EXPECT_GL_ERROR(GL_INVALID_OPERATION);

    glReadBufferIndexedEXT(GL_MULTIVIEW_EXT, mNumBuffers);
    EXPECT_GL_ERROR(GL_INVALID_ENUM);
}

// The test checks that we can draw into the default buffer
TEST_P(MultiviewDrawBuffersTest, DefaultFramebufferRender)
{
    if (!mHasMultiviewDrawBuffersExtension)
    {
        return;
    }

    // Check glDrawBuffersIndexedEXT with valid data
    GLenum locations[1] = {GL_MULTIVIEW_EXT};
    GLint indices[1];

    // Put red in the left, and blue in the right
    indices[0] = 0;
    glDrawBuffersIndexedEXT(1, locations, indices);
    EXPECT_GL_NO_ERROR();

    glClearColor(1.0f, 0.0f, 0.0f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT);

    EXPECT_GL_NO_ERROR();

    glReadBufferIndexedEXT(GL_MULTIVIEW_EXT, 0);
    EXPECT_GL_NO_ERROR();

    GLColor expectedColor;
    expectedColor.R = 255;
    expectedColor.G = 0;
    expectedColor.B = 0;
    expectedColor.A = 255;
    EXPECT_PIXEL_COLOR_EQ(50, 50, expectedColor);

    if (mNumBuffers > 1)
    {
        indices[0] = 1;
        glDrawBuffersIndexedEXT(1, locations, indices);
        EXPECT_GL_NO_ERROR();

        glClearColor(0.0f, 1.0f, 0.0f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT);
        EXPECT_GL_NO_ERROR();

        glReadBufferIndexedEXT(GL_MULTIVIEW_EXT, 1);
        EXPECT_GL_NO_ERROR();

        expectedColor.R = 0;
        expectedColor.G = 255;
        expectedColor.B = 0;
        expectedColor.A = 255;
        EXPECT_PIXEL_COLOR_EQ(50, 50, expectedColor);

        GLenum bothLocations[] = {GL_MULTIVIEW_EXT, GL_MULTIVIEW_EXT};
        GLint bothIndices[]    = {0, 1};
        glDrawBuffersIndexedEXT(2, bothLocations, bothIndices);
        EXPECT_GL_NO_ERROR();

        glClearColor(0.0f, 0.0f, 1.0f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT);
        EXPECT_GL_NO_ERROR();

        glReadBufferIndexedEXT(GL_MULTIVIEW_EXT, 0);
        EXPECT_GL_NO_ERROR();

        expectedColor.R = 0;
        expectedColor.G = 0;
        expectedColor.B = 255;
        expectedColor.A = 255;
        EXPECT_PIXEL_COLOR_EQ(50, 50, expectedColor);

        glReadBufferIndexedEXT(GL_MULTIVIEW_EXT, 1);
        EXPECT_GL_NO_ERROR();

        EXPECT_PIXEL_COLOR_EQ(50, 50, expectedColor);
    }

    EXPECT_GL_NO_ERROR();
}

ANGLE_INSTANTIATE_TEST(MultiviewDrawBuffersTest,
                       ES31_OPENGL(),
                       ES31_D3D11(),
                       ES2_D3D9(),
                       ES2_VULKAN());
