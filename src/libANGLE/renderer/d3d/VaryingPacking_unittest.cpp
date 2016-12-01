//
// Copyright 2016 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//
// VaryingPacking_unittest.cpp:
//   Tests for ANGLE's internal varying packing algorithm.
//

#include <gtest/gtest.h>
#include "libANGLE/renderer/d3d/VaryingPacking.h"
#include "libANGLE/Program.h"

namespace
{

class VaryingPackingTest : public ::testing::Test
{
  protected:
    VaryingPackingTest() {}

    bool testPackVaryings(GLuint maxVaryings, const std::vector<sh::Varying> &shVaryings)
    {
        std::vector<rx::PackedVarying> packedVaryings;
        for (const auto &shVarying : shVaryings)
        {
            packedVaryings.push_back(rx::PackedVarying(shVarying, shVarying.interpolation));
        }

        gl::InfoLog infoLog;
        std::vector<std::string> transformFeedbackVaryings;
        rx::VaryingPacking varyingPacking(maxVaryings);

        return varyingPacking.packVaryings(infoLog, packedVaryings, transformFeedbackVaryings);
    }
};

// Test that a single varying can't overflow the packing.
TEST_F(VaryingPackingTest, OneVaryingLargerThanMax)
{
    std::vector<sh::Varying> varyings;

    sh::Varying varying;
    varying.type          = GL_FLOAT_MAT4;
    varying.precision     = GL_MEDIUM_FLOAT;
    varying.name          = "test";
    varying.mappedName    = "test";
    varying.arraySize     = 1;
    varying.staticUse     = true;
    varying.interpolation = sh::INTERPOLATION_FLAT;
    varying.isInvariant   = false;

    varyings.push_back(varying);

    ASSERT_FALSE(testPackVaryings(1, varyings));
}

TEST_F(VaryingPackingTest, MaxVaryingVec4PlusFragCoord)
{
    std::vector<sh::Varying> varyings;

    sh::Varying varying;
    varying.type          = GL_FLOAT_VEC4;
    varying.precision     = GL_MEDIUM_FLOAT;
    varying.name          = "test";
    varying.mappedName    = "test";
    varying.arraySize     = 1;
    varying.staticUse     = true;
    varying.interpolation = sh::INTERPOLATION_FLAT;
    varying.isInvariant   = false;

    varyings.push_back(varying);

    rx::VaryingPacking varyingPacking(1);
    varyingPacking.builtins(rx::SHADER_PIXEL).glFragCoord.enable();

    // Generate shader code that uses gl_FragCoord, a special fragment shader variables.
    // This test should fail, since we are really using (maxVaryings + 1) varyings.
    VaryingTestBase(0, 0, 0, 0, 0, 0, maxVaryings, 0, true, false, false, false);
}

// Disabled because drivers are allowed to successfully compile shaders that have more than the
// maximum number of varyings. (http://anglebug.com/1296)
TEST_F(VaryingPackingTest, MaxVaryingVec4PlusPointCoord)
{
    GLint maxVaryings = 0;
    glGetIntegerv(GL_MAX_VARYING_VECTORS, &maxVaryings);

    // Generate shader code that uses gl_FragCoord, a special fragment shader variables.
    // This test should fail, since we are really using (maxVaryings + 1) varyings.
    VaryingTestBase(0, 0, 0, 0, 0, 0, maxVaryings, 0, false, true, false, false);
}

// Disabled because drivers are allowed to successfully compile shaders that have more than the
// maximum number of varyings. (http://anglebug.com/1296)
TEST_F(VaryingPackingTest, MaxPlusOneVaryingVec3)
{
    GLint maxVaryings = 0;
    glGetIntegerv(GL_MAX_VARYING_VECTORS, &maxVaryings);

    VaryingTestBase(0, 0, 0, 0, maxVaryings + 1, 0, 0, 0, false, false, false, false);
}

// Disabled because drivers are allowed to successfully compile shaders that have more than the
// maximum number of varyings. (http://anglebug.com/1296)
TEST_F(VaryingPackingTest, MaxPlusOneVaryingVec3Array)
{
    GLint maxVaryings = 0;
    glGetIntegerv(GL_MAX_VARYING_VECTORS, &maxVaryings);

    VaryingTestBase(0, 0, 0, 0, 0, maxVaryings / 2 + 1, 0, 0, false, false, false, false);
}

// Disabled because drivers are allowed to successfully compile shaders that have more than the
// maximum number of varyings. (http://anglebug.com/1296)
TEST_F(VaryingPackingTest, MaxVaryingVec3AndOneVec2)
{
    GLint maxVaryings = 0;
    glGetIntegerv(GL_MAX_VARYING_VECTORS, &maxVaryings);

    VaryingTestBase(0, 0, 1, 0, maxVaryings, 0, 0, 0, false, false, false, false);
}

// Disabled because drivers are allowed to successfully compile shaders that have more than the
// maximum number of varyings. (http://anglebug.com/1296)
TEST_F(VaryingPackingTest, MaxPlusOneVaryingVec2)
{
    GLint maxVaryings = 0;
    glGetIntegerv(GL_MAX_VARYING_VECTORS, &maxVaryings);

    VaryingTestBase(0, 0, 2 * maxVaryings + 1, 0, 0, 0, 0, 0, false, false, false, false);
}

// Disabled because drivers are allowed to successfully compile shaders that have more than the
// maximum number of varyings. (http://anglebug.com/1296)
TEST_F(VaryingPackingTest, MaxVaryingVec3ArrayAndMaxPlusOneFloatArray)
{
    GLint maxVaryings = 0;
    glGetIntegerv(GL_MAX_VARYING_VECTORS, &maxVaryings);

    VaryingTestBase(0, maxVaryings / 2 + 1, 0, 0, 0, 0, 0, maxVaryings / 2, false, false, false,
                    false);
}

}  // anonymous namespace
