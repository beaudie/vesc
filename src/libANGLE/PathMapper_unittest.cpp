//
// Copyright 2016 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//
// Unit tests for PathMapper.
//

#include "gmock/gmock.h"
#include "gtest/gtest.h"

#include "libANGLE/PathMapper.h"

#include <limits>

namespace
{

class PathMapperTest : public testing::Test
{
  protected:
    gl::PathMapper mMapper;
};

TEST_F(PathMapperTest, Basic)
{
    const GLuint kClient1Id  = 1;
    const GLuint kService1Id = 11;
    const GLuint kClient2Id  = 2;
    GLuint serviceId = 0;
    mMapper.createMapping(kClient1Id, kClient1Id, kService1Id);
    ASSERT_TRUE(mMapper.hasPathsInRange(kClient1Id, kClient1Id));
    EXPECT_TRUE(mMapper.getPath(kClient1Id, &serviceId));
    EXPECT_EQ(kService1Id, serviceId);

    // Check we get nothing for a non-existent path.
    serviceId = 123u;
    ASSERT_FALSE(mMapper.hasPathsInRange(kClient2Id, kClient2Id));
    EXPECT_FALSE(mMapper.getPath(kClient2Id, &serviceId));
    EXPECT_EQ(123u, serviceId);

    std::vector<gl::PathMapper::RemovedRange> vec;

    // Check trying to remove non-existent paths does not crash.
    mMapper.removeMapping(kClient2Id, kClient2Id, &vec);
    EXPECT_TRUE(vec.empty());

    mMapper.removeMapping(kClient1Id, kClient1Id, &vec);
    EXPECT_EQ(vec.size(), 1u);
    EXPECT_EQ(vec[0].mFirstServiceHandle, kService1Id);
    EXPECT_EQ(vec[0].mRange, 1u);

    EXPECT_FALSE(mMapper.hasPathsInRange(kClient1Id, kClient1Id));
    EXPECT_FALSE(mMapper.getPath(kClient1Id, &serviceId));
}

// Tests that path mapper does not merge ranges that contain service ids that
//  prevent the merging. Path ranges A and B can be merged iff:
//  - client ids of B start immediately after the last client id of A
//  - service ids of B start immediately after the last service id of A
// and similarly for the 'before' case.
TEST_F(PathMapperTest, NonContiguousServiceIds)
{
    const GLuint kMergeCheckRange = 54;

    const struct
    {
        GLuint firstClientId;
        GLuint lastClientId;
        GLuint firstServiceId;

        GLuint range() const
        {
            return lastClientId - firstClientId + 1;
        }
    }
    kIdRanges[] =
    {
        {500, 1000, 900},
        {1001, 1155, 1},
        {200, 499, 4888}
    };

    for (const auto& range : kIdRanges)
    {
        mMapper.createMapping(range.firstClientId, range.lastClientId, range.firstServiceId);
        ASSERT_TRUE(mMapper.hasPathsInRange(range.firstClientId, range.firstClientId));
        ASSERT_TRUE(mMapper.hasPathsInRange(range.lastClientId, range.lastClientId));
        ASSERT_TRUE(mMapper.hasPathsInRange(range.firstClientId, range.lastClientId));
        GLuint serviceId = 0u;
        EXPECT_TRUE(mMapper.getPath(range.firstClientId + 5u, &serviceId));
        EXPECT_EQ(range.firstServiceId + 5u, serviceId);
    }

    // Insert a mergeable range last, to check that merges work.
    // Otherwise the test could succeed because merges were not working.
    const auto& merge_candidate = kIdRanges[1];
    const GLuint merge_candidate_range = merge_candidate.range();
    mMapper.createMapping(
        merge_candidate.lastClientId + 1,
        merge_candidate.lastClientId + kMergeCheckRange,
        merge_candidate.firstServiceId + merge_candidate_range);

    std::vector<gl::PathMapper::RemovedRange> vec;

    mMapper.removeAll(&vec);
    EXPECT_EQ(vec.size(), 3);
    // note the order of items here is ascending based on client handle value.
    EXPECT_EQ(vec[0].mFirstServiceHandle, kIdRanges[2].firstServiceId);
    EXPECT_EQ(vec[0].mRange, kIdRanges[2].range());
    EXPECT_EQ(vec[1].mFirstServiceHandle, kIdRanges[0].firstServiceId);
    EXPECT_EQ(vec[1].mRange, kIdRanges[0].range());
    EXPECT_EQ(vec[2].mFirstServiceHandle, kIdRanges[1].firstServiceId);
    EXPECT_EQ(vec[2].mRange, kIdRanges[1].range() + kMergeCheckRange);
}

TEST_F(PathMapperTest, DeleteBigRange)
{
    // Allocates two ranges which in path manager end up merging as one
    // big range. The range will be too big to fit in one DeletePaths
    // call.  Test that the range is deleted correctly with two calls.
    const GLuint  kFirstClientId1  = 1;
    const GLsizei kRange1          = std::numeric_limits<GLsizei>::max() - 3;
    const GLuint  kLastClientId1   = kFirstClientId1 + kRange1 - 1;
    const GLuint  kFirstServiceId1 = 77;
    const GLuint  kLastServiceId1  = kFirstServiceId1 + kRange1 - 1;
    const GLuint  kFirstClientId2  = kLastClientId1 + 1;
    const GLsizei kRange2          = 15;
    const GLuint  kLastClientId2   = kFirstClientId2 + kRange2 - 1;
    const GLuint  kFirstServiceId2 = kLastServiceId1 + 1;

    std::vector<gl::PathMapper::RemovedRange> vec;

    mMapper.createMapping(kFirstClientId1, kLastClientId1, kFirstServiceId1);
    mMapper.createMapping(kFirstClientId2, kLastClientId2, kFirstServiceId2);
    mMapper.removeAll(&vec);

    EXPECT_EQ(vec.size(), 1);
    EXPECT_EQ(vec[0].mFirstServiceHandle, kFirstServiceId1);
    EXPECT_EQ(vec[0].mRange, GLuint(kRange1) + 15);

}




} // namespace