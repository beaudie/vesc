//
// Copyright 2019 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//
// EGLSyncTest.cpp:
//   Tests of EGL_KHR_fence_sync and EGL_KHR_wait_sync extensions.

#include <gtest/gtest.h>

#include "test_utils/ANGLETest.h"
#include "test_utils/angle_test_configs.h"
#include "util/EGLWindow.h"

// TODO

using namespace angle

    class EGLSyncTest : public ANGLETest
{
  protected:
    bool hasFenceSyncExtension() const
    {
        return eglDisplayExtensionEnabled(getEGLWindow()->getDisplay(), "EGL_KHR_fence_sync");
    }
    bool hasWaitSyncExtension() const
    {
        return hasEGLSyncExtension() &&
               eglDisplayExtensionEnabled(getEGLWindow()->getDisplay(), "EGL_KHR_wait_sync");
    }
    bool hasGLSyncExtension() const { return hasExtension("GL_OES_EGL_sync"); }
};

// Test error cases for all EGL_KHR_fence_sync functions
TEST_P(EGLSyncTest, FenceSyncErrors)
{
    ANGLE_SKIP_TEST_IF(!hasFenceSyncExtension());

    EGLDisplay display = getEGLWindow()->getDisplay();

    // If the client API doesn't have the necessary extension, test that sync creation fails and
    // ignore the rest of the tests.
    if (!hasGLSyncExtension())
    {
        EXPECT_EQ(EGL_NO_SYNC_KHR, eglCreateSyncKHR(display, EGL_SYNC_FENCE_KHR, nullptr));
        EXPECT_EGL_ERROR(EGL_BAD_MATCH);
    }

    ANGLE_SKIP_TEST_IF(!hasGLSyncExtension());

    EGLContext context     = eglGetCurrentContext();
    EGLSurface drawSurface = eglGetCurrentSurface(EGL_DRAW);
    EGLSurface readSurface = eglGetCurrentSurface(EGL_READ);

    EXPECT_NE(context, EGL_NO_CONTEXT);
    EXPECT_NE(drawSurface, EGL_NO_SURFACE);
    EXPECT_NE(readSurface, EGL_NO_SURFACE);

    EGLSync sync = eglCreateSyncKHR(display, EGL_SYNC_FENCE_KHR, nullptr);
    EXPECT_NE(sync, EGL_NO_SYNC_KHR);

    EXPECT_EQ(EGL_TRUE, eglDestroySyncKHR(display, sync));

    // CreateSync with empty attribute shouldn't cause an error
    const EGLint emptyAttributes[] = {EGL_NONE};
    sync                           = eglCreateSyncKHR(display, EGL_SYNC_FENCE_KHR, emptyAttributes);
    EXPECT_NE(sync, EGL_NO_SYNC_KHR);

    // DestroySync generates BAD_PARAMETER when the sync is not valid
    EXPECT_EQ(EGL_FALSE, eglDestroySyncKHR(display, reinterpret_cast<EGLsync>(20)));
    EXPECT_EGL_ERROR(EGL_BAD_PARAMETER);

    // CreateSync generates BAD_DISPLAY if display is not valid
    EXPECT_EQ(EGL_NO_SYNC_KHR, eglCreateSyncKHR(EGL_NO_DISPLAY, EGL_SYNC_FENCE_KHR, nullptr));
    EXPECT_EGL_ERROR(EGL_BAD_DISPLAY);

    // CreateSync generates BAD_ATTRIBUTE if attribute is neither nullptr nor empty.
    const EGLint nonEmptyAttributes[] = {
        EGL_CL_EVENT_HANDLE,
        0,
        EGL_NONE,
    };
    EXPECT_EQ(EGL_NO_SYNC_KHR, eglCreateSyncKHR(display, EGL_SYNC_FENCE_KHR, nonEmptyAttributes));
    EXPECT_EGL_ERROR(EGL_BAD_ATTRIBUTE);

    // CreateSync generates BAD_ATTRIBUTE if type is not valid
    EXPECT_EQ(EGL_NO_SYNC_KHR, eglCreateSyncKHR(display, 0, nullptr));
    EXPECT_EGL_ERROR(EGL_BAD_ATTRIBUTE);

    // CreateSync generates BAD_MATCH if no context is current
    eglMakeCurrent(display, EGL_NO_SURFACE, EGL_NO_SURFACE, EGL_NO_CONTEXT);
    EXPECT_EQ(EGL_NO_SYNC_KHR, eglCreateSyncKHR(display, EGL_SYNC_FENCE_KHR, nullptr));
    EXPECT_EGL_ERROR(EGL_BAD_MATCH);
    eglMakeCurrent(display, drawSurface, readSurface, context);

    // ClientWaitSync generates EGL_BAD_PARAMETER if the sync object is not valid
    EXPECT_EQ(EGL_FALSE, eglClientWaitSyncKHR(display, reinterpret_cast<EGLsync>(30), 0, 0));
    EXPECT_EGL_ERROR(EGL_BAD_PARAMETER);

    // TODO: from here

    // glGetSynciv generates GL_INVALID_VALUE if bufSize is less than zero, results should be
    // untouched
    GLsizei length = 20;
    GLint value    = 30;
    glGetSynciv(sync, GL_OBJECT_TYPE, -1, &length, &value);
    EXPECT_GL_ERROR(GL_INVALID_VALUE);
    EXPECT_EQ(20, length);
    EXPECT_EQ(30, value);

    // glGetSynciv generates GL_INVALID_VALUE if the sync object tis not valid, results should be
    // untouched
    glGetSynciv(reinterpret_cast<GLsync>(30), GL_OBJECT_TYPE, 1, &length, &value);
    EXPECT_GL_ERROR(GL_INVALID_VALUE);
    EXPECT_EQ(20, length);
    EXPECT_EQ(30, value);
}

// Test error cases for all EGL_KHR_wait_sync functions
TEST_P(EGLSyncTest, WaitSyncErrors)
{
    ANGLE_SKIP_TEST_IF(!hasWaitSyncExtension());

    EGLDisplay display = getEGLWindow()->getDisplay();

    EGLSync sync = eglCreateSyncKHR(display, EGL_SYNC_FENCE_KHR, nullptr);
    EXPECT_NE(sync, EGL_NO_SYNC_KHR);

    // TODO: from here

    // glWaitSync generates GL_INVALID_VALUE if flags is non-zero
    glWaitSync(sync, 1, GL_TIMEOUT_IGNORED);
    EXPECT_GL_ERROR(GL_INVALID_VALUE);

    // glWaitSync generates GL_INVALID_VALUE if GLuint64 is not GL_TIMEOUT_IGNORED
    glWaitSync(sync, 0, 0);
    EXPECT_GL_ERROR(GL_INVALID_VALUE);

    // glWaitSync generates GL_INVALID_VALUE if the sync object is not valid
    glWaitSync(reinterpret_cast<GLsync>(30), 0, GL_TIMEOUT_IGNORED);
    EXPECT_GL_ERROR(GL_INVALID_VALUE);
}

// TODO: from here
// Test usage of glGetSynciv
TEST_P(EGLSyncTest, BasicQueries)
{
    GLsizei length = 0;
    GLint value    = 0;
    GLsync sync    = glEGLSync(GL_SYNC_GPU_COMMANDS_COMPLETE, 0);

    glGetSynciv(sync, GL_SYNC_CONDITION, 1, &length, &value);
    EXPECT_GL_NO_ERROR();
    EXPECT_EQ(GL_SYNC_GPU_COMMANDS_COMPLETE, value);

    glGetSynciv(sync, GL_OBJECT_TYPE, 1, &length, &value);
    EXPECT_GL_NO_ERROR();
    EXPECT_EQ(GL_SYNC_FENCE, value);

    glGetSynciv(sync, GL_SYNC_FLAGS, 1, &length, &value);
    EXPECT_GL_NO_ERROR();
    EXPECT_EQ(0, value);
}

// Test that basic usage works and doesn't generate errors or crash
TEST_P(EGLSyncTest, BasicOperations)
{
    glClearColor(1.0f, 0.0f, 1.0f, 1.0f);

    GLsync sync = glEGLSync(GL_SYNC_GPU_COMMANDS_COMPLETE, 0);

    glClear(GL_COLOR_BUFFER_BIT);
    glWaitSync(sync, 0, GL_TIMEOUT_IGNORED);
    EXPECT_GL_NO_ERROR();

    GLsizei length         = 0;
    GLint value            = 0;
    unsigned int loopCount = 0;

    glFlush();

    // Use 'loopCount' to make sure the test doesn't get stuck in an infinite loop
    while (value != GL_SIGNALED && loopCount <= 1000000)
    {
        loopCount++;

        glGetSynciv(sync, GL_SYNC_STATUS, 1, &length, &value);
        ASSERT_GL_NO_ERROR();
    }

    ASSERT_GLENUM_EQ(GL_SIGNALED, value);

    for (size_t i = 0; i < 20; i++)
    {
        glClear(GL_COLOR_BUFFER_BIT);
        glClientWaitSync(sync, GL_SYNC_FLUSH_COMMANDS_BIT, GL_TIMEOUT_IGNORED);
        EXPECT_GL_NO_ERROR();
    }
}

// Use this to select which configurations (e.g. which renderer, which GLES major version) these
// tests should be run against.
ANGLE_INSTANTIATE_TEST(FenceNVTest,
                       ES2_D3D9(),
                       ES2_D3D11(),
                       ES3_D3D11(),
                       ES2_OPENGL(),
                       ES3_OPENGL(),
                       ES2_OPENGLES(),
                       ES3_OPENGLES(),
                       ES2_VULKAN());
ANGLE_INSTANTIATE_TEST(EGLSyncTest, ES3_D3D11(), ES3_OPENGL(), ES3_OPENGLES());
