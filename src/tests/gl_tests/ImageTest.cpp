//
// Copyright 2015 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//
// PackUnpackTest:
//   Tests the correctness of eglImage.
//

#include "test_utils/ANGLETest.h"

namespace angle
{

class ImageTest : public ANGLETest
{
protected:
    ImageTest()
    {
        setWindowWidth(128);
        setWindowHeight(128);
        setConfigRedBits(8);
        setConfigGreenBits(8);
        setConfigBlueBits(8);
        setConfigAlphaBits(8);
        setConfigDepthBits(24);
    }

    void SetUp() override
    {
        ANGLETest::SetUp();
    }

    void TearDown() override
    {
        ANGLETest::TearDown();
    }
};

TEST_P(ImageTest, NULL)
{
}

// Use this to select which configurations (e.g. which renderer, which GLES major version) these tests should be run against.
ANGLE_INSTANTIATE_TEST(ImageTest, ES2_D3D9(), ES2_D3D11(), ES2_OPENGL(), ES3_OPENGL());

}
