//
// Copyright 2016 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//

// SyncQueriesTest.cpp: Tests of the GL_CHROMIUM_sync_query extension

#include "test_utils/ANGLETest.h"

namespace angle
{

class SyncQueriesTest : public ANGLETest
{
  protected:
    SyncQueriesTest()
    {
        setWindowWidth(128);
        setWindowHeight(128);
        setConfigRedBits(8);
        setConfigGreenBits(8);
        setConfigBlueBits(8);
        setConfigAlphaBits(8);
        setConfigDepthBits(24);
    }
};

// Test basic usage of sync queries
TEST_P(SyncQueriesTest, Basic)
{
    if (!extensionEnabled("GL_CHROMIUM_sync_query"))
    {
        std::cout << "Test skipped because GL_CHROMIUM_sync_query is not available." << std::endl;
        return;
    }

    GLuint query;
    glGenQueriesEXT(1, &query);
    glBeginQueryEXT(GL_COMMANDS_COMPLETED_CHROMIUM, query);
    EXPECT_GL_NO_ERROR();

    glClearColor(0.0, 0.0, 1.0, 1.0);
    glClear(GL_COLOR_BUFFER_BIT);

    glEndQueryEXT(GL_COMMANDS_COMPLETED_CHROMIUM);

    glFlush();
    GLuint result = 0;
    glGetQueryObjectuivEXT(query, GL_QUERY_RESULT_EXT, &result);
    EXPECT_EQ(GL_TRUE, result);
    EXPECT_GL_NO_ERROR();

    glDeleteQueries(1, &query);

    EXPECT_GL_NO_ERROR();
}

// Test that the sync query enums are not accepted unless the extension is available
TEST_P(SyncQueriesTest, Validation)
{
    // Need the GL_EXT_occlusion_query_boolean extension for the entry points
    if (!extensionEnabled("GL_EXT_occlusion_query_boolean"))
    {
        std::cout << "Test skipped because GL_EXT_occlusion_query_boolean is not available."
                  << std::endl;
        return;
    }

    bool extensionAvailable = extensionEnabled("GL_CHROMIUM_sync_query");

    GLuint query;
    glGenQueriesEXT(1, &query);

    glBeginQueryEXT(GL_COMMANDS_COMPLETED_CHROMIUM, query);
    if (extensionAvailable)
    {
        EXPECT_GL_NO_ERROR();
    }
    else
    {
        EXPECT_GL_ERROR(GL_INVALID_ENUM);
    }

    glDeleteQueries(1, &query);

    EXPECT_GL_NO_ERROR();
}

// Use this to select which configurations (e.g. which renderer, which GLES major version) these
// tests should be run against.
ANGLE_INSTANTIATE_TEST(SyncQueriesTest,
                       ES2_D3D9(),
                       ES2_D3D11(),
                       ES3_D3D11(),
                       ES2_OPENGL(),
                       ES3_OPENGL(),
                       ES2_OPENGLES(),
                       ES3_OPENGLES());

}  // namespace angle
