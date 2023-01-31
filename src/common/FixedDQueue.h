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
// queue is not empty before pop and not full before push. This class supports concurrent push and
// pop from different threads. If caller want to push from two different threads, proper mutex must
// be used to ensure the access is serialized.
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
    // index for current head
    size_type mFrontIndex;
    // index for next write
    size_type mBackIndex;
    // atomic so that we can support concurrent push and pop.
    std::atomic<size_type> mSize;
};

template <class T, size_t N, class Storage>
FixedDQueue<T, N, Storage>::FixedDQueue() : mFrontIndex(0), mBackIndex(0), mSize(0)
{}

template <class T, size_t N, class Storage>
FixedDQueue<T, N, Storage>::~FixedDQueue()
{}

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
    mData[mBackIndex] = value;
    mBackIndex        = (mBackIndex + 1) % N;
    // We must increment size last, after we wrote data. That way if another thread is doing
    // `if(!dq.empty()){ s = dq.front(); }`, it will only see not empty until element is fully
    // pushed.
    mSize++;
}

template <class T, size_t N, class Storage>
void FixedDQueue<T, N, Storage>::push(value_type &&value)
{
    ASSERT(mSize < mData.size());
    mData[mBackIndex] = std::move(value);
    mBackIndex        = (mBackIndex + 1) % N;
    // We must increment size last, after we wrote data. That way if another thread is doing
    // `if(!dq.empty()){ s = dq.front(); }`, it will only see not empty until element is fully
    // pushed.
    mSize++;
}

template <class T, size_t N, class Storage>
void FixedDQueue<T, N, Storage>::pop()
{
    ASSERT(mSize > 0);
    mData[mFrontIndex] = value_type();
    mFrontIndex        = (mFrontIndex + 1) % N;
    // We must decrement size last, after we wrote data. That way if another thread is doing
    // `if(!dq.full()){ dq.push; }`, it will only see not full until element is fully popped.
    mSize--;
}
}  // namespace angle

#endif  // COMMON_FIXEDDQUEUE_H_
