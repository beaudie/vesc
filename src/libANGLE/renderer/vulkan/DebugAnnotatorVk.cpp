//
// Copyright 2020 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//
// DebugAnnotatorVk.cpp: Vulkan helpers for adding trace annotations.
//

#include "libANGLE/renderer/vulkan/DebugAnnotatorVk.h"
#include "libANGLE/Context.h"
#include "libANGLE/renderer/vulkan/ContextVk.h"

namespace rx
{

DebugAnnotatorVk::DebugAnnotatorVk() {}

DebugAnnotatorVk::~DebugAnnotatorVk() {}

void DebugAnnotatorVk::beginEvent(void *context, const char *eventName, const char *eventMessage)
{
    // TODO(ianelliott): Do we need to be careful about threads?
    angle::LoggingAnnotator::beginEvent(context, eventName, eventMessage);
    if (context && loggingEnabledForThisThread())
    {
        ContextVk *contextVk = vk::GetImpl(static_cast<gl::Context *>(context));
        contextVk->addOglApiString(eventMessage);
    }
}

void DebugAnnotatorVk::endEvent(const char *eventName)
{
    angle::LoggingAnnotator::endEvent(eventName);
}

void DebugAnnotatorVk::setMarker(const char *markerName)
{
    angle::LoggingAnnotator::setMarker(markerName);
}

bool DebugAnnotatorVk::getStatus()
{
    if (loggingEnabledForThisThread())
    {
        return true;
    }

    return false;
}

bool DebugAnnotatorVk::loggingEnabledForThisThread() const
{
    return std::this_thread::get_id() == mAnnotationThread;
}

void DebugAnnotatorVk::initialize()
{
    mAnnotationThread = std::this_thread::get_id();
}

void DebugAnnotatorVk::release() {}

}  // namespace rx
