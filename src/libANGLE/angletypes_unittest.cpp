//
// Copyright 2015 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//

// BinaryStream_unittest.cpp: Unit tests of the binary stream classes.

#include <gtest/gtest.h>

#include "libANGLE/angletypes.h"

namespace angle
{

#if defined(ANGLE_IS_64_BIT_CPU)
constexpr bool is64Bit = true;
#else
constexpr bool is64Bit = false;
#endif

void checkInitState(const gl::BlendStateExt &blendStateExt)
{
    for (size_t i = 0; i < blendStateExt.maxDrawBuffers; ++i)
    {
        ASSERT_FALSE(blendStateExt.blendEnabledMask.test(i));

        bool r, g, b, a;
        blendStateExt.getColorMaskIndexed(r, g, b, a, i);
        ASSERT_TRUE(r);
        ASSERT_TRUE(g);
        ASSERT_TRUE(b);
        ASSERT_TRUE(a);

        ASSERT_EQ(blendStateExt.getBlendEquationColorIndexed(i), static_cast<GLenum>(GL_FUNC_ADD));
        ASSERT_EQ(blendStateExt.getBlendEquationAlphaIndexed(i), static_cast<GLenum>(GL_FUNC_ADD));

        ASSERT_EQ(blendStateExt.getBlendFactorSrcColorIndexed(i), static_cast<GLenum>(GL_ONE));
        ASSERT_EQ(blendStateExt.getBlendFactorDstColorIndexed(i), static_cast<GLenum>(GL_ZERO));
        ASSERT_EQ(blendStateExt.getBlendFactorSrcAlphaIndexed(i), static_cast<GLenum>(GL_ONE));
        ASSERT_EQ(blendStateExt.getBlendFactorDstAlphaIndexed(i), static_cast<GLenum>(GL_ZERO));
    }
}

// Test the initial state of BlendStateExt
TEST(BlendStateExt, Init)
{
    {
        const gl::BlendStateExt blendStateExt = gl::BlendStateExt(1);
        ASSERT_EQ(blendStateExt.maxDrawBuffers, 1u);
        ASSERT_EQ(blendStateExt.maxBlendEnabedMask.to_ulong(), 1u);
        ASSERT_EQ(blendStateExt.maxColorMask, is64Bit ? 0xFF : 0xFu);
        ASSERT_EQ(blendStateExt.maxBlendEquationMask, is64Bit ? 0xFFu : 0xFu);
        ASSERT_EQ(blendStateExt.maxBlendFactorMask, 0xFFu);
        checkInitState(blendStateExt);
    }

    {
        const gl::BlendStateExt blendStateExt = gl::BlendStateExt(4);
        ASSERT_EQ(blendStateExt.maxDrawBuffers, 4u);
        ASSERT_EQ(blendStateExt.maxBlendEnabedMask.to_ulong(), 0xFu);
        ASSERT_EQ(blendStateExt.maxColorMask, is64Bit ? 0xFFFFFFFFu : 0xFFFFu);
        ASSERT_EQ(blendStateExt.maxBlendEquationMask, is64Bit ? 0xFFFFFFFFu : 0xFFFFu);
        ASSERT_EQ(blendStateExt.maxBlendFactorMask, 0xFFFFFFFFu);
        checkInitState(blendStateExt);
    }

    {
        const gl::BlendStateExt blendStateExt = gl::BlendStateExt(8);
        ASSERT_EQ(blendStateExt.maxDrawBuffers, 8u);
        ASSERT_EQ(blendStateExt.maxBlendEnabedMask.to_ulong(), 0xFFu);
        ASSERT_EQ(blendStateExt.maxColorMask, is64Bit ? 0xFFFFFFFFFFFFFFFFu : 0xFFFFFFFFu);
        ASSERT_EQ(blendStateExt.maxBlendEquationMask, is64Bit ? 0xFFFFFFFFFFFFFFFFu : 0xFFFFFFFFu);
        ASSERT_EQ(blendStateExt.maxBlendFactorMask, 0xFFFFFFFFFFFFFFFFu);
        checkInitState(blendStateExt);
    }
}

TEST(BlendStateExt, BlendEnabled)
{
    gl::BlendStateExt blendStateExt = gl::BlendStateExt(3);

    blendStateExt.setBlend(true);
    ASSERT_EQ(blendStateExt.blendEnabledMask.to_ulong(), 7u);

    blendStateExt.setBlendIndexed(false, 1);
    ASSERT_EQ(blendStateExt.blendEnabledMask.to_ulong(), 5u);
}

TEST(BlendStateExt, ColorMask)
{
    gl::BlendStateExt blendStateExt = gl::BlendStateExt(5);

    blendStateExt.setColorMask(true, false, true, false);
    ASSERT_EQ(blendStateExt.colorMask, is64Bit ? 0x0505050505u : 0x55555u);

    blendStateExt.setColorMaskIndexed(false, true, false, true, 3);
    ASSERT_EQ(blendStateExt.colorMask, is64Bit ? 0x050A050505u : 0x5A555u);

    bool r, g, b, a;
    blendStateExt.getColorMaskIndexed(r, g, b, a, 3);
    ASSERT_FALSE(r);
    ASSERT_TRUE(g);
    ASSERT_FALSE(b);
    ASSERT_TRUE(a);

    gl::BlendStateExt::ColorMaskStorage::Type otherColorMask =
        blendStateExt.expandColorMaskIndexed(3);
    ASSERT_EQ(otherColorMask, is64Bit ? 0x0A0A0A0A0Au : 0xAAAAAu);

    const gl::DrawBufferMask diff = blendStateExt.compareColorMask(otherColorMask);
    ASSERT_EQ(diff.to_ulong(), 23u);
}

TEST(BlendStateExt, BlendEquations)
{
    gl::BlendStateExt blendStateExt = gl::BlendStateExt(7);

    blendStateExt.setBlendEquation(GL_MIN, GL_FUNC_SUBTRACT);
    ASSERT_EQ(blendStateExt.blendEquationColor, is64Bit ? 0x01010101010101u : 0x1111111u);
    ASSERT_EQ(blendStateExt.blendEquationAlpha, is64Bit ? 0x04040404040404u : 0x4444444u);

    blendStateExt.setBlendEquationIndexed(GL_MAX, GL_FUNC_SUBTRACT, 3);
    blendStateExt.setBlendEquationIndexed(GL_MIN, GL_FUNC_ADD, 5);
    ASSERT_EQ(blendStateExt.blendEquationColor, is64Bit ? 0x01010102010101u : 0x1112111u);
    ASSERT_EQ(blendStateExt.blendEquationAlpha, is64Bit ? 0x04000404040404u : 0x4044444u);
    ASSERT_EQ(blendStateExt.getBlendEquationColorIndexed(3), static_cast<GLenum>(GL_MAX));
    ASSERT_EQ(blendStateExt.getBlendEquationAlphaIndexed(5), static_cast<GLenum>(GL_FUNC_ADD));

    gl::BlendStateExt::BlendEquationStorage::Type otherBlendEquationColor =
        blendStateExt.expandBlendEquationColorIndexed(0);
    gl::BlendStateExt::BlendEquationStorage::Type otherBlendEquationAlpha =
        blendStateExt.expandBlendEquationAlphaIndexed(0);

    ASSERT_EQ(otherBlendEquationColor, is64Bit ? 0x01010101010101u : 0x1111111u);
    ASSERT_EQ(otherBlendEquationAlpha, is64Bit ? 0x04040404040404u : 0x4444444u);

    const gl::DrawBufferMask diff =
        blendStateExt.compareBlendEquations(otherBlendEquationColor, otherBlendEquationAlpha);
    ASSERT_EQ(diff.to_ulong(), 40u);
}

TEST(BlendStateExt, BlendFactors)
{
    gl::BlendStateExt blendStateExt = gl::BlendStateExt(8);

    blendStateExt.setBlendFactors(GL_SRC_COLOR, GL_DST_COLOR, GL_SRC_ALPHA, GL_DST_ALPHA);
    ASSERT_EQ(blendStateExt.blendFactorSrcColor, 0x0202020202020202u);
    ASSERT_EQ(blendStateExt.blendFactorDstColor, 0x0808080808080808u);
    ASSERT_EQ(blendStateExt.blendFactorSrcAlpha, 0x0404040404040404u);
    ASSERT_EQ(blendStateExt.blendFactorDstAlpha, 0x0606060606060606u);

    blendStateExt.setBlendFactorsIndexed(GL_ONE, GL_DST_COLOR, GL_SRC_ALPHA, GL_DST_ALPHA, 0);
    blendStateExt.setBlendFactorsIndexed(GL_SRC_COLOR, GL_ONE, GL_SRC_ALPHA, GL_DST_ALPHA, 3);
    blendStateExt.setBlendFactorsIndexed(GL_SRC_COLOR, GL_DST_COLOR, GL_ONE, GL_DST_ALPHA, 5);
    blendStateExt.setBlendFactorsIndexed(GL_SRC_COLOR, GL_DST_COLOR, GL_SRC_ALPHA, GL_ONE, 7);
    ASSERT_EQ(blendStateExt.blendFactorSrcColor, 0x0202020202020201u);
    ASSERT_EQ(blendStateExt.blendFactorDstColor, 0x0808080801080808u);
    ASSERT_EQ(blendStateExt.blendFactorSrcAlpha, 0x0404010404040404u);
    ASSERT_EQ(blendStateExt.blendFactorDstAlpha, 0x0106060606060606u);

    ASSERT_EQ(blendStateExt.getBlendFactorSrcColorIndexed(0), static_cast<GLenum>(GL_ONE));
    ASSERT_EQ(blendStateExt.getBlendFactorDstColorIndexed(3), static_cast<GLenum>(GL_ONE));
    ASSERT_EQ(blendStateExt.getBlendFactorSrcAlphaIndexed(5), static_cast<GLenum>(GL_ONE));
    ASSERT_EQ(blendStateExt.getBlendFactorDstAlphaIndexed(7), static_cast<GLenum>(GL_ONE));

    gl::BlendStateExt::BlendFactorStorage::Type otherBlendFactorSrcColor =
        blendStateExt.expandBlendFactorSrcColorIndexed(1);
    gl::BlendStateExt::BlendFactorStorage::Type otherBlendFactorDstColor =
        blendStateExt.expandBlendFactorDstColorIndexed(1);
    gl::BlendStateExt::BlendFactorStorage::Type otherBlendFactorSrcAlpha =
        blendStateExt.expandBlendFactorSrcAlphaIndexed(1);
    gl::BlendStateExt::BlendFactorStorage::Type otherBlendFactorDstAlpha =
        blendStateExt.expandBlendFactorDstAlphaIndexed(1);

    ASSERT_EQ(otherBlendFactorSrcColor, 0x0202020202020202u);
    ASSERT_EQ(otherBlendFactorDstColor, 0x0808080808080808u);
    ASSERT_EQ(otherBlendFactorSrcAlpha, 0x0404040404040404u);
    ASSERT_EQ(otherBlendFactorDstAlpha, 0x0606060606060606u);

    const gl::DrawBufferMask diff =
        blendStateExt.compareBlendFactors(otherBlendFactorSrcColor, otherBlendFactorDstColor,
                                          otherBlendFactorSrcAlpha, otherBlendFactorDstAlpha);
    ASSERT_EQ(diff.to_ulong(), 169u);
}

}  // namespace angle
