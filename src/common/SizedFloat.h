//
// Copyright 2019 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//
// SizedFloat:
//   Helper class for encoding and decoding floating point.
//

#ifndef LIBANGLE_COMMON_SIZEDFLOAT_H_
#define LIBANGLE_COMMON_SIZEDFLOAT_H_

#include <math.h>
#include <algorithm>
#include <cstdint>

namespace angle
{
template <const int signBit, const int eBits, const int mBits>
struct SizedFloat
{
    static constexpr int kSignBit = signBit;
    static constexpr int kEBits   = eBits;
    static constexpr int kMBits   = mBits;

    static constexpr uint32_t Assemble(const uint32_t sVal,
                                       const uint32_t eVal,
                                       const uint32_t mVal)
    {
        return (signBit ? (sVal << (eBits + mBits)) : 0) | (eVal << mBits) | mVal;
    }

    static uint32_t Encode(const float signedV)
    {
        const float v = signBit ? fabsf(signedV) : std::max(0.0f, signedV);

        const int eBias   = (1 << (eBits - 1)) - 1;
        const int eValMax = (1 << eBits) - 1;

        const float eApprox = log2f(v);
        const auto eActual  = static_cast<int>(floorf(eApprox));

        int eVal      = eBias + eActual;
        uint32_t mVal = 0;
        if (v != v)
        {  // NaN
            eVal = eValMax;
            mVal = 1;
        }
        else if (eVal < 0)
        {  // underflow to zero
            eVal = 0;
            mVal = 0;
        }
        else if (eVal >= eValMax)
        {  // overfloat to Inf
            eVal = eValMax;
            mVal = 0;
        }
        else
        {
            float mFloat = 0.0;
            if (eVal == 0)
            {  // denormal
                mFloat = v * powf(2, 1 - eBias);
            }
            else
            {  // standard range
                mFloat = v * powf(2, -static_cast<float>(eActual)) - 1.0f;
            }
            mVal = static_cast<uint32_t>(mFloat * (1 << mBits) + 0.5);
        }

        const auto sVal = static_cast<uint32_t>(v < 0.0f);
        return Assemble(sVal, eVal, mVal);
    }

    static constexpr float Decode(const uint32_t sVal, const uint32_t eVal, const uint32_t mVal)
    {
        constexpr int eBias = (1 << (kEBits - 1)) - 1;
        constexpr int mDiv  = 1 << kMBits;
        float ret           = powf(-1.0f, static_cast<float>(sVal)) *
                    powf(2.0f, static_cast<float>(int(eVal) - eBias)) * (1.0f + float(mVal) / mDiv);
        return ret;
    }
};
using Float16  = SizedFloat<1, 5, 10>;
using UFloat11 = SizedFloat<0, 5, 6>;
using UFloat10 = SizedFloat<0, 5, 5>;

struct RGB9_E5 final
{
    // GLES 3.0.5 p129
    static constexpr int N = 9;   // number of mantissa bits per component
    static constexpr int B = 15;  // exponent bias

    static uint32_t Encode(const float red, const float green, const float blue)
    {
        const auto floori = [](const float x) { return static_cast<int>(floor(x)); };
        // GLES 3.0.5 p129
        constexpr int eMax = 31;  // max allowed biased exponent value

        const float twoToN       = powf(2.0f, static_cast<float>(N));
        const float sharedExpMax = (twoToN - 1.0f) / twoToN * powf(2.0f, eMax - B);

        const auto fnClampColor = [&](const float color) {
            return std::max(0.0f, std::min(color, sharedExpMax));
        };
        const float redC   = fnClampColor(red);
        const float greenC = fnClampColor(green);
        const float blueC  = fnClampColor(blue);

        const float maxC = std::max({redC, greenC, blueC});
        const int expP   = std::max(-B - 1, floori(log2f(maxC))) + 1 + B;

        const auto fnColorS = [&](const float colorC, const float exp) {
            return floori(colorC / powf(2, exp - B - N) + 0.5f);
        };
        const int maxS = fnColorS(maxC, static_cast<float>(expP));
        const int expS = expP + ((maxS == (1 << N)) ? 1 : 0);

        const int redS   = fnColorS(redC, static_cast<float>(expS));
        const int greenS = fnColorS(greenC, static_cast<float>(expS));
        const int blueS  = fnColorS(blueC, static_cast<float>(expS));

        // Pack as u32 EGBR.
        uint32_t ret = expS & 0x1f;
        ret <<= 9;
        ret |= blueS & 0x1ff;
        ret <<= 9;
        ret |= greenS & 0x1ff;
        ret <<= 9;
        ret |= redS & 0x1ff;
        return ret;
    }

    static void Decode(uint32_t packed,
                       float *const out_red,
                       float *const out_green,
                       float *const out_blue)
    {
        const auto redS = packed & 0x1ff;
        packed >>= 9;
        const auto greenS = packed & 0x1ff;
        packed >>= 9;
        const auto blueS = packed & 0x1ff;
        packed >>= 9;
        const auto expS = packed & 0x1f;

        // These are *not* IEEE-like UFloat14s.
        // GLES 3.0.5 p165:
        // red = redS*pow(2,expS-B-N)
        const auto fnToFloat = [&](const uint32_t x) {
            return x * powf(2.0f, static_cast<float>(int(expS) - B - N));
        };
        *out_red   = fnToFloat(redS);
        *out_green = fnToFloat(greenS);
        *out_blue  = fnToFloat(blueS);
    }
};
}  // namespace angle

#endif  // LIBANGLE_COMMON_SIZEDFLOAT_H_
