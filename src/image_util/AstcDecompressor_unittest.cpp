//
// Copyright 2022 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//
// AstcDecompressor_unittest.cpp: Unit tests for AstcDecompressor

#include "AstcDecompressor.h"

#include <gmock/gmock.h>
#include <vector>
#include "AstcDecompressorTestUtils.h"

using namespace angle;
using namespace testing;

namespace
{

TEST(AstcDecompressor, Decompress)
{
    const int width  = 1024;
    const int height = 1024;

    auto &decompressor = AstcDecompressor::get();
    if (!decompressor.available())
        skipTest("ASTC decompressor not available");

    std::vector<Rgba> output(width * height);
    std::vector<uint8_t> astcData = makeAstcCheckerboard(width, height);
    int32_t status = decompressor.decompress(width, height, 8, 8, astcData.data(), astcData.size(),
                                             (uint8_t *)output.data());
    EXPECT_EQ(status, 0);

    std::vector<Rgba> expected = makeCheckerboard(width, height);

    ASSERT_THAT(output, ElementsAreArray(expected));
}

TEST(AstcDecompressor, getStatusStringAlwaysNonNull)
{
    EXPECT_THAT(AstcDecompressor::get().getStatusString(-10000), NotNull());
}

}  // namespace