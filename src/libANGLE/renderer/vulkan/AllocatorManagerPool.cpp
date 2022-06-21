//
// Copyright 2022 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//
// AllocatorManagerPool:
//    Implements the pool allocator managers used in the command buffers.
//

#include "libANGLE/renderer/vulkan/AllocatorManagerPool.h"
#include "libANGLE/renderer/vulkan/SecondaryCommandBuffer.h"

namespace rx
{
namespace vk
{

void SharedPoolAllocatorManager::resetAllocator()
{
    mAllocator.pop();
    mAllocator.push();
}

void PoolAllocatorManager::reset(CommandBufferCommandTracker *commandBufferTracker)
{
    mCommandBufferPtr->clearCommand();
    mCurrentWritePointer   = nullptr;
    mCurrentBytesRemaining = 0;
    commandBufferTracker->reset();
}

// Initialize the SecondaryCommandBuffer by setting the allocator it will use
angle::Result PoolAllocatorManager::initialize(angle::PoolAllocator *allocator)
{
    ASSERT(allocator);
    ASSERT(mCommandBufferPtr->isCommandEmpty());
    mAllocator = allocator;
    allocateNewBlock();
    // Set first command to Invalid to start
    reinterpret_cast<uint16_t &>(*mCurrentWritePointer) = 0;

    return angle::Result::Continue;
}

bool PoolAllocatorManager::empty() const
{
    return mCommandBufferPtr->checkEmptyForPoolAlloc();
}

void PoolAllocatorManager::allocateNewBlock(size_t blockSize)
{
    ASSERT(mAllocator);
    mCurrentWritePointer   = mAllocator->fastAllocate(blockSize);
    mCurrentBytesRemaining = blockSize;
    mCommandBufferPtr->appendToCommand(mCurrentWritePointer);
}

void PoolAllocatorManager::getMemoryUsageStats(size_t *usedMemoryOut,
                                               size_t *allocatedMemoryOut) const
{
    mCommandBufferPtr->getMemoryUsageStatsForPoolAlloc(kBlockSize, usedMemoryOut,
                                                       allocatedMemoryOut);
}

}  // namespace vk
}  // namespace rx
