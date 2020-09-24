//
// Copyright 2020 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//
// DebugAnnotatorVk.cpp: Vulkan helpers for adding trace annotations.
//

#include "libANGLE/renderer/vulkan/DebugAnnotatorVk.h"

namespace rx
{

DebugAnnotatorVk::DebugAnnotatorVk() {}

DebugAnnotatorVk::~DebugAnnotatorVk() {}

void DebugAnnotatorVk::beginEvent(const char *eventName, const char *eventMessage)
{
    angle::LoggingAnnotator::beginEvent(eventName, eventMessage);
    if (loggingEnabledForThisThread())
    {
        // TODO(ianelliott): WRITE THIS CODE!!!
    }
}

void DebugAnnotatorVk::endEvent(const char *eventName)
{
    angle::LoggingAnnotator::endEvent(eventName);
    if (loggingEnabledForThisThread())
    {
        // TODO(ianelliott): WRITE THIS CODE!!!
        //
        // mUserDefinedAnnotation->EndEvent();
    }
}

void DebugAnnotatorVk::setMarker(const char *markerName)
{
    angle::LoggingAnnotator::setMarker(markerName);
    if (loggingEnabledForThisThread())
    {
        // TODO(ianelliott): WRITE THIS CODE!!!
    }
}

bool DebugAnnotatorVk::getStatus()
{
    if (loggingEnabledForThisThread())
    {
        // TODO(ianelliott): WRITE THIS CODE!!!
        return false;
    }

    return false;
}

bool DebugAnnotatorVk::loggingEnabledForThisThread() const
{
    // TODO(ianelliott): WRITE THIS CODE!!!
    //
    // return mUserDefinedAnnotation != nullptr && std::this_thread::get_id() == mAnnotationThread;
    return false;
}

void DebugAnnotatorVk::initialize(ContextVk *contextVk)
{
    // TODO(ianelliott): WRITE THIS CODE!!!
    mAnnotationThread = std::this_thread::get_id();
}

void DebugAnnotatorVk::release()
{
    // TODO(ianelliott): WRITE THIS CODE!!!
    //
    // mUserDefinedAnnotation.Reset();
}

}  // namespace rx
