//
// Copyright (c) 2015 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// utilities_unittest.cpp: Unit tests for ANGLE's GL utility functions

#include "gmock/gmock.h"
#include "gtest/gtest.h"

#include "common/utilities.h"

namespace
{

TEST(ParseResourceName, ArrayIndex)
{
    size_t index;
    EXPECT_EQ("foo", gl::ParseResourceName("foo[123]", &index));
    EXPECT_EQ(123u, index);

    EXPECT_EQ("bar", gl::ParseResourceName("bar[0]", &index));
    EXPECT_EQ(0u, index);
}

TEST(ParseResourceName, NegativeArrayIndex)
{
    size_t index;
    EXPECT_EQ("foo", gl::ParseResourceName("foo[-1]", &index));
    EXPECT_EQ(GL_INVALID_INDEX, index);
}

TEST(ParseResourceName, NoArrayIndex)
{
    size_t index;
    EXPECT_EQ("foo", gl::ParseResourceName("foo", &index));
    EXPECT_EQ(GL_INVALID_INDEX, index);
}

TEST(ParseResourceName, NULLArrayIndex)
{
    EXPECT_EQ("foo", gl::ParseResourceName("foo[10]", nullptr));
}

TEST(ParseResourceName, TrailingWhitespace)
{
    size_t index;
    EXPECT_EQ("foo ", gl::ParseResourceName("foo ", &index));
    EXPECT_EQ(GL_INVALID_INDEX, index);

    EXPECT_EQ("foo[10] ", gl::ParseResourceName("foo[10] ", &index));
    EXPECT_EQ(GL_INVALID_INDEX, index);
}

// GLES 3.10 Section 2.2.2
TEST(ConvertGLStateType, ToExternalGLboolean)
{
    int zeroInt      = 0;
    GLboolean result = gl::ConvertFrom<GLboolean>(zeroInt);
    EXPECT_EQ(GL_FALSE, result);

    int nonZeroInt = -4;
    result         = gl::ConvertFrom<GLboolean>(nonZeroInt);
    EXPECT_EQ(GL_TRUE, result);

    float zeroFloat = 0.0f;
    result          = gl::ConvertFrom<GLboolean>(zeroFloat);
    EXPECT_EQ(GL_FALSE, result);

    float nonZeroFloat = 0.1f;
    result             = gl::ConvertFrom<GLboolean>(nonZeroFloat);
    EXPECT_EQ(GL_TRUE, result);

    bool falseBool = false;
    result         = gl::ConvertFrom<GLboolean>(falseBool);
    EXPECT_EQ(GL_FALSE, result);

    bool trueBool = true;
    result        = gl::ConvertFrom<GLboolean>(trueBool);
    EXPECT_EQ(GL_TRUE, result);
}

TEST(ConvertGLStateType, ToExternalGLuint)
{
    int negativeInt = -9;
    GLuint result   = gl::ConvertFrom<GLuint>(negativeInt);
    EXPECT_EQ(0u, result);

    int positiveInt = 1;
    result          = gl::ConvertFrom<GLuint>(positiveInt);
    EXPECT_EQ(1u, result);
}

TEST(ConvertGLStateType, ToExternalGLint)
{
    bool trueBool = true;
    GLint result  = gl::ConvertFrom<GLint>(trueBool);
    EXPECT_EQ(1, result);

    bool falseBool = false;
    result         = gl::ConvertFrom<GLint>(falseBool);
    EXPECT_EQ(0, result);

    float roundFloat = 3.8f;
    result           = gl::ConvertFrom<GLint>(roundFloat);
    EXPECT_EQ(4, result);

    roundFloat = -3.8f;
    result     = gl::ConvertFrom<GLint>(roundFloat);
    EXPECT_EQ(-4, result);
}

TEST(ConvertGLStateType, ToExternalGLfloat)
{
    bool trueBool  = true;
    GLfloat result = gl::ConvertFrom<GLfloat>(trueBool);
    EXPECT_EQ(1.0f, result);

    bool falseBool = false;
    result         = gl::ConvertFrom<GLfloat>(falseBool);
    EXPECT_EQ(0.0f, result);

    int32_t signedInt = -1;
    result            = gl::ConvertFrom<GLfloat>(signedInt);
    EXPECT_EQ(-1.0f, result);

    uint32_t unsignedInt = 1;
    result               = gl::ConvertFrom<GLfloat>(unsignedInt);
    EXPECT_EQ(1.0f, result);
}

TEST(ConvertGLStateType, ClampToExternalGLintegeral)
{
    float bigFloat     = 9999999999.9f;
    GLint clampedGLint = gl::ConvertFrom<GLint>(bigFloat);
    EXPECT_EQ(std::numeric_limits<GLint>::max(), clampedGLint);

    GLuint clampedGLuint = gl::ConvertFrom<GLuint>(bigFloat);
    EXPECT_EQ(std::numeric_limits<GLuint>::max(), clampedGLuint);

    GLint64 clampedGLint64 = gl::ConvertFrom<GLint64>(bigFloat);
    EXPECT_EQ(10000000000, clampedGLint64);

    uint32_t bigUint32 = 0xffffffffu;
    clampedGLint       = gl::ConvertFrom<GLint>(bigUint32);
    EXPECT_EQ(std::numeric_limits<GLint>::max(), clampedGLint);

    uint64_t bigUint64 = 0xffffffffffffffffull;
    clampedGLint       = gl::ConvertFrom<GLint>(bigUint64);
    EXPECT_EQ(std::numeric_limits<GLint>::max(), clampedGLint);

    int64_t bigInt64 = 0x7fffffffffffffffll;
    clampedGLint     = gl::ConvertFrom<GLint>(bigInt64);
    EXPECT_EQ(std::numeric_limits<GLint>::max(), clampedGLint);

    clampedGLuint = gl::ConvertFrom<GLuint>(bigUint64);
    EXPECT_EQ(std::numeric_limits<GLuint>::max(), clampedGLuint);

    clampedGLuint = gl::ConvertFrom<GLuint>(bigInt64);
    EXPECT_EQ(std::numeric_limits<GLuint>::max(), clampedGLuint);

    clampedGLint64 = gl::ConvertFrom<GLint64>(bigUint64);
    EXPECT_EQ(std::numeric_limits<GLint64>::max(), clampedGLint64);
}

// GLES 3.10 Section 2.2.1
TEST(ConvertGLStateType, ToInternalBool)
{
    GLint zeroInt = 0;
    bool result   = gl::ConvertTo<bool>(zeroInt);
    EXPECT_EQ(false, result);

    GLint nonZeroInt = -4;
    result           = gl::ConvertTo<bool>(nonZeroInt);
    EXPECT_EQ(true, result);

    GLfloat zeroFloat = 0.0f;
    result            = gl::ConvertTo<bool>(zeroFloat);
    EXPECT_EQ(false, result);

    GLfloat nonZeroFloat = 0.1f;
    result               = gl::ConvertTo<bool>(nonZeroFloat);
    EXPECT_EQ(true, result);

    GLboolean falseBoolean = GL_FALSE;
    result                 = gl::ConvertTo<bool>(falseBoolean);
    EXPECT_EQ(false, result);

    GLboolean trueBoolean = GL_TRUE;
    result                = gl::ConvertTo<bool>(trueBoolean);
    EXPECT_EQ(true, result);
}

TEST(ConvertGLStateType, ToInternalInt)
{
    GLboolean trueBoolean = GL_TRUE;
    int result            = gl::ConvertTo<int>(trueBoolean);
    EXPECT_EQ(1, result);

    GLboolean falseBoolean = GL_FALSE;
    result                 = gl::ConvertTo<int>(falseBoolean);
    EXPECT_EQ(0, result);

    GLfloat roundFloat = 3.8f;
    result             = gl::ConvertTo<int>(roundFloat);
    EXPECT_EQ(4, result);

    roundFloat = -3.8f;
    result     = gl::ConvertTo<int>(roundFloat);
    EXPECT_EQ(-4, result);
}

TEST(ConvertGLStateType, ToInternalfloat)
{
    GLboolean trueBoolean = GL_TRUE;
    float result          = gl::ConvertTo<float>(trueBoolean);
    EXPECT_EQ(1.0f, result);

    GLboolean falseBoolean = GL_FALSE;
    result                 = gl::ConvertTo<float>(falseBoolean);
    EXPECT_EQ(0.0f, result);

    GLint signedInt = -1;
    result          = gl::ConvertTo<float>(signedInt);
    EXPECT_EQ(-1.0f, result);

    GLuint unsignedInt = 1;
    result             = gl::ConvertTo<float>(unsignedInt);
    EXPECT_EQ(1.0f, result);
}
}
