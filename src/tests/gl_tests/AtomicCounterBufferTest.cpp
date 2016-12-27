//
// Copyright 2017 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//

#include "test_utils/ANGLETest.h"
#include "test_utils/gl_raii.h"

using namespace angle;

namespace
{

class AtomicCounterBufferTest : public ANGLETest
{
  protected:
    AtomicCounterBufferTest()
    {
        setWindowWidth(128);
        setWindowHeight(128);
        setConfigRedBits(8);
        setConfigGreenBits(8);
        setConfigBlueBits(8);
        setConfigAlphaBits(8);
    }
    void SetUp() override
    {
        ANGLETest::SetUp();

        glGenBuffers(1, &mAtomicCounterBuffer);

        ASSERT_GL_NO_ERROR();
    }

    void TearDown() override
    {
        glDeleteBuffers(1, &mAtomicCounterBuffer);
        ANGLETest::TearDown();
    }

    GLuint mAtomicCounterBuffer;
};

// Test GL_ATOMIC_COUNTER_BUFFER is not supported with version lower than ES31.
TEST_P(AtomicCounterBufferTest, AtomicCounterBufferBindings)
{
    if (getClientMajorVersion() == 3 && getClientMinorVersion() < 1)
    {
        glBindBufferBase(GL_ATOMIC_COUNTER_BUFFER, 1, mAtomicCounterBuffer);
        EXPECT_GL_ERROR(GL_INVALID_ENUM);
    }
    else
    {
        glBindBufferBase(GL_ATOMIC_COUNTER_BUFFER, 1, mAtomicCounterBuffer);
        EXPECT_GL_NO_ERROR();
    }
}

ANGLE_INSTANTIATE_TEST(AtomicCounterBufferTest,
                       ES3_OPENGL(),
                       ES3_OPENGLES(),
                       ES31_OPENGL(),
                       ES31_OPENGLES());

}  // namespace
