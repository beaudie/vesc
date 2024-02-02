//
// Copyright 2024 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//
// RefCountedEvent:
//    Manages reference count of VkEvent and its associated functions.
//

#include "libANGLE/renderer/vulkan/RefCountedEvent.h"

namespace rx
{
namespace vk
{

void ReleaseRefcountedEvent(VkDevice device,
                            RefCountedEventAndStageMaskHandle atomicRefCountedEvent)
{
    atomicRefCountedEvent->releaseRef();
    if (!atomicRefCountedEvent->isReferenced())
    {
        atomicRefCountedEvent->get().event.destroy(device);
        SafeDelete(atomicRefCountedEvent);
    }
}

}  // namespace vk
}  // namespace rx
