//
// Copyright (c) 2002-2011 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//

// HandleAllocator.cpp: Implements the gl::HandleAllocator class, which is used
// to allocate GL handles.

#include "libANGLE/HandleAllocator.h"

#include <algorithm>

#include "common/debug.h"

namespace gl
{

struct HandleAllocator::HandleRangeComparator
{
    bool operator()(const HandleRange &range, GLuint handle) const
    {
        return (handle < range.begin);
    }
};

HandleAllocator::HandleAllocator() : mBaseValue(1), mNextValue(1)
{
    mFreeList.push_back(HandleRange(1, std::numeric_limits<GLuint>::max() - 1));
}

HandleAllocator::~HandleAllocator()
{
}

void HandleAllocator::setBaseHandle(GLuint value)
{
    ASSERT(mBaseValue == mNextValue);
    mBaseValue = value;
    mNextValue = value;
}

GLuint HandleAllocator::allocate()
{
    ASSERT(!mFreeList.empty());
    auto freeListIt = mFreeList.begin();

    GLuint freeListHandle = freeListIt->begin;
    ASSERT(freeListHandle > 0);

    freeListIt->begin++;
    if (freeListIt->begin == freeListIt->end)
    {
        mFreeList.erase(freeListIt);
    }

    return freeListHandle;
}

void HandleAllocator::release(GLuint)
{
    // Can re-use handles if we feel like it
}

void HandleAllocator::reserve(GLuint handle)
{
    auto boundIt = std::lower_bound(mFreeList.begin(), mFreeList.end(), handle, HandleRangeComparator());

    ASSERT(boundIt != mFreeList.end());

    GLuint begin = boundIt->begin;
    GLuint end = boundIt->end;

    if (handle == begin || handle == end)
    {
        if (begin + 1 == end)
        {
            mFreeList.erase(boundIt);
        }
        else if (handle == begin)
        {
            boundIt->begin++;
        }
        else
        {
            ASSERT(handle == end);
            boundIt->end--;
        }
        return;
    }

    // need to split the range
    auto placementIt = mFreeList.erase(boundIt);

    if (begin != handle)
    {
        placementIt = mFreeList.insert(placementIt, HandleRange(begin, handle));
    }
    if (handle + 1 != end)
    {
        mFreeList.insert(placementIt, HandleRange(handle + 1, end));
    }
}

}
