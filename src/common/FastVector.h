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

#include "bitset_utils.h"
#include "common/debug.h"

#include <algorithm>
#include <array>
#include <initializer_list>
#include <iterator>

namespace angle
{

template <class Iter>
class WrapIter
{
  public:
    typedef Iter iterator_type;
    typedef typename std::iterator_traits<iterator_type>::value_type value_type;
    typedef typename std::iterator_traits<iterator_type>::difference_type difference_type;
    typedef typename std::iterator_traits<iterator_type>::pointer pointer;
    typedef typename std::iterator_traits<iterator_type>::reference reference;
    typedef typename std::iterator_traits<iterator_type>::iterator_category iterator_category;

    WrapIter() : mIter() {}
    WrapIter(const WrapIter &x)            = default;
    WrapIter &operator=(const WrapIter &x) = default;
    WrapIter(const Iter &iter) : mIter(iter) {}
    ~WrapIter() = default;

    bool operator==(const WrapIter &x) const { return mIter == x.mIter; }
    bool operator!=(const WrapIter &x) const { return mIter != x.mIter; }
    bool operator<(const WrapIter &x) const { return mIter < x.mIter; }
    bool operator<=(const WrapIter &x) const { return mIter <= x.mIter; }
    bool operator>(const WrapIter &x) const { return mIter > x.mIter; }
    bool operator>=(const WrapIter &x) const { return mIter >= x.mIter; }

    WrapIter &operator++()
    {
        mIter++;
        return *this;
    }

    WrapIter operator++(int)
    {
        WrapIter tmp(mIter);
        mIter++;
        return tmp;
    }

    WrapIter operator+(difference_type n)
    {
        WrapIter tmp(mIter);
        tmp.mIter += n;
        return tmp;
    }

    WrapIter operator-(difference_type n)
    {
        WrapIter tmp(mIter);
        tmp.mIter -= n;
        return tmp;
    }

    difference_type operator-(const WrapIter &x) const { return mIter - x.mIter; }

    iterator_type operator->() const { return mIter; }

    reference operator*() const { return *mIter; }

    pointer ptr() const { return mIter; }

  private:
    iterator_type mIter;
};

template <class T, size_t N>
class FastVector final
{
  public:
    using value_type      = T;
    using size_type       = size_t;
    using reference       = value_type &;
    using const_reference = const value_type &;
    using pointer         = value_type *;
    using const_pointer   = const value_type *;
    using iterator        = WrapIter<pointer>;
    using const_iterator  = WrapIter<const_pointer>;

    FastVector();
    FastVector(size_type count, const_reference value);
    FastVector(size_type count);

    FastVector(const FastVector &other);
    FastVector(FastVector &&other);
    FastVector(std::initializer_list<value_type> init);

    template <class PointerT>
    FastVector(WrapIter<PointerT> first, WrapIter<PointerT> last);
    FastVector(const_pointer first, const_pointer last);

    FastVector &operator=(const FastVector &other);
    FastVector &operator=(FastVector &&other);
    FastVector &operator=(std::initializer_list<value_type> init);

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

    void push_back(const_reference value);
    void push_back(value_type &&value);

    template <typename... Args>
    void emplace_back(Args &&...args);

    void pop_back();

    reference front();
    const_reference front() const;

    reference back();
    const_reference back() const;

    void swap(FastVector &other);

    void resize(size_type count);
    void resize(size_type count, const_reference value);

    void reserve(size_type count);

    // Specialty function that removes a known element and might shuffle the list.
    void remove_and_permute(const_reference element);
    void remove_and_permute(iterator pos);

  private:
    // This will be No-op for trivially constructible types
    static void construct_items(pointer first, pointer last);
    template <typename... Args>
    static void construct_items(pointer first, pointer last, Args &&...args);
    // This will be No-op for trivially destructible types
    static void destruct_items(pointer first, pointer last);
    // Uses copy assignment
    static void copy_items(pointer first, pointer last, const_pointer source);
    // Uses copy constructor
    static void copy_construct_items(pointer first, pointer last, const_pointer source);

    // Generic version
    template <class T2,
              std::enable_if_t<!std::is_trivially_move_constructible_v<T2> ||
                                   !std::is_trivially_destructible_v<T2>,
                               bool> = true>
    static void move_construct_destruct_items(T2 *first, T2 *last, T2 *source);
    // Specialized version that uses memcpy()
    template <class T2,
              std::enable_if_t<std::is_trivially_move_constructible_v<T2> &&
                                   std::is_trivially_destructible_v<T2>,
                               bool> = true>
    static void move_construct_destruct_items(T2 *first, T2 *last, T2 *source);

    // Generic version
    template <class T2,
              std::enable_if_t<!std::is_trivially_copy_assignable_v<T2> ||
                                   !std::is_trivially_copy_constructible_v<T2> ||
                                   !std::is_trivially_destructible_v<T2>,
                               bool> = true>
    void assign_from_pointers(const T2 *first, const T2 *last);
    // Specialized version that uses memcpy()
    template <class T2,
              std::enable_if_t<std::is_trivially_copy_assignable_v<T2> &&
                                   std::is_trivially_copy_constructible_v<T2> &&
                                   std::is_trivially_destructible_v<T2>,
                               bool> = true>
    void assign_from_pointers(const T2 *first, const T2 *last);

    void reset();
    template <typename... Args>
    void resizeImpl(size_type count, Args &&...args);
    void ensure_capacity(size_t capacity);
    bool uses_fixed_storage() const;
    pointer fixed_data();

    struct alignas(value_type) ItemStorage
    {
        char storage[sizeof(value_type)];

        pointer ptr() { return reinterpret_cast<pointer>(storage); }
        const_pointer ptr() const { return reinterpret_cast<const_pointer>(storage); }
    };

    std::array<ItemStorage, N> mFixedStorage;
    pointer mData           = fixed_data();
    size_type mSize         = 0;
    size_type mReservedSize = N;
};

template <class T, size_t N, size_t M>
bool operator==(const FastVector<T, N> &a, const FastVector<T, M> &b)
{
    return a.size() == b.size() && std::equal(a.begin(), a.end(), b.begin());
}

template <class T, size_t N, size_t M>
bool operator!=(const FastVector<T, N> &a, const FastVector<T, M> &b)
{
    return !(a == b);
}

template <class T, size_t N>
ANGLE_INLINE void FastVector<T, N>::construct_items(pointer first, pointer last)
{
    while (first != last)
    {
        new (first++) value_type;  // Do not calling constructor explicitly (default initialization)
    }
}

template <class T, size_t N>
template <typename... Args>
ANGLE_INLINE void FastVector<T, N>::construct_items(pointer first, pointer last, Args &&...args)
{
    while (first != last)
    {
        new (first++) value_type(std::forward<Args>(args)...);
    }
}

template <class T, size_t N>
ANGLE_INLINE void FastVector<T, N>::destruct_items(pointer first, pointer last)
{
    while (first != last)
    {
        (first++)->~value_type();
    }
}

template <class T, size_t N>
ANGLE_INLINE void FastVector<T, N>::copy_items(pointer first, pointer last, const_pointer source)
{
    while (first != last)
    {
        *(first++) = *(source++);
    }
}

template <class T, size_t N>
ANGLE_INLINE void FastVector<T, N>::copy_construct_items(pointer first,
                                                         pointer last,
                                                         const_pointer source)
{
    while (first != last)
    {
        new (first++) value_type(*(source++));
    }
}

template <class T, size_t N>
template <class T2,
          std::enable_if_t<!std::is_trivially_move_constructible_v<T2> ||
                               !std::is_trivially_destructible_v<T2>,
                           bool>>
ANGLE_INLINE void FastVector<T, N>::move_construct_destruct_items(T2 *first, T2 *last, T2 *source)
{
    while (first != last)
    {
        new (first++) value_type(std::move(*source));
        (source++)->~value_type();
    }
}

template <class T, size_t N>
template <class T2,
          std::enable_if_t<std::is_trivially_move_constructible_v<T2> &&
                               std::is_trivially_destructible_v<T2>,
                           bool>>
ANGLE_INLINE void FastVector<T, N>::move_construct_destruct_items(T2 *first, T2 *last, T2 *source)
{
    std::memcpy(first, source, (last - first) * sizeof(T2));
}

template <class T, size_t N>
ANGLE_INLINE bool FastVector<T, N>::uses_fixed_storage() const
{
    return mData == mFixedStorage.data()->ptr();
}

template <class T, size_t N>
ANGLE_INLINE typename FastVector<T, N>::pointer FastVector<T, N>::fixed_data()
{
    return mFixedStorage.data()->ptr();
}

template <class T, size_t N>
FastVector<T, N>::FastVector() = default;

template <class T, size_t N>
FastVector<T, N>::FastVector(size_type count, const_reference value)
{
    ensure_capacity(count);
    construct_items(mData, mData + count, value);
    mSize = count;
}

template <class T, size_t N>
FastVector<T, N>::FastVector(size_type count)
{
    ensure_capacity(count);
    construct_items(mData, mData + count);
    mSize = count;
}

template <class T, size_t N>
FastVector<T, N>::FastVector(const FastVector &other) : FastVector(other.begin(), other.end())
{}

template <class T, size_t N>
FastVector<T, N>::FastVector(FastVector &&other)
{
    *this = std::move(other);
}

template <class T, size_t N>
FastVector<T, N>::FastVector(std::initializer_list<value_type> init)
    : FastVector(init.begin(), init.end())
{}

template <class T, size_t N>
template <class PointerT>
FastVector<T, N>::FastVector(WrapIter<PointerT> first, WrapIter<PointerT> last)
    : FastVector(first.ptr(), last.ptr())
{}

template <class T, size_t N>
FastVector<T, N>::FastVector(const_pointer first, const_pointer last)
{
    assign_from_pointers(first, last);
}

template <class T, size_t N>
FastVector<T, N> &FastVector<T, N>::operator=(const FastVector &other)
{
    assign_from_pointers(other.begin().ptr(), other.end().ptr());
    return *this;
}

template <class T, size_t N>
template <class T2,
          std::enable_if_t<!std::is_trivially_copy_assignable_v<T2> ||
                               !std::is_trivially_copy_constructible_v<T2> ||
                               !std::is_trivially_destructible_v<T2>,
                           bool>>
ANGLE_INLINE void FastVector<T, N>::assign_from_pointers(const T2 *first, const T2 *last)
{
    const size_t newSize = last - first;
    if (newSize > mSize)
    {
        if (newSize > mReservedSize)
        {
            clear();  // Clear to avoid moving items that will be overwritten.
            ensure_capacity(newSize);
        }
        else if (mSize > 0)
        {
            copy_items(mData, mData + mSize, first);
        }
        copy_construct_items(mData + mSize, mData + newSize, first + mSize);
    }
    else
    {
        if (newSize < mSize)
        {
            destruct_items(mData + newSize, mData + mSize);
        }
        if (newSize > 0)
        {
            copy_items(mData, mData + newSize, first);
        }
    }
    mSize = newSize;
}

template <class T, size_t N>
template <class T2,
          std::enable_if_t<std::is_trivially_copy_assignable_v<T2> &&
                               std::is_trivially_copy_constructible_v<T2> &&
                               std::is_trivially_destructible_v<T2>,
                           bool>>
ANGLE_INLINE void FastVector<T, N>::assign_from_pointers(const T2 *first, const T2 *last)
{
    const size_t newSize = last - first;
    if (newSize > mReservedSize)
    {
        clear();  // Clear to avoid moving items that will be overwritten.
        ensure_capacity(newSize);
    }
    if (newSize > 0)
    {
        std::memcpy(mData, first, newSize * sizeof(T2));
    }
    mSize = newSize;
}

template <class T, size_t N>
FastVector<T, N> &FastVector<T, N>::operator=(FastVector &&other)
{
    reset();
    if (other.uses_fixed_storage())
    {
        if (other.mSize > 0)
        {
            move_construct_destruct_items(mData, mData + other.mSize, other.mData);
        }
    }
    else
    {
        mData               = other.mData;
        mReservedSize       = other.mReservedSize;
        other.mData         = other.fixed_data();
        other.mReservedSize = N;
    }
    mSize       = other.mSize;
    other.mSize = 0;
    return *this;
}

template <class T, size_t N>
FastVector<T, N> &FastVector<T, N>::operator=(std::initializer_list<value_type> init)
{
    assign_from_pointers(init.begin(), init.end());
    return *this;
}

template <class T, size_t N>
FastVector<T, N>::~FastVector()
{
    reset();
}

template <class T, size_t N>
void FastVector<T, N>::reset()
{
    clear();
    if (!uses_fixed_storage())
    {
        delete[] reinterpret_cast<ItemStorage *>(mData);
        mData         = fixed_data();
        mReservedSize = N;
    }
}

template <class T, size_t N>
typename FastVector<T, N>::reference FastVector<T, N>::at(size_type pos)
{
    ASSERT(pos < mSize);
    return mData[pos];
}

template <class T, size_t N>
typename FastVector<T, N>::const_reference FastVector<T, N>::at(size_type pos) const
{
    ASSERT(pos < mSize);
    return mData[pos];
}

template <class T, size_t N>
ANGLE_INLINE typename FastVector<T, N>::reference FastVector<T, N>::operator[](size_type pos)
{
    ASSERT(pos < mSize);
    return mData[pos];
}

template <class T, size_t N>
ANGLE_INLINE typename FastVector<T, N>::const_reference FastVector<T, N>::operator[](
    size_type pos) const
{
    ASSERT(pos < mSize);
    return mData[pos];
}

template <class T, size_t N>
ANGLE_INLINE typename FastVector<T, N>::const_pointer angle::FastVector<T, N>::data() const
{
    return mData;
}

template <class T, size_t N>
ANGLE_INLINE typename FastVector<T, N>::pointer angle::FastVector<T, N>::data()
{
    return mData;
}

template <class T, size_t N>
ANGLE_INLINE typename FastVector<T, N>::iterator FastVector<T, N>::begin()
{
    return mData;
}

template <class T, size_t N>
ANGLE_INLINE typename FastVector<T, N>::const_iterator FastVector<T, N>::begin() const
{
    return mData;
}

template <class T, size_t N>
ANGLE_INLINE typename FastVector<T, N>::iterator FastVector<T, N>::end()
{
    return mData + mSize;
}

template <class T, size_t N>
ANGLE_INLINE typename FastVector<T, N>::const_iterator FastVector<T, N>::end() const
{
    return mData + mSize;
}

template <class T, size_t N>
ANGLE_INLINE bool FastVector<T, N>::empty() const
{
    return mSize == 0;
}

template <class T, size_t N>
ANGLE_INLINE typename FastVector<T, N>::size_type FastVector<T, N>::size() const
{
    return mSize;
}

template <class T, size_t N>
void FastVector<T, N>::clear()
{
    resize(0);
}

template <class T, size_t N>
ANGLE_INLINE void FastVector<T, N>::push_back(const_reference value)
{
    emplace_back(value);
}

template <class T, size_t N>
ANGLE_INLINE void FastVector<T, N>::push_back(value_type &&value)
{
    emplace_back(std::move(value));
}

template <class T, size_t N>
template <typename... Args>
ANGLE_INLINE void FastVector<T, N>::emplace_back(Args &&...args)
{
    if (mSize == mReservedSize)
        ensure_capacity(mSize + 1);
    new (&mData[mSize++]) value_type(std::forward<Args>(args)...);
}

template <class T, size_t N>
ANGLE_INLINE void FastVector<T, N>::pop_back()
{
    ASSERT(mSize > 0);
    mData[--mSize].~value_type();
}

template <class T, size_t N>
ANGLE_INLINE typename FastVector<T, N>::reference FastVector<T, N>::front()
{
    ASSERT(mSize > 0);
    return mData[0];
}

template <class T, size_t N>
ANGLE_INLINE typename FastVector<T, N>::const_reference FastVector<T, N>::front() const
{
    ASSERT(mSize > 0);
    return mData[0];
}

template <class T, size_t N>
ANGLE_INLINE typename FastVector<T, N>::reference FastVector<T, N>::back()
{
    ASSERT(mSize > 0);
    return mData[mSize - 1];
}

template <class T, size_t N>
ANGLE_INLINE typename FastVector<T, N>::const_reference FastVector<T, N>::back() const
{
    ASSERT(mSize > 0);
    return mData[mSize - 1];
}

template <class T, size_t N>
void FastVector<T, N>::swap(FastVector &other)
{
    std::swap(*this, other);
}

template <class T, size_t N>
void FastVector<T, N>::resize(size_type count)
{
    resizeImpl(count);
}

template <class T, size_t N>
void FastVector<T, N>::resize(size_type count, const_reference value)
{
    resizeImpl(count, value);
}

template <class T, size_t N>
template <typename... Args>
void FastVector<T, N>::resizeImpl(size_type count, Args &&...args)
{
    if (count > mSize)
    {
        ensure_capacity(count);
        construct_items(mData + mSize, mData + count, std::forward<Args>(args)...);
        mSize = count;
    }
    else if (count < mSize)
    {
        destruct_items(mData + count, mData + mSize);
        mSize = count;
    }
}

template <class T, size_t N>
void FastVector<T, N>::reserve(size_type count)
{
    ensure_capacity(count);
}

template <class T, size_t N>
ANGLE_INLINE void FastVector<T, N>::remove_and_permute(const_reference element)
{
    size_t len = mSize - 1;
    for (size_t index = 0; index < len; ++index)
    {
        if (mData[index] == element)
        {
            mData[index] = std::move(mData[len]);
            break;
        }
    }
    pop_back();
}

template <class T, size_t N>
ANGLE_INLINE void FastVector<T, N>::remove_and_permute(iterator pos)
{
    ASSERT(pos >= begin());
    ASSERT(pos < end());
    size_t len = mSize - 1;
    *pos       = std::move(mData[len]);
    pop_back();
}

template <class T, size_t N>
void FastVector<T, N>::ensure_capacity(size_t capacity)
{
    // We have a minimum capacity of N.
    if (mReservedSize < capacity)
    {
        ASSERT(capacity > N);
        size_type newSize = std::max(mReservedSize, N);
        while (newSize < capacity)
        {
            newSize *= 2;
        }

        pointer newData = (new ItemStorage[newSize])->ptr();

        if (mSize > 0)
        {
            move_construct_destruct_items(newData, newData + mSize, mData);
        }

        if (!uses_fixed_storage())
        {
            delete[] reinterpret_cast<ItemStorage *>(mData);
        }

        mData         = newData;
        mReservedSize = newSize;
    }
}

template <class Value, size_t N>
class FastMap final
{
  public:
    FastMap() {}
    ~FastMap() {}

    Value &operator[](uint32_t key)
    {
        if (mData.size() <= key)
        {
            mData.resize(key + 1, {});
        }
        return mData[key];
    }

    const Value &operator[](uint32_t key) const
    {
        ASSERT(key < mData.size());
        return mData[key];
    }

    void clear() { mData.clear(); }

    bool empty() const { return mData.empty(); }
    size_t size() const { return mData.size(); }

    const Value *data() const { return mData.data(); }

    bool operator==(const FastMap<Value, N> &other) const
    {
        return (size() == other.size()) &&
               (memcmp(data(), other.data(), size() * sizeof(Value)) == 0);
    }

  private:
    FastVector<Value, N> mData;
};

template <class Key, class Value, size_t N>
class FlatUnorderedMap final
{
  public:
    using Pair           = std::pair<Key, Value>;
    using Storage        = FastVector<Pair, N>;
    using iterator       = typename Storage::iterator;
    using const_iterator = typename Storage::const_iterator;

    FlatUnorderedMap()  = default;
    ~FlatUnorderedMap() = default;

    iterator begin() { return mData.begin(); }
    const_iterator begin() const { return mData.begin(); }
    iterator end() { return mData.end(); }
    const_iterator end() const { return mData.end(); }

    iterator find(const Key &key)
    {
        for (auto it = mData.begin(); it != mData.end(); ++it)
        {
            if (it->first == key)
            {
                return it;
            }
        }
        return mData.end();
    }

    const_iterator find(const Key &key) const
    {
        for (auto it = mData.begin(); it != mData.end(); ++it)
        {
            if (it->first == key)
            {
                return it;
            }
        }
        return mData.end();
    }

    Value &operator[](const Key &key)
    {
        iterator it = find(key);
        if (it != end())
        {
            return it->second;
        }

        mData.push_back(Pair(key, {}));
        return mData.back().second;
    }

    void insert(Pair pair)
    {
        ASSERT(!contains(pair.first));
        mData.push_back(std::move(pair));
    }

    void insert(const Key &key, Value value) { insert(Pair(key, value)); }

    void erase(iterator pos) { mData.remove_and_permute(pos); }

    bool contains(const Key &key) const { return find(key) != end(); }

    void clear() { mData.clear(); }

    bool get(const Key &key, Value *value) const
    {
        auto it = find(key);
        if (it != end())
        {
            *value = it->second;
            return true;
        }
        return false;
    }

    bool empty() const { return mData.empty(); }
    size_t size() const { return mData.size(); }

  private:
    FastVector<Pair, N> mData;
};

template <class T, size_t N>
class FlatUnorderedSet final
{
  public:
    using Storage        = FastVector<T, N>;
    using iterator       = typename Storage::iterator;
    using const_iterator = typename Storage::const_iterator;

    FlatUnorderedSet()  = default;
    ~FlatUnorderedSet() = default;

    iterator begin() { return mData.begin(); }
    const_iterator begin() const { return mData.begin(); }
    iterator end() { return mData.end(); }
    const_iterator end() const { return mData.end(); }

    iterator find(const T &value)
    {
        for (auto it = mData.begin(); it != mData.end(); ++it)
        {
            if (*it == value)
            {
                return it;
            }
        }
        return mData.end();
    }

    const_iterator find(const T &value) const
    {
        for (auto it = mData.begin(); it != mData.end(); ++it)
        {
            if (*it == value)
            {
                return it;
            }
        }
        return mData.end();
    }

    bool empty() const { return mData.empty(); }

    void insert(const T &value)
    {
        ASSERT(!contains(value));
        mData.push_back(value);
    }

    void erase(const T &value)
    {
        ASSERT(contains(value));
        mData.remove_and_permute(value);
    }

    void remove(const T &value) { erase(value); }

    bool contains(const T &value) const { return find(value) != end(); }

    void clear() { mData.clear(); }

    bool operator==(const FlatUnorderedSet<T, N> &other) const { return mData == other.mData; }

  private:
    Storage mData;
};

class FastIntegerSet final
{
  public:
    static constexpr size_t kWindowSize             = 64;
    static constexpr size_t kOneLessThanKWindowSize = kWindowSize - 1;
    static constexpr size_t kShiftForDivision =
        static_cast<size_t>(rx::Log2(static_cast<unsigned int>(kWindowSize)));
    using KeyBitSet = angle::BitSet64<kWindowSize>;

    ANGLE_INLINE FastIntegerSet();
    ANGLE_INLINE ~FastIntegerSet();

    ANGLE_INLINE void ensureCapacity(size_t size)
    {
        if (capacity() <= size)
        {
            reserve(size * 2);
        }
    }

    ANGLE_INLINE void insert(uint64_t key)
    {
        size_t sizedKey = static_cast<size_t>(key);

        ASSERT(!contains(sizedKey));
        ensureCapacity(sizedKey);
        ASSERT(capacity() > sizedKey);

        size_t index  = sizedKey >> kShiftForDivision;
        size_t offset = sizedKey & kOneLessThanKWindowSize;

        mKeyData[index].set(offset, true);
    }

    ANGLE_INLINE bool contains(uint64_t key) const
    {
        size_t sizedKey = static_cast<size_t>(key);

        size_t index  = sizedKey >> kShiftForDivision;
        size_t offset = sizedKey & kOneLessThanKWindowSize;

        return (sizedKey < capacity()) && (mKeyData[index].test(offset));
    }

    ANGLE_INLINE void clear()
    {
        for (KeyBitSet &it : mKeyData)
        {
            it.reset();
        }
    }

    ANGLE_INLINE bool empty() const
    {
        for (const KeyBitSet &it : mKeyData)
        {
            if (it.any())
            {
                return false;
            }
        }
        return true;
    }

    ANGLE_INLINE size_t size() const
    {
        size_t valid_entries = 0;
        for (const KeyBitSet &it : mKeyData)
        {
            valid_entries += it.count();
        }
        return valid_entries;
    }

  private:
    ANGLE_INLINE size_t capacity() const { return kWindowSize * mKeyData.size(); }

    ANGLE_INLINE void reserve(size_t newSize)
    {
        size_t alignedSize = rx::roundUpPow2(newSize, kWindowSize);
        size_t count       = alignedSize >> kShiftForDivision;

        mKeyData.resize(count, KeyBitSet::Zero());
    }

    std::vector<KeyBitSet> mKeyData;
};

// This is needed to accommodate the chromium style guide error -
//      [chromium-style] Complex constructor has an inlined body.
ANGLE_INLINE FastIntegerSet::FastIntegerSet() {}
ANGLE_INLINE FastIntegerSet::~FastIntegerSet() {}

template <typename Value>
class FastIntegerMap final
{
  public:
    FastIntegerMap() {}
    ~FastIntegerMap() {}

    ANGLE_INLINE void ensureCapacity(size_t size)
    {
        // Ensure key set has capacity
        mKeySet.ensureCapacity(size);

        // Ensure value vector has capacity
        ensureCapacityImpl(size);
    }

    ANGLE_INLINE void insert(uint64_t key, Value value)
    {
        // Insert key
        ASSERT(!mKeySet.contains(key));
        mKeySet.insert(key);

        // Insert value
        size_t sizedKey = static_cast<size_t>(key);
        ensureCapacityImpl(sizedKey);
        ASSERT(capacity() > sizedKey);
        mValueData[sizedKey] = value;
    }

    ANGLE_INLINE bool contains(uint64_t key) const { return mKeySet.contains(key); }

    ANGLE_INLINE bool get(uint64_t key, Value *out) const
    {
        if (!mKeySet.contains(key))
        {
            return false;
        }

        size_t sizedKey = static_cast<size_t>(key);
        *out            = mValueData[sizedKey];
        return true;
    }

    ANGLE_INLINE void clear() { mKeySet.clear(); }

    ANGLE_INLINE bool empty() const { return mKeySet.empty(); }

    ANGLE_INLINE size_t size() const { return mKeySet.size(); }

  private:
    ANGLE_INLINE size_t capacity() const { return mValueData.size(); }

    ANGLE_INLINE void ensureCapacityImpl(size_t size)
    {
        if (capacity() <= size)
        {
            reserve(size * 2);
        }
    }

    ANGLE_INLINE void reserve(size_t newSize)
    {
        size_t alignedSize = rx::roundUpPow2(newSize, FastIntegerSet::kWindowSize);
        mValueData.resize(alignedSize);
    }

    FastIntegerSet mKeySet;
    std::vector<Value> mValueData;
};
}  // namespace angle

#endif  // COMMON_FASTVECTOR_H_
