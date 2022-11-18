//
// Copyright 2022 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//
// AllocatorManagerRing:
//    Implements the ring buffer allocator managers used in the command buffers.
//

#include "libANGLE/renderer/vulkan/AllocatorManagerRing.h"
#include "libANGLE/renderer/vulkan/SecondaryCommandBuffer.h"

namespace rx
{
namespace vk
{

void SharedRingAllocatorHelper::resetAllocator()
{
    ASSERT(!mAllocator || !mAllocator->isShared());

    if (mAllocSharedCP)
    {
        mAllocSharedCP->releaseAndUpdate(&mAllocReleaseCP);
        mAllocSharedCP = nullptr;
    }

    ASSERT(!mAllocSharedCP && !mAllocReleaseCP.valid());
}

void SharedRingAllocatorHelper::attachAllocator(CommandsRingAllocator *allocator)
{
    ASSERT(allocator);
    ASSERT(!mAllocator);
    mAllocator = allocator;
    if (mAllocator->isShared())
    {
        mAllocator->releaseToSharedCP();
    }
}

CommandsRingAllocator *SharedRingAllocatorHelper::detachAllocator(bool isCommandBufferEmpty)
{
    if (!isCommandBufferEmpty)
    {
        // Must call reset() after detach from non-empty command buffer (OK to have an empty RP)
        ASSERT(!mAllocSharedCP && !mAllocReleaseCP.valid());
        mAllocSharedCP  = mAllocator->acquireSharedCP();
        mAllocReleaseCP = mAllocator->get().getReleaseCheckPoint();
    }
    CommandsRingAllocator *result = mAllocator;
    mAllocator                    = nullptr;
    return result;
}

void RingAllocatorHelper::attachAllocator(CommandsRingAllocator *source)
{
    ASSERT(source);
    RingBufferAllocator &sourceIn = source->get();

    ASSERT(sourceIn.valid());
    ASSERT(mCommandBuffer->hasEmptyCommands());
    ASSERT(mLastCommandBlock == nullptr);
    ASSERT(mFinishedCommandSize == 0);
    ASSERT(!mAllocator.valid());
    mAllocator = std::move(sourceIn);
    mAllocator.setFragmentReserve(kCommandHeaderSize);
    pushNewCommandBlock(mAllocator.allocate(0));
    mAllocator.setListener(this);
}

void RingAllocatorHelper::detachAllocator(CommandsRingAllocator *destination)
{
    ASSERT(destination);
    RingBufferAllocator &destinationOut = destination->get();
    ASSERT(!destinationOut.valid());

    ASSERT(mAllocator.valid());
    mAllocator.setListener(nullptr);
    finishLastCommandBlock();
    if (mFinishedCommandSize == 0)
    {
        mCommandBuffer->clearCommands();
    }
    else
    {
        mAllocator.setFragmentReserve(0);
        (void)mAllocator.allocate(sizeof(kCommandHeaderSize));
    }
    destinationOut = std::move(mAllocator);
}

uint32_t RingAllocatorHelper::getCommandSize() const
{
    uint32_t result = mFinishedCommandSize;
    if (mLastCommandBlock)
    {
        ASSERT(mAllocator.valid());
        ASSERT(mAllocator.getPointer() >= mLastCommandBlock);
        result += static_cast<uint32_t>(mAllocator.getPointer() - mLastCommandBlock);
    }
    return result;
}

void RingAllocatorHelper::pushNewCommandBlock(uint8_t *block)
{
    mLastCommandBlock = block;
    mCommandBuffer->pushToCommands(block);
}

void RingAllocatorHelper::getMemoryUsageStats(size_t *usedMemoryOut,
                                              size_t *allocatedMemoryOut) const
{
    *usedMemoryOut      = getCommandSize();
    *allocatedMemoryOut = getCommandSize();
}

}  // namespace vk
}  // namespace rx
