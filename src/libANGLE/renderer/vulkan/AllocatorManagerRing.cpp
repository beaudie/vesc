//
// Copyright 2022 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//
// AllocatorManager:
//    Implements the ring buffer allocator managers used in the command buffers.
//

#include "libANGLE/renderer/vulkan/AllocatorManagerRing.h"
#include "libANGLE/renderer/vulkan/SecondaryCommandBuffer.h"

namespace rx
{
namespace vk
{

void SharedRingAllocatorManager::resetAllocator()
{
    ASSERT(!mAllocator || !mAllocator->isShared());

    if (mAllocSharedCP)
    {
        mAllocSharedCP->releaseAndUpdate(&mAllocReleaseCP);
        mAllocSharedCP = nullptr;
    }

    ASSERT(!mAllocSharedCP && !mAllocReleaseCP.valid());
}

void SharedRingAllocatorManager::checkForRecycle(priv::SecondaryCommandBuffer *commandBuffer,
                                                 bool isCommandBufferEmpty)
{
    ASSERT(isCommandBufferEmpty);
    ASSERT(!hasAllocatorLinks());
    commandBuffer->open();
}

void SharedRingAllocatorManager::attachAllocator(RenderPassCommandsRingAllocatorType *allocator)
{
    ASSERT(allocator);
    ASSERT(!mAllocator);
    mAllocator = allocator;
    if (mAllocator->isShared())
    {
        mAllocator->releaseToSharedCP();
    }
}

RenderPassCommandsRingAllocatorType *SharedRingAllocatorManager::detachAllocator(
    bool isCommandBufferEmpty)
{
    if (!isCommandBufferEmpty)
    {
        // Must call reset() after detach from non-empty command buffer (OK to have an empty RP)
        ASSERT(!mAllocSharedCP && !mAllocReleaseCP.valid());
        mAllocSharedCP  = mAllocator->acquireSharedCP();
        mAllocReleaseCP = mAllocator->get().getReleaseCheckPoint();
    }
    RenderPassCommandsRingAllocatorType *result = mAllocator;
    mAllocator                                  = nullptr;
    return result;
}

void RingAllocatorManager::attachAllocator(RenderPassCommandsRingAllocatorType *source)
{
    ASSERT(source);
    RingBufferAllocatorType &sourceIn = source->get();

    ASSERT(sourceIn.valid());
    ASSERT(mCommandBufferPtr->isCommandEmpty());
    ASSERT(mLastCommandBlock == nullptr);
    ASSERT(mFinishedCommandSize == 0);
    ASSERT(!mAllocator.valid());
    mAllocator = std::move(sourceIn);
    mAllocator.setFragmentReserve(kCommandHeaderSize);
    pushNewCommandBlock(mAllocator.allocate(0));
    mAllocator.setListener(this);
}

void RingAllocatorManager::detachAllocator(RenderPassCommandsRingAllocatorType *destination)
{
    ASSERT(destination);
    RingBufferAllocatorType &destinationOut = destination->get();
    ASSERT(!destinationOut.valid());

    ASSERT(mAllocator.valid());
    mAllocator.setListener(nullptr);
    finishLastCommandBlock();
    if (mFinishedCommandSize == 0)
    {
        mCommandBufferPtr->clearCommand();
    }
    else
    {
        mAllocator.setFragmentReserve(0);
        (void)mAllocator.allocate(sizeof(kCommandHeaderSize));
    }
    destinationOut = std::move(mAllocator);
}

uint32_t RingAllocatorManager::getCommandSize() const
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

void RingAllocatorManager::pushNewCommandBlock(uint8_t *block)
{
    mCommandBufferPtr->appendToCommand(block);
    mLastCommandBlock = block;
}

void RingAllocatorManager::getMemoryUsageStats(size_t *usedMemoryOut,
                                               size_t *allocatedMemoryOut) const
{
    *usedMemoryOut      = getCommandSize();
    *allocatedMemoryOut = getCommandSize();
}

}  // namespace vk
}  // namespace rx
