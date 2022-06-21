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
#include "libANGLE/renderer/vulkan/CommandParamTypes.h"

#if ANGLE_ENABLE_VULKAN_SHARED_RING_BUFFER_CMD_ALLOC

namespace rx
{
namespace vk
{
namespace priv
{
class SecondaryCommandBuffer;
}  // namespace priv

using RenderPassCommandsAllocatorType = angle::SharedRingBufferAllocator;
using RingBufferAllocatorType         = angle::RingBufferAllocator;

class SharedRingAllocatorManager;
using SharedAllocatorManager = SharedRingAllocatorManager;

class RingAllocatorManager;
using CommandBufferAllocatorManager = RingAllocatorManager;

class SharedRingAllocatorManager
{
  public:
    SharedRingAllocatorManager() : mAllocator(nullptr), mAllocSharedCP(nullptr) {}
    void resetAllocator(uint32_t *usedBufferCountOut);
    bool hasAllocatorLinks() const { return mAllocator || mAllocSharedCP; }

    void checkForRecycle(priv::SecondaryCommandBuffer *commandBuffer, bool isEmpty);

    bool initialize(uint32_t *usedBufferCount) { return false; }

    void attachAllocator(RenderPassCommandsAllocatorType *allocator);
    RenderPassCommandsAllocatorType *detachAllocator(bool isCommandBufferEmpty);

    RenderPassCommandsAllocatorType *getAllocator() { return mAllocator; }

  private:
    RenderPassCommandsAllocatorType *mAllocator;
    angle::SharedRingBufferCheckPoint *mAllocSharedCP;
    angle::RingBufferCheckPoint mAllocReleaseCP;
};

class RingAllocatorManager final : angle::RingBufferAllocateListener
{
  public:
    RingAllocatorManager()
        : mLastCommandBlock(nullptr), mFinishedCommandSize(0), mCommandsPtr(nullptr)
    {}

    void setCommands(std::vector<priv::CommandHeader *> *commands) { mCommandsPtr = commands; }
    void resetCommands() { mCommandsPtr = nullptr; }

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

    angle::Result initialize(RenderPassCommandsAllocatorType *allocator)
    {
        return angle::Result::Continue;
    }

    bool valid() const { return mAllocator.valid(); }
    bool empty() const { return getCommandSize() == 0; }

    priv::CommandHeader *getHeader(priv::CommandID cmdID, size_t allocationSize);

    void getMemoryUsageStats(size_t *usedMemoryOut, size_t *allocatedMemoryOut) const;

    virtual void onRingBufferNewFragment() override
    {
        pushNewCommandBlock(mAllocator.getPointer());
    }

    virtual void onRingBufferFragmentEnd() override { finishLastCommandBlock(); }

    void pushNewCommandBlock(uint8_t *block)
    {
        mCommandsPtr->push_back(reinterpret_cast<priv::CommandHeader *>(block));
        mLastCommandBlock = block;
    }

    void terminateLastCommandBlock()
    {
        if (mLastCommandBlock)
        {
            ASSERT(mAllocator.valid());
            ASSERT(mAllocator.getPointer() >= mLastCommandBlock);
            ASSERT(mAllocator.getFragmentSize() >= sizeof(priv::CommandHeader));
            reinterpret_cast<priv::CommandHeader *>(mAllocator.getPointer())->id =
                priv::CommandID::Invalid;
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
    void attachAllocator(RenderPassCommandsAllocatorType *source);
    void detachAllocator(RenderPassCommandsAllocatorType *destination);

    template <class StructType>
    ANGLE_INLINE StructType *initCommand(priv::CommandID cmdID,
                                         size_t variableSize,
                                         uint8_t **variableDataPtr)
    {
        constexpr size_t fixedAllocationSize = sizeof(StructType) + sizeof(priv::CommandHeader);
        const size_t allocationSize          = fixedAllocationSize + variableSize;
        StructType *const result             = commonInit<StructType>(cmdID, allocationSize);
        *variableDataPtr                     = priv::Offset<uint8_t>(result, sizeof(StructType));
        return result;
    }

    template <class StructType>
    ANGLE_INLINE StructType *initCommand(priv::CommandID cmdID)
    {
        constexpr size_t paramSize =
            std::is_same<StructType, priv::EmptyParams>::value ? 0 : sizeof(StructType);
        constexpr size_t allocationSize = paramSize + sizeof(priv::CommandHeader);
        return commonInit<StructType>(cmdID, allocationSize);
    }

    template <class StructType>
    ANGLE_INLINE StructType *commonInit(priv::CommandID cmdID, size_t allocationSize)
    {
        ASSERT(allocationSize <= std::numeric_limits<uint16_t>::max());
        priv::CommandHeader *header = getHeader(cmdID, allocationSize);
        return priv::Offset<StructType>(header, sizeof(priv::CommandHeader));
    }

  private:
    RingBufferAllocatorType mAllocator;
    uint8_t *mLastCommandBlock;
    uint32_t mFinishedCommandSize;

    // Points to the command header of the parent command buffer.
    std::vector<priv::CommandHeader *> *mCommandsPtr;
};

}  // namespace vk
}  // namespace rx

#endif  // ANGLE_ENABLE_VULKAN_SHARED_RING_BUFFER_CMD_ALLOC
#endif  // LIBANGLE_RENDERER_VULKAN_ALLOCATORMANAGERRING_H_
