//
// Copyright 2021 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//
// ListNode_unittest:
//   Tests of the ListNode class
//

#include <gtest/gtest.h>

#include "common/ListNode.h"

namespace angle
{
// Test basic constructors
TEST(ListNode, BasicConstructors)
{
    ListNode defaultContructor;
    EXPECT_EQ(&defaultContructor, defaultContructor.getPrevious());
    EXPECT_EQ(&defaultContructor, defaultContructor.getNext());

    ListNode copyConstructor(defaultContructor);
    EXPECT_NE(&defaultContructor, copyConstructor.getPrevious());
    EXPECT_NE(&defaultContructor, copyConstructor.getNext());
    EXPECT_EQ(&copyConstructor, copyConstructor.getPrevious());
    EXPECT_EQ(&copyConstructor, copyConstructor.getNext());

    ListNode copyAssignment = defaultContructor;
    EXPECT_NE(&defaultContructor, copyAssignment.getPrevious());
    EXPECT_NE(&defaultContructor, copyAssignment.getNext());
    EXPECT_EQ(&copyAssignment, copyAssignment.getPrevious());
    EXPECT_EQ(&copyAssignment, copyAssignment.getNext());
}

// Test constructors with inheritance
TEST(ListNode, InheritedConstructors)
{
    class TestList : public ListNode
    {
      public:
        TestList()                      = default;
        TestList(const TestList &other) = default;
        TestList &operator=(const TestList &other) = default;
    };

    TestList defaultContructor;
    EXPECT_EQ(&defaultContructor, defaultContructor.getPrevious());
    EXPECT_EQ(&defaultContructor, defaultContructor.getNext());

    TestList copyConstructor(defaultContructor);
    EXPECT_NE(&defaultContructor, copyConstructor.getPrevious());
    EXPECT_NE(&defaultContructor, copyConstructor.getNext());
    EXPECT_EQ(&copyConstructor, copyConstructor.getPrevious());
    EXPECT_EQ(&copyConstructor, copyConstructor.getNext());

    TestList copyAssignment = defaultContructor;
    EXPECT_NE(&defaultContructor, copyAssignment.getPrevious());
    EXPECT_NE(&defaultContructor, copyAssignment.getNext());
    EXPECT_EQ(&copyAssignment, copyAssignment.getPrevious());
    EXPECT_EQ(&copyAssignment, copyAssignment.getNext());
}

// Test traversal and updates
TEST(ListNode, Traversal)
{
    class GarbageCollector
    {
      public:
        void addGarbage(ListNode *node)
        {
            // Verify that we haven't added this node already
            ASSERT(node->getNext() == node);
            mSentinelNode.insertListNode(node);
        }
        void removeGarbage(ListNode *node)
        {
            // Verify that the node was added
            ASSERT(node->getNext() != node);
            mSentinelNode.removeListNode(node);
        }
        size_t garbageObjectCount()
        {
            size_t garbageObjectCount = 0;
            ListNode *node            = mSentinelNode.getNext();
            while (node != &mSentinelNode)
            {
                garbageObjectCount++;
                node = node->getNext();
            }
            return garbageObjectCount;
        }
        size_t collectGarbage()
        {
            size_t garbageObjectCount = 0;
            ListNode *node            = mSentinelNode.getNext();
            ListNode *next            = nullptr;
            while (node != &mSentinelNode)
            {
                next = node->getNext();
                node->detachListNode();
                delete node;
                node = next;
                garbageObjectCount++;
            }
            return garbageObjectCount;
        }

      private:
        ListNode mSentinelNode;
    };

    class Image : public ListNode
    {};

    class Buffer : public ListNode
    {};

    class Surface : public ListNode
    {};

    GarbageCollector gc;
    std::vector<Image *> images;
    std::vector<Buffer *> buffers;
    std::vector<Surface *> surfaces;

    images.emplace_back(new Image());
    buffers.emplace_back(new Buffer());
    buffers.emplace_back(new Buffer());
    surfaces.emplace_back(new Surface());

    // Add all objects to GarbageCollector
    size_t expectedGarbageObjectCount = 0;
    for (auto ptr : images)
    {
        gc.addGarbage(ptr);
        expectedGarbageObjectCount++;
    }
    for (auto ptr : buffers)
    {
        gc.addGarbage(ptr);
        expectedGarbageObjectCount++;
    }
    for (auto ptr : surfaces)
    {
        gc.addGarbage(ptr);
        expectedGarbageObjectCount++;
    }

    // Verify garbage object count
    EXPECT_EQ(expectedGarbageObjectCount, gc.garbageObjectCount());

    // Remove all buffers from GarbageCollector
    for (auto ptr : buffers)
    {
        gc.removeGarbage(ptr);
        expectedGarbageObjectCount--;
    }

    // Verify garbage object count
    EXPECT_EQ(expectedGarbageObjectCount, gc.garbageObjectCount());

    // Add back all buffers to GarbageCollector
    for (auto ptr : buffers)
    {
        gc.addGarbage(ptr);
        expectedGarbageObjectCount++;
    }

    // Verify garbage object count
    EXPECT_EQ(expectedGarbageObjectCount, gc.collectGarbage());
}
}  // namespace angle
