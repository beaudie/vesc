//
// Copyright 2018 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//
// FastVector.h:
//   A vector class with a initial fixed size and variable growth.
//   Based on FixedVector.
//

#ifndef COMMON_FASTVECTOR_H_
#define COMMON_FASTVECTOR_H_

#include "common/debug.h"

#include <algorithm>
#include <array>
#include <initializer_list>

namespace angle
{
template <class T, size_t N, class Storage = std::array<T, N>>
class FastVector final
{
  public:
    using value_type      = typename Storage::value_type;
    using size_type       = typename Storage::size_type;
    using reference       = typename Storage::reference;
    using const_reference = typename Storage::const_reference;
    using pointer         = typename Storage::pointer;
    using const_pointer   = typename Storage::const_pointer;
    using iterator        = T *;
    using const_iterator  = const T *;

    FastVector();
    FastVector(size_type count, const value_type &value);
    FastVector(size_type count);

    FastVector(const FastVector<T, N, Storage> &other);
    FastVector(FastVector<T, N, Storage> &&other);
    FastVector(std::initializer_list<value_type> init);

    FastVector<T, N, Storage> &operator=(const FastVector<T, N, Storage> &other);
    FastVector<T, N, Storage> &operator=(FastVector<T, N, Storage> &&other);
    FastVector<T, N, Storage> &operator=(std::initializer_list<value_type> init);

    ~FastVector();

    reference at(size_type pos);
    const_reference at(size_type pos) const;

    reference operator[](size_type pos);
    const_reference operator[](size_type pos) const;

    pointer data();
    const_pointer data() const;

    iterator begin();
    const_iterator begin() const;

    iterator end();
    const_iterator end() const;

    bool empty() const;
    size_type size() const;

    void clear();

    void push_back(const value_type &value);
    void push_back(value_type &&value);

    void pop_back();
    reference back();
    const_reference back() const;

    void swap(FastVector<T, N, Storage> &other);

    void resize(size_type count);
    void resize(size_type count, const value_type &value);

  private:
    void assign_from_initializer_list(std::initializer_list<value_type> init);
    void ensure_capacity();

    pointer mData = nullptr;
    Storage mFixedStorage;
    size_type mSize         = 0;
    size_type mReservedSize = 0;
};

template <class T, size_t N, class Storage>
bool operator==(const FastVector<T, N, Storage> &a, const FastVector<T, N, Storage> &b)
{
    return a.size() == b.size() && std::equal(a.begin(), a.end(), b.begin());
}

template <class T, size_t N, class Storage>
bool operator!=(const FastVector<T, N, Storage> &a, const FastVector<T, N, Storage> &b)
{
    return !(a == b);
}

template <class T, size_t N, class Storage>
FastVector<T, N, Storage>::FastVector() = default;

template <class T, size_t N, class Storage>
FastVector<T, N, Storage>::FastVector(size_type count, const value_type &value) : mSize(count)
{
    ensure_capacity();
    std::fill(begin(), end(), value);
}

template <class T, size_t N, class Storage>
FastVector<T, N, Storage>::FastVector(size_type count) : mSize(count)
{
    ensure_capacity();
}

template <class T, size_t N, class Storage>
FastVector<T, N, Storage>::FastVector(const FastVector<T, N, Storage> &other) : mSize(other.mSize)
{
    ensure_capacity();
    memcpy(mData, other.mData, mSize * sizeof(T));
}

template <class T, size_t N, class Storage>
FastVector<T, N, Storage>::FastVector(FastVector<T, N, Storage> &&other) : FastVector()
{
    swap(other);
}

template <class T, size_t N, class Storage>
FastVector<T, N, Storage>::FastVector(std::initializer_list<value_type> init)
{
    assign_from_initializer_list(init);
}

template <class T, size_t N, class Storage>
FastVector<T, N, Storage> &FastVector<T, N, Storage>::operator=(
    const FastVector<T, N, Storage> &other)
{
    mSize = other.size();
    ensure_capacity();
    memcpy(mData, other.mData, sizeof(T) * mSize);
    return *this;
}

template <class T, size_t N, class Storage>
FastVector<T, N, Storage> &FastVector<T, N, Storage>::operator=(FastVector<T, N, Storage> &&other)
{
    swap(*this, other);
    return *this;
}

template <class T, size_t N, class Storage>
FastVector<T, N, Storage> &FastVector<T, N, Storage>::operator=(
    std::initializer_list<value_type> init)
{
    assign_from_initializer_list(init);
    return *this;
}

template <class T, size_t N, class Storage>
FastVector<T, N, Storage>::~FastVector()
{
    clear();
}

template <class T, size_t N, class Storage>
typename FastVector<T, N, Storage>::reference FastVector<T, N, Storage>::at(size_type pos)
{
    ASSERT(pos < mSize);
    return mData[pos];
}

template <class T, size_t N, class Storage>
typename FastVector<T, N, Storage>::const_reference FastVector<T, N, Storage>::at(
    size_type pos) const
{
    ASSERT(pos < mSize);
    return mData[pos];
}

template <class T, size_t N, class Storage>
typename FastVector<T, N, Storage>::reference FastVector<T, N, Storage>::operator[](size_type pos)
{
    ASSERT(pos < mSize);
    return mData[pos];
}

template <class T, size_t N, class Storage>
typename FastVector<T, N, Storage>::const_reference FastVector<T, N, Storage>::operator[](
    size_type pos) const
{
    ASSERT(pos < mSize);
    return mData[pos];
}

template <class T, size_t N, class Storage>
typename FastVector<T, N, Storage>::const_pointer angle::FastVector<T, N, Storage>::data() const
{
    ASSERT(!empty());
    return mData;
}

template <class T, size_t N, class Storage>
typename FastVector<T, N, Storage>::pointer angle::FastVector<T, N, Storage>::data()
{
    ASSERT(!empty());
    return mData;
}

template <class T, size_t N, class Storage>
typename FastVector<T, N, Storage>::iterator FastVector<T, N, Storage>::begin()
{
    return mData;
}

template <class T, size_t N, class Storage>
typename FastVector<T, N, Storage>::const_iterator FastVector<T, N, Storage>::begin() const
{
    return mData;
}

template <class T, size_t N, class Storage>
typename FastVector<T, N, Storage>::iterator FastVector<T, N, Storage>::end()
{
    return mData + mSize;
}

template <class T, size_t N, class Storage>
typename FastVector<T, N, Storage>::const_iterator FastVector<T, N, Storage>::end() const
{
    return mData + mSize;
}

template <class T, size_t N, class Storage>
bool FastVector<T, N, Storage>::empty() const
{
    return mSize == 0;
}

template <class T, size_t N, class Storage>
typename FastVector<T, N, Storage>::size_type FastVector<T, N, Storage>::size() const
{
    return mSize;
}

template <class T, size_t N, class Storage>
void FastVector<T, N, Storage>::clear()
{
    resize(0);
}

template <class T, size_t N, class Storage>
void FastVector<T, N, Storage>::push_back(const value_type &value)
{
    mSize++;
    ensure_capacity();
    mData[mSize - 1] = value;
}

template <class T, size_t N, class Storage>
void FastVector<T, N, Storage>::push_back(value_type &&value)
{
    mSize++;
    ensure_capacity();
    mData[mSize - 1] = std::move(value);
}

template <class T, size_t N, class Storage>
void FastVector<T, N, Storage>::pop_back()
{
    ASSERT(mSize > 0);
    mSize--;
}

template <class T, size_t N, class Storage>
typename FastVector<T, N, Storage>::reference FastVector<T, N, Storage>::back()
{
    ASSERT(mSize > 0);
    return mData[mSize - 1];
}

template <class T, size_t N, class Storage>
typename FastVector<T, N, Storage>::const_reference FastVector<T, N, Storage>::back() const
{
    ASSERT(mSize > 0);
    return mData[mSize - 1];
}

template <class T, size_t N, class Storage>
void FastVector<T, N, Storage>::swap(FastVector<T, N, Storage> &other)
{
    std::swap(mSize, other.mSize);

    pointer tempData = other.mData;
    if (mData == mFixedStorage.data())
        other.mData = other.mFixedStorage.data();
    else
        other.mData = mData;
    if (tempData == other.mFixedStorage.data())
        mData = mFixedStorage.data();
    else
        mData = tempData;
    std::swap(mReservedSize, other.mReservedSize);
    std::swap(mFixedStorage, other.mFixedStorage);
}

template <class T, size_t N, class Storage>
void FastVector<T, N, Storage>::resize(size_type count)
{
    resize(count, value_type());
}

template <class T, size_t N, class Storage>
void FastVector<T, N, Storage>::resize(size_type count, const value_type &value)
{
    if (count < mSize)
    {
        if (count == 0 && mData && mData != mFixedStorage.data())
        {
            delete[] mData;
            mData         = nullptr;
            mReservedSize = 0;
        }
        mSize = count;
    }
    else if (count > mSize)
    {
        size_t oldSize = mSize;
        mSize          = count;
        ensure_capacity();
        for (size_t index = oldSize; index < count; ++index)
        {
            mData[index] = value;
        }
    }
}

template <class T, size_t N, class Storage>
void FastVector<T, N, Storage>::assign_from_initializer_list(std::initializer_list<value_type> init)
{
    mSize = init.size();
    ensure_capacity();
    size_t index = 0;
    for (auto &value : init)
    {
        mData[index++] = std::move(value);
    }
}

template <class T, size_t N, class Storage>
void FastVector<T, N, Storage>::ensure_capacity()
{
    if (mReservedSize < mSize)
    {
        pointer newData   = nullptr;
        size_type newSize = mReservedSize;

        if (mReservedSize <= mFixedStorage.size())
        {
            newData = mFixedStorage.data();
            newSize = mFixedStorage.size();
        }
        else
        {
            while (newSize < mSize)
            {
                newSize *= 2;
            }
            newData = new value_type[newSize];
        }

        if (mReservedSize > 0)
        {
            memcpy(newData, mData, mReservedSize);
        }

        if (mData && mData != mFixedStorage.data())
        {
            delete[] mData;
        }

        mData         = newData;
        mReservedSize = newSize;
    }
}
}  // namespace angle

#endif  // COMMON_FASTVECTOR_H_
