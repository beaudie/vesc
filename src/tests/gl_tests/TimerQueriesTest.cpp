//
// Copyright 2016 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//

#include "system_utils.h"
#include "test_utils/ANGLETest.h"
#include "random_utils.h"

using namespace angle;

class TimerQueriesTest : public ANGLETest
{
  protected:
    TimerQueriesTest()
    {
        setWindowWidth(128);
        setWindowHeight(128);
        setConfigRedBits(8);
        setConfigGreenBits(8);
        setConfigBlueBits(8);
        setConfigAlphaBits(8);
        setConfigDepthBits(24);

        mProgram = 0;
    }

    virtual void SetUp()
    {
        ANGLETest::SetUp();

        const std::string passthroughVS =
            SHADER_SOURCE(attribute highp vec4 position; void main(void)
                          {
                              gl_Position = position;
                          });

        const std::string passthroughPS = SHADER_SOURCE(precision highp float; void main(void)
                                                        {
                                                            gl_FragColor = vec4(1.0, 1.0, 1.0, 1.0);
                                                        });

        mProgram = CompileProgram(passthroughVS, passthroughPS);
        if (mProgram == 0)
        {
            FAIL() << "shader compilation failed.";
        }
    }

    virtual void TearDown()
    {
        glDeleteProgram(mProgram);

        ANGLETest::TearDown();
    }

    GLuint mProgram;
};

TEST_P(TimerQueriesTest, TimeElapsed)
{
    if (!extensionEnabled("GL_EXT_disjoint_timer_query"))
    {
        std::cout << "Test skipped because GL_EXT_disjoint_timer_query is not available."
                  << std::endl;
        return;
    }

    glDepthMask(GL_TRUE);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);

    GLuint query = 0;
    glGenQueriesEXT(1, &query);
    glBeginQueryEXT(GL_TIME_ELAPSED_EXT, query);
    drawQuad(mProgram, "position", 0.8f);
    glEndQueryEXT(GL_TIME_ELAPSED_EXT);

    EXPECT_GL_NO_ERROR();

    swapBuffers();

    GLuint ready = GL_FALSE;
    while (ready == GL_FALSE)
    {
        angle::Sleep(0);
        glGetQueryObjectuivEXT(query, GL_QUERY_RESULT_AVAILABLE_EXT, &ready);
    }

    GLuint result = GL_TRUE;
    glGetQueryObjectuivEXT(query, GL_QUERY_RESULT_EXT, &result);

    EXPECT_GL_NO_ERROR();

    glDeleteQueriesEXT(1, &query);

    std::cout << "Elapsed time: " << result << std::endl;
    EXPECT_GT(result, (GLuint)0);
}

TEST_P(TimerQueriesTest, ValidationTest)
{
    if (!extensionEnabled("GL_EXT_disjoint_timer_query"))
    {
        std::cout << "Test skipped because GL_EXT_disjoint_timer_query is not available."
                  << std::endl;
        return;
    }

    GLuint query = 0;
    glGenQueriesEXT(-1, &query);
    EXPECT_GL_ERROR(GL_INVALID_VALUE);

    glGenQueriesEXT(1, &query);
    EXPECT_GL_NO_ERROR();

    glBeginQueryEXT(GL_TIMESTAMP_EXT, query);
    EXPECT_GL_ERROR(GL_INVALID_ENUM);

    glBeginQueryEXT(GL_TIME_ELAPSED_EXT, 0);
    EXPECT_GL_ERROR(GL_INVALID_OPERATION);

    glEndQueryEXT(GL_TIME_ELAPSED_EXT);
    EXPECT_GL_ERROR(GL_INVALID_OPERATION);

    glBeginQueryEXT(GL_TIME_ELAPSED_EXT, query);
    EXPECT_GL_NO_ERROR();

    glBeginQueryEXT(GL_TIME_ELAPSED_EXT, query);
    EXPECT_GL_ERROR(GL_INVALID_OPERATION);

    glEndQueryEXT(GL_TIME_ELAPSED_EXT);
    EXPECT_GL_NO_ERROR();
}

ANGLE_INSTANTIATE_TEST(TimerQueriesTest,
                       ES2_D3D9(),
                       ES2_D3D11(),
                       ES3_D3D11(),
                       ES2_OPENGL(),
                       ES3_OPENGL());
