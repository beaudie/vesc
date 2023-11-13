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
    // bool operator==(RGBColor &rhs) { return r == rhs.r}
    uint8_t r;
    uint8_t g;
    uint8_t b;
};

template <uint8_t fourthValue>
bool TestLoadUbyteRGBToRGBA(ImageLoadContext &context, size_t width, size_t height, size_t depth)
{
    size_t pixelCount = width * height * depth;

    size_t inputPixelBytes = 3;
    size_t inputRowPitch   = width * inputPixelBytes;
    size_t inputDepthPitch = height * inputRowPitch;

    size_t outputPixelBytes = 4;
    size_t outputRowPitch   = width * outputPixelBytes;
    size_t outputDepthPitch = height * outputRowPitch;

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

    // Prepare the input and output for copy.
    std::vector<uint8_t> rgbInput;
    for (auto color : rgbData)
    {
        rgbInput.push_back(color.r);
        rgbInput.push_back(color.g);
        rgbInput.push_back(color.b);
    }
    std::vector<uint8_t> rgbaOutput(outputPixelBytes * pixelCount, 0);

    // Call loading function.
    LoadToNativeUbyte3To4<fourthValue>(context, width, height, depth, rgbInput.data(),
                                       inputRowPitch, inputDepthPitch, rgbaOutput.data(),
                                       outputRowPitch, outputDepthPitch);

    // Compare the input and output data.
    for (size_t pixel = 0; pixel < pixelCount; pixel++)
    {
        size_t inputIndex  = pixel * inputPixelBytes;
        size_t outputIndex = pixel * outputPixelBytes;
        bool rMatch        = rgbInput[inputIndex] == rgbaOutput[outputIndex];
        bool gMatch        = rgbInput[inputIndex + 1] == rgbaOutput[outputIndex + 1];
        bool bMatch        = rgbInput[inputIndex + 2] == rgbaOutput[outputIndex + 2];
        bool aMatch        = rgbaOutput[outputIndex + 3] == fourthValue;
        if (!(rMatch && gMatch && bMatch && aMatch))
        {
            ERR() << "Mismatch at pixel " << pixel << std::endl
                  << "Input: (" << rgbInput[inputIndex] << rgbInput[inputIndex + 1]
                  << rgbInput[inputIndex + 2] << ")" << std::endl
                  << "Output: (" << rgbaOutput[outputIndex] << rgbaOutput[outputIndex + 1]
                  << rgbaOutput[outputIndex + 2] << rgbaOutput[outputIndex + 3] << ")";
            return false;
        }
    }
    return true;
}

// Tests the ubyte RGB to RGBA loading function when the width is 4-byte aligned. This loading
// function can copy 4 bytes at a time in a row.
TEST(LoadToNative, LoadUbyteRGBToRGBADataAlignedWidth)
{
    ImageLoadContext context;
    bool isSuccess = TestLoadUbyteRGBToRGBA<0xFF>(context, 128, 128, 1);
    EXPECT_EQ(isSuccess, true);
}

// Tests the ubyte RGB to RGBA loading function when the width is not 4-byte aligned, which will
// cause the loading function to copy some bytes in the beginning and end of some rows individually.
TEST(LoadToNative, LoadUbyteRGBToRGBADataUnalignedWidth)
{
    ImageLoadContext context;
    bool isSuccess = TestLoadUbyteRGBToRGBA<0xFF>(context, 127, 128, 1);
    EXPECT_EQ(isSuccess, true);
}

// Tests the ubyte RGB to RGBA loading function when the width is less than 4 bytes. Therefore the
// loading function will copy data one byte at a time.
TEST(LoadToNative, LoadUbyteRGBToRGBADataWidthLessThanUint32)
{
    ImageLoadContext context;
    bool isSuccess = TestLoadUbyteRGBToRGBA<0xFF>(context, 3, 256, 1);
    EXPECT_EQ(isSuccess, true);
}

}  // namespace