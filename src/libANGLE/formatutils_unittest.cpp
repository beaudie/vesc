//
// Copyright (c) 2015 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//

// formatutils_unittests.cpp: Unit tests of format utils functions

#include "gmock/gmock.h"
#include "gtest/gtest.h"
#include "libANGLE/formatutils.h"

TEST(FormatUtils, GetAllSizedInternalFormats)
{
    const gl::FormatSet &allFormats = gl::GetAllSizedInternalFormats();
    for (GLenum internalFormat : allFormats)
    {
        const gl::InternalFormat& internalFormatInfo = gl::GetInternalFormatInfo(internalFormat);
        EXPECT_GT(internalFormatInfo.pixelBytes, 0u);

        if (internalFormatInfo.compressed)
        {
            EXPECT_GT(internalFormatInfo.compressedBlockWidth, 0u);
            EXPECT_GT(internalFormatInfo.compressedBlockHeight, 0u);
        }
        else
        {
            EXPECT_GT(internalFormatInfo.redBits +
                      internalFormatInfo.greenBits +
                      internalFormatInfo.blueBits +
                      internalFormatInfo.luminanceBits +
                      internalFormatInfo.alphaBits +
                      internalFormatInfo.depthBits +
                      internalFormatInfo.stencilBits,
                      0u);
        }
    }
}

// Test that all sized formats map back to the same format
TEST(FormatUtils, GetExactSizedColorInternalFormat)
{
    const gl::FormatSet &allFormats = gl::GetAllSizedInternalFormats();
    for (GLenum internalFormat : allFormats)
    {
        const gl::InternalFormat& internalFormatInfo = gl::GetInternalFormatInfo(internalFormat);

        // Only check if this is a non-compressed color format
        if (!internalFormatInfo.compressed &&
            (internalFormatInfo.redBits > 0 ||
             internalFormatInfo.greenBits > 0 ||
             internalFormatInfo.blueBits > 0 ||
             internalFormatInfo.luminanceBits > 0 ||
             internalFormatInfo.alphaBits > 0))
        {
            GLenum exactSizedFormat = gl::GetExactSizedColorInternalFormat(internalFormatInfo.componentType,
                                                                           internalFormatInfo.redBits,
                                                                           internalFormatInfo.greenBits,
                                                                           internalFormatInfo.blueBits,
                                                                           internalFormatInfo.luminanceBits,
                                                                           internalFormatInfo.alphaBits);

            EXPECT_EQ(internalFormat, exactSizedFormat);

            if (internalFormat != exactSizedFormat)
            {
                int a = 5;
                a++;
            }
        }
    }
}

// Test that all sized formats map back to the same format
TEST(FormatUtils, GetExactSizedDepthStencilInternalFormat)
{
    const gl::FormatSet &allFormats = gl::GetAllSizedInternalFormats();
    for (GLenum internalFormat : allFormats)
    {
        const gl::InternalFormat& internalFormatInfo = gl::GetInternalFormatInfo(internalFormat);

        // Only check if this is a non-compressed depth stencil format
        if (!internalFormatInfo.compressed && (internalFormatInfo.depthBits > 0 || internalFormatInfo.stencilBits > 0))
        {
            GLenum exactSizedFormat = gl::GetExactSizedDepthStencilInternalFormat(internalFormatInfo.componentType,
                                                                                  internalFormatInfo.depthBits,
                                                                                  internalFormatInfo.stencilBits);

            EXPECT_EQ(internalFormat, exactSizedFormat);
        }
    }
}
