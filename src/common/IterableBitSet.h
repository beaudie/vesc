//
// Copyright 2015 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//
// IterableBitSet:
//   A bitset class that has a fast iterator using bit scanning.
//

#include <stdint.h>

#include <bitset>

#include "common/platform.h"

namespace angle
{
template <size_t N>
class IterableBitSet final : public std::bitset<N>
{
  public:
    IterableBitSet() { static_assert(N <= 64, "IterableBitSet only supports up to 64 bits."); }
    class Iterator final
    {
      public:
        Iterator(const IterableBitSet *bits)
            : mBits(bits ? bits->to_ullong() : 0ull), mCurrentBit(bits ? getNextBit() : 0ull)
        {
        }

        Iterator &operator++()
        {
            mBits &= ~(1ull << mCurrentBit);
            mCurrentBit = getNextBit();
            return *this;
        }

        bool operator!=(const Iterator &other) const { return mBits != other.mBits; }
        size_t operator*() const { return mCurrentBit; }
        size_t getNextBit() const;

      private:
        unsigned long long mBits;
        size_t mCurrentBit;
    };

    Iterator begin() const { return Iterator(this); }
    Iterator end() const { return Iterator(nullptr); }
};

template <size_t N>
size_t IterableBitSet<N>::Iterator::getNextBit() const
{
#if defined(_WIN64)
    static_assert(sizeof(uint64_t) == sizeof(unsigned long long), "Incompatible sizes");

    unsigned long firstBitIndex = 0ul;
    unsigned char ret           = _BitScanForward64(&firstBitIndex, mBits);

    if (ret != 0)
        return static_cast<size_t>(firstBitIndex);
#elif defined(_WIN32)
    static_assert(sizeof(size_t) == sizeof(unsigned long), "Incompatible sizes");

    unsigned long firstBitIndex     = 0ul;
    const unsigned long BitsPerWord = 32ul;

    for (unsigned long offset = 0; offset <= BitsPerWord; offset += BitsPerWord)
    {
        unsigned long wordBits = mBits >> offset;
        unsigned char ret      = _BitScanForward(&firstBitIndex, wordBits);

        if (ret != 0)
        {
            return static_cast<size_t>(firstBitIndex + offset);
        }
    }
#elif defined(ANGLE_PLATFORM_LINUX) || defined(ANGLE_PLATFORM_APPLE)
    if (mBits != 0)
    {
        return __builtin_ctzll(mBits);
    }
#else
#error Please implement bit-scan-forward for your platform!
#endif
    return 0;
}

}  // gl
