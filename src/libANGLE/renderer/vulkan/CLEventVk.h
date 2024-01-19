//
// Copyright 2021 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//
// CLEventVk.h: Defines the class interface for CLEventVk, implementing CLEventImpl.

#ifndef LIBANGLE_RENDERER_VULKAN_CLEVENTVK_H_
#define LIBANGLE_RENDERER_VULKAN_CLEVENTVK_H_

#include "libANGLE/renderer/vulkan/ResourceVk.h"
#include "libANGLE/renderer/vulkan/cl_types.h"

#include "libANGLE/renderer/CLEventImpl.h"

#include "vulkan/vulkan_core.h"

namespace rx
{

class CLEventVk : public CLEventImpl, public vk::Resource
{
  public:
    CLEventVk(const cl::Event &event);
    ~CLEventVk() override;

    // Initialize the event resource with the command buffer of the associated command
    angle::Result init(bool completed);

    angle::Result getCommandExecutionStatus(cl_int &executionStatus) override;

    angle::Result setUserEventStatus(cl_int executionStatus) override;

    angle::Result setCallback(cl::Event &event, cl_int commandExecCallbackType) override;

    angle::Result getProfilingInfo(cl::ProfilingInfo name,
                                   size_t valueSize,
                                   void *value,
                                   size_t *valueSizeRet) override;

    angle::Result clientWait(bool flushCommands, uint64_t timeout, VkResult *outResult);
    angle::Result serverWait();

  private:
    cl_int mStatus;
    CLCommandQueueVk *mCommandQueueVk;
    CLContextVk *mContextVk;
};

}  // namespace rx

#endif  // LIBANGLE_RENDERER_VULKAN_CLEVENTVK_H_
