//
// Copyright 2016 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//
// VaryingPacking_unittest.cpp:
//   Tests for ANGLE's internal varying packing algorithm.
//

#include "libANGLE/renderer/d3d/hlsl/VaryingPacking.h"

#include <gtest/gtest.h>

#include "libANGLE/Program.h"

namespace
{

class VaryingPackingTest : public ::testing::Test
{
  protected:
    VaryingPackingTest() {}

    bool testVaryingPacking(const std::vector<sh::Varying> &shVaryings,
                            rx::VaryingPacking *varyingPacking)
    {
        std::vector<rx::PackedVarying> packedVaryings;
        for (const auto &shVarying : shVaryings)
        {
            packedVaryings.push_back(rx::PackedVarying(shVarying, shVarying.interpolation));
        }

        gl::InfoLog infoLog;
        std::vector<std::string> transformFeedbackVaryings;

        if (!varyingPacking->packUserVaryings(infoLog, packedVaryings, transformFeedbackVaryings))
            return false;

        return varyingPacking->validateBuiltins();
    }

    bool packVaryings(GLuint maxVaryings, const std::vector<sh::Varying> &shVaryings)
    {
        rx::VaryingPacking varyingPacking(maxVaryings);
        return testVaryingPacking(shVaryings, &varyingPacking);
    }

    static const int MaxVaryings = 4;
};

std::vector<sh::Varying> MakeVaryings(GLenum type, size_t count, size_t arraySize)
{
    std::vector<sh::Varying> varyings;

    for (size_t index = 0; index < count; ++index)
    {
        std::stringstream strstr;
        strstr << type << index;

        sh::Varying varying;
        varying.type          = type;
        varying.precision     = GL_MEDIUM_FLOAT;
        varying.name          = strstr.str();
        varying.mappedName    = strstr.str();
        varying.arraySize     = static_cast<unsigned int>(arraySize);
        varying.staticUse     = true;
        varying.interpolation = sh::INTERPOLATION_FLAT;
        varying.isInvariant   = false;

        varyings.push_back(varying);
    }

    return varyings;
}

void AddVaryings(std::vector<sh::Varying> *varyings, GLenum type, size_t count, size_t arraySize)
{
    const auto &newVaryings = MakeVaryings(type, count, arraySize);
    varyings->insert(varyings->end(), newVaryings.begin(), newVaryings.end());
}

// Test that a single varying can't overflow the packing.
TEST_F(VaryingPackingTest, OneVaryingLargerThanMax)
{
    ASSERT_FALSE(packVaryings(1, MakeVaryings(GL_FLOAT_MAT4, 1, 0)));
}

TEST_F(VaryingPackingTest, MaxVaryingVec4PlusFragCoord)
{
    const std::string &userSemantic = rx::GetVaryingSemantic(4, false);

    rx::VaryingPacking varyingPacking(MaxVaryings);
    unsigned int reservedSemanticIndex = varyingPacking.getMaxSemanticIndex();

    varyingPacking.builtins(rx::SHADER_PIXEL)
        .glFragCoord.enable(userSemantic, reservedSemanticIndex);

    const auto &varyings = MakeVaryings(GL_FLOAT_VEC4, MaxVaryings, 0);

    ASSERT_FALSE(testVaryingPacking(varyings, &varyingPacking));
}

TEST_F(VaryingPackingTest, MaxVaryingVec4PlusPointCoord)
{
    const std::string &userSemantic = rx::GetVaryingSemantic(4, false);

    rx::VaryingPacking varyingPacking(MaxVaryings);
    unsigned int reservedSemanticIndex = varyingPacking.getMaxSemanticIndex();

    varyingPacking.builtins(rx::SHADER_PIXEL)
        .glPointCoord.enable(userSemantic, reservedSemanticIndex);

    const auto &varyings = MakeVaryings(GL_FLOAT_VEC4, MaxVaryings, 0);

    ASSERT_FALSE(testVaryingPacking(varyings, &varyingPacking));
}

// This will overflow the available varying space.
TEST_F(VaryingPackingTest, MaxPlusOneVaryingVec3)
{
    ASSERT_FALSE(packVaryings(MaxVaryings, MakeVaryings(GL_FLOAT_VEC3, MaxVaryings + 1, 0)));
}

// This will overflow the available varying space.
TEST_F(VaryingPackingTest, MaxPlusOneVaryingVec3Array)
{
    ASSERT_FALSE(packVaryings(MaxVaryings, MakeVaryings(GL_FLOAT_VEC3, MaxVaryings / 2 + 1, 2)));
}

// This will overflow the available varying space.
TEST_F(VaryingPackingTest, MaxVaryingVec3AndOneVec2)
{
    std::vector<sh::Varying> varyings = MakeVaryings(GL_FLOAT_VEC3, MaxVaryings, 0);
    AddVaryings(&varyings, GL_FLOAT_VEC2, 1, 0);
    ASSERT_FALSE(packVaryings(MaxVaryings, varyings));
}

// This should work since two vec2s are packed in a single register.
TEST_F(VaryingPackingTest, MaxPlusOneVaryingVec2)
{
    ASSERT_TRUE(packVaryings(MaxVaryings, MakeVaryings(GL_FLOAT_VEC2, MaxVaryings + 1, 0)));
}

// Same for this one as above.
TEST_F(VaryingPackingTest, TwiceMaxVaryingVec2)
{
    ASSERT_TRUE(packVaryings(MaxVaryings, MakeVaryings(GL_FLOAT_VEC2, MaxVaryings * 2, 0)));
}

// This should not work since it overflows available varying space.
TEST_F(VaryingPackingTest, TooManyVaryingVec2)
{
    ASSERT_FALSE(packVaryings(MaxVaryings, MakeVaryings(GL_FLOAT_VEC2, MaxVaryings * 2 + 1, 0)));
}

// This should work according to the example GL packing rules - the float varyings are slotted
// into the end of the vec3 varying arrays.
TEST_F(VaryingPackingTest, MaxVaryingVec3ArrayAndFloatArrays)
{
    std::vector<sh::Varying> varyings = MakeVaryings(GL_FLOAT_VEC3, MaxVaryings / 2, 2);
    AddVaryings(&varyings, GL_FLOAT, MaxVaryings / 2, 2);
    ASSERT_TRUE(packVaryings(MaxVaryings, varyings));
}

// This should not work - it has one too many float arrays.
TEST_F(VaryingPackingTest, MaxVaryingVec3ArrayAndMaxPlusOneFloatArray)
{
    std::vector<sh::Varying> varyings = MakeVaryings(GL_FLOAT_VEC3, MaxVaryings / 2, 2);
    AddVaryings(&varyings, GL_FLOAT, MaxVaryings / 2 + 1, 2);
    ASSERT_FALSE(packVaryings(MaxVaryings, varyings));
}

}  // anonymous namespace
