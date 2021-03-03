//
// Copyright 2021 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//
// ListNode.h:
//  What this is -
//      A node class providing mechanism for bidirectional traversal.
//      At initial state, a node points to itself.
//      Through composition or inheritance ListNode can be used to build a circular doubly linked
//      list.
//  What this isn't -
//      This is not a node container in a typical linked list, there is no "data" stored in
//      ListNode.
//   Based on linux kernel linked list implementation (https://kernelnewbies.org/FAQ/LinkedLists)
//

#ifndef COMMON_LISTNODE_H_
#define COMMON_LISTNODE_H_

#include "common/debug.h"

namespace angle
{
class ListNode
{
  public:
    ListNode() { resetLinks(); }
    ListNode(const ListNode &other) { resetLinks(); }
    ListNode &operator=(const ListNode &other)
    {
        resetLinks();
        return *this;
    }
    ANGLE_INLINE ListNode *getNext() const { return mNext; }
    ANGLE_INLINE ListNode *getPrevious() const { return mPrev; }
    ANGLE_INLINE void resetLinks()
    {
        mPrev = static_cast<angle::ListNode *>(this);
        mNext = static_cast<angle::ListNode *>(this);
    }
    ANGLE_INLINE void insertListNode(ListNode *node)
    {
        ASSERT(node);

        // Update node's links
        node->mNext = mNext;
        node->mPrev = this;

        // Update current next's link
        mNext->mPrev = node;

        // Update current node's link
        mNext = node;
    }
    ANGLE_INLINE void removeListNode(ListNode *node)
    {
        // Update node's previous and next nodes' links
        node->mPrev->mNext = node->mNext;
        node->mNext->mPrev = node->mPrev;

        // Reset node's links
        node->resetLinks();
    }

  private:
    ListNode *mPrev;
    ListNode *mNext;
};
}  // namespace angle

#endif  // COMMON_LISTNODE_H_
