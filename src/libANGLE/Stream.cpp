//
// Copyright (c) 2002-2014 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//

// Stream.cpp: Implements the egl::Stream class, representing the stream
// where frames are streamed in. Implements EGLStreanKHR.

#include "libANGLE/Stream.h"

#include <algorithm>
#include <iterator>
#include <map>
#include <sstream>
#include <vector>

#include <platform/Platform.h>
#include <EGL/eglext.h>

#include "common/debug.h"
#include "common/mathutil.h"
#include "common/platform.h"
#include "common/utilities.h"
#include "libANGLE/Context.h"
#include "libANGLE/Stream.h"
#include "libANGLE/histogram_macros.h"
#include "libANGLE/Image.h"
#include "libANGLE/Surface.h"
#include "libANGLE/renderer/DisplayImpl.h"
#include "libANGLE/renderer/ImageImpl.h"
#include "third_party/trace_event/trace_event.h"

#if defined(ANGLE_ENABLE_D3D9) || defined(ANGLE_ENABLE_D3D11)
#include "libANGLE/renderer/d3d/DisplayD3D.h"
#endif

#if defined(ANGLE_ENABLE_OPENGL)
#if defined(ANGLE_PLATFORM_WINDOWS)
#include "libANGLE/renderer/gl/wgl/DisplayWGL.h"
#elif defined(ANGLE_USE_X11)
#include "libANGLE/renderer/gl/glx/DisplayGLX.h"
#elif defined(ANGLE_PLATFORM_APPLE)
#include "libANGLE/renderer/gl/cgl/DisplayCGL.h"
#else
#error Unsupported OpenGL platform.
#endif
#endif

namespace egl
{

Stream::Stream()
    : mState(EGL_STREAM_STATE_CREATED_KHR),
      mProducerFrame(0),
      mConsumerFrame(0),
      mConsumerLatency(0)
{
}

Stream::Stream(const AttributeMap &attribs)
    : mState(EGL_STREAM_STATE_CREATED_KHR), mProducerFrame(0), mConsumerFrame(0)
{
    mConsumerLatency = attribs.get(EGL_CONSUMER_LATENCY_USEC_KHR, 0);
}

Stream::~Stream()
{
}

Error Stream::streamAttribute(EGLint attribute, EGLint value)
{
    if (mState == EGL_STREAM_STATE_DISCONNECTED_KHR)
    {
        return Error(EGL_BAD_STATE_KHR, "Bad stream state");
    }

    switch (attribute)
    {
        case EGL_CONSUMER_LATENCY_USEC_KHR:
            mConsumerLatency = value;
            break;
        default:
            UNREACHABLE();
            break;
    }

    return Error(EGL_SUCCESS);
}

EGLint Stream::queryStream(EGLint attribute)
{
    switch (attribute)
    {
        case EGL_STREAM_STATE_KHR:
            return mState;
        case EGL_CONSUMER_LATENCY_USEC_KHR:
            return mConsumerLatency;
    }

    UNREACHABLE();
    return 0;
}

EGLuint64KHR Stream::queryStreamu64(EGLint attribute)
{
    switch (attribute)
    {
        case EGL_PRODUCER_FRAME_KHR:
            return mProducerFrame;
        case EGL_CONSUMER_FRAME_KHR:
            return mConsumerFrame;
    }

    UNREACHABLE();
    return 0;
}
}