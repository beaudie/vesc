// Copyright 2017 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//
// PackedGLEnums_autogen.h:
//   Declares ANGLE-specific enums classes for GLEnum and functions operating
//   on them.

#ifndef LIBANGLE_PACKEDGLENUMS_H_
#define LIBANGLE_PACKEDGLENUMS_H_

#include "libANGLE/PackedGLEnums_autogen.h"

#include <array>
#include <bitset>
#include <cstddef>

namespace angle
{

// Implementation of AllEnums which allows iterating over all the possible values for a packed enums
// like so:
//     for (auto value : AllEnums<MyPackedEnum>()) {
//         // Do something with the enum.
//     }

template <typename E>
class EnumIterator final
{
  private:
    using UnderlyingType = typename std::underlying_type<E>::type;

  public:
    EnumIterator(E value) : mValue(static_cast<UnderlyingType>(value)) {}
    EnumIterator &operator++()
    {
        mValue++;
        return *this;
    }
    bool operator==(const EnumIterator &other) const { return mValue == other.mValue; }
    bool operator!=(const EnumIterator &other) const { return mValue != other.mValue; }
    E operator*() const { return static_cast<E>(mValue); }

  private:
    UnderlyingType mValue;
};

template <typename E>
struct AllEnums
{
    EnumIterator<E> begin() const { return {static_cast<E>(0)}; }
    EnumIterator<E> end() const { return {E::InvalidEnum}; }
};

// PackedEnumMap<E, T> is like an std::array<T, E::EnumCount> but is indexed with enum values. It
// implements all of the std::array interface except with enum values instead of indices.
template <typename E, typename T>
class PackedEnumMap
{
  private:
    using UnderlyingType          = typename std::underlying_type<E>::type;
    static constexpr size_t kSize = static_cast<UnderlyingType>(E::EnumCount);
    using Storage                 = std::array<T, kSize>;

    Storage mData;

  public:
    // types:
    using value_type      = T;
    using pointer         = T *;
    using const_pointer   = const T *;
    using reference       = T &;
    using const_reference = const T &;

    using size_type       = size_t;
    using difference_type = ptrdiff_t;

    using iterator               = typename Storage::iterator;
    using const_iterator         = typename Storage::const_iterator;
    using reverse_iterator       = std::reverse_iterator<iterator>;
    using const_reverse_iterator = std::reverse_iterator<const_iterator>;

    // No explicit construct/copy/destroy for aggregate type
    void fill(const T &u) { mData.fill(u); }
    void swap(PackedEnumMap<E, T> &a) noexcept { mData.swap(a.mData); }

    // iterators:
    iterator begin() noexcept { return mData.begin(); }
    const_iterator begin() const noexcept { return mData.begin(); }
    iterator end() noexcept { return mData.end(); }
    const_iterator end() const noexcept { return mData.end(); }

    reverse_iterator rbegin() noexcept { return mData.rbegin(); }
    const_reverse_iterator rbegin() const noexcept { return mData.rbegin(); }
    reverse_iterator rend() noexcept { return mData.rend(); }
    const_reverse_iterator rend() const noexcept { return mData.rend(); }

    // capacity:
    constexpr size_type size() const noexcept { return mData.size(); }
    constexpr size_type max_size() const noexcept { return mData.max_size(); }
    constexpr bool empty() const noexcept { return mData.empty(); }

    // element access:
    reference operator[](E n) { return mData[static_cast<UnderlyingType>(n)]; }
    const_reference operator[](E n) const { return mData[static_cast<UnderlyingType>(n)]; }
    const_reference at(E n) const { return mData.at(static_cast<UnderlyingType>(n)); }
    reference at(E n) { return mData.at(static_cast<UnderlyingType>(n)); }

    reference front() { return mData.front(); }
    const_reference front() const { return mData.front(); }
    reference back() { return mData.back(); }
    const_reference back() const { return mData.back(); }

    T *data() noexcept { return mData.data(); }
    const T *data() const noexcept { return mData.data(); }
};

// PackedEnumBitSetE> is like an std::bitset<E::EnumCount> but is indexed with enum values. It
// implements parts of the std::bitset interface except with enum values instead of indices. Some of
// the std::bitset interface with shifts and casts to/from integer isn't present because it doesn't
// make sense for enum bitsets.
template <typename E>
class PackedEnumBitSet
{
  private:
    using UnderlyingType          = typename std::underlying_type<E>::type;
    static constexpr size_t kSize = static_cast<UnderlyingType>(E::EnumCount);
    using Storage                 = std::bitset<kSize>;

    Storage mData;

  public:
    PackedEnumBitSet() = default;
    explicit PackedEnumBitSet(Storage data) : mData(data) {}

    // Non-operatior modifiers
    PackedEnumBitSet &set() noexcept
    {
        mData.set();
        return *this;
    }
    PackedEnumBitSet &set(E pos, bool val = true)
    {
        mData.set(static_cast<UnderlyingType>(pos), val);
        return *this;
    };
    PackedEnumBitSet &reset() noexcept
    {
        mData.reset();
        return *this;
    }
    PackedEnumBitSet &reset(E pos)
    {
        mData.reset(static_cast<UnderlyingType>(pos));
        return *this;
    }
    PackedEnumBitSet &flip() noexcept
    {
        mData.flip();
        return *this;
    }
    PackedEnumBitSet &flip(E pos)
    {
        mData.flip(static_cast<UnderlyingType>(pos));
        return *this;
    }

    // Operator overloads
    bool operator[](E pos) const { return mData[static_cast<UnderlyingType>(pos)]; }
    PackedEnumBitSet &operator&=(const PackedEnumBitSet &rhs) noexcept
    {
        mData = mData & rhs.mData;
        return *this;
    }
    PackedEnumBitSet &operator|=(const PackedEnumBitSet &rhs) noexcept
    {
        mData = mData | rhs.mData;
        return *this;
    }
    PackedEnumBitSet &operator^=(const PackedEnumBitSet &rhs) noexcept
    {
        mData = mData ^ rhs.mData;
        return *this;
    }
    bool operator==(const PackedEnumBitSet &rhs) const noexcept { return mData == rhs.mData; }
    bool operator!=(const PackedEnumBitSet &rhs) const noexcept { return mData != rhs.mData; }
    PackedEnumBitSet operator~() const noexcept {}

    // Queries
    size_t count() const noexcept { return mData.count(); }
    constexpr size_t size() const noexcept { return kSize; }
    bool test(E pos) const { return mData.test(static_cast<UnderlyingType>(pos)); }
    bool all() const noexcept { return mData.all(); }
    bool any() const noexcept { return mData.any(); }
    bool none() const noexcept { return mData.none(); }

    const Storage &getData() const noexcept { return mData; }
};

// Operator overloads for PackedEnumBitSet
template <typename E>
PackedEnumBitSet<E> operator&(const PackedEnumBitSet<E> &a, const PackedEnumBitSet<E> &b) noexcept
{
    return {a.getData() & b.getData()};
}
template <typename E>
PackedEnumBitSet<E> operator|(const PackedEnumBitSet<E> &a, const PackedEnumBitSet<E> &b) noexcept
{
    return {a.getData() | b.getData()};
}
template <typename E>
PackedEnumBitSet<E> operator^(const PackedEnumBitSet<E> &a, const PackedEnumBitSet<E> &b) noexcept
{
    return {a.getData() ^ b.getData()};
}

}  // namespace angle

#endif  // LIBANGLE_PACKEDGLENUMS_H_
