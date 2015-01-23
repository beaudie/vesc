//
// Copyright (c) 2015 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//

// Debug.cpp: Implements the Debug class for handling debug events and markers

#include "libANGLE/Debug.h"

#include <cstdarg>

#include "common/debug.h"
#include "libANGLE/renderer/DebugImpl.h"

namespace gl
{


Debug::Debug(rx::DebugImpl *impl)
    : mImplementation(impl)
{
    ASSERT(impl);
}

Debug::~Debug()
{
    SafeDelete(mImplementation);
}

void Debug::beginEvent(const std::string &msg)
{
    mImplementation->beginEvent(msg);
}

void Debug::endEvent()
{
    mImplementation->endEvent();
}

void Debug::setMarker(const std::string &msg)
{
    mImplementation->setMarker(msg);
}

ScopedEvent::ScopedEvent(Debug *debug, const char *msg, ...)
    : mDebug(debug)
{
    ASSERT(mDebug);

    va_list vararg;
    va_start(vararg, msg);
    mDebug->beginEvent(FormatString(msg, vararg));
    va_end(vararg);
}

ScopedEvent::~ScopedEvent()
{
    mDebug->endEvent();
}

}
