//
// Copyright (c) 2016 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//

// Color.inl : Inline definitions of some functions from Color.h

namespace angle
{

template <typename T>
Color<T>::Color() : Color(0, 0, 0, 0)
{
}

template <typename T>
Color<T>::Color(T r, T g, T b, T a) : red(r), green(g), blue(b), alpha(a)
{
}

template <typename T>
bool operator==(const Color<T> &a, const Color<T> &b)
{
    return a.red == b.red &&
           a.green == b.green &&
           a.blue == b.blue &&
           a.alpha == b.alpha;
}

template <typename T>
bool operator!=(const Color<T> &a, const Color<T> &b)
{
    return !(a == b);
}


ColorVariant::ColorVariant() : colorF(), type(Type::Float)
{
}

ColorVariant::ColorVariant(const ColorF& color) : colorF(color), type(Type::Float)
{
}

ColorVariant::ColorVariant(const ColorI& color) : colorI(color), type(Type::Int)
{
}

ColorVariant::ColorVariant(const ColorUI& color) : colorUI(color), type(Type::UInt)
{
}

bool operator==(const ColorVariant &a, const ColorVariant &b)
{
    if (a.type != b.type)
    {
        return false;
    }
    switch (a.type)
    {
    default:
    case ColorVariant::Type::Float:
        return a.colorF == b.colorF;
    case ColorVariant::Type::Int:
        return a.colorI == b.colorI;
    case ColorVariant::Type::UInt:
        return a.colorUI == b.colorUI;
    }
}

bool operator!=(const ColorVariant &a, const ColorVariant &b)
{
    return !(a == b);
}

}  // namespace angle
