//
// Copyright 2018 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//
// blocklayout_unittest: Unit tests for the block layout encoder classes.

#include <gtest/gtest.h>

#include "compiler/translator/blocklayout.h"

using namespace sh;

namespace
{

// Tests an array of structs inside a block.
TEST(BlockLayoutTest, Std140ArrayOfStructs)
{
    // The input for this test case is:
    // struct S
    // {
    //     uvec2 uvData;
    //     uint uiData[2];
    // };
    // layout(std140) buffer {
    //     S s[2];
    //     uint lastData;
    // };

    Std140BlockEncoder encoder;

    std::vector<ShaderVariable> fields;
    fields.emplace_back(GL_UNSIGNED_INT_VEC2);
    fields.emplace_back(GL_UNSIGNED_INT, 2);

    std::vector<BlockMemberInfo> fieldMemberInfo;
    BlockMemberInfo lastDataMemberInfo;
    BlockMemberInfo structMemberInfo;

    for (int arrayElement = 0; arrayElement < 2; ++arrayElement)
    {
        encoder.enterAggregateType(fields.data(), 2);

        for (const ShaderVariable &field : fields)
        {
            fieldMemberInfo.push_back(encoder.encodeType(field.type, field.arraySizes, false));
        }

        structMemberInfo = encoder.exitAggregateType(false);
    }

    lastDataMemberInfo = encoder.encodeType(GL_UNSIGNED_INT, std::vector<unsigned int>(), false);
}

// Tests an array of arrays inside a block.
TEST(BlockLayoutTest, Std140ArrayOfArrays)
{
    // The input for this test case is:
    // layout(std140) buffer
    // {
    //     uint a[2][2][2];
    //     uint b;
    // };

    Std140BlockEncoder encoder;

    ShaderVariable arrayType(GL_UNSIGNED_INT);
    arrayType.setArraySize(2);
    arrayType.setArraySize(2);
    arrayType.setArraySize(2);

    BlockMemberInfo arrayMemberInfo;

    encoder.enterAggregateType(&arrayType, 1);
    for (unsigned int index0 = 0; index0 < 2; ++index0)
    {
        for (unsigned int index1 = 0; index1 < 2; ++index1)
        {
            BlockMemberInfo memberInfo =
                encoder.encodeType(arrayType.type, arrayType.arraySizes, false);
            if (arrayMemberInfo.offset == -1)
            {
                arrayMemberInfo = memberInfo;
            }
        }
    }
    encoder.exitAggregateType(false);

    BlockMemberInfo lastDataMemberInfo =
        encoder.encodeType(GL_UNSIGNED_INT, std::vector<unsigned int>(), false);

    EXPECT_EQ(0, arrayMemberInfo.offset);
    EXPECT_EQ(16, arrayMemberInfo.arrayStride);
    EXPECT_EQ(128, lastDataMemberInfo.offset);
}

}  // namespace
