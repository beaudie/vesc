//
// Copyright 2017 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//
// constexpr_array:
//   C++14 relaxes constexpr requirements enough that we can support sorting
//   a constexpr std::array at compile-time. This is useful for defining fast
//   lookup tables and other data at compile-time.
//   The code is adapted from https://stackoverflow.com/a/40030044.

#include <algorithm>

namespace angle
{

template <class T>
constexpr void constexpr_swap(T &l, T &r)
{
    T tmp = std::move(l);
    l     = std::move(r);
    r     = std::move(tmp);
}

template <typename T, size_t N>
struct constexpr_array
{
    constexpr T &operator[](size_t i) { return arr[i]; }

    constexpr const T &operator[](size_t i) const { return arr[i]; }

    constexpr const T *begin() const { return arr; }
    constexpr const T *end() const { return arr + N; }

    T arr[N];
};

template <typename T, size_t N>
constexpr void constexpr_sort_impl(constexpr_array<T, N> &arr, size_t left, size_t right)
{
    if (left < right)
    {
        size_t m = left;

        for (size_t i = left + 1; i < right; i++)
            if (arr[i] < arr[left])
                constexpr_swap(arr[++m], arr[i]);

        constexpr_swap(arr[left], arr[m]);

        constexpr_sort_impl(arr, left, m);
        constexpr_sort_impl(arr, m + 1, right);
    }
}

template <typename T, size_t N>
constexpr constexpr_array<T, N> constexpr_sort(constexpr_array<T, N> arr)
{
    auto sorted = arr;
    constexpr_sort_impl(sorted, 0, N);
    return sorted;
}

}  // namespace angle
