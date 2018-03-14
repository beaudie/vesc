//
// Copyright 2018 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//
// RequestExtensionTest:
//   Tests that extensions can be requested and are disabled by default when using
//   EGL_ANGLE_request_extension
//

#include "test_utils/ANGLETest.h"

namespace angle
{

class RequestExtensionTest : public ANGLETest
{
  protected:
    RequestExtensionTest() { setExtensionsEnabled(false); }
};

TEST_P(RequestExtensionTest, ExtensionsDisabledByDefault)
{
    EXPECT_FALSE(extensionEnabled("GL_OES_rgb8_rgba8"));

    if (extensionRequestable("GL_OES_rgb8_rgba8"))
    {
        glRequestExtensionANGLE("GL_OES_rgb8_rgba8");
        EXPECT_TRUE(extensionEnabled("GL_OES_rgb8_rgba8"));
    }
}

// Use this to select which configurations (e.g. which renderer, which GLES major version) these
// tests should be run against.
ANGLE_INSTANTIATE_TEST(RequestExtensionTest,
                       ES2_D3D11(),
                       ES2_OPENGL(),
                       ES2_OPENGLES(),
                       ES2_VULKAN());

}  // namespace angle
