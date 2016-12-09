//
// Copyright (c) 2017 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//

#include "gmock/gmock.h"
#include "gtest/gtest.h"

#include "common/BitSetIterator.h"
#include "common/utilities.h"
#include "libANGLE/VertexArray.h"

using namespace gl;

TEST(VertexArrayTest, VerifyGetAttribIndex)
{
    VertexArray::DirtyBits dirtyBits;
    size_t bits[] = {1, 4, 9, 16, 25, 36, 49, 64, 81, 90};
    int count     = sizeof(bits) / sizeof(size_t);
    for (int i = 0; i < count; i++)
    {
        dirtyBits.set(bits[i]);
    }

    for (unsigned long dirtyBit : angle::IterateBitSet(dirtyBits))
    {
        size_t index = VertexArray::GetAttribIndex(dirtyBit);
        if (dirtyBit < VertexArray::DIRTY_BIT_ATTRIB_MAX_ENABLED)
        {
            EXPECT_EQ(index, dirtyBit - VertexArray::DIRTY_BIT_ATTRIB_0_ENABLED);
        }
        else if (dirtyBit < VertexArray::DIRTY_BIT_ATTRIB_MAX_POINTER)
        {
            EXPECT_EQ(index, dirtyBit - VertexArray::DIRTY_BIT_ATTRIB_0_POINTER);
        }
        else if (dirtyBit < VertexArray::DIRTY_BIT_ATTRIB_MAX_FORMAT)
        {
            EXPECT_EQ(index, dirtyBit - VertexArray::DIRTY_BIT_ATTRIB_0_FORMAT);
        }
        else if (dirtyBit < VertexArray::DIRTY_BIT_ATTRIB_MAX_BINDING)
        {
            EXPECT_EQ(index, dirtyBit - VertexArray::DIRTY_BIT_ATTRIB_0_BINDING);
        }
        else if (dirtyBit < VertexArray::DIRTY_BIT_BINDING_MAX_BUFFER)
        {
            EXPECT_EQ(index, dirtyBit - VertexArray::DIRTY_BIT_BINDING_0_BUFFER);
        }
        else if (dirtyBit < VertexArray::DIRTY_BIT_BINDING_MAX_DIVISOR)
        {
            EXPECT_EQ(index, dirtyBit - VertexArray::DIRTY_BIT_BINDING_0_DIVISOR);
        }
        else
            UNREACHABLE();
    }
}
