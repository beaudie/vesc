//
// Copyright 2021 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//
// FixedDQueue.h:
//   An array class with two indices that works similar to std::dqueue.
//

#ifndef COMMON_FIXEDDQUEUE_H_
#define COMMON_FIXEDDQUEUE_H_

#include "common/debug.h"

#include <algorithm>
#include <array>

namespace angle
{
// class FixedDQueue: An array class with two indices that point to the front and back of the double
// ended queue. Works similar to std::dqueue except with a fixed size storage. Caller must ensure
// queue is not empty before pop and not full before push.
template <class T, size_t N, class Storage = std::array<T, N>>
class FixedDQueue final : angle::NonCopyable
{
  public:
    using value_type      = typename Storage::value_type;
    using size_type       = typename Storage::size_type;
    using reference       = typename Storage::reference;
    using const_reference = typename Storage::const_reference;

    FixedDQueue();
    ~FixedDQueue();

    size_type size() const;
    bool empty() const;
    bool full() const;

    reference front();
    const_reference front() const;

    void push(const value_type &value);
    void push(value_type &&value);

    void pop();

  private:
    Storage mData;
    size_type mFrontIndex;
    size_type mBackIndex;
    std::atomic<size_type> mSize;
};

template <class T, size_t N, class Storage>
FixedDQueue<T, N, Storage>::FixedDQueue() : mFrontIndex(0), mBackIndex(0), mSize(0)
{}

template <class T, size_t N, class Storage>
FixedDQueue<T, N, Storage>::~FixedDQueue() = default;

template <class T, size_t N, class Storage>
ANGLE_INLINE typename FixedDQueue<T, N, Storage>::size_type FixedDQueue<T, N, Storage>::size() const
{
    return mSize;
}

template <class T, size_t N, class Storage>
ANGLE_INLINE bool FixedDQueue<T, N, Storage>::empty() const
{
    return mSize == 0;
}

template <class T, size_t N, class Storage>
ANGLE_INLINE bool FixedDQueue<T, N, Storage>::full() const
{
    return mSize >= N;
}

template <class T, size_t N, class Storage>
ANGLE_INLINE typename FixedDQueue<T, N, Storage>::reference FixedDQueue<T, N, Storage>::front()
{
    ASSERT(mSize > 0);
    return mData[mFrontIndex];
}

template <class T, size_t N, class Storage>
ANGLE_INLINE typename FixedDQueue<T, N, Storage>::const_reference
FixedDQueue<T, N, Storage>::front() const
{
    ASSERT(mSize > 0);
    return mData[mFrontIndex];
}

template <class T, size_t N, class Storage>
void FixedDQueue<T, N, Storage>::push(const value_type &value)
{
    ASSERT(mSize < mData.size());
    mSize++;
    mData[mBackIndex] = value;
    mBackIndex        = (mBackIndex + 1) % N;
}

template <class T, size_t N, class Storage>
void FixedDQueue<T, N, Storage>::push(value_type &&value)
{
    ASSERT(mSize < mData.size());
    mSize++;
    mData[mBackIndex] = std::move(value);
    mBackIndex        = (mBackIndex + 1) % N;
}

template <class T, size_t N, class Storage>
void FixedDQueue<T, N, Storage>::pop()
{
    ASSERT(mSize > 0);
    mFrontIndex = (mFrontIndex + 1) % N;
    mSize--;
}
}  // namespace angle

#endif  // COMMON_FIXEDDQUEUE_H_
