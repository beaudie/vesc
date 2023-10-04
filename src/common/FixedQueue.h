//
// Copyright 2023 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//
// FixedQueue.h:
//   An array based fifo queue class that supports concurrent push and pop.
//

#ifndef COMMON_FIXEDQUEUE_H_
#define COMMON_FIXEDQUEUE_H_

#include "common/debug.h"

#include <algorithm>
#include <array>
#include <atomic>

namespace angle
{
// class FixedQueue: An vector based fifo queue class that supports concurrent push and
// pop. Caller must ensure queue is not empty before pop and not full before push. This class
// supports concurrent push and pop from different threads, but only with single producer single
// consumer usage. If caller want to push from two different threads, proper mutex must be used to
// ensure the access is serialized. You can also updateCapacity to adjust the storage size, but
// caller must take proper mutex lock to ensure no one is accessing the storage.
template <class T, size_t N>
class FixedQueue final : angle::NonCopyable
{
  public:
    using Storage         = std::vector<T>;
    using value_type      = typename Storage::value_type;
    using size_type       = typename Storage::size_type;
    using reference       = typename Storage::reference;
    using const_reference = typename Storage::const_reference;

    FixedQueue();
    ~FixedQueue();

    size_type size() const;
    bool empty() const;
    bool full() const;

    size_type capacity() const;
    // Caller must ensure no one is accessing the data.
    void updateCapacity(size_t newCapacity);

    reference front();
    const_reference front() const;

    void push(const value_type &value);
    void push(value_type &&value);

    reference back();
    const_reference back() const;

    void pop();
    void clear();

  private:
    Storage mData;
    // The front and back indices are virtual indices (think about queue sizd is infinite). They
    // will never wrap around when hit N. The wrap around occur when element is referenced. Virtual
    // index for current head
    size_type mFrontIndex;
    // Virtual index for next write.
    size_type mEndIndex;
    // Atomic so that we can support concurrent push and pop.
    std::atomic<size_type> mSize;
};

template <class T, size_t N>
FixedQueue<T, N>::FixedQueue() : mData(N), mFrontIndex(0), mEndIndex(0), mSize(0)
{
    mData.resize(N);
}

template <class T, size_t N>
FixedQueue<T, N>::~FixedQueue()
{
    mData.clear();
}

template <class T, size_t N>
ANGLE_INLINE typename FixedQueue<T, N>::size_type FixedQueue<T, N>::size() const
{
    return mSize;
}

template <class T, size_t N>
ANGLE_INLINE bool FixedQueue<T, N>::empty() const
{
    return mSize == 0;
}

template <class T, size_t N>
ANGLE_INLINE bool FixedQueue<T, N>::full() const
{
    return mSize >= N;
}

template <class T, size_t N>
ANGLE_INLINE typename FixedQueue<T, N>::size_type FixedQueue<T, N>::capacity() const
{
    return mData.size();
}

template <class T, size_t N>
ANGLE_INLINE void FixedQueue<T, N>::updateCapacity(size_t newCapacity)
{
    ASSERT(newCapacity > mSize);
    if (newCapacity < mData.size())
    {
        Storage newData(newCapacity);
        for (value_type &v : mData)
        {
            newData.push_back(std::move(v));
        }
        std::swap(newData, mData);
    }
    else
    {
        mData.resize(newCapacity);
    }
}

template <class T, size_t N>
ANGLE_INLINE typename FixedQueue<T, N>::reference FixedQueue<T, N>::front()
{
    ASSERT(mSize > 0);
    return mData[mFrontIndex % N];
}

template <class T, size_t N>
ANGLE_INLINE typename FixedQueue<T, N>::const_reference FixedQueue<T, N>::front() const
{
    ASSERT(mSize > 0);
    return mData[mFrontIndex % N];
}

template <class T, size_t N>
void FixedQueue<T, N>::push(const value_type &value)
{
    ASSERT(mSize < N);
    mData[mEndIndex % N] = value;
    mEndIndex++;
    // We must increment size last, after we wrote data. That way if another thread is doing
    // `if(!dq.empty()){ s = dq.front(); }`, it will only see not empty until element is fully
    // pushed.
    mSize++;
}

template <class T, size_t N>
void FixedQueue<T, N>::push(value_type &&value)
{
    ASSERT(mSize < N);
    mData[mEndIndex % N] = std::move(value);
    mEndIndex++;
    // We must increment size last, after we wrote data. That way if another thread is doing
    // `if(!dq.empty()){ s = dq.front(); }`, it will only see not empty until element is fully
    // pushed.
    mSize++;
}

template <class T, size_t N>
ANGLE_INLINE typename FixedQueue<T, N>::reference FixedQueue<T, N>::back()
{
    ASSERT(mSize > 0);
    return mData[(mEndIndex + (N - 1)) % N];
}

template <class T, size_t N>
ANGLE_INLINE typename FixedQueue<T, N>::const_reference FixedQueue<T, N>::back() const
{
    ASSERT(mSize > 0);
    return mData[(mEndIndex + (N - 1)) % N];
}

template <class T, size_t N>
void FixedQueue<T, N>::pop()
{
    ASSERT(mSize > 0);
    mData[mFrontIndex % N] = value_type();
    mFrontIndex++;
    // We must decrement size last, after we wrote data. That way if another thread is doing
    // `if(!dq.full()){ dq.push; }`, it will only see not full until element is fully popped.
    mSize--;
}

template <class T, size_t N>
void FixedQueue<T, N>::clear()
{
    // Size will change in the "pop()" and also by "push()" calls from other thread.
    const size_type localSize = mSize;
    for (size_type i = 0; i < localSize; i++)
    {
        pop();
    }
}
}  // namespace angle

#endif  // COMMON_FIXEDQUEUE_H_
