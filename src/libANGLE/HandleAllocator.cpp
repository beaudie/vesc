//
// Copyright 2002 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//

// HandleAllocator.cpp: Implements the gl::HandleAllocator class, which is used
// to allocate GL handles.

#include "libANGLE/HandleAllocator.h"

#include <algorithm>
#include <functional>
#include <limits>

#include "common/debug.h"

namespace gl
{

namespace
{
template <typename T, size_t Count, typename Pred>
ANGLE_INLINE void FastVectorEraseIf(angle::FastVector<T, Count> &vec, Pred p)
{
    while (!vec.empty())
    {
        auto iter = std::find_if(vec.begin(), vec.end(), p);
        if (iter == vec.end())
        {
            return;
        }

        vec.remove_and_permute(iter);
    }
}
}  // namespace

HandleAllocator::HandleAllocator() : HandleAllocator(std::numeric_limits<GLuint>::max()) {}

HandleAllocator::HandleAllocator(GLuint maximumHandleValue)
    : mNextValue(1), mMaxValue(maximumHandleValue), mLoggingEnabled(false)
{}

HandleAllocator::~HandleAllocator() {}

void HandleAllocator::setBaseHandle(GLuint value)
{
    mNextValue = std::max(mNextValue, value);
    FastVectorEraseIf(mReleasedList, [=](GLuint x) { return x < value; });
}

GLuint HandleAllocator::allocate()
{
    // Allocate from released list if possible
    if (!mReleasedList.empty())
    {
        GLuint reusedHandle = mReleasedList.back();
        mReleasedList.pop_back();

        if (mLoggingEnabled)
        {
            WARN() << "HandleAllocator::allocate reusing " << reusedHandle << std::endl;
        }

        return reusedHandle;
    }

    // The next value may be reserved. Iterate until an unreserved value is found
    if (!mReservedList.empty())
    {
        while (std::find(mReservedList.begin(), mReservedList.end(), mNextValue) !=
               mReservedList.end())
        {
            mNextValue++;
        }
    }

    GLuint nextHandle = mNextValue++;

    if (mLoggingEnabled)
    {
        WARN() << "HandleAllocator::allocate allocating " << nextHandle << std::endl;
    }

    return nextHandle;
}

void HandleAllocator::release(GLuint handle)
{
    if (mLoggingEnabled)
    {
        WARN() << "HandleAllocator::release releasing " << handle << std::endl;
    }

    FastVectorEraseIf(mReleasedList, [=](GLuint x) { return x == handle; });
    if (handle <= mNextValue)
    {
        mReleasedList.push_back(handle);
    }
}

void HandleAllocator::reserve(GLuint handle)
{
    if (mLoggingEnabled)
    {
        WARN() << "HandleAllocator::reserve reserving " << handle << std::endl;
    }

    mReservedList.push_back(handle);
    FastVectorEraseIf(mReleasedList, [=](GLuint x) { return x == handle; });
}

void HandleAllocator::reset()
{
    mReservedList.clear();
    mReleasedList.clear();
    mNextValue = 1;
}

bool HandleAllocator::anyHandleAvailableForAllocation() const
{
    return !mReleasedList.empty() || mNextValue < mMaxValue;
}

void HandleAllocator::enableLogging(bool enabled)
{
    mLoggingEnabled = enabled;
}

}  // namespace gl
