//
// Copyright 2023 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//
// LoadToNative_unittest.cpp: Unit tests for pixel loading functions.

#include <gmock/gmock.h>
#include <vector>
#include "common/debug.h"
#include "image_util/loadimage.h"

using namespace angle;
using namespace testing;

namespace
{

struct RGBColor
{
    RGBColor(uint8_t r, uint8_t g, uint8_t b) : r(r), g(g), b(b) {}
    uint8_t r;
    uint8_t g;
    uint8_t b;
};

size_t roundUp(size_t size, size_t alignment)
{
    if (alignment == 0)
    {
        return size;
    }

    ASSERT((alignment & (alignment - 1)) == 0);
    return ((size - 1) | (alignment - 1)) + 1;
}

template <uint8_t fourthValue>
bool TestLoadUbyteRGBToRGBA(ImageLoadContext &context,
                            size_t width,
                            size_t height,
                            size_t depth,
                            size_t inputOffset,
                            size_t outputOffset,
                            size_t inputRowAlignment)
{
    size_t inputPixelBytes  = 3;
    size_t inputRowPitch    = roundUp(width * inputPixelBytes, inputRowAlignment);
    size_t inputDepthPitch  = height * inputRowPitch;
    size_t inputActualBytes = depth * inputDepthPitch;

    size_t outputPixelBytes  = 4;
    size_t outputRowPitch    = width * outputPixelBytes;
    size_t outputDepthPitch  = height * outputRowPitch;
    size_t outputActualBytes = depth * outputDepthPitch;

    // Define input pixel data.
    std::vector<RGBColor> rgbData;
    for (size_t z = 0; z < depth; z++)
    {
        for (size_t y = 0; y < height; y++)
        {
            for (size_t x = 0; x < width; x++)
            {
                rgbData.emplace_back(RGBColor(x % 256, y % 256, z % 256));
            }
        }
    }

    // Prepare the input and output for copy. The offset values are used to add unused bytes to the
    // beginning of the input and output data, in order to test address alignments.
    std::vector<uint8_t> rgbInput(inputOffset + inputActualBytes, 0xAA);
    for (size_t z = 0; z < depth; z++)
    {
        for (size_t y = 0; y < height; y++)
        {
            for (size_t x = 0; x < width; x++)
            {
                size_t inputIndex =
                    inputOffset + z * inputDepthPitch + y * inputRowPitch + x * inputPixelBytes;
                size_t pixelId = z * height * width + y * width + x;

                rgbInput[inputIndex]     = rgbData[pixelId].r;
                rgbInput[inputIndex + 1] = rgbData[pixelId].g;
                rgbInput[inputIndex + 2] = rgbData[pixelId].b;
            }
        }
    }
    std::vector<uint8_t> rgbaOutput(outputOffset + outputActualBytes, 0xAA);

    // Call loading function.
    LoadToNative3To4<uint8_t, fourthValue>(
        context, width, height, depth, rgbInput.data() + inputOffset, inputRowPitch,
        inputDepthPitch, rgbaOutput.data() + outputOffset, outputRowPitch, outputDepthPitch);

    // Compare the input and output data.
    for (size_t z = 0; z < depth; z++)
    {
        for (size_t y = 0; y < height; y++)
        {
            for (size_t x = 0; x < width; x++)
            {
                size_t inputIndex =
                    inputOffset + z * inputDepthPitch + y * inputRowPitch + x * inputPixelBytes;
                size_t outputIndex =
                    outputOffset + z * outputDepthPitch + y * outputRowPitch + x * outputPixelBytes;

                bool rMatch = rgbInput[inputIndex] == rgbaOutput[outputIndex];
                bool gMatch = rgbInput[inputIndex + 1] == rgbaOutput[outputIndex + 1];
                bool bMatch = rgbInput[inputIndex + 2] == rgbaOutput[outputIndex + 2];
                bool aMatch = rgbaOutput[outputIndex + 3] == fourthValue;
                if (!(rMatch && gMatch && bMatch && aMatch))
                {
                    ERR() << "Mismatch at Index (" << x << ", " << y << ", " << z << ")"
                          << std::endl
                          << "Expected output: (" << static_cast<uint32_t>(rgbInput[inputIndex])
                          << ", " << static_cast<uint32_t>(rgbInput[inputIndex + 1]) << ", "
                          << static_cast<uint32_t>(rgbInput[inputIndex + 2]) << ", "
                          << static_cast<uint32_t>(fourthValue) << ")" << std::endl
                          << "Actual output: (" << static_cast<uint32_t>(rgbaOutput[outputIndex])
                          << ", " << static_cast<uint32_t>(rgbaOutput[outputIndex + 1]) << ", "
                          << static_cast<uint32_t>(rgbaOutput[outputIndex + 2]) << ", "
                          << static_cast<uint32_t>(rgbaOutput[outputIndex + 3]) << ")";
                    return false;
                }
            }
        }
    }
    return true;
}

// Tests the ubyte RGB to RGBA loading function when the width is 4-byte aligned. This loading
// function can copy 4 bytes at a time in a row.
TEST(LoadToNative3To4, LoadUbyteRGBToRGBADataAlignedWidth)
{
    ImageLoadContext context;
    bool isSuccess;
    size_t alignedTestWidths[] = {8, 20, 128, 1000, 4096};
    for (auto &width : alignedTestWidths)
    {
        ASSERT(width % 4 == 0);
        isSuccess = TestLoadUbyteRGBToRGBA<0xFF>(context, width, 3, 1, 0, 0, 0);
        EXPECT_EQ(isSuccess, true);
    }
}

// Tests the ubyte RGB to RGBA loading function when the width is not 4-byte aligned, which will
// cause the loading function to copy some bytes in the beginning and end of some rows individually.
TEST(LoadToNative3To4, LoadUbyteRGBToRGBADataUnalignedWidth)
{
    ImageLoadContext context;
    bool isSuccess;
    size_t unalignedTestWidths[] = {5, 22, 127, 1022, 4097};
    for (auto &width : unalignedTestWidths)
    {
        ASSERT(width % 4 != 0);
        isSuccess = TestLoadUbyteRGBToRGBA<0xFF>(context, width, 3, 1, 0, 0, 0);
        EXPECT_EQ(isSuccess, true);
    }
}

// Tests the ubyte RGB to RGBA loading function when there is depth.
TEST(LoadToNative3To4, LoadUbyteRGBToRGBADataWithDepth)
{
    ImageLoadContext context;
    bool isSuccess = TestLoadUbyteRGBToRGBA<0xFF>(context, 3, 3, 3, 0, 0, 0);
    EXPECT_EQ(isSuccess, true);
}

// Tests the ubyte RGB to RGBA loading function when the width is less than 4 bytes. Therefore the
// loading function will copy data one byte at a time.
TEST(LoadToNative3To4, LoadUbyteRGBToRGBADataWidthLessThanUint32)
{
    ImageLoadContext context;
    bool isSuccess;
    size_t smallTestWidths[] = {1, 2, 3};
    for (auto &width : smallTestWidths)
    {
        isSuccess = TestLoadUbyteRGBToRGBA<0xFF>(context, width, 3, 1, 0, 0, 0);
        EXPECT_EQ(isSuccess, true);
    }
}

// Tests the ubyte RGB to RGBA loading function when when the width is 4-byte-aligned and the input
// address has an offset.
TEST(LoadToNative3To4, LoadUbyteRGBToRGBAWithAlignedWidthAndInputAddressOffset)
{
    ImageLoadContext context;
    bool isSuccess = TestLoadUbyteRGBToRGBA<0xFF>(context, 8, 8, 1, 1, 0, 0);
    EXPECT_EQ(isSuccess, true);
}

// Tests the ubyte RGB to RGBA loading function when when the width is not 4-byte-aligned and the
// input address has an offset.
TEST(LoadToNative3To4, LoadUbyteRGBToRGBAWithUnalignedWidthAndInputAddressOffset)
{
    ImageLoadContext context;
    bool isSuccess = TestLoadUbyteRGBToRGBA<0xFF>(context, 7, 7, 1, 3, 0, 0);
    EXPECT_EQ(isSuccess, true);
}

// Tests the ubyte RGB to RGBA loading function when the width is 4-byte-aligned and the output
// address has an offset.
TEST(LoadToNative3To4, LoadUbyteRGBToRGBAWithAlignedWidthAndOutputAddressOffset)
{
    ImageLoadContext context;
    bool isSuccess = TestLoadUbyteRGBToRGBA<0xFF>(context, 8, 8, 1, 0, 1, 0);
    EXPECT_EQ(isSuccess, true);
}

// Tests the ubyte RGB to RGBA loading function when the width is not 4-byte-aligned and the output
// address has an offset.
TEST(LoadToNative3To4, LoadUbyteRGBToRGBAWithUnalignedWidthAndOutputAddressOffset)
{
    ImageLoadContext context;
    bool isSuccess = TestLoadUbyteRGBToRGBA<0xFF>(context, 7, 7, 1, 0, 3, 0);
    EXPECT_EQ(isSuccess, true);
}

// Tests the ubyte RGB to RGBA loading function when the width is 4-byte-aligned and the input row
// alignment is 4.
TEST(LoadToNative3To4, LoadUbyteRGBToRGBAWithAlignedWidthAndAlignment4)
{
    ImageLoadContext context;
    bool isSuccess = TestLoadUbyteRGBToRGBA<0xFF>(context, 4, 4, 1, 0, 0, 4);
    EXPECT_EQ(isSuccess, true);
}

// Tests the ubyte RGB to RGBA loading function when the width is not 4-byte-aligned and the input
// row alignment is 4.
TEST(LoadToNative3To4, LoadUbyteRGBToRGBAWithUnalignedWidthAndAlignment4)
{
    ImageLoadContext context;
    bool isSuccess = TestLoadUbyteRGBToRGBA<0xFF>(context, 5, 5, 1, 0, 0, 4);
    EXPECT_EQ(isSuccess, true);
}

}  // namespace