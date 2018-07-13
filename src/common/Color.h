//
// Copyright (c) 2016 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//

// Color.h : Defines the Color type used throughout the ANGLE libraries

#ifndef COMMON_COLOR_H_
#define COMMON_COLOR_H_

namespace angle
{

template <typename T>
struct Color
{
    Color();
    Color(T r, T g, T b, T a);

    const T *data() const { return &red; }
    T *ptr() { return &red; }

    static Color fromData(const T *data) { return Color(data[0], data[1], data[2], data[3]); }
    void writeData(T *data) const
    {
        data[0] = red;
        data[1] = green;
        data[2] = blue;
        data[3] = alpha;
    }

    T red;
    T green;
    T blue;
    T alpha;
};

template <typename T>
struct DepthStencil
{
    DepthStencil();
    DepthStencil(T d, T s);

    const T *data() const { return &depth; }
    T *ptr() { return &depth; }

    static DepthStencil fromData(const T *data) { return DepthStencil(data[0], data[1]); }
    void writeData(T *data) const
    {
        data[0] = depth;
        data[1] = stencil;
    }

    T depth;
    T stencil;
};

template <typename T>
bool operator==(const Color<T> &a, const Color<T> &b);

template <typename T>
bool operator!=(const Color<T> &a, const Color<T> &b);

typedef DepthStencil<float> DepthStencilF;
typedef Color<float> ColorF;
typedef Color<int> ColorI;
typedef Color<unsigned int> ColorUI;

}  // namespace angle

// TODO: Move this fully into the angle namespace
namespace gl
{

template <typename T>
using Color   = angle::Color<T>;
using ColorF  = angle::ColorF;
using ColorI  = angle::ColorI;
using ColorUI = angle::ColorUI;

template <typename T>
using DepthStencil   = angle::DepthStencil<T>;
using DepthStencilF  = angle::DepthStencilF;

}  // namespace gl

#include "Color.inl"

#endif  // COMMON_COLOR_H_
