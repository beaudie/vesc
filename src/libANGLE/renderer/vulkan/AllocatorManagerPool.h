//
// Copyright 2022 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//
// AllocatorManagerPool:
//    Manages the pool allocators used in the command buffers.
//

#ifndef LIBANGLE_RENDERER_VULKAN_ALLOCATORMANAGERPOOL_H_
#define LIBANGLE_RENDERER_VULKAN_ALLOCATORMANAGERPOOL_H_

#include "common/PoolAlloc.h"
#include "libANGLE/renderer/vulkan/CommandParamTypes.h"

#if ANGLE_ENABLE_VULKAN_SHARED_RING_BUFFER_CMD_ALLOC
#else

namespace rx
{
namespace vk
{
namespace priv
{
class SecondaryCommandBuffer;
}  // namespace priv

// Pool Alloc uses 16kB pages w/ 16byte header = 16368bytes. To minimize waste
//  using a 16368/12 = 1364. Also better perf than 1024 due to fewer block allocations
static constexpr size_t kBlockSize = 1364;
// Make sure block size is 4-byte aligned to avoid Android errors
static_assert((kBlockSize % 4) == 0, "Check kBlockSize alignment");

constexpr size_t kDefaultPoolAllocatorPageSize = 16 * 1024;

using RenderPassCommandsAllocatorType = angle::PoolAllocator;

class SharedPoolAllocatorManager;
using SharedAllocatorManager = SharedPoolAllocatorManager;

class PoolAllocatorManager;
using CommandBufferAllocatorManager = PoolAllocatorManager;

class SharedPoolAllocatorManager
{
  public:
    SharedPoolAllocatorManager() = default;
    void resetAllocator(uint32_t *usedBufferCountOut);

    void checkForRecycle(priv::SecondaryCommandBuffer *commandBuffer, bool isEmpty) {}

    bool initialize(uint32_t *usedBufferCountOut)
    {
        mAllocator.initialize(kDefaultPoolAllocatorPageSize, 1);
        // Push a scope into the pool allocator so we can easily free and re-init on reset()
        mAllocator.push();
        *usedBufferCountOut = 0;
        return true;
    }

    // Placeholder functions for attaching and detaching the allocator.
    void attachAllocator(RenderPassCommandsAllocatorType *allocator) {}
    RenderPassCommandsAllocatorType *detachAllocator(bool isCommandBufferEmpty) { return nullptr; }

    angle::PoolAllocator *getAllocator() { return &mAllocator; }

  private:
    // Allocator used by this class. Using a pool allocator per CBH to avoid threading issues
    //  that occur w/ shared allocator between multiple CBHs.
    angle::PoolAllocator mAllocator;
};

class PoolAllocatorManager final
{
  public:
    PoolAllocatorManager()
        : mAllocator(nullptr),
          mCurrentWritePointer(nullptr),
          mCurrentBytesRemaining(0),
          mCommandsPtr(nullptr)
    {}

    void setCommands(std::vector<priv::CommandHeader *> *commands) { mCommandsPtr = commands; }
    void resetCommands() { mCommandsPtr = nullptr; }

    void reset(CommandBufferCommandTracker *commandBufferTracker)
    {
        mCommandsPtr->clear();
        mCurrentWritePointer   = nullptr;
        mCurrentBytesRemaining = 0;
        commandBufferTracker->reset();
    }

    void resetHandle() { mAllocator = nullptr; }

    // Initialize the SecondaryCommandBuffer by setting the allocator it will use
    angle::Result initialize(angle::PoolAllocator *allocator)
    {
        ASSERT(allocator);
        ASSERT(mCommandsPtr->empty());
        mAllocator = allocator;
        allocateNewBlock();
        // Set first command to Invalid to start
        reinterpret_cast<priv::CommandHeader *>(mCurrentWritePointer)->id =
            priv::CommandID::Invalid;

        return angle::Result::Continue;
    }

    bool valid() const { return mAllocator != nullptr; }
    bool empty() const
    {
        return mCommandsPtr->size() == 0 || mCommandsPtr->at(0)->id == priv::CommandID::Invalid;
    }

    void getMemoryUsageStats(size_t *usedMemoryOut, size_t *allocatedMemoryOut) const;

    // Placeholder function
    void terminateLastCommandBlock() {}

    // Placeholder functions for attaching and detaching the allocator.
    void attachAllocator(vk::RenderPassCommandsAllocatorType *source) {}
    void detachAllocator(vk::RenderPassCommandsAllocatorType *destination) {}

    ANGLE_INLINE void allocateNewBlock(size_t blockSize = kBlockSize)
    {
        ASSERT(mAllocator);
        mCurrentWritePointer   = mAllocator->fastAllocate(blockSize);
        mCurrentBytesRemaining = blockSize;
        mCommandsPtr->push_back(reinterpret_cast<priv::CommandHeader *>(mCurrentWritePointer));
    }

    template <class StructType>
    ANGLE_INLINE StructType *initCommand(priv::CommandID cmdID,
                                         size_t variableSize,
                                         uint8_t **variableDataPtr)
    {
        constexpr size_t fixedAllocationSize = sizeof(StructType) + sizeof(priv::CommandHeader);
        const size_t allocationSize          = fixedAllocationSize + variableSize;
        // Make sure we have enough room to mark follow-on header "Invalid"
        const size_t requiredSize = allocationSize + sizeof(priv::CommandHeader);
        if (mCurrentBytesRemaining < requiredSize)
        {
            // variable size command can potentially exceed default cmd allocation blockSize
            if (requiredSize <= kBlockSize)
                allocateNewBlock();
            else
            {
                // Make sure allocation is 4-byte aligned
                const size_t alignedSize = roundUpPow2<size_t>(requiredSize, 4);
                ASSERT((alignedSize % 4) == 0);
                allocateNewBlock(alignedSize);
            }
        }
        *variableDataPtr = priv::Offset<uint8_t>(mCurrentWritePointer, fixedAllocationSize);
        return commonInit<StructType>(cmdID, allocationSize);
    }

    template <class StructType>
    ANGLE_INLINE StructType *initCommand(priv::CommandID cmdID)
    {
        constexpr size_t paramSize =
            std::is_same<StructType, priv::EmptyParams>::value ? 0 : sizeof(StructType);
        constexpr size_t allocationSize = paramSize + sizeof(priv::CommandHeader);
        // Make sure we have enough room to mark follow-on header "Invalid"
        if (mCurrentBytesRemaining < (allocationSize + sizeof(priv::CommandHeader)))
        {
            ASSERT((allocationSize + sizeof(priv::CommandHeader)) < kBlockSize);
            allocateNewBlock();
        }
        return commonInit<StructType>(cmdID, allocationSize);
    }

    template <class StructType>
    ANGLE_INLINE StructType *commonInit(priv::CommandID cmdID, size_t allocationSize)
    {
        mCurrentBytesRemaining -= allocationSize;

        priv::CommandHeader *header = reinterpret_cast<priv::CommandHeader *>(mCurrentWritePointer);
        header->id                  = cmdID;
        header->size                = static_cast<uint16_t>(allocationSize);
        ASSERT(allocationSize <= std::numeric_limits<uint16_t>::max());

        mCurrentWritePointer += allocationSize;
        // Set next cmd header to Invalid (0) so cmd sequence will be terminated
        reinterpret_cast<priv::CommandHeader *>(mCurrentWritePointer)->id =
            priv::CommandID::Invalid;
        return priv::Offset<StructType>(header, sizeof(priv::CommandHeader));
    }

  private:
    angle::PoolAllocator *mAllocator;
    uint8_t *mCurrentWritePointer;
    size_t mCurrentBytesRemaining;

    // Points to the command header of the parent command buffer.
    std::vector<priv::CommandHeader *> *mCommandsPtr;
};

}  // namespace vk
}  // namespace rx

#endif  // ANGLE_ENABLE_VULKAN_SHARED_RING_BUFFER_CMD_ALLOC
#endif  // LIBANGLE_RENDERER_VULKAN_ALLOCATORMANAGERPOOL_H_
