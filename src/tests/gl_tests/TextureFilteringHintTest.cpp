//
// Copyright 2020 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//

// TextureFilteringHintTest.cpp : Tests of the GL_CHROMIUM_texture_filtering_hint extension.

#include "test_utils/ANGLETest.h"

#include "test_utils/gl_raii.h"

namespace angle
{

class TextureFilteringHintTest : public ANGLETest
{
  protected:
    TextureFilteringHintTest()
    {
        setWindowWidth(128);
        setWindowHeight(128);
        setConfigRedBits(8);
        setConfigGreenBits(8);
        setConfigBlueBits(8);
        setConfigAlphaBits(8);
    }
};

TEST_P(TextureFilteringHintTest, Validation)
{
    ANGLE_SKIP_TEST_IF(!IsGLExtensionEnabled("GL_CHROMIUM_texture_filtering_hint"));

    GLint intValue = 0;
    glGetIntegerv(GL_TEXTURE_FILTERING_HINT_CHROMIUM, &intValue);
    EXPECT_GL_NO_ERROR();
    EXPECT_GL_TRUE(intValue == GL_DONT_CARE);

    glHint(GL_TEXTURE_FILTERING_HINT_CHROMIUM, GL_FASTEST);
    glGetIntegerv(GL_TEXTURE_FILTERING_HINT_CHROMIUM, &intValue);
    EXPECT_GL_NO_ERROR();
    EXPECT_GL_TRUE(intValue == GL_FASTEST);

    glHint(GL_TEXTURE_FILTERING_HINT_CHROMIUM, GL_NICEST);
    glGetIntegerv(GL_TEXTURE_FILTERING_HINT_CHROMIUM, &intValue);
    EXPECT_GL_NO_ERROR();
    EXPECT_GL_TRUE(intValue == GL_NICEST);
}

// Use this to select which configurations (e.g. which renderer, which GLES major version) these
// tests should be run against.
ANGLE_INSTANTIATE_TEST_ES2_AND_ES3(TextureFilteringHintTest);
}  // namespace angle
