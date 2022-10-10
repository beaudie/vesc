//
// Copyright 2022 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//

// AstcDecompressor.h: Decodes ASTC-encoded textures.

#ifndef IMAGE_UTIL_ASTC_CPU_DECOMPRESSOR_H_
#define IMAGE_UTIL_ASTC_CPU_DECOMPRESSOR_H_

#include <memory>
#include <string>

namespace angle
{

// This class is responsible for decompressing ASTC textures on the CPU.
// This class is thread-safe and all its methods can be called by any thread.
class AstcDecompressor
{
  public:
    // Returns the global singleton instance of this class.
    static AstcDecompressor &get();

    virtual ~AstcDecompressor() = default;

    // Whether the ASTC decompressor is available. Reasons why it may not be available include:
    //   - It wasn't compiled on this platform.
    //   - The CPU doesn't support AVX2 instructions.
    // If this returns false, decompress() will fail.
    virtual bool available() const = 0;

    // Decompress an ASTC texture.
    //
    // imgWidth, imgHeight: width and height of the texture, in texels.
    // blockWidth, blockHeight: ASTC encoding block size.
    // input: pointer to the ASTC data to decompress
    // inputLength: size of astData
    // output: where to white the decompressed output. This buffer must be able to hold at least
    //         imgWidth * imgHeight * 4 bytes.
    //
    // Returns 0 on success, or a non-zero status code on error. Use getStatusString() to convert
    // this status code to an error string.
    virtual int32_t decompress(uint32_t imgWidth,
                               uint32_t imgHeight,
                               uint32_t blockWidth,
                               uint32_t blockHeight,
                               const uint8_t *input,
                               size_t inputLength,
                               uint8_t *output) = 0;

    // Returns an error string for a given status code. Will always return non-null.
    virtual const char *getStatusString(int32_t statusCode) const = 0;
};

}  // namespace angle

#endif  // IMAGE_UTIL_ASTC_CPU_DECOMPRESSOR_H_
