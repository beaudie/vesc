//
// Copyright 2022 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//
// AllocatorHelperPool:
//    Implements the pool allocator helpers used in the command buffers.
//

#include "libANGLE/renderer/vulkan/AllocatorHelperPool.h"
#include "libANGLE/renderer/vulkan/SecondaryCommandBuffer.h"

namespace rx
{
namespace vk
{

void CBHPoolAllocatorHelper::resetAllocator()
{
    mAllocator.pop();
    mAllocator.push();
}

void CommandPoolAllocatorHelper::reset(CommandBufferCommandTracker *commandBufferTracker)
{
    mCommandBuffer->clearCommands();
    mCurrentWritePointer   = nullptr;
    mCurrentBytesRemaining = 0;
    commandBufferTracker->reset();
}

// Initialize the SecondaryCommandBuffer by setting the allocator it will use
angle::Result CommandPoolAllocatorHelper::initialize(CommandsPoolAllocator *allocator)
{
    ASSERT(allocator);
    ASSERT(mCommandBuffer->hasEmptyCommands());
    mAllocator = allocator;
    allocateNewBlock();
    // Set first command to Invalid to start
    reinterpret_cast<CommandHeaderIDType &>(*mCurrentWritePointer) = 0;

    return angle::Result::Continue;
}

bool CommandPoolAllocatorHelper::empty() const
{
    return mCommandBuffer->checkEmptyForPoolAlloc();
}

void CommandPoolAllocatorHelper::allocateNewBlock(size_t blockSize)
{
    ASSERT(mAllocator);
    mCurrentWritePointer   = mAllocator->fastAllocate(blockSize);
    mCurrentBytesRemaining = blockSize;
    mCommandBuffer->pushToCommands(mCurrentWritePointer);
}

void CommandPoolAllocatorHelper::getMemoryUsageStats(size_t *usedMemoryOut,
                                                     size_t *allocatedMemoryOut) const
{
    mCommandBuffer->getMemoryUsageStatsForPoolAlloc(kBlockSize, usedMemoryOut, allocatedMemoryOut);
}

}  // namespace vk
}  // namespace rx
