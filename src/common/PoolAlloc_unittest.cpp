//
// Copyright 2019 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//
// PoolAlloc_unittest:
//   Tests of the PoolAlloc class
//

#include <gtest/gtest.h>

#include "common/PoolAlloc.h"

/* TODO : Tmp pasting this here for ref
 class PoolAllocator : angle::NonCopyable
{
  public:
    static const int kDefaultAlignment = 16;
    PoolAllocator(int growthIncrement = 8 * 1024, int allocationAlignment = kDefaultAlignment);
    PoolAllocator(PoolAllocator &&rhs) noexcept;
    PoolAllocator &operator=(PoolAllocator &&);

    //
    // Don't call the destructor just to free up the memory, call pop()
    //
    ~PoolAllocator();

    //
    // Call push() to establish a new place to pop memory to.  Does not
    // have to be called to get things started.
    //
    void push();

    //
    // Call pop() to free all memory allocated since the last call to push(),
    // or if no last call to push, frees all memory since first allocation.
    //
    void pop();

    //
    // Call popAll() to free all memory allocated.
    //
    void popAll();

    //
    // Call allocate() to actually acquire memory.  Returns 0 if no memory
    // available, otherwise a properly aligned pointer to 'numBytes' of memory.
    //
    void *allocate(size_t numBytes);

    //
    // Call reallocate() to resize a previous allocation.  Returns 0 if no memory
    // available, otherwise a properly aligned pointer to 'numBytes' of memory
    // where any contents from the original allocation will be preserved.
    //
    void *reallocate(void *originalAllocation, size_t numBytes);

    //
    // There is no deallocate.  The point of this class is that
    // deallocation can be skipped by the user of it, as the model
    // of use is to simultaneously deallocate everything at once
    // by calling pop(), and to not have to solve memory leak problems.
    //

    // Catch unwanted allocations.
    // TODO(jmadill): Remove this when we remove the global allocator.
    void lock();
    void unlock();

  private:
#if !defined(ANGLE_DISABLE_POOL_ALLOC)
    friend struct Header;

    struct Header
    {
        Header(Header *nextPage, size_t pageCount)
            : nextPage(nextPage),
              pageCount(pageCount)
#    ifdef GUARD_BLOCKS
              ,
              lastAllocation(0)
#    endif
        {}

        ~Header()
        {
#    ifdef GUARD_BLOCKS
            if (lastAllocation)
                lastAllocation->checkAllocList();
#    endif
        }

        Header *nextPage;
        size_t pageCount;
        Allocation *lastAllocation;
    };

    struct AllocState
    {
        size_t offset;
        Header *page;
    };
    using AllocStack = std::vector<AllocState>;

    // Track allocations if and only if we're using guard blocks
    void *initializeAllocation(Header *block, unsigned char *memory, size_t numBytes)
    {
        // Init Allocation by default for reallocation support.
        new (memory) Allocation(numBytes, memory, block->lastAllocation);
        block->lastAllocation = reinterpret_cast<Allocation *>(memory);
        return Allocation::OffsetAllocation(memory);
    }

    Allocation *getAllocationHeader(void *memAllocation) const;

    size_t mAlignment;  // all returned allocations will be aligned at
                        // this granularity, which will be a power of 2
    size_t mAlignmentMask;
    size_t mPageSize;           // granularity of allocation from the OS
    size_t mHeaderSkip;         // amount of memory to skip to make room for the
                                //      header (basically, size of header, rounded
                                //      up to make it aligned
    size_t mCurrentPageOffset;  // next offset in top of inUseList to allocate from
    Header *mFreeList;          // list of popped memory
    Header *mInUseList;         // list of all memory currently being used
    AllocStack mStack;          // stack of where to allocate from, to partition pool

    int mNumCalls;       // just an interesting statistic
    size_t mTotalBytes;  // just an interesting statistic

#else  // !defined(ANGLE_DISABLE_POOL_ALLOC)
    std::vector<std::vector<void *>> mStack;
#endif

    bool mLocked;
};
 */

namespace angle
{
// Verify the public interfact of PoolAllocator class
TEST(PoolAllocator, Interface)
{
    // Start w/ default alignment
    int alignment   = PoolAllocator::kDefaultAlignment;
    size_t numBytes = 1024;
    // Create a default pool allocator and allocate from it
    PoolAllocator poolAllocator;
    void *allocation = poolAllocator.allocate(numBytes);
    // Verify non-zero ptr returned
    EXPECT_NE(nullptr, allocation);
    // Verify alignment of allocation matches expected default
    printf("Allocation: %p, alignment: %d\n", allocation, alignment);
    EXPECT_EQ(0u, (reinterpret_cast<std::uintptr_t>(allocation) % alignment));
}

// Verify allocations are correctly aligned for different alignments
TEST(PoolAllocator, Alignment)
{
    // Test a range of alignments
    int alignmentVals[] = {2, 4, 8, 16, 32, 64, 128};
    std::vector<int> alignments(std::begin(alignmentVals), std::end(alignmentVals));
    size_t numBytes = 1024;
    for (int alignment : alignments)
    {
        // Create a default pool allocator and allocate from it
        PoolAllocator poolAllocator(2048, alignment);
        void *allocation = poolAllocator.allocate(numBytes);
        // Verify alignment of allocation matches expected default
        EXPECT_EQ(0u, (reinterpret_cast<std::uintptr_t>(allocation) % alignment));
    }
}
}  // namespace angle
