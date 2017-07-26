//
// Copyright (c) 2012-2015 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//

// angletypes.inl : Inline definitions of some functions from angletypes.h

namespace gl
{

template <typename T>
bool operator==(const TypedRectangle<T> &a, const TypedRectangle<T> &b)
{
    return a.x == b.x && a.y == b.y && a.width == b.width && a.height == b.height;
}

template <typename T>
bool operator!=(const TypedRectangle<T> &a, const TypedRectangle<T> &b)
{
    return !(a == b);
}

template <typename TO, typename FROM>
TypedRectangle<TO> ConvertTypedRectangle(const TypedRectangle<FROM> &rectangle)
{
    return TypedRectangle<TO>(static_cast<TO>(rectangle.x), static_cast<TO>(rectangle.y), static_cast<TO>(rectangle.width), static_cast<TO>(rectangle.height));
}

inline bool operator==(const SamplerState &a, const SamplerState &b)
{
    return memcmp(&a, &b, sizeof(SamplerState)) == 0;
}

inline bool operator!=(const SamplerState &a, const SamplerState &b)
{
    return !(a == b);
}

}  // namespace gl
