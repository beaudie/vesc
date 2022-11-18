//
// Copyright 2022 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//
// AllocatorManagerRing:
//    Manages the ring buffer allocators used in the command buffers.
//

#ifndef LIBANGLE_RENDERER_VULKAN_ALLOCATORMANAGERRING_H_
#define LIBANGLE_RENDERER_VULKAN_ALLOCATORMANAGERRING_H_

#include "common/RingBufferAllocator.h"
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

using CommandsRingAllocator = angle::SharedRingBufferAllocator;
using RingBufferAllocator   = angle::RingBufferAllocator;

class SharedRingAllocatorHelper
{
  public:
    SharedRingAllocatorHelper() : mAllocator(nullptr), mAllocSharedCP(nullptr) {}
    void resetAllocator();
    bool hasAllocatorLinks() const { return mAllocator || mAllocSharedCP; }

    bool initAndCheckIfPoolInitNeeded() { return false; }

    void attachAllocator(CommandsRingAllocator *allocator);
    CommandsRingAllocator *detachAllocator(bool isCommandBufferEmpty);

    CommandsRingAllocator *getAllocator() { return mAllocator; }

  private:
    // Using a ring buffer allocator for less memory overhead (observed with enabled async queue)
    CommandsRingAllocator *mAllocator;
    angle::SharedRingBufferCheckPoint *mAllocSharedCP;
    angle::RingBufferCheckPoint mAllocReleaseCP;
};

class RingAllocatorHelper final : angle::RingBufferAllocateListener
{
  public:
    RingAllocatorHelper()
        : mLastCommandBlock(nullptr), mFinishedCommandSize(0), mCommandBuffer(nullptr)
    {}

    static constexpr size_t kCommandHeaderSize = 4;
    using CommandHeaderIDType                  = uint16_t;
    // Make sure the size of command header ID type is less than total command header size.
    static_assert(sizeof(CommandHeaderIDType) < kCommandHeaderSize, "Check size of CommandHeader");

    void setCommandBuffer(priv::SecondaryCommandBuffer *commandBuffer)
    {
        mCommandBuffer = commandBuffer;
    }
    void resetCommandBuffer() { mCommandBuffer = nullptr; }

    void reset(CommandBufferCommandTracker *commandBufferTracker)
    {
        mLastCommandBlock    = nullptr;
        mFinishedCommandSize = 0;
        if (mAllocator.valid())
        {
            mAllocator.release(mAllocator.getReleaseCheckPoint());
            pushNewCommandBlock(mAllocator.allocate(0));
        }
    }

    angle::Result initialize(CommandsRingAllocator *allocator) { return angle::Result::Continue; }

    bool valid() const { return mAllocator.valid(); }
    bool empty() const { return getCommandSize() == 0; }

    void getMemoryUsageStats(size_t *usedMemoryOut, size_t *allocatedMemoryOut) const;

    void pushNewCommandBlock(uint8_t *block);

    void terminateLastCommandBlock()
    {
        if (mLastCommandBlock)
        {
            ASSERT(mAllocator.valid());
            ASSERT(mAllocator.getPointer() >= mLastCommandBlock);
            ASSERT(mAllocator.getFragmentSize() >= kCommandHeaderSize);
            reinterpret_cast<CommandHeaderIDType &>(*(mAllocator.getPointer())) = 0;
        }
    }

    void finishLastCommandBlock()
    {
        mFinishedCommandSize = getCommandSize();
        terminateLastCommandBlock();
        mLastCommandBlock = nullptr;
    }

    // The following is used to give the size of the command buffer in bytes
    uint32_t getCommandSize() const;

    // Initialize the SecondaryCommandBuffer by setting the allocator it will use
    void attachAllocator(CommandsRingAllocator *source);
    void detachAllocator(CommandsRingAllocator *destination);

    // Functions derived from RingBufferAllocateListener
    virtual void onRingBufferNewFragment() override
    {
        pushNewCommandBlock(mAllocator.getPointer());
    }

    virtual void onRingBufferFragmentEnd() override { finishLastCommandBlock(); }

    void onNewVariableSizedCommand(const size_t requiredSize,
                                   const size_t allocationSize,
                                   bool *usesCommandHeaderSizeForOffsetOut,
                                   uint8_t **ptrOut,
                                   uint8_t **headerOut)
    {
        *usesCommandHeaderSizeForOffsetOut = false;
        *headerOut                         = updateHeaderAndAllocatorParams(allocationSize);
    }
    void onNewCommand(const size_t requiredSize,
                      const size_t allocationSize,
                      uint8_t **ptrOut,
                      uint8_t **header)
    {
        *header = updateHeaderAndAllocatorParams(allocationSize);
    }

  private:
    uint8_t *updateHeaderAndAllocatorParams(size_t allocationSize)
    {
        return mAllocator.allocate(static_cast<uint32_t>(allocationSize));
    }

    // Using a ring buffer allocator for less memory overhead (observed with enabled async queue)
    RingBufferAllocator mAllocator;
    uint8_t *mLastCommandBlock;
    uint32_t mFinishedCommandSize;

    // Points to the parent command buffer.
    priv::SecondaryCommandBuffer *mCommandBuffer;
};

}  // namespace vk
}  // namespace rx

#endif  // LIBANGLE_RENDERER_VULKAN_ALLOCATORMANAGERRING_H_
