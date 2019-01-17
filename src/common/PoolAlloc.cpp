//
// Copyright 2019 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//
// PoolAlloc.cpp:
//    Implements the class methods for PoolAllocator and Allocation classes.
//

#include "common/PoolAlloc.h"

#include <assert.h>
#include <stdint.h>
#include <stdio.h>

#include "common/angleutils.h"
#include "common/debug.h"
#include "common/platform.h"
#include "common/tls.h"

namespace angle
{

//
// Implement the functionality of the PoolAllocator class, which
// is documented in PoolAlloc.h.
//
PoolAllocator::PoolAllocator(int growthIncrement, int allocationAlignment)
    : mAlignment(allocationAlignment),
#if !defined(ANGLE_DISABLE_POOL_ALLOC)
      mPageSize(growthIncrement),
      mFreeList(0),
      mInUseList(0),
      mNumCalls(0),
      mTotalBytes(0),
#endif
      mLocked(false)
{
    //
    // Adjust mAlignment to be at least pointer aligned and
    // power of 2.
    //
    size_t minAlign = sizeof(void *);
    mAlignment &= ~(minAlign - 1);
    if (mAlignment < minAlign)
        mAlignment = minAlign;
    size_t a = 1;
    while (a < mAlignment)
        a <<= 1;
    mAlignment     = a;
    mAlignmentMask = a - 1;

#if !defined(ANGLE_DISABLE_POOL_ALLOC)
    //
    // Don't allow page sizes we know are smaller than all common
    // OS page sizes.
    //
    if (mPageSize < 4 * 1024)
        mPageSize = 4 * 1024;

    //
    // A large mCurrentPageOffset indicates a new page needs to
    // be obtained to allocate memory.
    //
    mCurrentPageOffset = mPageSize;

    //
    // Align header skip
    //
    mHeaderSkip = minAlign;
    if (mHeaderSkip < sizeof(Header))
    {
        mHeaderSkip = (sizeof(Header) + mAlignmentMask) & ~mAlignmentMask;
    }
#else  // !defined(ANGLE_DISABLE_POOL_ALLOC)
    mStack.push_back({});
#endif
}

PoolAllocator::PoolAllocator(PoolAllocator &&rhs) noexcept
    : mAlignment(std::exchange(rhs.mAlignment, 0)),
      mAlignmentMask(std::exchange(rhs.mAlignmentMask, 0)),
#if !defined(ANGLE_DISABLE_POOL_ALLOC)
      mPageSize(std::exchange(rhs.mPageSize, 0)),
      mHeaderSkip(std::exchange(rhs.mHeaderSkip, 0)),
      mCurrentPageOffset(std::exchange(rhs.mCurrentPageOffset, 0)),
      mFreeList(std::exchange(rhs.mFreeList, nullptr)),
      mInUseList(std::exchange(rhs.mInUseList, nullptr)),
      mStack(std::move(rhs.mStack)),
      mNumCalls(std::exchange(rhs.mNumCalls, 0)),
      mTotalBytes(std::exchange(rhs.mTotalBytes, 0)),
#else
      mStack(std::move(rhs.mStack)),
#endif
      mLocked(std::exchange(rhs.mLocked, false))
{}

PoolAllocator &PoolAllocator::operator=(PoolAllocator &&rhs)
{
    if (this != &rhs)
    {
        std::swap(mAlignment, rhs.mAlignment);
        std::swap(mAlignmentMask, rhs.mAlignmentMask);
#if !defined(ANGLE_DISABLE_POOL_ALLOC)
        std::swap(mPageSize, rhs.mPageSize);
        std::swap(mHeaderSkip, rhs.mHeaderSkip);
        std::swap(mCurrentPageOffset, rhs.mCurrentPageOffset);
        std::swap(mFreeList, rhs.mFreeList);
        std::swap(mInUseList, rhs.mInUseList);
        std::swap(mNumCalls, rhs.mNumCalls);
        std::swap(mTotalBytes, rhs.mTotalBytes);
#endif
        std::swap(mStack, rhs.mStack);
        std::swap(mLocked, rhs.mLocked);
    }
    return *this;
}

PoolAllocator::~PoolAllocator()
{
#if !defined(ANGLE_DISABLE_POOL_ALLOC)
    while (mInUseList)
    {
        Header *next = mInUseList->nextPage;
        mInUseList->~Header();
        delete[] reinterpret_cast<char *>(mInUseList);
        mInUseList = next;
    }

    // We should not check the guard blocks
    // here, because we did it already when the block was
    // placed into the free list.
    //
    while (mFreeList)
    {
        Header *next = mFreeList->nextPage;
        delete[] reinterpret_cast<char *>(mFreeList);
        mFreeList = next;
    }
#else  // !defined(ANGLE_DISABLE_POOL_ALLOC)
    for (auto &allocs : mStack)
    {
        for (auto alloc : allocs)
        {
            free(alloc);
        }
    }
    mStack.clear();
#endif
}

//
// Check a single guard block for damage
//
void Allocation::checkGuardBlock(unsigned char *blockMem,
                                 unsigned char val,
                                 const char *locText) const
{
#ifdef GUARD_BLOCKS
    for (size_t x = 0; x < kGuardBlockSize; x++)
    {
        if (blockMem[x] != val)
        {
            char assertMsg[80];
            // We don't print the assert message.  It's here just to be helpful.
            snprintf(assertMsg, sizeof(assertMsg),
                     "PoolAlloc: Damage %s %zu byte allocation at 0x%p\n", locText, mSize, data());

            assert(0 && "PoolAlloc: Damage in guard block");
        }
    }
#endif
}

void PoolAllocator::push()
{
#if !defined(ANGLE_DISABLE_POOL_ALLOC)
    AllocState state = {mCurrentPageOffset, mInUseList};

    mStack.push_back(state);

    //
    // Indicate there is no current page to allocate from.
    //
    mCurrentPageOffset = mPageSize;
#else  // !defined(ANGLE_DISABLE_POOL_ALLOC)
    mStack.push_back({});
#endif
}

//
// Do a mass-deallocation of all the individual allocations
// that have occurred since the last push(), or since the
// last pop(), or since the object's creation.
//
// The deallocated pages are saved for future allocations.
//
void PoolAllocator::pop()
{
    if (mStack.size() < 1)
        return;

#if !defined(ANGLE_DISABLE_POOL_ALLOC)
    Header *page       = mStack.back().page;
    mCurrentPageOffset = mStack.back().offset;

    while (mInUseList != page)
    {
        // invoke destructor to free allocation list
        mInUseList->~Header();

        Header *nextInUse = mInUseList->nextPage;
        if (mInUseList->pageCount > 1)
            delete[] reinterpret_cast<char *>(mInUseList);
        else
        {
            mInUseList->nextPage = mFreeList;
            mFreeList            = mInUseList;
        }
        mInUseList = nextInUse;
    }

    mStack.pop_back();
#else  // !defined(ANGLE_DISABLE_POOL_ALLOC)
    for (auto &alloc : mStack.back())
    {
        free(alloc);
    }
    mStack.pop_back();
#endif
}

//
// Do a mass-deallocation of all the individual allocations
// that have occurred.
//
void PoolAllocator::popAll()
{
    while (mStack.size() > 0)
        pop();
}

//
// Return a pointer to the Allocation Header for an existing memAllocation.
// Pre-condition: memAllocation must be non-null
//
Allocation *PoolAllocator::getAllocationHeader(void *memAllocation) const
{
    ASSERT(memAllocation != nullptr);
    uint8_t *origAllocAddress = static_cast<uint8_t *>(memAllocation);
    return reinterpret_cast<Allocation *>(origAllocAddress - sizeof(Allocation));
}

//
// Do a reallocation, resizing the the given originalAllocation to numBytes while
// preserving the contents of originalAllocation.
//
void *PoolAllocator::reallocate(void *originalAllocation, size_t numBytes)
{
    if (originalAllocation == nullptr)
    {
        return allocate(numBytes);
    }
    if (numBytes == 0)
    {
        // this is a no-op given the current way we use new pool allocators. Memory will be freed
        // when allocator is destroyed.
        return nullptr;
    }

    // Compare the original allocation size to requested realloc size
    Allocation *origAllocationHeader = getAllocationHeader(originalAllocation);
    size_t origSize                  = origAllocationHeader->getSize();
    if (origSize > numBytes)
    {
        // For growing allocation, create new allocation and copy over original contents
        void *newAlloc = allocate(numBytes);
        memcpy(newAlloc, originalAllocation, origSize);
        return newAlloc;
    }
    // For shrinking allocation, shrink size and return original alloc ptr
    origAllocationHeader->setSize(numBytes);
    return originalAllocation;
}

void *PoolAllocator::allocate(size_t numBytes)
{
    ASSERT(!mLocked);

#if !defined(ANGLE_DISABLE_POOL_ALLOC)
    //
    // Just keep some interesting statistics.
    //
    ++mNumCalls;
    mTotalBytes += numBytes;

    // If we are using guard blocks, all allocations are bracketed by
    // them: [guardblock][allocation][guardblock].  numBytes is how
    // much memory the caller asked for.  allocationSize is the total
    // size including guard blocks.  In release build,
    // kGuardBlockSize=0 and this all gets optimized away.
    size_t allocationSize = Allocation::AllocationSize(numBytes);
    // Detect integer overflow.
    if (allocationSize < numBytes)
        return 0;

    //
    // Do the allocation, most likely case first, for efficiency.
    // This step could be moved to be inline sometime.
    //
    if (allocationSize <= mPageSize - mCurrentPageOffset)
    {
        //
        // Safe to allocate from mCurrentPageOffset.
        //
        unsigned char *memory = reinterpret_cast<unsigned char *>(mInUseList) + mCurrentPageOffset;
        mCurrentPageOffset += allocationSize;
        mCurrentPageOffset = (mCurrentPageOffset + mAlignmentMask) & ~mAlignmentMask;

        return initializeAllocation(mInUseList, memory, numBytes);
    }

    if (allocationSize > mPageSize - mHeaderSkip)
    {
        //
        // Do a multi-page allocation.  Don't mix these with the others.
        // The OS is efficient in allocating and freeing multiple pages.
        //
        size_t numBytesToAlloc = allocationSize + mHeaderSkip;
        // Detect integer overflow.
        if (numBytesToAlloc < allocationSize)
            return 0;

        Header *memory = reinterpret_cast<Header *>(::new char[numBytesToAlloc]);
        if (memory == 0)
            return 0;

        // Use placement-new to initialize header
        new (memory) Header(mInUseList, (numBytesToAlloc + mPageSize - 1) / mPageSize);
        mInUseList = memory;

        mCurrentPageOffset = mPageSize;  // make next allocation come from a new page

        // No guard blocks for multi-page allocations (yet)
        return reinterpret_cast<void *>(reinterpret_cast<uintptr_t>(memory) + mHeaderSkip);
    }

    //
    // Need a simple page to allocate from.
    //
    Header *memory;
    if (mFreeList)
    {
        memory    = mFreeList;
        mFreeList = mFreeList->nextPage;
    }
    else
    {
        memory = reinterpret_cast<Header *>(::new char[mPageSize]);
        if (memory == 0)
            return 0;
    }

    // Use placement-new to initialize header
    new (memory) Header(mInUseList, 1);
    mInUseList = memory;

    unsigned char *ret = reinterpret_cast<unsigned char *>(mInUseList) + mHeaderSkip;
    mCurrentPageOffset = (mHeaderSkip + allocationSize + mAlignmentMask) & ~mAlignmentMask;

    return initializeAllocation(mInUseList, ret, numBytes);
#else  // !defined(ANGLE_DISABLE_POOL_ALLOC)
    void *alloc = malloc(numBytes + mAlignmentMask);
    mStack.back().push_back(alloc);

    intptr_t intAlloc = reinterpret_cast<intptr_t>(alloc);
    intAlloc = (intAlloc + mAlignmentMask) & ~mAlignmentMask;
    return reinterpret_cast<void *>(intAlloc);
#endif
}

void PoolAllocator::lock()
{
    ASSERT(!mLocked);
    mLocked = true;
}

void PoolAllocator::unlock()
{
    ASSERT(mLocked);
    mLocked = false;
}

//
// Check all allocations in a list for damage by calling check on each.
//
void Allocation::checkAllocList() const
{
    for (const Allocation *alloc = this; alloc != 0; alloc = alloc->mPrevAlloc)
        alloc->check();
}

}  // namespace angle