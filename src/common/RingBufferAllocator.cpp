#include "common/RingBufferAllocator.h"

namespace angle
{

// RingBufferAllocator implementation.
RingBufferAllocator::RingBufferAllocator(RingBufferAllocator &&other)
{
    *this = std::move(other);
}

RingBufferAllocator &RingBufferAllocator::operator=(RingBufferAllocator &&other)
{
#if ANGLE_RING_BUFFER_ALLOCATOR_DEBUG
    debugName = std::move(other.debugName);
#endif

    mOldBuffers = std::move(other.mOldBuffers);
    mBuffer = std::move(other.mBuffer);
    mDataBegin = other.mDataBegin;
    mDataEnd = other.mDataEnd;
    mFragmentEnd = other.mFragmentEnd;
    mFragmentEndR = other.mFragmentEndR;
    mFragmentReserve = other.mFragmentReserve;

    mMinCapacity = other.mMinCapacity;
    mCurrentCapacity = other.mCurrentCapacity;
    mAllocationMargin = other.mAllocationMargin;
    mDecaySpeedFactor = other.mDecaySpeedFactor;

    ASSERT(other.mOldBuffers.size() == 0);
    ASSERT(other.mBuffer.storage.size() == 0);
    other.mBuffer.id = 0;
    other.mDataBegin = nullptr;
    other.mDataEnd = nullptr;
    other.mFragmentEnd = nullptr;
    other.mFragmentEndR = nullptr;
    other.mFragmentReserve = 0;

    other.mMinCapacity = 0;
    other.mCurrentCapacity = 0;
    other.mAllocationMargin = 0;
    other.mDecaySpeedFactor = 0;

    return *this;
}

void RingBufferAllocator::reset(uint32_t minCapacity)
{
    mListener = nullptr;
    mFragmentReserve = 0;
    mDecaySpeedFactor = 10;
    mMinCapacity = minCapacity;
    resize(mMinCapacity);
    mOldBuffers.clear();
}

void RingBufferAllocator::setListener(IAllocateListener *listener)
{
    ASSERT(!mListener || !listener);
    mListener = listener;
}

void RingBufferAllocator::setDecaySpeedFactor(uint32_t decaySpeedFactor)
{
    ASSERT(valid());
    mDecaySpeedFactor = std::max(decaySpeedFactor, 1u);
}

RingBufferAllocator::CheckPoint RingBufferAllocator::getReleaseCheckPoint() const
{
    ASSERT(valid());
    CheckPoint result;
    result.mBuffferId = mBuffer.id;
    result.mReleasePtr = mDataEnd;
    return result;
}

void RingBufferAllocator::release(const CheckPoint &checkPoint)
{
#if ANGLE_RING_BUFFER_ALLOCATOR_DEBUG
    INFO() << "RingBufferAllocator[" << debugName
            << "] mOldBuffers.size(): " << mOldBuffers.size()
            << "; mCurrentCapacity: " << mCurrentCapacity
            << "; NumAllocated: " << getNumAllocatedInBuffer()
            << "; mAllocationMargin: " << mAllocationMargin
            << "; mDecaySpeedFactor: " << mDecaySpeedFactor << ";";
#endif

    ASSERT(valid());
    ASSERT(checkPoint.valid());

    if (mOldBuffers.size() > 0)
    {
        // mOldBuffers are sorted by id
        int removeCount = 0;
        for (uint32_t i = 0; (i < mOldBuffers.size()) &&
                (mOldBuffers[i].id < checkPoint.mBuffferId); ++i)
        {
            ++removeCount;
        }
        mOldBuffers.erase(mOldBuffers.begin(), mOldBuffers.begin() + removeCount);
    }

    if (checkPoint.mBuffferId == mBuffer.id)
    {
        const uint32_t allocatedBefore = getNumAllocatedInBuffer();

        release(checkPoint.mReleasePtr);

        if (allocatedBefore >= mAllocationMargin)
        {
            if ((mCurrentCapacity > mMinCapacity) &&
                (allocatedBefore * 6 <= mCurrentCapacity))
            {
                resize(std::max(allocatedBefore * 3, mMinCapacity));
            }
            else
            {
                mAllocationMargin = mCurrentCapacity;
            }
        }
        else
        {
            const uint64_t numReleased = (allocatedBefore - getNumAllocatedInBuffer());
            const uint64_t distanceToMargin = (mAllocationMargin - allocatedBefore);
            mAllocationMargin -= std::max(static_cast<uint32_t>(
                numReleased * distanceToMargin / mAllocationMargin / mDecaySpeedFactor), 1u);
        }
    }
}

void  RingBufferAllocator::setFragmentReserve(uint32_t reserve)
{
    ASSERT(valid());
    mFragmentReserve = reserve;
    mFragmentEndR = mBuffer.decClamped(mFragmentEnd, mFragmentReserve);
}

uint8_t *RingBufferAllocator::allocateInNewFragment(uint32_t size)
{
    if (mListener)
    {
        mListener->onRingBufferFragmentEnd();
    }

    if (mFragmentEnd != mDataBegin)
    {
        uint8_t *const result = mBuffer.data();
        uint8_t *const newFragmentEnd = mDataBegin;
        uint8_t *const newFragmentEndR = mBuffer.decClamped(newFragmentEnd, mFragmentReserve);

        // Wrap around only if can allocate!
        if (newFragmentEndR - result >= static_cast<ptrdiff_t>(size))
        {
            mDataEnd = result;
            mFragmentEnd = newFragmentEnd;
            mFragmentEndR = newFragmentEndR;

            if (mListener)
            {
                mListener->onRingBufferNewFragment();
            }

            mDataEnd = result + size;
            return result;
        }
    }

#if ANGLE_RING_BUFFER_ALLOCATOR_DEBUG
    const uint32_t numAllocated = getNumAllocatedInBuffer();
    INFO() << "RingBufferAllocator[" << debugName
            << "] mOldBuffers.size(): " << mOldBuffers.size()
            << "; mCurrentCapacity: " << mCurrentCapacity
            << "; NumAllocated: " << numAllocated
            << "; NumAllocated + size: " << (numAllocated + size) << ";";
#endif
    resize(std::max(mCurrentCapacity + mCurrentCapacity / 2, size + mFragmentReserve));

    if (mListener)
    {
        mListener->onRingBufferNewFragment();
    }

    ASSERT(mFragmentEndR - mDataEnd >= static_cast<ptrdiff_t>(size));
    uint8_t *const result = mDataEnd;
    mDataEnd = result + size;
    return result;
}

void RingBufferAllocator::resize(uint32_t newCapacity)
{
#if ANGLE_RING_BUFFER_ALLOCATOR_DEBUG
    INFO() << "RingBufferAllocator[" << debugName << "] newCapacity: " << newCapacity << ";";
#endif
    ASSERT(newCapacity >= mMinCapacity);

    if (mBuffer.id != 0)
    {
        mOldBuffers.emplace_back(std::move(mBuffer));
    }

    mCurrentCapacity = newCapacity;
    mBuffer.id += 1;
    mBuffer.resize(mCurrentCapacity);
    resetPointers();

    mAllocationMargin = mCurrentCapacity;
}

void RingBufferAllocator::release(uint8_t *releasePtr)
{
    if (releasePtr == mDataEnd)  // Ensures "mDataEnd == mBuffer.data()" with 0 allocations
    {
        resetPointers();
        return;
    }
    if (mDataBegin == mFragmentEnd)
    {
        ASSERT((releasePtr >= mBuffer.data() && releasePtr < mDataEnd) ||
                (releasePtr >= mDataBegin && releasePtr <= mBuffer.data() + mCurrentCapacity));
        if (releasePtr < mDataBegin)
        {
            mFragmentEnd = mBuffer.data() + mCurrentCapacity;
        }
        else
        {
            mFragmentEnd = releasePtr;
        }
        mFragmentEndR = mBuffer.decClamped(mFragmentEnd, mFragmentReserve);
    }
    else
    {
        ASSERT(releasePtr >= mDataBegin && releasePtr < mDataEnd);
    }
    mDataBegin = releasePtr;
}

uint32_t RingBufferAllocator::getNumAllocatedInBuffer() const
{
    // 2 free fragments: [mBuffer.begin, mDataBegin)                    [mDataEnd, mBuffer.end);
    // 1 used fragment:                             [DataBegin, DataEnd)
    if (mFragmentEnd != mDataBegin)
    {
        ASSERT(mDataEnd >= mDataBegin);
        return static_cast<uint32_t>(mDataEnd - mDataBegin);
    }

    // 1 free fragment:                           [mDataEnd, mDataBegin)
    // 2 used fragments: [mBuffer.begin, mDataEnd)                      [mDataBegin, mBuffer.end)
    ASSERT(mDataBegin >= mDataEnd);
    return (mCurrentCapacity - static_cast<uint32_t>(mDataBegin - mDataEnd));
}

void RingBufferAllocator::resetPointers()
{
    mDataBegin = mBuffer.data();
    mDataEnd = mDataBegin;
    mFragmentEnd = mDataEnd + mCurrentCapacity;
    mFragmentEndR = mBuffer.decClamped(mFragmentEnd, mFragmentReserve);
}

// SharedRingBufferAllocator implementation.
SharedRingBufferAllocator::SharedRingBufferAllocator()
{
    mAllocator.reset();
}

SharedRingBufferAllocator::~SharedRingBufferAllocator()
{
#if defined(ANGLE_ENABLE_ASSERTS)
    ASSERT(!mSharedCP || mSharedCP->mRefCount == 1);
#endif
    SafeDelete(mSharedCP);
}

SharedRingBufferAllocator::CheckPoint *SharedRingBufferAllocator::acquireSharedCP()
{
    if (!mSharedCP)
    {
        mSharedCP = new CheckPoint();
    }
#if defined(ANGLE_ENABLE_ASSERTS)
    ASSERT(++mSharedCP->mRefCount > 1);  // Must always be 1 ref before
#endif
    return mSharedCP;
}

void SharedRingBufferAllocator::releaseToSharedCP()
{
    ASSERT(mSharedCP);
    const auto releaseCP = mSharedCP->pop();
    if (releaseCP.valid())
    {
        mAllocator.release(releaseCP);
    }
}

void SharedRingBufferAllocator::CheckPoint::releaseAndUpdate(
        RingBufferAllocator::CheckPoint *newValue)
{
    ASSERT(newValue && newValue->valid());
#if defined(ANGLE_ENABLE_ASSERTS)
    ASSERT(--mRefCount >= 1);  // Must always remain 1 ref
#endif
    {
        std::lock_guard<std::mutex> lock(mMutex);
        mValue = *newValue;
    }
    newValue->reset();
}

RingBufferAllocator::CheckPoint SharedRingBufferAllocator::CheckPoint::pop()
{
    std::lock_guard<std::mutex> lock(mMutex);
    RingBufferAllocator::CheckPoint value = mValue;
    mValue.reset();
    return value;
}

}  // namespace angle
