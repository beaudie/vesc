//
// Copyright 2013 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//

// copyimage.cpp: Defines image copying functions

#include "image_util/copyimage.h"

namespace angle
{

inline uint32_t SwizzleBGRAToRGBA(uint32_t argb)
{
    return ((argb & 0x000000FF) << 16) |  // Move BGRA blue to RGBA blue
           ((argb & 0x00FF0000) >> 16) |  // Move BGRA red to RGBA red
           ((argb & 0xFF00FF00));         // Keep alpha and green
}

void CopyBGRA8ToRGBA8(const uint8_t *source, uint8_t *dest)
{
    *reinterpret_cast<uint32_t *>(dest) =
        SwizzleBGRAToRGBA(*reinterpret_cast<const uint32_t *>(source));
}

void CopyRowBGRA8ToRGBA8(const uint8_t *source, uint8_t *dest, int destWidth)
{
    const uint32_t *src32 = reinterpret_cast<const uint32_t *>(source);
    uint32_t *dest32      = reinterpret_cast<uint32_t *>(dest);
    const uint32_t *end32 = src32 + destWidth;
    while (src32 != end32)
    {
        *dest32++ = SwizzleBGRAToRGBA(*src32++);
    }
}

}  // namespace angle
