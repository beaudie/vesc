//
// Copyright 2020 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//
// TextureParameterLargeValueTest:
//   We use the same template for all glTexParameter* calls so this helps to make sure we don't
//   improperly cast an int into a float

#include "test_utils/ANGLETest.h"
#include "test_utils/angle_test_instantiate.h"
// 'None' is defined as 'struct None {};' in
// third_party/googletest/src/googletest/include/gtest/internal/gtest-type-util.h.
// But 'None' is also defined as a numeric constant 0L in <X11/X.h>.
// So we need to include ANGLETest.h first to avoid this conflict.

#include <limits.h>
#include "libANGLE/Context.h"
#include "libANGLE/formatutils.h"
#include "libANGLE/gl_enum_utils.h"
#include "util/EGLWindow.h"

using namespace angle;

namespace
{

class TextureParameterLargeValueTest : public ANGLETest
{};

TEST_P(TextureParameterLargeValueTest, IntConversionsAndIntBounds)
{
    // Hack the angle!
    gl::Context *context = static_cast<gl::Context *>(getEGLWindow()->getContext());

    // Test integers that can't be represented as floats, INT_MIN, and INT_MAX
    constexpr GLint kFirstIntThatCannotBeFloat = 16777217;
    GLint crop[4] = {-kFirstIntThatCannotBeFloat, kFirstIntThatCannotBeFloat,
                     std::numeric_limits<GLint>::max(), std::numeric_limits<GLint>::min()};
    glTexParameteriv(GL_TEXTURE_2D, GL_TEXTURE_CROP_RECT_OES, crop);
    gl::TextureType targetPacked = gl::FromGL<gl::TextureType>(GL_TEXTURE_2D);
    gl::Texture *const texture   = context->getTextureByType(targetPacked);
    gl::Rectangle rectangle      = texture->getCrop();

    ASSERT_EQ(rectangle.x, -kFirstIntThatCannotBeFloat);
    ASSERT_EQ(rectangle.y, kFirstIntThatCannotBeFloat);
    ASSERT_EQ(rectangle.width, std::numeric_limits<GLint>::max());
    ASSERT_EQ(rectangle.height, std::numeric_limits<GLint>::min());
}

ANGLE_INSTANTIATE_TEST_ES1(TextureParameterLargeValueTest);

}  // anonymous namespace
