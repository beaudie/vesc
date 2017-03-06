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

    bool hasExtension()
    {
        EGLDisplay display = getEGLWindow()->getDisplay();
        ASSERT(display != EGL_NO_DISPLAY);

        return (eglDisplayExtensionEnabled(
            display, "EGL_ANGLE_create_context_robust_resource_initialization"));
    }

    bool setup()
    {
        if (!hasExtension())
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
};

// Context creation should fail if EGL_ANGLE_create_context_robust_resource_initialization
// is not available, and succeed otherwise.
TEST_P(RobustResourceInitTest, ExtensionInit)
{
    if (hasExtension())
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

// Tests that buffers start zero-filled if the data pointer is null.
TEST_P(RobustResourceInitTest, BufferData)
{
    if (!setup() || !(extensionEnabled("GL_EXT_map_buffer_range") || getClientMajorVersion() < 3))
    {
        return;
    }

    GLBuffer buffer;
    glBindBuffer(GL_ARRAY_BUFFER, buffer);
    glBufferData(GL_ARRAY_BUFFER, 128, nullptr, GL_STATIC_DRAW);

    void *mappedPointer = nullptr;

    if (getClientMajorVersion() >= 3)
    {
        mappedPointer = glMapBufferRange(GL_ARRAY_BUFFER, 0, 128, GL_MAP_READ_BIT);
    }
    else
    {
        mappedPointer = glMapBufferRangeEXT(GL_ARRAY_BUFFER, 0, 128, GL_MAP_READ_BIT);
    }

    ASSERT_GL_NO_ERROR();
    ASSERT_NE(nullptr, mappedPointer);

    std::vector<uint8_t> expected(128, 0);
    std::vector<uint8_t> actual(128);
    memcpy(actual.data(), mappedPointer, 128);

    EXPECT_EQ(expected, actual);
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
