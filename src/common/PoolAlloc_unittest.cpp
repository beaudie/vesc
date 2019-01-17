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
// Verify the public interface of PoolAllocator class
TEST(PoolAllocator, Interface)
{
    size_t numBytes               = 1024;
    constexpr uint32_t kTestValue = 0xbaadbeef;
    // Create a default pool allocator and allocate from it
    PoolAllocator poolAllocator;
    void *allocation = poolAllocator.allocate(numBytes);
    // Verify non-zero ptr returned
    EXPECT_NE(nullptr, allocation);
    // Write to allocation to check later
    uint32_t *writePtr = static_cast<uint32_t *>(allocation);
    *writePtr          = kTestValue;
    // Test push and creating a new allocation
    poolAllocator.push();
    allocation = poolAllocator.allocate(numBytes);
    EXPECT_NE(nullptr, allocation);
    // Make an allocation that spans multiple pages
    allocation = poolAllocator.allocate(10 * 1024);
    // pop previous two allocations
    poolAllocator.pop();
    // Verify first allocation still has data
    EXPECT_EQ(*writePtr, kTestValue);
    // Make a bunch of allocations
    for (uint32_t i = 0; i < 1000; ++i)
    {
        allocation = poolAllocator.allocate(rand() % 2000 + 1);
        EXPECT_NE(nullptr, allocation);
    }
    // Free everything
    poolAllocator.popAll();
}

#ifndef _DEBUG  // Guard bands in debug build may alter alignment
// Verify allocations are correctly aligned for different alignments
class PoolAllocatorAlignmentTest : public testing::TestWithParam<int>
{};

TEST_P(PoolAllocatorAlignmentTest, Alignment)
{
    int alignment = GetParam();
    // Create a pool allocator to allocate from
    PoolAllocator poolAllocator(2048, alignment);
    // Test a number of allocation sizes for each alignment
    for (uint32_t i = 0; i < 100; ++i)
    {
        void *allocation = poolAllocator.allocate(rand() % 2000 + 1);
        // Verify alignment of allocation matches expected default
        EXPECT_EQ(0u, (reinterpret_cast<std::uintptr_t>(allocation) % alignment));
    }
}

INSTANTIATE_TEST_SUITE_P(, PoolAllocatorAlignmentTest, testing::Values(2, 4, 8, 16, 32, 64, 128));
#endif
}  // namespace angle
