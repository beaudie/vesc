//
// Copyright 2021 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//
// CLEventVk.cpp: Implements the class methods for CLEventVk.

#include "libANGLE/renderer/vulkan/CLEventVk.h"
#include "libANGLE/renderer/vulkan/CLCommandQueueVk.h"
#include "libANGLE/renderer/vulkan/CLContextVk.h"
#include "libANGLE/renderer/vulkan/RendererVk.h"

#include "libANGLE/CLCommandQueue.h"
#include "libANGLE/CLContext.h"
#include "libANGLE/CLEvent.h"
#include "libANGLE/cl_utils.h"

namespace rx
{

CLEventVk::CLEventVk(const cl::Event &event)
    : CLEventImpl(event),
      mStatus(CL_QUEUED),
      mCommandQueueVk(nullptr),
      mContextVk(&mEvent.getContext().getImpl<CLContextVk>())
{
    if (event.getCommandType() == CL_COMMAND_USER)
    {
        // User commands do not have command queue association.
        // Set to submitted here rather than queued.
        mStatus = CL_SUBMITTED;
    }
    else
    {
        mCommandQueueVk = &mEvent.getCommandQueue()->getImpl<CLCommandQueueVk>();
    }
}

CLEventVk::~CLEventVk() = default;

angle::Result CLEventVk::init(bool completed)
{
    // User Events are handled by runtime and should not be associated with
    // VkQueue
    assert(mEvent.getCommandType() != CL_COMMAND_USER);
    if (completed)
    {
        mStatus = CL_COMPLETE;
    }
    else
    {
        mCommandQueueVk->onEventInit(this);
    }
    return angle::Result::Continue;
}

angle::Result CLEventVk::getCommandExecutionStatus(cl_int &executionStatus)
{
    UNIMPLEMENTED();
    ANGLE_CL_RETURN_ERROR(CL_OUT_OF_RESOURCES);
}

angle::Result CLEventVk::setUserEventStatus(cl_int executionStatus)
{
    if (executionStatus > CL_COMPLETE || executionStatus < CL_SUCCESS)
    {
        ANGLE_CL_RETURN_ERROR(CL_INVALID_VALUE);
    }
    if (mStatus == CL_COMPLETE)
    {
        ANGLE_CL_RETURN_ERROR(CL_INVALID_OPERATION);
    }
    mStatus = CL_COMPLETE;
    return angle::Result::Continue;
}

angle::Result CLEventVk::setCallback(cl::Event &event, cl_int commandExecCallbackType)
{
    UNIMPLEMENTED();
    ANGLE_CL_RETURN_ERROR(CL_OUT_OF_RESOURCES);
}

angle::Result CLEventVk::getProfilingInfo(cl::ProfilingInfo name,
                                          size_t valueSize,
                                          void *value,
                                          size_t *valueSizeRet)
{
    UNIMPLEMENTED();
    ANGLE_CL_RETURN_ERROR(CL_OUT_OF_RESOURCES);
}

angle::Result CLEventVk::clientWait(bool flushCommands, uint64_t timeout, VkResult *outResult)
{
    UNIMPLEMENTED();
    ANGLE_CL_RETURN_ERROR(CL_OUT_OF_RESOURCES);
}

angle::Result CLEventVk::serverWait()
{
    UNIMPLEMENTED();
    ANGLE_CL_RETURN_ERROR(CL_OUT_OF_RESOURCES);
}

}  // namespace rx
