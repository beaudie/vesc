//
// Copyright 2020 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//
// PolygonOffsetClampTest.cpp: Test cases for GL_EXT_polygon_offset_clamp
//

#include "test_utils/ANGLETest.h"
#include "test_utils/gl_raii.h"
#include "util/EGLWindow.h"
#include "util/test_utils.h"

using namespace angle;

class PolygonOffsetClampTest : public ANGLETest<>
{
  protected:
    PolygonOffsetClampTest()
    {
        setConfigRedBits(8);
        setConfigGreenBits(8);
        setConfigBlueBits(8);
        setConfigAlphaBits(8);
        setConfigDepthBits(24);
        setExtensionsEnabled(false);
    }
};

// Test state queries and updates
TEST_P(PolygonOffsetClampTest, State)
{
    // New state query fails without the extension
    {
        GLfloat clamp = -1.0f;
        glGetFloatv(GL_POLYGON_OFFSET_CLAMP_EXT, &clamp);
        EXPECT_GL_ERROR(GL_INVALID_ENUM);
        EXPECT_EQ(clamp, -1.0f);

        ASSERT_GL_NO_ERROR();
    }

    // New function does nothing without enabling the extension
    {
        glPolygonOffsetClampEXT(1.0f, 2.0f, 3.0f);
        EXPECT_GL_ERROR(GL_INVALID_OPERATION);

        GLfloat factor = -1.0f;
        glGetFloatv(GL_POLYGON_OFFSET_FACTOR, &factor);
        EXPECT_EQ(factor, 0.0f);

        GLfloat units = -1.0f;
        glGetFloatv(GL_POLYGON_OFFSET_UNITS, &units);
        EXPECT_EQ(units, 0.0f);

        ASSERT_GL_NO_ERROR();
    }

    ANGLE_SKIP_TEST_IF(!EnsureGLExtensionEnabled("GL_EXT_polygon_offset_clamp"));

    // Default state
    {
        GLfloat factor = -1.0f;
        glGetFloatv(GL_POLYGON_OFFSET_FACTOR, &factor);
        EXPECT_EQ(factor, 0.0f);

        GLfloat units = -1.0f;
        glGetFloatv(GL_POLYGON_OFFSET_UNITS, &units);
        EXPECT_EQ(units, 0.0f);

        GLfloat clamp = -1.0f;
        glGetFloatv(GL_POLYGON_OFFSET_CLAMP_EXT, &clamp);
        EXPECT_EQ(clamp, 0.0f);

        ASSERT_GL_NO_ERROR();
    }

    // Full state update using the new function
    {
        glPolygonOffsetClampEXT(1.0f, 2.0f, 3.0f);

        GLfloat factor = -1.0f;
        glGetFloatv(GL_POLYGON_OFFSET_FACTOR, &factor);
        EXPECT_EQ(factor, 1.0f);

        GLfloat units = -1.0f;
        glGetFloatv(GL_POLYGON_OFFSET_UNITS, &units);
        EXPECT_EQ(units, 2.0f);

        GLfloat clamp = -1.0f;
        glGetFloatv(GL_POLYGON_OFFSET_CLAMP_EXT, &clamp);
        EXPECT_EQ(clamp, 3.0f);

        ASSERT_GL_NO_ERROR();
    }

    // Core function resets the clamp value to zero
    {
        glPolygonOffset(4.0f, 5.0f);

        GLfloat factor = -1.0f;
        glGetFloatv(GL_POLYGON_OFFSET_FACTOR, &factor);
        EXPECT_EQ(factor, 4.0f);

        GLfloat units = -1.0f;
        glGetFloatv(GL_POLYGON_OFFSET_UNITS, &units);
        EXPECT_EQ(units, 5.0f);

        GLfloat clamp = -1.0f;
        glGetFloatv(GL_POLYGON_OFFSET_CLAMP_EXT, &clamp);
        EXPECT_EQ(clamp, 0.0f);

        ASSERT_GL_NO_ERROR();
    }

    // NaNs must be accepted and replaced with zeros
    {
        glPolygonOffsetClampEXT(6.0f, 7.0f, 8.0f);

        float nan = std::numeric_limits<float>::quiet_NaN();
        glPolygonOffsetClampEXT(nan, nan, nan);

        GLfloat factor = -1.0f;
        glGetFloatv(GL_POLYGON_OFFSET_FACTOR, &factor);
        EXPECT_EQ(factor, 0.0f);

        GLfloat units = -1.0f;
        glGetFloatv(GL_POLYGON_OFFSET_UNITS, &units);
        EXPECT_EQ(units, 0.0f);

        GLfloat clamp = -1.0f;
        glGetFloatv(GL_POLYGON_OFFSET_CLAMP_EXT, &clamp);
        EXPECT_EQ(clamp, 0.0f);

        ASSERT_GL_NO_ERROR();
    }
}

ANGLE_INSTANTIATE_TEST_ES2_AND_ES3(PolygonOffsetClampTest);
