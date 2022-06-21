//
// Copyright 2022 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//
// AllocatorManagerPool:
//    Implements the pool allocator managers used in the command buffers.
//

#if !ANGLE_ENABLE_VULKAN_SHARED_RING_BUFFER_CMD_ALLOC

#    include "libANGLE/renderer/vulkan/AllocatorManagerPool.h"
#    include "libANGLE/renderer/vulkan/SecondaryCommandBuffer.h"

namespace rx
{
namespace vk
{

void SharedPoolAllocatorManager::resetAllocator(uint32_t *usedBufferCountOut)
{
    mAllocator.pop();
    mAllocator.push();

    *usedBufferCountOut = 0;
}

void PoolAllocatorManager::getMemoryUsageStats(size_t *usedMemoryOut,
                                               size_t *allocatedMemoryOut) const
{
    *allocatedMemoryOut = kBlockSize * mCommandsPtr->size();

    *usedMemoryOut = 0;
    for (const priv::CommandHeader *command : *mCommandsPtr)
    {
        const priv::CommandHeader *commandEnd = command;
        while (commandEnd->id != priv::CommandID::Invalid)
        {
            commandEnd = priv::NextCommand(commandEnd);
        }

        *usedMemoryOut += reinterpret_cast<const uint8_t *>(commandEnd) -
                          reinterpret_cast<const uint8_t *>(command) +
                          sizeof(priv::CommandHeader::id);
    }

    ASSERT(*usedMemoryOut <= *allocatedMemoryOut);
}

}  // namespace vk
}  // namespace rx

#endif  // ANGLE_ENABLE_VULKAN_SHARED_RING_BUFFER_CMD_ALLOC
