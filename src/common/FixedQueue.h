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

namespace angle
{
// class FixedQueue: An array based fix storage fifo queue class that supports concurrent push and
// pop. Caller must ensure queue is not empty before pop and not full before push. This class
// supports concurrent push and pop from different threads. If caller want to push from two
// different threads, proper mutex must be used to ensure the access is serialized.
template <class T, size_t N, class Storage = std::array<T, N>>
class FixedQueue final : angle::NonCopyable
{
  public:
    using value_type      = typename Storage::value_type;
    using size_type       = typename Storage::size_type;
    using reference       = typename Storage::reference;
    using const_reference = typename Storage::const_reference;

    class iterator
    {
      public:
        iterator() : mData(nullptr), mIndex(-1) {}

        T &operator*() const { return mData[mIndex % N]; }
        T *operator->() const { return &mData[mIndex % N]; }

        bool operator==(const iterator &rhs) const { return mIndex == rhs.mIndex; }
        bool operator!=(const iterator &rhs) const { return mIndex != rhs.mIndex; }

        iterator &operator++()
        {
            mIndex++;
            return *this;
        }

      private:
        Storage &mData;
        size_type mIndex;
        friend class FixedQueue<T, N, Storage>;
        iterator(Storage &data, size_type index) : mData(data), mIndex(index) {}
    };

    class const_iterator
    {
      public:
        const_iterator() : mData(nullptr), mIndex(-1) {}

        const T &operator*() const { return mData[mIndex % N]; }
        const T *operator->() const { return &mData[mIndex % N]; }

        bool operator==(const iterator &rhs) const { mIndex == rhs.mIndex; }
        bool operator!=(const iterator &rhs) const { mIndex != rhs.mIndex; }

        const_iterator &operator++()
        {
            mIndex++;
            return *this;
        }

      private:
        const Storage &mData;
        size_type mIndex;
        friend class FixedQueue<T, N, Storage>;
        const_iterator(const Storage &data, size_type index) : mData(data), mIndex(index) {}
    };

    class Producer
    {
      public:
        Producer() = default;

        bool full() const;

        reference back();
        const_reference back() const;

        void push(const value_type &value);
        void push(value_type &&value);

      private:
        Storage &mData;
        std::atomic<size_type> &mSize;
        // Virtual index for next write.
        size_type mBackIndex;
        friend class FixedQueue<T, N, Storage>;
        Producer(Storage &data, std::atomic<size_type> &size)
            : mData(data), mSize(size), mBackIndex(0)
        {}
    };

    class Consumer
    {
      public:
        Consumer() = default;

        bool empty() const;

        reference front();
        const_reference front() const;

        void pop();
        void clear();

        reference operator[](size_type pos);
        const_reference operator[](size_type pos) const;

        iterator begin();
        const_iterator begin() const;

        iterator end();
        const_iterator end() const;

      private:
        Storage &mData;
        std::atomic<size_type> &mSize;
        // Virtual index for next write.
        size_type mFrontIndex;
        friend class FixedQueue<T, N, Storage>;
        Consumer(Storage &data, std::atomic<size_type> &size)
            : mData(data), mSize(size), mFrontIndex(0)
        {}
    };

    FixedQueue();
    ~FixedQueue();

    // size_type size() const;
    Producer &getProducer() { return mProducer; }
    Consumer &getConsumer() { return mConsumer; }

  private:
    Storage mData;
    // Atomic so that we can support concurrent push and pop.
    std::atomic<size_type> mSize;

    Producer mProducer;
    Consumer mConsumer;
};

// FixedQueue implementation
template <class T, size_t N, class Storage>
FixedQueue<T, N, Storage>::FixedQueue() : mSize(0), mProducer(mData, mSize), mConsumer(mData, mSize)
{}

template <class T, size_t N, class Storage>
FixedQueue<T, N, Storage>::~FixedQueue()
{}

/*template <class T, size_t N, class Storage>
ANGLE_INLINE typename FixedQueue<T, N, Storage>::size_type FixedQueue<T, N, Storage>::size() const
{
    return mSize;
}*/

// The front and back indices are virtual indices (think about queue sizd is infinite). They
// will never wrap around when hit N. The wrap around occur when element is referenced. Virtual
// index for current head
// FixedQueue::Consumer implementation
template <class T, size_t N, class Storage>
ANGLE_INLINE bool FixedQueue<T, N, Storage>::Consumer::empty() const
{
    return mSize == 0;
}

template <class T, size_t N, class Storage>
typename FixedQueue<T, N, Storage>::reference FixedQueue<T, N, Storage>::Consumer::operator[](
    size_type pos)
{
    ASSERT(pos < mSize);
    return mData[(pos + mFrontIndex) % N];
}

template <class T, size_t N, class Storage>
typename FixedQueue<T, N, Storage>::const_reference FixedQueue<T, N, Storage>::Consumer::operator[](
    size_type pos) const
{
    ASSERT(pos < mSize);
    return mData[(pos + mFrontIndex) % N];
}

template <class T, size_t N, class Storage>
ANGLE_INLINE typename FixedQueue<T, N, Storage>::reference
FixedQueue<T, N, Storage>::Consumer::front()
{
    ASSERT(mSize > 0);
    return mData[mFrontIndex % N];
}

template <class T, size_t N, class Storage>
ANGLE_INLINE typename FixedQueue<T, N, Storage>::const_reference
FixedQueue<T, N, Storage>::Consumer::front() const
{
    ASSERT(mSize > 0);
    return mData[mFrontIndex % N];
}

template <class T, size_t N, class Storage>
void FixedQueue<T, N, Storage>::Consumer::pop()
{
    ASSERT(mSize > 0);
    mData[mFrontIndex % N] = value_type();
    mFrontIndex++;
    // We must decrement size last, after we wrote data. That way if another thread is doing
    // `if(!dq.full()){ dq.push; }`, it will only see not full until element is fully popped.
    mSize--;
}

template <class T, size_t N, class Storage>
void FixedQueue<T, N, Storage>::Consumer::Consumer::clear()
{
    for (size_type i = 0; i < mSize; i++)
    {
        pop();
    }
}

template <class T, size_t N, class Storage>
typename FixedQueue<T, N, Storage>::iterator FixedQueue<T, N, Storage>::Consumer::begin()
{
    return iterator(mData, mFrontIndex);
}

template <class T, size_t N, class Storage>
typename FixedQueue<T, N, Storage>::const_iterator FixedQueue<T, N, Storage>::Consumer::begin()
    const
{
    return const_iterator(mData, mFrontIndex);
}

template <class T, size_t N, class Storage>
typename FixedQueue<T, N, Storage>::iterator FixedQueue<T, N, Storage>::Consumer::end()
{
    return iterator(mData, mFrontIndex + mSize);
}

template <class T, size_t N, class Storage>
typename FixedQueue<T, N, Storage>::const_iterator FixedQueue<T, N, Storage>::Consumer::end() const
{
    return const_iterator(mData, mFrontIndex + mSize);
}

// FixedQueue::Producer implementation
template <class T, size_t N, class Storage>
ANGLE_INLINE bool FixedQueue<T, N, Storage>::Producer::full() const
{
    return mSize >= N;
}

template <class T, size_t N, class Storage>
void FixedQueue<T, N, Storage>::Producer::push(const value_type &value)
{
    ASSERT(mSize < N);
    mData[mBackIndex % N] = value;
    mBackIndex++;
    // We must increment size last, after we wrote data. That way if another thread is doing
    // `if(!dq.empty()){ s = dq.front(); }`, it will only see not empty until element is fully
    // pushed.
    mSize++;
}

template <class T, size_t N, class Storage>
void FixedQueue<T, N, Storage>::Producer::push(value_type &&value)
{
    ASSERT(mSize < N);
    mData[mBackIndex % N] = std::move(value);
    mBackIndex++;
    // We must increment size last, after we wrote data. That way if another thread is doing
    // `if(!dq.empty()){ s = dq.front(); }`, it will only see not empty until element is fully
    // pushed.
    mSize++;
}

template <class T, size_t N, class Storage>
ANGLE_INLINE typename FixedQueue<T, N, Storage>::reference
FixedQueue<T, N, Storage>::Producer::back()
{
    ASSERT(mSize > 0);
    return mData[(mBackIndex + (N - 1)) % N];
}

template <class T, size_t N, class Storage>
ANGLE_INLINE typename FixedQueue<T, N, Storage>::const_reference
FixedQueue<T, N, Storage>::Producer::back() const
{
    ASSERT(mSize > 0);
    return mData[(mBackIndex + (N - 1)) % N];
}
}  // namespace angle

#endif  // COMMON_FIXEDQUEUE_H_
