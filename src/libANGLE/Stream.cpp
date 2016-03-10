//
// Copyright (c) 2016 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//

// Stream.cpp: Implements the egl::Stream class, representing the stream
// where frames are streamed in. Implements EGLStreanKHR.

#include "libANGLE/Stream.h"

#include <platform/Platform.h>
#include <EGL/eglext.h>

#include "common/debug.h"
#include "common/mathutil.h"
#include "common/platform.h"
#include "common/utilities.h"
#include "libANGLE/Context.h"

namespace egl
{

Stream::Stream(const AttributeMap &attribs)
    : mState(EGL_STREAM_STATE_CREATED_KHR), mProducerFrame(0), mConsumerFrame(0)
{
    mConsumerLatency        = attribs.get(EGL_CONSUMER_LATENCY_USEC_KHR, 0);
    mConsumerAcquireTimeout = attribs.get(EGL_CONSUMER_ACQUIRE_TIMEOUT_USEC_KHR, 0);
}

Stream::~Stream()
{
}

void Stream::streamAttribute(EGLint attribute, EGLint value)
{
    switch (attribute)
    {
        case EGL_CONSUMER_LATENCY_USEC_KHR:
            mConsumerLatency = value;
            break;
        case EGL_CONSUMER_ACQUIRE_TIMEOUT_USEC_KHR:
            mConsumerAcquireTimeout = value;
            break;
        default:
            UNREACHABLE();
            break;
    }
}

EGLint Stream::queryStream(EGLint attribute)
{
    switch (attribute)
    {
        case EGL_STREAM_STATE_KHR:
            return mState;
        case EGL_CONSUMER_LATENCY_USEC_KHR:
            return mConsumerLatency;
        case EGL_CONSUMER_ACQUIRE_TIMEOUT_USEC_KHR:
            return mConsumerAcquireTimeout;
        default:
            UNREACHABLE();
            return 0;
    }
}

EGLuint64KHR Stream::queryStreamu64(EGLint attribute)
{
    switch (attribute)
    {
        case EGL_PRODUCER_FRAME_KHR:
            return mProducerFrame;
        case EGL_CONSUMER_FRAME_KHR:
            return mConsumerFrame;
        default:
            UNREACHABLE();
            return 0;
    }
}

EGLenum Stream::getState() const
{
    return mState;
}

}  // namespace egl