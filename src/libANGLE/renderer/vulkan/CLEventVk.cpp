//
// Copyright 2021 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//
// CLEventVk.cpp: Implements the class methods for CLEventVk.

#include "libANGLE/renderer/vulkan/CLEventVk.h"
#include "libANGLE/renderer/vulkan/CLCommandQueueVk.h"

#include "libANGLE/cl_utils.h"

namespace rx
{

CLEventVk::CLEventVk(const cl::Event &event)
    : CLEventImpl(event), mStatus(isUserEvent() ? CL_SUBMITTED : CL_QUEUED)
{}

CLEventVk::~CLEventVk()
{
    if (!isUserEvent())
    {
        // Remove reference from associated command queue
        mEvent.getCommandQueue()->getImpl<CLCommandQueueVk>().removeAssociatedEvent(this);
    }
}

angle::Result CLEventVk::getCommandExecutionStatus(cl_int &executionStatus)
{
    executionStatus = mStatus;
    return angle::Result::Continue;
}

angle::Result CLEventVk::setUserEventStatus(cl_int executionStatus)
{
    // not much to do here other than storing the user supplied state. Error checking and single
    // call enforcement is responsibility of the front end.
    ASSERT(isUserEvent());
    ANGLE_TRY(setStatusAndExecuteCallback(executionStatus));
    return angle::Result::Continue;
}

angle::Result CLEventVk::setCallback(cl::Event &event, cl_int commandExecCallbackType)
{
    // Not much to do, acknowledge the presence of callback and return
    mHaveCallbacks[commandExecCallbackType] = true;

    return angle::Result::Continue;
}

angle::Result CLEventVk::getProfilingInfo(cl::ProfilingInfo name,
                                          size_t valueSize,
                                          void *value,
                                          size_t *valueSizeRet)
{
    UNIMPLEMENTED();
    ANGLE_CL_RETURN_ERROR(CL_OUT_OF_RESOURCES);
}

angle::Result CLEventVk::waitForUserEventStatus()
{
    ASSERT(isUserEvent());

    // User is responsible for setting the user-event object, we need to wait
    cl_int status = CL_QUEUED;
    ANGLE_TRY(getCommandExecutionStatus(status));
    while (status > CL_COMPLETE)
    {
        WARN() << "Still waiting for user-event (" << &mEvent
               << ") to be set! (aka clSetUserEventStatus)";
        std::this_thread::sleep_for(kUserEventStatusPollSleepDuration);
        ANGLE_TRY(getCommandExecutionStatus(status));
    }

    return angle::Result::Continue;
}

angle::Result CLEventVk::setStatusAndExecuteCallback(cl_int status)
{
    mStatus = status;

    if (mHaveCallbacks.contains(status) && mHaveCallbacks[status])
    {
        getFrontendObject().callback(status);
        mHaveCallbacks[status] = false;
    }

    return angle::Result::Continue;
}

}  // namespace rx
