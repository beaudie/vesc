//
// Copyright 2024 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//
// Decompress_unittest.cpp: Unit tests for the |(Compress/Decompress)*Blob| functions.

#include <gtest/gtest.h>

#include "libANGLE/angletypes.h"

namespace angle
{
namespace
{
std::vector<uint8_t> GetTestData()
{
    constexpr size_t testDataSize = 100'000;

    std::vector<uint8_t> testData(testDataSize);

    for (size_t i = 0; i < testDataSize; ++i)
    {
        testData[i] = static_cast<uint8_t>(i);
    }

    return testData;
}
}  // anonymous namespace

// Tests that decompressing full data has no errors.
TEST(DecompressTest, FullData)
{
    const std::vector<uint8_t> testData = GetTestData();

    MemoryBuffer compressedData;
    MemoryBuffer uncompressedData;

    ASSERT_TRUE(CompressBlob(testData.size(), testData.data(), &compressedData));

    EXPECT_TRUE(DecompressBlob(compressedData.data(), compressedData.size(), testData.size(),
                               &uncompressedData));
    EXPECT_EQ(testData.size(), uncompressedData.size());
    EXPECT_EQ(memcmp(testData.data(), uncompressedData.data(),
                     std::min(testData.size(), uncompressedData.size())),
              0);
    uncompressedData.clear();

    bool partial = false;
    EXPECT_TRUE(DecompressPartialBlob(compressedData.data(), compressedData.size(), testData.size(),
                                      &uncompressedData, &partial));
    EXPECT_FALSE(partial);
    EXPECT_EQ(testData.size(), uncompressedData.size());
    EXPECT_EQ(memcmp(testData.data(), uncompressedData.data(),
                     std::min(testData.size(), uncompressedData.size())),
              0);
    uncompressedData.clear();
}

// Tests decompressing invalid data or with invalid arguments.
TEST(DecompressTest, InvalidUse)
{
    const std::vector<uint8_t> testData = GetTestData();

    MemoryBuffer compressedData;
    MemoryBuffer uncompressedData;

    ASSERT_TRUE(CompressBlob(testData.size(), testData.data(), &compressedData));

    // Pass |maxUncompressedDataSize| less than test data size.
    EXPECT_FALSE(DecompressBlob(compressedData.data(), compressedData.size(), testData.size() - 1,
                                &uncompressedData));

    // Pass |uncompressedSize| less than test data size.
    bool partial = false;
    EXPECT_FALSE(DecompressPartialBlob(compressedData.data(), compressedData.size(),
                                       testData.size() - 1, &uncompressedData, &partial));

    // Pass |uncompressedSize| more than test data size.
    partial = false;
    EXPECT_TRUE(DecompressPartialBlob(compressedData.data(), compressedData.size(),
                                      testData.size() + 1, &uncompressedData, &partial));

    // Despite passing invalid value, function will succeed and uncompressedData will be valid.
    EXPECT_FALSE(partial);
    EXPECT_EQ(testData.size(), uncompressedData.size());
    EXPECT_EQ(memcmp(testData.data(), uncompressedData.data(),
                     std::min(testData.size(), uncompressedData.size())),
              0);
    uncompressedData.clear();

    // Try to decompress partial compressed data.
    EXPECT_FALSE(DecompressBlob(compressedData.data(), compressedData.size() - 1, 256 * 1024 * 1024,
                                &uncompressedData));

    // Corrupt the compressed data.
    const size_t corruptIndex = compressedData.size() / 2;
    compressedData[corruptIndex] ^= 255;
    // Try to decompress corrupted data.
    EXPECT_FALSE(DecompressBlob(compressedData.data(), compressedData.size(), testData.size(),
                                &uncompressedData));

    // Restore the compressed data.
    compressedData[corruptIndex] ^= 255;
    // Check that data was restored successfully.
    EXPECT_TRUE(DecompressBlob(compressedData.data(), compressedData.size(), testData.size(),
                               &uncompressedData));
    EXPECT_EQ(testData.size(), uncompressedData.size());
    EXPECT_EQ(memcmp(testData.data(), uncompressedData.data(),
                     std::min(testData.size(), uncompressedData.size())),
              0);
    uncompressedData.clear();

    // Decrease decompressed size in the compressed data last dword.
    ASSERT_TRUE(IsLittleEndian());
    uint32_t newUncompressedSize = static_cast<uint32_t>(testData.size() - 1);
    ASSERT_GT(compressedData.size(), sizeof(newUncompressedSize));
    memcpy(compressedData.data() + compressedData.size() - sizeof(newUncompressedSize),
           &newUncompressedSize, sizeof(newUncompressedSize));

    // Try to decompress with decreased size in the last dword.
    EXPECT_FALSE(DecompressBlob(compressedData.data(), compressedData.size(), newUncompressedSize,
                                &uncompressedData));

    // Increase decompressed size in the compressed data last dword.
    newUncompressedSize = static_cast<uint32_t>(testData.size() + 1);
    memcpy(compressedData.data() + compressedData.size() - sizeof(newUncompressedSize),
           &newUncompressedSize, sizeof(newUncompressedSize));

    // Try to decompress with increased size in the last dword.
    EXPECT_FALSE(DecompressBlob(compressedData.data(), compressedData.size(), newUncompressedSize,
                                &uncompressedData));

    // Set zero to the compressed data last dword.
    newUncompressedSize = 0;
    memcpy(compressedData.data() + compressedData.size() - sizeof(newUncompressedSize),
           &newUncompressedSize, sizeof(newUncompressedSize));

    // Try to decompress with increased size in the last dword.
    EXPECT_FALSE(DecompressBlob(compressedData.data(), compressedData.size(), testData.size(),
                                &uncompressedData));
}

// Tests that decompressing partial data has no errors.
TEST(DecompressTest, PartialData)
{
    const std::vector<uint8_t> testData = GetTestData();

    MemoryBuffer compressedData;
    MemoryBuffer uncompressedData;

    ASSERT_TRUE(CompressBlob(testData.size(), testData.data(), &compressedData));

    // Decompress partial data where only last dword is missing.
    bool partial = false;
    EXPECT_TRUE(DecompressPartialBlob(compressedData.data(), compressedData.size() - 1,
                                      testData.size(), &uncompressedData, &partial));
    EXPECT_TRUE(partial);

    // Despite decompress is partial, we should still get full data because compressed data was only
    // missing last dword with uncompressed data size.
    EXPECT_EQ(testData.size(), uncompressedData.size());
    EXPECT_EQ(memcmp(testData.data(), uncompressedData.data(),
                     std::min(testData.size(), uncompressedData.size())),
              0);
    uncompressedData.clear();

    // Decompress half of the data.
    partial = false;
    EXPECT_TRUE(DecompressPartialBlob(compressedData.data(), compressedData.size() / 2,
                                      testData.size(), &uncompressedData, &partial));
    EXPECT_TRUE(partial);

    // Decompressed partial data should match beginning of the test data.
    EXPECT_GT(testData.size(), uncompressedData.size());
    EXPECT_EQ(memcmp(testData.data(), uncompressedData.data(),
                     std::min(testData.size(), uncompressedData.size())),
              0);
    uncompressedData.clear();
}

}  // namespace angle
