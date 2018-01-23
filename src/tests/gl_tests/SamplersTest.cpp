//
// Copyright 2017 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//

// SamplerTest.cpp : Tests for samplers.

#include "test_utils/ANGLETest.h"

#include "test_utils/gl_raii.h"

namespace angle
{

class SamplersTest : public ANGLETest
{
  protected:
    SamplersTest()
    {
    }

    // Sets a value for GL_TEXTURE_MAX_ANISOTROPY_EXT and expects it to fail.
    void validateInvalidAnisotropy(GLSampler &sampler, float invalidValue)
    {
        glSamplerParameterf(sampler, GL_TEXTURE_MAX_ANISOTROPY_EXT, invalidValue);
        EXPECT_GL_ERROR(GL_INVALID_VALUE);
    }

    // Sets a value for GL_TEXTURE_MAX_ANISOTROPY_EXT and expects it to work.
    void validateValidAnisotropy(GLSampler &sampler, float validValue)
    {
        glSamplerParameterf(sampler, GL_TEXTURE_MAX_ANISOTROPY_EXT, validValue);
        EXPECT_GL_NO_ERROR();

        GLfloat valueToVerify;
        glGetSamplerParameterfv(sampler, GL_TEXTURE_MAX_ANISOTROPY_EXT, &valueToVerify);
        ASSERT_EQ(valueToVerify, validValue);
    }
};

// Verify that samplerParameterf supports TEXTURE_MAX_ANISOTROPY_EXT correctly.
TEST_P(SamplersTest, TextureSamplerMaxAnisotropyExt)
{
    GLSampler sampler;

    // Exact min
    validateValidAnisotropy(sampler, 1.0f);

    // In-between value
    validateValidAnisotropy(sampler, 5.0f);

    // Approx max
    validateValidAnisotropy(sampler, 16.0f);

    // Under min
    validateInvalidAnisotropy(sampler, 0.0f);

    // Over max
    validateInvalidAnisotropy(sampler, 1000000.0f);
}

// Use this to select which configurations (e.g. which renderer, which GLES major version) these
// tests should be run against.
// Samplers are only supported on ES3.
ANGLE_INSTANTIATE_TEST(SamplersTest,
                       ES3_D3D11(),
                       ES3_OPENGL(),
                       ES3_OPENGLES());
}  // namespace
