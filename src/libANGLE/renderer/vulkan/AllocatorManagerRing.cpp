//
// Copyright 2022 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//
// AllocatorManager:
//    Implements the ring buffer allocator managers used in the command buffers.
//

#if ANGLE_ENABLE_VULKAN_SHARED_RING_BUFFER_CMD_ALLOC

#    include "libANGLE/renderer/vulkan/AllocatorManagerRing.h"
#    include "libANGLE/renderer/vulkan/SecondaryCommandBuffer.h"

namespace rx
{
namespace vk
{

void SharedRingAllocatorManager::resetAllocator(uint32_t *usedBufferCountOut)
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
                                                 bool isEmpty)
{
    ASSERT(isEmpty);
    ASSERT(!hasAllocatorLinks());
    commandBuffer->open();
}

void SharedRingAllocatorManager::attachAllocator(RenderPassCommandsAllocatorType *allocator)
{
    ASSERT(allocator);
    ASSERT(!mAllocator);
    mAllocator = allocator;
    if (mAllocator->isShared())
    {
        mAllocator->releaseToSharedCP();
    }
}

RenderPassCommandsAllocatorType *SharedRingAllocatorManager::detachAllocator(
    bool isCommandBufferEmpty)
{
    if (!isCommandBufferEmpty)
    {
        // Must call reset() after detach from non-empty command buffer (OK to have an empty RP)
        ASSERT(!mAllocSharedCP && !mAllocReleaseCP.valid());
        mAllocSharedCP  = mAllocator->acquireSharedCP();
        mAllocReleaseCP = mAllocator->get().getReleaseCheckPoint();
    }
    RenderPassCommandsAllocatorType *result = mAllocator;
    mAllocator                              = nullptr;
    return result;
}

void RingAllocatorManager::attachAllocator(RenderPassCommandsAllocatorType *source)
{
    ASSERT(source);
    RingBufferAllocatorType &sourceIn = source->get();

    ASSERT(sourceIn.valid());
    ASSERT(mCommandsPtr->empty());
    ASSERT(mLastCommandBlock == nullptr);
    ASSERT(mFinishedCommandSize == 0);
    ASSERT(!mAllocator.valid());
    mAllocator = std::move(sourceIn);
    mAllocator.setFragmentReserve(sizeof(priv::CommandHeader));
    pushNewCommandBlock(mAllocator.allocate(0));
    mAllocator.setListener(this);
}

void RingAllocatorManager::detachAllocator(RenderPassCommandsAllocatorType *destination)
{
    ASSERT(destination);
    RingBufferAllocatorType &destinationOut = destination->get();
    ASSERT(!destinationOut.valid());

    ASSERT(mAllocator.valid());
    mAllocator.setListener(nullptr);
    finishLastCommandBlock();
    if (mFinishedCommandSize == 0)
    {
        mCommandsPtr->clear();
    }
    else
    {
        mAllocator.setFragmentReserve(0);
        (void)mAllocator.allocate(sizeof(priv::CommandHeader));
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

priv::CommandHeader *RingAllocatorManager::getHeader(priv::CommandID cmdID, size_t allocationSize)
{
    priv::CommandHeader *header = reinterpret_cast<priv::CommandHeader *>(
        mAllocator.allocate(static_cast<uint32_t>(allocationSize)));
    header->id   = cmdID;
    header->size = static_cast<uint16_t>(allocationSize);

    return header;
}

void RingAllocatorManager::getMemoryUsageStats(size_t *usedMemoryOut,
                                               size_t *allocatedMemoryOut) const
{
    *usedMemoryOut      = getCommandSize();
    *allocatedMemoryOut = getCommandSize();
}

}  // namespace vk
}  // namespace rx

#endif  // ANGLE_ENABLE_VULKAN_SHARED_RING_BUFFER_CMD_ALLOC
