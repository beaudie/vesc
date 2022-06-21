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

using RenderPassCommandsRingAllocatorType = angle::SharedRingBufferAllocator;
using RingBufferAllocatorType             = angle::RingBufferAllocator;

class SharedRingAllocatorManager
{
  public:
    SharedRingAllocatorManager() : mAllocator(nullptr), mAllocSharedCP(nullptr) {}
    void resetAllocator();
    bool hasAllocatorLinks() const { return mAllocator || mAllocSharedCP; }

    void checkForRecycle(priv::SecondaryCommandBuffer *commandBuffer, bool isCommandBufferEmpty);

    bool initAndCheckIfPoolInitNeeded() { return false; }

    void attachAllocator(RenderPassCommandsRingAllocatorType *allocator);
    RenderPassCommandsRingAllocatorType *detachAllocator(bool isCommandBufferEmpty);

    RenderPassCommandsRingAllocatorType *getAllocator() { return mAllocator; }

  private:
    RenderPassCommandsRingAllocatorType *mAllocator;
    angle::SharedRingBufferCheckPoint *mAllocSharedCP;
    angle::RingBufferCheckPoint mAllocReleaseCP;
};

class RingAllocatorManager final : angle::RingBufferAllocateListener
{
  public:
    RingAllocatorManager()
        : mLastCommandBlock(nullptr), mFinishedCommandSize(0), mCommandBufferPtr(nullptr)
    {}

    static constexpr size_t kCommandHeaderSize = 4;

    void setCommands(priv::SecondaryCommandBuffer *commands) { mCommandBufferPtr = commands; }
    void resetCommands() { mCommandBufferPtr = nullptr; }

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

    void resetHandle() {}

    angle::Result initialize(RenderPassCommandsRingAllocatorType *allocator)
    {
        return angle::Result::Continue;
    }

    bool valid() const { return mAllocator.valid(); }
    bool empty() const { return getCommandSize() == 0; }

    uint8_t *updateHeaderAndAllocatorParams(size_t allocationSize)
    {
        return mAllocator.allocate(static_cast<uint32_t>(allocationSize));
    }

    bool checkSizeToAllocateNewBlock(const size_t requiredSize, uint8_t **ptrOut) { return false; }

    void checkSizeToAllocateNewBlockWithoutVariable(const size_t requiredSize, uint8_t **ptrOut) {}

    void getMemoryUsageStats(size_t *usedMemoryOut, size_t *allocatedMemoryOut) const;

    virtual void onRingBufferNewFragment() override
    {
        pushNewCommandBlock(mAllocator.getPointer());
    }

    virtual void onRingBufferFragmentEnd() override { finishLastCommandBlock(); }

    void pushNewCommandBlock(uint8_t *block);

    void terminateLastCommandBlock()
    {
        if (mLastCommandBlock)
        {
            ASSERT(mAllocator.valid());
            ASSERT(mAllocator.getPointer() >= mLastCommandBlock);
            ASSERT(mAllocator.getFragmentSize() >= kCommandHeaderSize);
            reinterpret_cast<uint16_t &>(*(mAllocator.getPointer())) = 0;
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
    void attachAllocator(RenderPassCommandsRingAllocatorType *source);
    void detachAllocator(RenderPassCommandsRingAllocatorType *destination);

  private:
    RingBufferAllocatorType mAllocator;
    uint8_t *mLastCommandBlock;
    uint32_t mFinishedCommandSize;

    // Points to the parent command buffer.
    priv::SecondaryCommandBuffer *mCommandBufferPtr;
};

}  // namespace vk
}  // namespace rx

#endif  // LIBANGLE_RENDERER_VULKAN_ALLOCATORMANAGERRING_H_
