//
// Copyright 2022 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//

// storeimage.h: Defines image storing functions

#ifndef IMAGEUTIL_STOREIMAGE_H_
#define IMAGEUTIL_STOREIMAGE_H_

#include <stddef.h>
#include <stdint.h>
#include <memory>

namespace angle
{

namespace priv
{

template <typename T>
inline T *OffsetDataPointer(uint8_t *data, size_t y, size_t z, size_t rowPitch, size_t depthPitch)
{
    return reinterpret_cast<T *>(data + (y * rowPitch) + (z * depthPitch));
}

template <typename T>
inline const T *OffsetDataPointer(const uint8_t *data,
                                  size_t y,
                                  size_t z,
                                  size_t rowPitch,
                                  size_t depthPitch)
{
    return reinterpret_cast<const T *>(data + (y * rowPitch) + (z * depthPitch));
}

}  // namespace priv

void StoreRGBA8ToPalettedImpl(size_t width,
                              size_t height,
                              size_t depth,
                              uint32_t indexBits,
                              uint32_t redBlueBits,
                              uint32_t greenBits,
                              uint32_t alphaBits,
                              const uint8_t *input,
                              size_t inputRowPitch,
                              size_t inputDepthPitch,
                              uint8_t *output,
                              size_t outputRowPitch,
                              size_t outputDepthPitch);  // namespace priv

}  // namespace angle

#endif
