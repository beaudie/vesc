//
// Copyright 2017 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//

// ExtensionsTest.cpp : Tests generic aspects of extensions.

#include "test_utils/ANGLETest.h"

namespace angle
{

class ExtensionsTest : public ANGLETest
{
};

// Verify that GL_EXT_sRGB can only be available in ES2
TEST_P(ExtensionsTest, GL_EXT_sRGB_ES2Only)
{
    if (getClientMajorVersion() != 2)
    {
        EXPECT_FALSE(extensionEnabled("GL_EXT_sRGB"));
    }
}

// Verify that GL_EXT_texture_norm16 can only be available in ES3 and up
TEST_P(ExtensionsTest, GL_EXT_texture_norm16_ES3AndAbove)
{
    if (getClientMajorVersion() < 3)
    {
        EXPECT_FALSE(extensionEnabled("GL_EXT_texture_norm16"));
    }
}

// Verify that GL_EXT_color_buffer_float can only be available in ES3 and up
TEST_P(ExtensionsTest, GL_EXT_color_buffer_float_ES3AndAbove)
{
    if (getClientMajorVersion() < 3)
    {
        EXPECT_FALSE(extensionEnabled("GL_EXT_color_buffer_float"));
    }
}

// Verify that GL_OES_EGL_image_external_essl3 can only be available in ES3 and up
TEST_P(ExtensionsTest, GL_OES_EGL_image_external_essl3_ES3AndAbove)
{
    if (getClientMajorVersion() < 3)
    {
        EXPECT_FALSE(extensionEnabled("GL_OES_EGL_image_external_essl3"));
    }
}

// Use this to select which configurations (e.g. which renderer, which GLES major version) these
// tests should be run against.
ANGLE_INSTANTIATE_TEST(ExtensionsTest,
                       ES2_D3D9(),
                       ES2_D3D11(),
                       ES3_D3D11(),
                       ES31_D3D11(),
                       ES2_D3D11_FL9_3(),
                       ES2_OPENGL(),
                       ES3_OPENGL(),
                       ES31_OPENGL(),
                       ES2_OPENGLES(),
                       ES3_OPENGLES(),
                       ES31_OPENGLES());

}  // namespace angle
