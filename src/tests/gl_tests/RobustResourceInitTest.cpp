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
    RobustResourceInitTest()
    {
        setWindowWidth(128);
        setWindowHeight(128);
        setConfigRedBits(8);
        setConfigGreenBits(8);
        setConfigBlueBits(8);
        setConfigAlphaBits(8);

        // Defer context init until the test body.
        setDeferContextInit(true);
        getEGLWindow()->setRobustResourceInit(true);
    }
};

// Context creation should fail if EGL_ANGLE_create_context_robust_resource_initialization
// is not available, and succeed otherwise.
TEST_P(RobustResourceInitTest, ExtensionInit)
{
    EGLDisplay display = getEGLWindow()->getDisplay();
    ASSERT(display != EGL_NO_DISPLAY);

    if (eglDisplayExtensionEnabled(display,
                                   "EGL_ANGLE_create_context_robust_resource_initialization"))
    {
        EXPECT_TRUE(getEGLWindow()->initializeContext());

        GLboolean enabled = 0;
        glGetBooleanv(GL_CONTEXT_ROBUST_RESOURCE_INITIALIZATION_ANGLE, &enabled);
        EXPECT_GL_NO_ERROR();
        EXPECT_GL_TRUE(enabled);
    }
    else
    {
        EXPECT_FALSE(getEGLWindow()->initializeContext());
    }
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
