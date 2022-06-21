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
#include "common/vulkan/vk_headers.h"
#include "libANGLE/renderer/vulkan/vk_command_buffer_utils.h"
#include "libANGLE/renderer/vulkan/vk_wrapper.h"

namespace rx
{
namespace vk
{
namespace priv
{
class SecondaryCommandBuffer;
}  // namespace priv

using RenderPassCommandsPoolAllocatorType = angle::PoolAllocator;

class SharedPoolAllocatorManager
{
  public:
    SharedPoolAllocatorManager() = default;
    void resetAllocator();

    static constexpr size_t kDefaultPoolAllocatorPageSize = 16 * 1024;

    void checkForRecycle(priv::SecondaryCommandBuffer *commandBuffer, bool isCommandBufferEmpty) {}

    bool initAndCheckIfPoolInitNeeded()
    {
        mAllocator.initialize(kDefaultPoolAllocatorPageSize, 1);
        // Push a scope into the pool allocator so we can easily free and re-init on reset()
        mAllocator.push();
        return true;
    }

    // Placeholder functions for attaching and detaching the allocator.
    void attachAllocator(RenderPassCommandsPoolAllocatorType *allocator) {}
    RenderPassCommandsPoolAllocatorType *detachAllocator(bool isCommandBufferEmpty)
    {
        return nullptr;
    }

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
          mCommandBufferPtr(nullptr)
    {}

    static constexpr size_t kCommandHeaderSize = 4;
    // Pool Alloc uses 16kB pages w/ 16byte header = 16368bytes. To minimize waste
    //  using a 16368/12 = 1364. Also better perf than 1024 due to fewer block allocations
    static constexpr size_t kBlockSize = 1364;
    // Make sure block size is 4-byte aligned to avoid Android errors
    static_assert((kBlockSize % 4) == 0, "Check kBlockSize alignment");

    void setCommands(priv::SecondaryCommandBuffer *commands) { mCommandBufferPtr = commands; }
    void resetCommands() { mCommandBufferPtr = nullptr; }

    void reset(CommandBufferCommandTracker *commandBufferTracker);

    void resetHandle() { mAllocator = nullptr; }

    // Initialize the SecondaryCommandBuffer by setting the allocator it will use
    angle::Result initialize(angle::PoolAllocator *allocator);

    bool valid() const { return mAllocator != nullptr; }
    bool empty() const;

    uint8_t *updateHeaderAndAllocatorParams(size_t allocationSize)
    {
        mCurrentBytesRemaining -= allocationSize;
        uint8_t *headerPointer = mCurrentWritePointer;
        mCurrentWritePointer += allocationSize;
        // Set next cmd header to Invalid (0) so cmd sequence will be terminated
        reinterpret_cast<uint16_t &>(*mCurrentWritePointer) = 0;

        return headerPointer;
    }

    bool checkSizeToAllocateNewBlock(const size_t requiredSize, uint8_t **ptrOut)
    {
        ASSERT(*ptrOut == nullptr);
        if (mCurrentBytesRemaining < requiredSize)
        {
            // variable size command can potentially exceed default cmd allocation blockSize
            if (requiredSize <= kBlockSize)
            {
                allocateNewBlock();
            }
            else
            {
                // Make sure allocation is 4-byte aligned
                const size_t alignedSize = roundUpPow2<size_t>(requiredSize, 4);
                ASSERT((alignedSize % 4) == 0);
                allocateNewBlock(alignedSize);
            }
        }
        *ptrOut = mCurrentWritePointer;
        return true;
    }

    void checkSizeToAllocateNewBlockWithoutVariable(const size_t requiredSize, uint8_t **ptrOut)
    {
        ASSERT(*ptrOut == nullptr);
        if (mCurrentBytesRemaining < requiredSize)
        {
            ASSERT(requiredSize < kBlockSize);
            allocateNewBlock();
            *ptrOut = mCurrentWritePointer;
        }
    }

    void getMemoryUsageStats(size_t *usedMemoryOut, size_t *allocatedMemoryOut) const;

    // Placeholder function
    void terminateLastCommandBlock() {}

    // Placeholder functions for attaching and detaching the allocator.
    void attachAllocator(vk::RenderPassCommandsPoolAllocatorType *source) {}
    void detachAllocator(vk::RenderPassCommandsPoolAllocatorType *destination) {}

    void allocateNewBlock(size_t blockSize = kBlockSize);

  private:
    // Using a pool allocator per CBH to avoid threading issues that occur w/ shared allocator
    // between multiple CBHs.
    angle::PoolAllocator *mAllocator;
    uint8_t *mCurrentWritePointer;
    size_t mCurrentBytesRemaining;

    // Points to the parent command buffer.
    priv::SecondaryCommandBuffer *mCommandBufferPtr;
};

}  // namespace vk
}  // namespace rx

#endif  // LIBANGLE_RENDERER_VULKAN_ALLOCATORMANAGERPOOL_H_
