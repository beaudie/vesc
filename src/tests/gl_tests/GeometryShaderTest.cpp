//
// Copyright 2017 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//

// GeometryShaderTest.cpp : Tests of the implementation of geometry shader

#include "test_utils/ANGLETest.h"

using namespace angle;

namespace
{

class GeometryShaderTest : public ANGLETest
{
  protected:
    static bool ValidateGeometryShaderLimits(GLenum constantEnum, int minimunValue)
    {
        GLint value = 0;
        glGetIntegerv(constantEnum, &value);
        EXPECT_GL_NO_ERROR();
        return value >= minimunValue;
    }
};

// Verify that Geometry Shader constants can be queried on supported platforms.
TEST_P(GeometryShaderTest, QueryConstants)
{
    ANGLE_SKIP_TEST_IF(!extensionEnabled("GL_OES_geometry_shader"));

    const std::map<GLenum, int> kGeometryShaderLimits = {
        {GL_MAX_FRAMEBUFFER_LAYERS_OES, 256},
        {GL_MAX_GEOMETRY_UNIFORM_COMPONENTS_OES, 1024},
        {GL_MAX_GEOMETRY_UNIFORM_BLOCKS_OES, 12},
        {GL_MAX_GEOMETRY_INPUT_COMPONENTS_OES, 64},
        {GL_MAX_GEOMETRY_OUTPUT_COMPONENTS_OES, 64},
        {GL_MAX_GEOMETRY_OUTPUT_VERTICES_OES, 256},
        {GL_MAX_GEOMETRY_TOTAL_OUTPUT_COMPONENTS_OES, 1024},
        {GL_MAX_GEOMETRY_TEXTURE_IMAGE_UNITS_OES, 16},
        {GL_MAX_GEOMETRY_ATOMIC_COUNTER_BUFFERS_OES, 0},
        {GL_MAX_GEOMETRY_ATOMIC_COUNTERS_OES, 0},
        {GL_MAX_GEOMETRY_SHADER_STORAGE_BLOCKS_OES, 0},
        {GL_MAX_GEOMETRY_SHADER_INVOCATIONS_OES, 32},
        {GL_MAX_GEOMETRY_IMAGE_UNIFORMS_OES, 0}};

    for (const auto &resourceLimit : kGeometryShaderLimits)
    {
        EXPECT_TRUE(ValidateGeometryShaderLimits(resourceLimit.first, resourceLimit.second));
    }
}

ANGLE_INSTANTIATE_TEST(GeometryShaderTest, ES31_OPENGL(), ES31_OPENGLES(), ES31_D3D11());
}
