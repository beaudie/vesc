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

        mProgram       = 0;
        mProgramCostly = 0;
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

        const std::string costlyVS =
            SHADER_SOURCE(attribute highp vec4 position; varying highp vec4 testPos; void main(void)
                          {
                              testPos     = position;
                              gl_Position = position;
                          });

        const std::string costlyPS =
            SHADER_SOURCE(precision highp float; varying highp vec4 testPos; void main(void)
                          {
                              vec4 test = testPos;
                              for (int i = 0; i < 500; i++)
                              {
                                  test = sqrt(test);
                              }
                              gl_FragColor = test;
                          });

        mProgram = CompileProgram(passthroughVS, passthroughPS);
        if (mProgram == 0)
        {
            FAIL() << "shader compilation failed.";
        }

        mProgramCostly = CompileProgram(costlyVS, costlyPS);
        if (mProgramCostly == 0)
        {
            FAIL() << "shader compilation failed.";
        }
    }

    virtual void TearDown()
    {
        glDeleteProgram(mProgram);
        glDeleteProgram(mProgramCostly);
        ANGLETest::TearDown();
    }

    GLuint mProgram;
    GLuint mProgramCostly;
};

// Tests the time elapsed query
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

    GLuint query1 = 0;
    GLuint query2 = 0;
    glGenQueriesEXT(1, &query1);
    glGenQueriesEXT(1, &query2);

    // Test time elapsed for a single quad
    glBeginQueryEXT(GL_TIME_ELAPSED_EXT, query1);
    drawQuad(mProgram, "position", 0.8f);
    glEndQueryEXT(GL_TIME_ELAPSED_EXT);
    EXPECT_GL_NO_ERROR();

    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);

    // Test time elapsed for costly quad
    glBeginQueryEXT(GL_TIME_ELAPSED_EXT, query2);
    drawQuad(mProgramCostly, "position", 0.8f);
    glEndQueryEXT(GL_TIME_ELAPSED_EXT);
    EXPECT_GL_NO_ERROR();

    swapBuffers();

    int timeout  = 10000;
    GLuint ready = GL_FALSE;
    while (ready == GL_FALSE && timeout > 0)
    {
        angle::Sleep(0);
        glGetQueryObjectuivEXT(query1, GL_QUERY_RESULT_AVAILABLE_EXT, &ready);
        timeout--;
    }
    ready = GL_FALSE;
    while (ready == GL_FALSE && timeout > 0)
    {
        angle::Sleep(0);
        glGetQueryObjectuivEXT(query2, GL_QUERY_RESULT_AVAILABLE_EXT, &ready);
        timeout--;
    }
    if (timeout <= 0)
    {
        FAIL() << "Query result available timed out" << std::endl;
    }

    GLuint64 result1 = 0;
    GLuint64 result2 = 0;
    glGetQueryObjectui64vEXT(query1, GL_QUERY_RESULT_EXT, &result1);
    glGetQueryObjectui64vEXT(query2, GL_QUERY_RESULT_EXT, &result2);
    EXPECT_GL_NO_ERROR();

    glDeleteQueriesEXT(1, &query1);
    glDeleteQueriesEXT(1, &query2);
    EXPECT_GL_NO_ERROR();

    std::cout << "Elapsed time: " << result1 << " cheap quad" << std::endl;
    std::cout << "Elapsed time: " << result2 << " costly quad" << std::endl;

    // The time elapsed should be nonzero
    EXPECT_GT(result1, (GLuint64)0);
    EXPECT_GT(result2, (GLuint64)0);

    // The costly quad should take longer than the cheap quad
    EXPECT_GT(result2, result1);
}

// Tests validation of query functions with respect to elapsed time query
TEST_P(TimerQueriesTest, TimeElapsedValidationTest)
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

    glEndQueryEXT(GL_TIME_ELAPSED_EXT);
    EXPECT_GL_ERROR(GL_INVALID_OPERATION);
}

// Tests GPU timestamp functionality
TEST_P(TimerQueriesTest, Timestamp)
{
    if (!extensionEnabled("GL_EXT_disjoint_timer_query"))
    {
        std::cout << "Test skipped because GL_EXT_disjoint_timer_query is not available."
                  << std::endl;
        return;
    }

    glDepthMask(GL_TRUE);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);

    GLuint query1 = 0;
    GLuint query2 = 0;
    glGenQueriesEXT(1, &query1);
    glGenQueriesEXT(1, &query2);
    glQueryCounterEXT(query1, GL_TIMESTAMP_EXT);
    drawQuad(mProgram, "position", 0.8f);
    glQueryCounterEXT(query2, GL_TIMESTAMP_EXT);

    EXPECT_GL_NO_ERROR();

    swapBuffers();

    int timeout  = 10000;
    GLuint ready = GL_FALSE;
    while (ready == GL_FALSE && timeout > 0)
    {
        angle::Sleep(0);
        glGetQueryObjectuivEXT(query1, GL_QUERY_RESULT_AVAILABLE_EXT, &ready);
        timeout--;
    }
    ready = GL_FALSE;
    while (ready == GL_FALSE && timeout > 0)
    {
        angle::Sleep(0);
        glGetQueryObjectuivEXT(query2, GL_QUERY_RESULT_AVAILABLE_EXT, &ready);
        timeout--;
    }
    if (timeout <= 0)
    {
        FAIL() << "Query result available timed out" << std::endl;
    }

    GLuint64 result1 = 0;
    GLuint64 result2 = 0;
    glGetQueryObjectui64vEXT(query1, GL_QUERY_RESULT_EXT, &result1);
    glGetQueryObjectui64vEXT(query2, GL_QUERY_RESULT_EXT, &result2);

    EXPECT_GL_NO_ERROR();

    glDeleteQueriesEXT(1, &query1);
    glDeleteQueriesEXT(1, &query2);

    std::cout << "Timestamps: " << result1 << " " << result2 << std::endl;
    EXPECT_GT(result1, (GLuint64)0);
    EXPECT_GT(result2, (GLuint64)0);
    EXPECT_GT(result2, result1);
}

ANGLE_INSTANTIATE_TEST(TimerQueriesTest,
                       ES2_D3D9(),
                       ES2_D3D11(),
                       ES3_D3D11(),
                       ES2_OPENGL(),
                       ES3_OPENGL());
