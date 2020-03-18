//
// Copyright 2020 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//
// PrimtestTests.cpp:
//   GLES1 conformance primtest tests.
//   Function prototypes taken from tproto.h and turned into gtest tests using a macro.
//

#include "GLES/gl.h"
#include "test_utils/ANGLETest.h"
#include "test_utils/gl_raii.h"

#ifdef __cplusplus
extern "C" {
#endif

// ES 1.0
extern void DrawPrims(void);

#include "primtest/driver.h"
#include "primtest/tproto.h"

#ifdef __cplusplus
}

#endif

namespace angle
{
class GLES1PrimtestTest : public ANGLETest
{
  protected:
    GLES1PrimtestTest()
    {
        setWindowWidth(48);
        setWindowHeight(48);
        setConfigRedBits(8);
        setConfigGreenBits(8);
        setConfigBlueBits(8);
        setConfigAlphaBits(8);
        setConfigDepthBits(24);
        setConfigStencilBits(8);
    }

    int execTest(long test)
    {
        int ret = 1;
        long i;
        for (i = 0; driver[i].test != TEST_NULL; i++)
        {
            if (driver[i].test == test)
            {
                break;
            }
        }

        if (driver[i].test == TEST_NULL)
        {
            return 1;
        }

        driverRec &op = driver[i];

        void *data;
        op.funcInit((void *)&data);

        for (;;)
        {
            if (glGetError() != GL_NO_ERROR)
            {
                goto err;
            }

            op.funcStatus(1, data);
            if (glGetError() != GL_NO_ERROR)
            {
                goto err;
            }

            op.funcSet(1, data);
            if (glGetError() != GL_NO_ERROR)
            {
                goto err;
            }

            DrawPrims();
            if (glGetError() != GL_NO_ERROR)
            {
                goto err;
            }

            long finish = op.funcUpdate(data);
            if (glGetError() != GL_NO_ERROR)
            {
                goto err;
            }
            if (finish)
            {
                break;
            }
        };

        ret = 0;
    err:
        if (data != nullptr)
        {
            free(data);
        }
        return ret;
    }
};

TEST_P(GLES1PrimtestTest, Hint)
{
    ASSERT_EQ(0, execTest(TEST_HINT));
}

TEST_P(GLES1PrimtestTest, Alias)
{
    ASSERT_EQ(0, execTest(TEST_ALIAS));
}

TEST_P(GLES1PrimtestTest, Alpha)
{
    ASSERT_EQ(0, execTest(TEST_ALPHA));
}

TEST_P(GLES1PrimtestTest, Blend)
{
    ASSERT_EQ(0, execTest(TEST_BLEND));
}

TEST_P(GLES1PrimtestTest, Depth)
{
    ASSERT_EQ(0, execTest(TEST_DEPTH));
}

TEST_P(GLES1PrimtestTest, Dither)
{
    ASSERT_EQ(0, execTest(TEST_DITHER));
}

TEST_P(GLES1PrimtestTest, Fog)
{
    ASSERT_EQ(0, execTest(TEST_FOG));
}

TEST_P(GLES1PrimtestTest, Light)
{
    ASSERT_EQ(0, execTest(TEST_LIGHT));
}

TEST_P(GLES1PrimtestTest, Logic)
{
    ASSERT_EQ(0, execTest(TEST_LOGICOP));
}

TEST_P(GLES1PrimtestTest, Scissor)
{
    ASSERT_EQ(0, execTest(TEST_SCISSOR));
}

TEST_P(GLES1PrimtestTest, Shade)
{
    ASSERT_EQ(0, execTest(TEST_SHADE));
}

TEST_P(GLES1PrimtestTest, Stencil)
{
    ASSERT_EQ(0, execTest(TEST_STENCIL));
}

TEST_P(GLES1PrimtestTest, Texture)
{
    ASSERT_EQ(0, execTest(TEST_TEXTURE));
}

ANGLE_INSTANTIATE_TEST(GLES1PrimtestTest, ES1_OPENGL(), ES1_VULKAN());
}  // namespace angle

// Included here to fix a compile error due to white box tests using angle_end2end_tests_main.
void RegisterContextCompatibilityTests() {}