//
// Copyright (c) 2012-2015 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//

// angletypes.inl : Inline definitions of some functions from angletypes.h

namespace gl
{

inline bool operator==(const Rectangle &a, const Rectangle &b)
{
    return a.x == b.x &&
           a.y == b.y &&
           a.width == b.width &&
           a.height == b.height;
}

inline bool operator!=(const Rectangle &a, const Rectangle &b)
{
    return !(a == b);
}

inline bool operator==(const SamplerState &a, const SamplerState &b)
{
    return a.minFilter == b.minFilter &&
           a.magFilter == b.magFilter &&
           a.wrapS == b.wrapS &&
           a.wrapT == b.wrapT &&
           a.wrapR == b.wrapR &&
           a.maxAnisotropy == b.maxAnisotropy &&
           a.minLod == b.minLod &&
           a.maxLod == b.maxLod &&
           a.compareMode == b.compareMode &&
           a.compareFunc == b.compareFunc;
}

inline bool operator!=(const SamplerState &a, const SamplerState &b)
{
    return !(a == b);
}

inline bool operator==(const ImageUnit &a, const ImageUnit &b)
{
    return (a.texture == b.texture && a.level == b.level && a.layered == b.layered &&
            a.layer == b.layer && a.access == b.access && a.format == b.format);
}

inline bool operator!=(const ImageUnit &a, const ImageUnit &b)
{
    return !(a == b);
}

}
