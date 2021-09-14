#ifndef COMMON_RING_BUFFER_ALLOCATOR_H_
#define COMMON_RING_BUFFER_ALLOCATOR_H_

#include "angleutils.h"
#include "common/debug.h"

#define ANGLE_RING_BUFFER_ALLOCATOR_DEBUG 0

namespace angle
{

class RingBufferAllocator final : angle::NonCopyable
{
  public:
    // Only called from the "allocate()". Other function may also change the fragment
    class IAllocateListener
    {
    public:
        virtual void onRingBufferNewFragment() = 0;
        virtual void onRingBufferFragmentEnd() = 0;
    protected:
        ~IAllocateListener() = default;
    };

    class CheckPoint final
    {
      public:
        void reset() { *this = {}; }
        bool valid() const { return (mReleasePtr != nullptr); }
      private:
        friend class RingBufferAllocator;
        uint64_t mBuffferId = 0;
        uint8_t *mReleasePtr = nullptr;
    };

  public:
#if ANGLE_RING_BUFFER_ALLOCATOR_DEBUG
    std::string debugName;
#endif

    RingBufferAllocator() = default;
    RingBufferAllocator(RingBufferAllocator &&other);
    RingBufferAllocator &operator=(RingBufferAllocator &&other);

    void reset(uint32_t minCapacity = 1024);
    bool valid() const { return (getPointer() != nullptr); }

    void setListener(IAllocateListener *listener);

    // 1 - fastest decay speed
    // 2 - 2x slower than fastest, and so on...
    // Default is 10
    void setDecaySpeedFactor(uint32_t decaySpeedFactor);

    void setFragmentReserve(uint32_t reserve);

    uint8_t *allocate(uint32_t size)
    {
        ASSERT(valid());
        if (ANGLE_LIKELY(mFragmentEndR - mDataEnd >= static_cast<ptrdiff_t>(size)))
        {
            uint8_t *const result = mDataEnd;
            mDataEnd = result + size;
            return result;
        }
        return allocateInNewFragment(size);
    }

    uint8_t *getPointer() const { return mDataEnd; }
    uint32_t getFragmentSize() const
    {
        ASSERT(mFragmentEnd >= mDataEnd);
        return static_cast<uint32_t>(mFragmentEnd - mDataEnd);
    }

    CheckPoint getReleaseCheckPoint() const;
    void release(const CheckPoint &checkPoint);

  private:
    void release(uint8_t *releasePtr);
    uint32_t getNumAllocatedInBuffer() const;
    void resetPointers();

    uint8_t *allocateInNewFragment(uint32_t size);
    void resize(uint32_t newCapacity);

  private:
    struct Buffer final
    {
        static constexpr uint32_t kBaseOffset = alignof(std::max_align_t);

        uint64_t id = 0;
        std::vector<uint8_t> storage;

        void resize(uint32_t size) { storage.resize(size + kBaseOffset); }
        uint8_t *data() { return storage.data() + kBaseOffset; }

        uint8_t *decClamped(uint8_t *ptr, uint32_t offset) const
        {
            ASSERT(ptr >= storage.data() + kBaseOffset && ptr <= storage.data() + storage.size());
            return ptr - std::min(offset, static_cast<uint32_t>(ptr - storage.data()));
        }
    };

  private:
    IAllocateListener *mListener = nullptr;

    std::vector<Buffer> mOldBuffers;
    Buffer mBuffer;
    uint8_t *mDataBegin = nullptr;
    uint8_t *mDataEnd = nullptr;
    uint8_t *mFragmentEnd = nullptr;
    uint8_t *mFragmentEndR = nullptr;
    uint32_t mFragmentReserve = 0;

    uint32_t mMinCapacity = 0;
    uint32_t mCurrentCapacity = 0;
    uint32_t mAllocationMargin = 0;
    uint32_t mDecaySpeedFactor = 0;
};

class SharedRingBufferAllocator final : angle::NonCopyable
{
public:
    class CheckPoint final : angle::NonCopyable
    {
    public:
        void releaseAndUpdate(RingBufferAllocator::CheckPoint *newValue);
    private:
        RingBufferAllocator::CheckPoint pop();
    private:
        std::mutex mMutex;
        RingBufferAllocator::CheckPoint mValue;
#if defined(ANGLE_ENABLE_ASSERTS)
        std::atomic<uint32_t> mRefCount{ 1 };
#endif
        friend class SharedRingBufferAllocator;
    };

public:
    SharedRingBufferAllocator();
    ~SharedRingBufferAllocator();

    RingBufferAllocator &get() { return mAllocator; }

    // Once Shared - always Shared
    bool isShared() const { return mSharedCP != nullptr; }

    // Once acquired must be released with "releaseAndUpdate()"
    CheckPoint *acquireSharedCP();
    void releaseToSharedCP();

private:
    RingBufferAllocator mAllocator;
    CheckPoint *mSharedCP = nullptr;
};

}  // namespace angle

#endif  // COMMON_RING_BUFFER_ALLOCATOR_H_
