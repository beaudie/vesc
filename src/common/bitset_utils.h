//
// Copyright 2015 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//
// bitset_utils:
//   Bitset-related helper classes, such as a fast iterator to scan for set bits.
//

#ifndef COMMON_BITSETITERATOR_H_
#define COMMON_BITSETITERATOR_H_

#include <stdint.h>

#include <bitset>

#include "common/angleutils.h"
#include "common/debug.h"
#include "common/mathutil.h"
#include "common/platform.h"

namespace angle
{
template <typename BitsT, typename ParamT>
constexpr static BitsT Bit(ParamT x)
{
    // It's undefined behavior if the shift size is equal to or larger than the width of the type.
    ASSERT(static_cast<size_t>(x) < sizeof(BitsT) * 8);

    return (static_cast<BitsT>(1) << static_cast<size_t>(x));
}

template <size_t N, typename BitsT, typename ParamT = std::size_t>
class BitSetT final
{
  public:
    class Reference final
    {
      public:
        ~Reference() {}
        Reference &operator=(bool x)
        {
            mParent->set(mBit, x);
            return *this;
        }
        explicit operator bool() const { return mParent->test(mBit); }

      private:
        friend class BitSetT;

        Reference(BitSetT *parent, ParamT bit) : mParent(parent), mBit(bit) {}

        BitSetT *mParent;
        ParamT mBit;
    };

    class Iterator final
    {
      public:
        Iterator(const BitSetT &bits);
        Iterator &operator++();

        bool operator==(const Iterator &other) const;
        bool operator!=(const Iterator &other) const;
        ParamT operator*() const;

        // These helper functions allow mutating an iterator in-flight.
        // They only operate on later bits to ensure we don't iterate the same bit twice.
        void resetLaterBit(std::size_t index)
        {
            ASSERT(index > mCurrentBit);
            mBitsCopy.reset(index);
        }

        void setLaterBit(std::size_t index)
        {
            ASSERT(index > mCurrentBit);
            mBitsCopy.set(index);
        }

      private:
        std::size_t getNextBit();

        BitSetT mBitsCopy;
        std::size_t mCurrentBit;
    };

    using value_type = BitsT;

    BitSetT();
    constexpr explicit BitSetT(BitsT value);

    BitSetT(const BitSetT &other);
    BitSetT &operator=(const BitSetT &other);

    bool operator==(const BitSetT &other) const;
    bool operator!=(const BitSetT &other) const;

    constexpr bool operator[](ParamT pos) const;
    Reference operator[](ParamT pos) { return Reference(this, pos); }

    bool test(ParamT pos) const;

    bool all() const;
    bool any() const;
    bool none() const;
    std::size_t count() const;

    constexpr std::size_t size() const { return N; }

    BitSetT &operator&=(const BitSetT &other);
    BitSetT &operator|=(const BitSetT &other);
    BitSetT &operator^=(const BitSetT &other);
    BitSetT operator~() const;

    BitSetT &operator&=(BitsT value);
    BitSetT &operator|=(BitsT value);
    BitSetT &operator^=(BitsT value);

    BitSetT operator<<(std::size_t pos) const;
    BitSetT &operator<<=(std::size_t pos);
    BitSetT operator>>(std::size_t pos) const;
    BitSetT &operator>>=(std::size_t pos);

    BitSetT &set();
    BitSetT &set(ParamT pos, bool value = true);

    BitSetT &reset();
    BitSetT &reset(ParamT pos);

    BitSetT &flip();
    BitSetT &flip(ParamT pos);

    unsigned long to_ulong() const { return static_cast<unsigned long>(mBits); }
    BitsT bits() const { return mBits; }

    Iterator begin() const { return Iterator(*this); }
    Iterator end() const { return Iterator(BitSetT()); }

    constexpr static BitSetT Zero() { return BitSetT(); }

    ParamT first() const;
    ParamT last() const;

  private:
    // Produces a mask of ones up to the "x"th bit.
    constexpr static BitsT Mask(std::size_t x)
    {
        return ((Bit<BitsT>(static_cast<ParamT>(x - 1)) - 1) << 1) + 1;
    }

    BitsT mBits;
};

template <size_t N>
class IterableBitSet : public std::bitset<N>
{
  public:
    IterableBitSet() {}
    IterableBitSet(const std::bitset<N> &implicitBitSet) : std::bitset<N>(implicitBitSet) {}

    class Iterator final
    {
      public:
        Iterator(const std::bitset<N> &bits);
        Iterator &operator++();

        bool operator==(const Iterator &other) const;
        bool operator!=(const Iterator &other) const;
        unsigned long operator*() const { return mCurrentBit; }

        // These helper functions allow mutating an iterator in-flight.
        // They only operate on later bits to ensure we don't iterate the same bit twice.
        void resetLaterBit(std::size_t index)
        {
            ASSERT(index > mCurrentBit);
            mBits.reset(index - mOffset);
        }

        void setLaterBit(std::size_t index)
        {
            ASSERT(index > mCurrentBit);
            mBits.set(index - mOffset);
        }

      private:
        unsigned long getNextBit();

        static constexpr size_t BitsPerWord = sizeof(uint32_t) * 8;
        std::bitset<N> mBits;
        unsigned long mCurrentBit;
        unsigned long mOffset;
    };

    Iterator begin() const { return Iterator(*this); }
    Iterator end() const { return Iterator(std::bitset<N>(0)); }
};

template <size_t N>
IterableBitSet<N>::Iterator::Iterator(const std::bitset<N> &bitset)
    : mBits(bitset), mCurrentBit(0), mOffset(0)
{
    if (mBits.any())
    {
        mCurrentBit = getNextBit();
    }
    else
    {
        mOffset = static_cast<unsigned long>(rx::roundUpPow2(N, BitsPerWord));
    }
}

template <size_t N>
ANGLE_INLINE typename IterableBitSet<N>::Iterator &IterableBitSet<N>::Iterator::operator++()
{
    ASSERT(mBits.any());
    mBits.set(mCurrentBit - mOffset, 0);
    mCurrentBit = getNextBit();
    return *this;
}

template <size_t N>
bool IterableBitSet<N>::Iterator::operator==(const Iterator &other) const
{
    return mOffset == other.mOffset && mBits == other.mBits;
}

template <size_t N>
bool IterableBitSet<N>::Iterator::operator!=(const Iterator &other) const
{
    return !(*this == other);
}

template <size_t N>
unsigned long IterableBitSet<N>::Iterator::getNextBit()
{
    // TODO(jmadill): Use 64-bit scan when possible.
    static constexpr std::bitset<N> wordMask(std::numeric_limits<uint32_t>::max());

    while (mOffset < N)
    {
        uint32_t wordBits = static_cast<uint32_t>((mBits & wordMask).to_ulong());
        if (wordBits != 0)
        {
            return gl::ScanForward(wordBits) + mOffset;
        }

        mBits >>= BitsPerWord;
        mOffset += BitsPerWord;
    }
    return 0;
}

template <size_t N, typename BitsT, typename ParamT>
BitSetT<N, BitsT, ParamT>::BitSetT() : mBits(0)
{
    static_assert(N > 0, "Bitset type cannot support zero bits.");
    static_assert(N <= sizeof(BitsT) * 8, "Bitset type cannot support a size this large.");
}

template <size_t N, typename BitsT, typename ParamT>
constexpr BitSetT<N, BitsT, ParamT>::BitSetT(BitsT value) : mBits(value & Mask(N))
{}

template <size_t N, typename BitsT, typename ParamT>
BitSetT<N, BitsT, ParamT>::BitSetT(const BitSetT &other) : mBits(other.mBits)
{}

template <size_t N, typename BitsT, typename ParamT>
BitSetT<N, BitsT, ParamT> &BitSetT<N, BitsT, ParamT>::operator=(const BitSetT &other)
{
    mBits = other.mBits;
    return *this;
}

template <size_t N, typename BitsT, typename ParamT>
bool BitSetT<N, BitsT, ParamT>::operator==(const BitSetT &other) const
{
    return mBits == other.mBits;
}

template <size_t N, typename BitsT, typename ParamT>
bool BitSetT<N, BitsT, ParamT>::operator!=(const BitSetT &other) const
{
    return mBits != other.mBits;
}

template <size_t N, typename BitsT, typename ParamT>
constexpr bool BitSetT<N, BitsT, ParamT>::operator[](ParamT pos) const
{
    return test(pos);
}

template <size_t N, typename BitsT, typename ParamT>
bool BitSetT<N, BitsT, ParamT>::test(ParamT pos) const
{
    return (mBits & Bit<BitsT>(pos)) != 0;
}

template <size_t N, typename BitsT, typename ParamT>
bool BitSetT<N, BitsT, ParamT>::all() const
{
    ASSERT(mBits == (mBits & Mask(N)));
    return mBits == Mask(N);
}

template <size_t N, typename BitsT, typename ParamT>
bool BitSetT<N, BitsT, ParamT>::any() const
{
    ASSERT(mBits == (mBits & Mask(N)));
    return (mBits != 0);
}

template <size_t N, typename BitsT, typename ParamT>
bool BitSetT<N, BitsT, ParamT>::none() const
{
    ASSERT(mBits == (mBits & Mask(N)));
    return (mBits == 0);
}

template <size_t N, typename BitsT, typename ParamT>
std::size_t BitSetT<N, BitsT, ParamT>::count() const
{
    return gl::BitCount(mBits);
}

template <size_t N, typename BitsT, typename ParamT>
BitSetT<N, BitsT, ParamT> &BitSetT<N, BitsT, ParamT>::operator&=(const BitSetT &other)
{
    mBits &= other.mBits;
    return *this;
}

template <size_t N, typename BitsT, typename ParamT>
BitSetT<N, BitsT, ParamT> &BitSetT<N, BitsT, ParamT>::operator|=(const BitSetT &other)
{
    mBits |= other.mBits;
    return *this;
}

template <size_t N, typename BitsT, typename ParamT>
BitSetT<N, BitsT, ParamT> &BitSetT<N, BitsT, ParamT>::operator^=(const BitSetT &other)
{
    mBits = mBits ^ other.mBits;
    return *this;
}

template <size_t N, typename BitsT, typename ParamT>
BitSetT<N, BitsT, ParamT> BitSetT<N, BitsT, ParamT>::operator~() const
{
    return BitSetT<N, BitsT, ParamT>(~mBits & Mask(N));
}

template <size_t N, typename BitsT, typename ParamT>
BitSetT<N, BitsT, ParamT> &BitSetT<N, BitsT, ParamT>::operator&=(BitsT value)
{
    mBits &= value;
    return *this;
}

template <size_t N, typename BitsT, typename ParamT>
BitSetT<N, BitsT, ParamT> &BitSetT<N, BitsT, ParamT>::operator|=(BitsT value)
{
    mBits |= value & Mask(N);
    return *this;
}

template <size_t N, typename BitsT, typename ParamT>
BitSetT<N, BitsT, ParamT> &BitSetT<N, BitsT, ParamT>::operator^=(BitsT value)
{
    mBits ^= value & Mask(N);
    return *this;
}

template <size_t N, typename BitsT, typename ParamT>
BitSetT<N, BitsT, ParamT> BitSetT<N, BitsT, ParamT>::operator<<(std::size_t pos) const
{
    return BitSetT<N, BitsT, ParamT>((mBits << pos) & Mask(N));
}

template <size_t N, typename BitsT, typename ParamT>
BitSetT<N, BitsT, ParamT> &BitSetT<N, BitsT, ParamT>::operator<<=(std::size_t pos)
{
    mBits = (mBits << pos & Mask(N));
    return *this;
}

template <size_t N, typename BitsT, typename ParamT>
BitSetT<N, BitsT, ParamT> BitSetT<N, BitsT, ParamT>::operator>>(std::size_t pos) const
{
    return BitSetT<N, BitsT, ParamT>(mBits >> pos);
}

template <size_t N, typename BitsT, typename ParamT>
BitSetT<N, BitsT, ParamT> &BitSetT<N, BitsT, ParamT>::operator>>=(std::size_t pos)
{
    mBits = ((mBits >> pos) & Mask(N));
    return *this;
}

template <size_t N, typename BitsT, typename ParamT>
BitSetT<N, BitsT, ParamT> &BitSetT<N, BitsT, ParamT>::set()
{
    ASSERT(mBits == (mBits & Mask(N)));
    mBits = Mask(N);
    return *this;
}

template <size_t N, typename BitsT, typename ParamT>
BitSetT<N, BitsT, ParamT> &BitSetT<N, BitsT, ParamT>::set(ParamT pos, bool value)
{
    ASSERT(mBits == (mBits & Mask(N)));
    if (value)
    {
        mBits |= Bit<BitsT>(pos) & Mask(N);
    }
    else
    {
        reset(pos);
    }
    return *this;
}

template <size_t N, typename BitsT, typename ParamT>
BitSetT<N, BitsT, ParamT> &BitSetT<N, BitsT, ParamT>::reset()
{
    ASSERT(mBits == (mBits & Mask(N)));
    mBits = 0;
    return *this;
}

template <size_t N, typename BitsT, typename ParamT>
BitSetT<N, BitsT, ParamT> &BitSetT<N, BitsT, ParamT>::reset(ParamT pos)
{
    ASSERT(mBits == (mBits & Mask(N)));
    mBits &= ~Bit<BitsT>(pos);
    return *this;
}

template <size_t N, typename BitsT, typename ParamT>
BitSetT<N, BitsT, ParamT> &BitSetT<N, BitsT, ParamT>::flip()
{
    ASSERT(mBits == (mBits & Mask(N)));
    mBits ^= Mask(N);
    return *this;
}

template <size_t N, typename BitsT, typename ParamT>
BitSetT<N, BitsT, ParamT> &BitSetT<N, BitsT, ParamT>::flip(ParamT pos)
{
    ASSERT(mBits == (mBits & Mask(N)));
    mBits ^= Bit<BitsT>(pos) & Mask(N);
    return *this;
}

template <size_t N, typename BitsT, typename ParamT>
ParamT BitSetT<N, BitsT, ParamT>::first() const
{
    ASSERT(!none());
    return static_cast<ParamT>(gl::ScanForward(mBits));
}

template <size_t N, typename BitsT, typename ParamT>
ParamT BitSetT<N, BitsT, ParamT>::last() const
{
    ASSERT(!none());
    return static_cast<ParamT>(gl::ScanReverse(mBits));
}

template <size_t N, typename BitsT, typename ParamT>
BitSetT<N, BitsT, ParamT>::Iterator::Iterator(const BitSetT &bits) : mBitsCopy(bits), mCurrentBit(0)
{
    if (bits.any())
    {
        mCurrentBit = getNextBit();
    }
}

template <size_t N, typename BitsT, typename ParamT>
ANGLE_INLINE typename BitSetT<N, BitsT, ParamT>::Iterator &
BitSetT<N, BitsT, ParamT>::Iterator::operator++()
{
    ASSERT(mBitsCopy.any());
    mBitsCopy.reset(static_cast<ParamT>(mCurrentBit));
    mCurrentBit = getNextBit();
    return *this;
}

template <size_t N, typename BitsT, typename ParamT>
bool BitSetT<N, BitsT, ParamT>::Iterator::operator==(const Iterator &other) const
{
    return mBitsCopy == other.mBitsCopy;
}

template <size_t N, typename BitsT, typename ParamT>
bool BitSetT<N, BitsT, ParamT>::Iterator::operator!=(const Iterator &other) const
{
    return !(*this == other);
}

template <size_t N, typename BitsT, typename ParamT>
ParamT BitSetT<N, BitsT, ParamT>::Iterator::operator*() const
{
    return static_cast<ParamT>(mCurrentBit);
}

template <size_t N, typename BitsT, typename ParamT>
std::size_t BitSetT<N, BitsT, ParamT>::Iterator::getNextBit()
{
    if (mBitsCopy.none())
    {
        return 0;
    }

    return gl::ScanForward(mBitsCopy.mBits);
}

template <size_t N>
using BitSet8 = BitSetT<N, uint8_t>;

template <size_t N>
using BitSet16 = BitSetT<N, uint16_t>;

template <size_t N>
using BitSet32 = BitSetT<N, uint32_t>;

template <size_t N>
using BitSet64 = BitSetT<N, uint64_t>;

namespace priv
{

template <size_t N, typename T>
using EnableIfBitsFit = typename std::enable_if<N <= sizeof(T) * 8>::type;

template <size_t N, typename Enable = void>
struct GetBitSet
{
    using Type = IterableBitSet<N>;
};

// Prefer 64-bit bitsets on 64-bit CPUs. They seem faster than 32-bit.
#if defined(ANGLE_IS_64_BIT_CPU)
template <size_t N>
struct GetBitSet<N, EnableIfBitsFit<N, uint64_t>>
{
    using Type = BitSet64<N>;
};
#else
template <size_t N>
struct GetBitSet<N, EnableIfBitsFit<N, uint32_t>>
{
    using Type = BitSet32<N>;
};
#endif  // defined(ANGLE_IS_64_BIT_CPU)

}  // namespace priv

template <size_t N>
using BitSet = typename priv::GetBitSet<N>::Type;

template <size_t N>
class BitSetLarge final
{
  private:
// Prefer 64-bit bitsets on 64-bit CPUs.
#if defined(ANGLE_IS_64_BIT_CPU)
    static constexpr size_t kBaseBitSetSize = 64;
    using BaseBitSet                        = BitSet64<kBaseBitSetSize>;
#else
    static constexpr size_t kBaseBitSetSize = 32;
    using BaseBitSet                        = BitSet32<kBaseBitSetSize>;
#endif  // defined(ANGLE_IS_64_BIT_CPU)
    static constexpr size_t kOneLessThanKBaseBitSetSize = kBaseBitSetSize - 1;
    static constexpr size_t kStartIndex                 = 0;
    static constexpr size_t kShiftForDivision =
        static_cast<size_t>(rx::Log2(static_cast<unsigned int>(kBaseBitSetSize)));
    static constexpr size_t kArraySize = ((N + kOneLessThanKBaseBitSetSize) >> kShiftForDivision);

    std::array<BaseBitSet, kArraySize> mBaseBitSetArray;

  public:
    BitSetLarge()
    {
        static_assert(N > 64, "BitSetLarge type can't support <= 64 bits.");
        reset();
    }

    BitSetLarge(const BitSetLarge<N> &other)
    {
        for (std::size_t index = kStartIndex; index < size(); index++)
        {
            mBaseBitSetArray[index] = other.mBaseBitSetArray[index];
        }
    }

    class Reference final
    {
      public:
        ~Reference() {}
        Reference &operator=(bool x)
        {
            mParent.set(mPosition, x);
            return *this;
        }
        explicit operator bool() const { return mParent.test(mPosition); }

      private:
        friend class BitSetLarge;

        Reference(BitSetLarge &parent, std::size_t pos) : mParent(parent), mPosition(pos) {}

        BitSetLarge &mParent;
        std::size_t mPosition;
    };

    class Iterator final
    {
      public:
        Iterator(const BitSetLarge<N> &bitSetLarge, size_t index)
            : mParent(bitSetLarge),
              mIndex(index),
              mCurrentIterator(mParent.mBaseBitSetArray[kStartIndex].begin())
        {
            while (mIndex < mParent.kArraySize)
            {
                if (mParent.mBaseBitSetArray[mIndex].any())
                {
                    break;
                }
                mIndex++;
            }

            if (mIndex < mParent.kArraySize)
            {
                mCurrentIterator = mParent.mBaseBitSetArray[mIndex].begin();
            }
            else
            {
                mCurrentIterator = mParent.mBaseBitSetArray[mParent.kArraySize - 1].end();
            }
        }

        Iterator &operator++()
        {
            ++mCurrentIterator;
            if (mCurrentIterator == mParent.mBaseBitSetArray[mIndex].end())
            {
                mIndex++;
                if (mIndex < mParent.kArraySize)
                {
                    mCurrentIterator = mParent.mBaseBitSetArray[mIndex].begin();
                }
            }
            return *this;
        }

        bool operator==(const Iterator &other) const
        {
            return mCurrentIterator == other.mCurrentIterator;
        }
        bool operator!=(const Iterator &other) const
        {
            return mCurrentIterator != other.mCurrentIterator;
        }

        size_t operator*() const { return (mIndex * kBaseBitSetSize) + *mCurrentIterator; }

      private:
        const BitSetLarge &mParent;
        size_t mIndex;
        typename BaseBitSet::Iterator mCurrentIterator;
    };

    constexpr std::size_t size() const { return N; }
    Iterator begin() const { return Iterator(*this, 0); }
    Iterator end() const { return Iterator(*this, kArraySize); }
    unsigned long to_ulong() const
    {
        // Fixme: When serializing BitSetLarge, it no longer fits in a ulong
        return static_cast<unsigned long>(mBaseBitSetArray[kStartIndex].to_ulong());
    }

    BitSetLarge &operator=(const BitSetLarge &other)
    {
        for (std::size_t index = kStartIndex; index < size(); index++)
        {
            mBaseBitSetArray[index] = other.mBaseBitSetArray[index];
        }
        return *this;
    }

    bool operator[](std::size_t pos) const
    {
        ASSERT(pos < size());
        return test(pos);
    }

    Reference operator[](std::size_t pos)
    {
        ASSERT(pos < size());
        return Reference(*this, pos);
    }

    BitSetLarge &set(std::size_t pos, bool value = true)
    {
        ASSERT(pos < size());
        // Get the index and offset, then set the bit
        size_t index  = pos >> kShiftForDivision;
        size_t offset = pos & kOneLessThanKBaseBitSetSize;
        mBaseBitSetArray[index].set(offset, value);
        return *this;
    }

    BitSetLarge &reset()
    {
        for (BaseBitSet &baseBitSet : mBaseBitSetArray)
        {
            baseBitSet.reset();
        }
        return *this;
    }

    BitSetLarge &reset(std::size_t pos)
    {
        ASSERT(pos < size());
        return set(pos, false);
    }

    bool test(std::size_t pos) const
    {
        ASSERT(pos < size());
        // Get the index and offset, then test the bit
        size_t index  = pos >> kShiftForDivision;
        size_t offset = pos & kOneLessThanKBaseBitSetSize;
        return mBaseBitSetArray[index].test(offset);
    }

    bool all() const
    {
        for (std::size_t index = kStartIndex; index < kArraySize; index++)
        {
            const BaseBitSet &baseBitSet = mBaseBitSetArray[index];

            if (index == kArraySize - 1)
            {
                // The last BaseBitSet needs special handling
                std::size_t remainingBitCount = size() - index * kBaseBitSetSize;
                return (baseBitSet.count() == remainingBitCount);
            }
            else
            {
                if (!baseBitSet.all())
                {
                    return false;
                }
            }
        }
        return true;
    }

    bool any() const
    {
        for (const BaseBitSet &baseBitSet : mBaseBitSetArray)
        {
            if (baseBitSet.any())
            {
                return true;
            }
        }
        return false;
    }

    bool none() const
    {
        for (const BaseBitSet &baseBitSet : mBaseBitSetArray)
        {
            if (!baseBitSet.none())
            {
                return false;
            }
        }
        return true;
    }

    std::size_t count() const
    {
        size_t count = 0;
        for (const BaseBitSet &baseBitSet : mBaseBitSetArray)
        {
            count += baseBitSet.count();
        }
        return count;
    }

    bool intersects(const BitSetLarge &other) const
    {
        for (std::size_t index = kStartIndex; index < kArraySize; index++)
        {
            if (mBaseBitSetArray[index].bits() & other.mBaseBitSetArray[index].bits())
            {
                return true;
            }
        }
        return false;
    }

    BitSetLarge<N> &flip()
    {
        // Fixme: This is not quite right, the last element in mBaseBitSetArray may need special
        // handling
        for (BaseBitSet &baseBitSet : mBaseBitSetArray)
        {
            baseBitSet.flip();
        }
        return *this;
    }
};

template <std::size_t N>
class BitSet128 final
{
  private:
// Prefer 64-bit bitsets on 64-bit CPUs.
#if defined(ANGLE_IS_64_BIT_CPU)
    static constexpr size_t kBaseBitSetSize = 64;
    using BaseBitSet                        = BitSet64<kBaseBitSetSize>;
#else
    static constexpr size_t kBaseBitSetSize = 32;
    using BaseBitSet                        = BitSet32<kBaseBitSetSize>;
#endif  // defined(ANGLE_IS_64_BIT_CPU)
    static constexpr size_t kOneLessThanKBaseBitSetSize = kBaseBitSetSize - 1;
    static constexpr size_t kIndexZero                  = 0;
    static constexpr size_t kIndexOne                   = 1;
    static constexpr size_t kShiftForDivision =
        static_cast<size_t>(rx::Log2(static_cast<unsigned int>(kBaseBitSetSize)));
    static constexpr size_t kArraySize = 2;

    BaseBitSet mBaseBitSetArray[kArraySize];

  public:
    BitSet128()
    {
        static_assert(N > 64, "BitSet128 type can't support <= 64 bits.");
        static_assert(N <= 128, "BitSet128 type can't support more than 128 bits.");
        reset();
    }

    BitSet128(const BitSet128<N> &other)
    {
        mBaseBitSetArray[kIndexZero] = other.mBaseBitSetArray[kIndexZero];
        mBaseBitSetArray[kIndexOne]  = other.mBaseBitSetArray[kIndexOne];
    }

    class Reference final
    {
      public:
        ~Reference() {}
        Reference &operator=(bool x)
        {
            mParent.set(mPosition, x);
            return *this;
        }
        explicit operator bool() const { return mParent.test(mPosition); }

      private:
        friend class BitSet128<N>;

        Reference(BitSet128<N> &parent, std::size_t pos) : mParent(parent), mPosition(pos) {}

        BitSet128<N> &mParent;
        std::size_t mPosition;
    };

    class Iterator final
    {
      public:
        Iterator(const BitSet128<N> &bitSet, size_t index)
            : mParent(bitSet),
              mIndex(index),
              mCurrentIterator(mParent.mBaseBitSetArray[kIndexZero].begin())
        {
            ASSERT((mIndex == mParent.kArraySize) || (mIndex == kIndexZero));
            if (mIndex == mParent.kArraySize)
            {
                mCurrentIterator = mParent.mBaseBitSetArray[kIndexOne].end();
            }
            else
            {
                if (!mParent.mBaseBitSetArray[mIndex].any())
                {
                    mIndex++;
                    mCurrentIterator = mParent.mBaseBitSetArray[mIndex].begin();
                }
            }
        }

        Iterator &operator++()
        {
            ++mCurrentIterator;
            if ((mCurrentIterator == mParent.mBaseBitSetArray[mIndex].end()) &&
                (mIndex == kIndexZero))
            {
                mIndex++;
                mCurrentIterator = mParent.mBaseBitSetArray[mIndex].begin();
            }
            return *this;
        }

        bool operator==(const Iterator &other) const
        {
            return mCurrentIterator == other.mCurrentIterator;
        }
        bool operator!=(const Iterator &other) const
        {
            return mCurrentIterator != other.mCurrentIterator;
        }

        size_t operator*() const { return (mIndex * kBaseBitSetSize) + *mCurrentIterator; }

      private:
        const BitSet128<N> &mParent;
        size_t mIndex;
        typename BaseBitSet::Iterator mCurrentIterator;
    };

    constexpr std::size_t size() const { return N; }
    Iterator begin() const { return Iterator(*this, 0); }
    Iterator end() const { return Iterator(*this, kArraySize); }
    unsigned long to_ulong() const
    {
        // Fixme: When serializing BitSet128, it no longer fits in a ulong
        return static_cast<unsigned long>(mBaseBitSetArray[kIndexZero].to_ulong());
    }

    BitSet128<N> &operator=(const BitSet128<N> &other)
    {
        mBaseBitSetArray[kIndexZero] = other.mBaseBitSetArray[kIndexZero];
        mBaseBitSetArray[kIndexOne]  = other.mBaseBitSetArray[kIndexOne];
        return *this;
    }

    bool operator[](std::size_t pos) const
    {
        ASSERT(pos < size());
        return test(pos);
    }

    Reference operator[](std::size_t pos)
    {
        ASSERT(pos < size());
        return Reference(*this, pos);
    }

    BitSet128<N> &set(std::size_t pos, bool value = true)
    {
        ASSERT(pos < size());
        // Get the index and offset, then set the bit
        size_t index  = pos >> kShiftForDivision;
        size_t offset = pos & kOneLessThanKBaseBitSetSize;
        mBaseBitSetArray[index].set(offset, value);
        return *this;
    }

    BitSet128<N> &reset()
    {
        mBaseBitSetArray[kIndexZero].reset();
        mBaseBitSetArray[kIndexOne].reset();
        return *this;
    }

    BitSet128<N> &reset(std::size_t pos)
    {
        ASSERT(pos < size());
        return set(pos, false);
    }

    bool test(std::size_t pos) const
    {
        ASSERT(pos < size());
        // Get the index and offset, then test the bit
        size_t index  = pos >> kShiftForDivision;
        size_t offset = pos & kOneLessThanKBaseBitSetSize;
        return mBaseBitSetArray[index].test(offset);
    }

    bool all() const
    {
        return ((mBaseBitSetArray[kIndexZero].all()) &&
                (mBaseBitSetArray[kIndexOne].count() == size() - kBaseBitSetSize));
    }

    bool any() const
    {
        return mBaseBitSetArray[kIndexZero].any() || mBaseBitSetArray[kIndexOne].any();
    }

    bool none() const
    {
        return mBaseBitSetArray[kIndexZero].none() && mBaseBitSetArray[kIndexOne].none();
    }

    std::size_t count() const
    {
        return mBaseBitSetArray[kIndexZero].count() + mBaseBitSetArray[kIndexOne].count();
    }

    bool intersects(const BitSet128<N> &other) const
    {
        return ((mBaseBitSetArray[kIndexZero].bits() & other.mBaseBitSetArray[kIndexZero].bits()) ||
                (mBaseBitSetArray[kIndexOne].bits() & other.mBaseBitSetArray[kIndexOne].bits()));
    }

    BitSet128<N> &flip()
    {
        // Fixme: This is not quite right, the last element in mBaseBitSetArray may need special
        // handling
        mBaseBitSetArray[kIndexZero].flip();
        mBaseBitSetArray[kIndexOne].flip();
        return *this;
    }
};
}  // namespace angle

template <size_t N, typename BitsT, typename ParamT>
inline angle::BitSetT<N, BitsT, ParamT> operator&(const angle::BitSetT<N, BitsT, ParamT> &lhs,
                                                  const angle::BitSetT<N, BitsT, ParamT> &rhs)
{
    angle::BitSetT<N, BitsT, ParamT> result(lhs);
    result &= rhs.bits();
    return result;
}

template <size_t N, typename BitsT, typename ParamT>
inline angle::BitSetT<N, BitsT, ParamT> operator|(const angle::BitSetT<N, BitsT, ParamT> &lhs,
                                                  const angle::BitSetT<N, BitsT, ParamT> &rhs)
{
    angle::BitSetT<N, BitsT, ParamT> result(lhs);
    result |= rhs.bits();
    return result;
}

template <size_t N, typename BitsT, typename ParamT>
inline angle::BitSetT<N, BitsT, ParamT> operator^(const angle::BitSetT<N, BitsT, ParamT> &lhs,
                                                  const angle::BitSetT<N, BitsT, ParamT> &rhs)
{
    angle::BitSetT<N, BitsT, ParamT> result(lhs);
    result ^= rhs.bits();
    return result;
}
#endif  // COMMON_BITSETITERATOR_H_
