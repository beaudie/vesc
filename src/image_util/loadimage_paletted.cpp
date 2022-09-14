//
// Copyright 2022 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//

// loadimage_paletted.cpp: Decodes GL_PALETTE_* textures.

#include "image_util/loadimage.h"

#include <type_traits>
#include "common/mathutil.h"

#include "image_util/imageformats.h"

namespace angle
{

template <typename T>
R8G8B8A8 readColor(const T *src)
{
    gl::ColorF tmp;
    T::readColor(&tmp, src);
    R8G8B8A8 rgba;
    R8G8B8A8::writeColor(&rgba, &tmp);
    return rgba;
}

size_t decodeIndexIntoPalette(const uint8_t *row, size_t i, uint32_t indexBits)
{
    switch (indexBits)
    {
        case 4:
        {
            bool even = i % 2 == 0;
            return (row[i / 2] >> (even ? 0 : 4)) & 0x0f;
        }

        case 8:
            return row[i];

        default:
            UNREACHABLE();
            return 0;
    }
}

R8G8B8A8 decodeColor(const uint8_t *src,
                     uint32_t redBlueBits,
                     uint32_t greenBits,
                     uint32_t alphaBits)
{
    switch (redBlueBits)
    {
        case 8:
            ASSERT(greenBits == 8);
            switch (alphaBits)
            {
                case 0:
                    return readColor<>(reinterpret_cast<const R8G8B8 *>(src));
                case 8:
                    return readColor<>(reinterpret_cast<const R8G8B8A8 *>(src));
                default:
                    break;
            }
            break;

        case 5:
            switch (greenBits)
            {
                case 6:
                    ASSERT(alphaBits == 0);
                    return readColor<>(reinterpret_cast<const R5G6B5 *>(src));
                case 5:
                    ASSERT(alphaBits == 1);
                    return readColor<>(reinterpret_cast<const R5G5B5A1 *>(src));
                default:
                    break;
            }
            break;

        case 4:
            ASSERT(greenBits == 4 && alphaBits == 4);
            return readColor<>(reinterpret_cast<const R4G4B4A4 *>(src));

        default:
            break;
    }

    UNREACHABLE();
    return R8G8B8A8{0, 0, 0, 255};
}

// See LoadPalettedToRGBA8.
void LoadPalettedToRGBA8Impl(size_t width,
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
                             size_t outputDepthPitch)
{
    fprintf(stderr,
            "width=%d height=%d depth=%d inputRowPitch=%d inputDepthPitch=%d outputRowPitch=%d "
            "outputDepthPitch=%d\n",
            (int)width, (int)height, (int)depth, (int)inputRowPitch, (int)inputDepthPitch,
            (int)outputRowPitch, (int)outputDepthPitch);

    size_t colorBytes   = (2 * redBlueBits + greenBits + alphaBits) / 8;
    size_t paletteSize  = 1 << indexBits;
    size_t paletteBytes = paletteSize * colorBytes;

    const uint8_t *palette = input;

    const uint8_t *texels = input + paletteBytes;  // + mip levels

    for (size_t z = 0; z < depth; z++)
    {
        for (size_t y = 0; y < height; y++)
        {
            const uint8_t *srcRow =
                priv::OffsetDataPointer<uint8_t>(texels, y, z, inputRowPitch, inputDepthPitch);
            R8G8B8A8 *dstRow =
                priv::OffsetDataPointer<R8G8B8A8>(output, y, z, outputRowPitch, outputDepthPitch);

            for (size_t x = 0; x < width; x++)
            {
                size_t indexIntoPalette = decodeIndexIntoPalette(srcRow, x, indexBits);

                dstRow[x] = decodeColor(palette + indexIntoPalette * colorBytes, redBlueBits,
                                        greenBits, alphaBits);
            }
        }
    }
}

}  // namespace angle
