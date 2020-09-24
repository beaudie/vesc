//
// Copyright 2020 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//
// DebugAnnotatorVk.h: Vulkan helpers for adding trace annotations.
//

#ifndef LIBANGLE_RENDERER_VULKAN_DEBUGANNOTATORVK_H_
#define LIBANGLE_RENDERER_VULKAN_DEBUGANNOTATORVK_H_

#include "libANGLE/LoggingAnnotator.h"
#include "libANGLE/renderer/vulkan/ContextVk.h"

#include <thread>

namespace rx
{

class DebugAnnotatorVk : public angle::LoggingAnnotator
{
  public:
    DebugAnnotatorVk();
    ~DebugAnnotatorVk() override;
    // TODO: SHOULD IT BE ContextVk?
    // void initialize(gl::Context *context);
    void initialize(ContextVk *contextVk);
    void release();
    void beginEvent(const char *eventName, const char *eventMessage) override;
    void endEvent(const char *eventName) override;
    void setMarker(const char *markerName) override;
    bool getStatus() override;

  private:
    // TODO: WHAT PRIVATE METHODS/MEMBERS DO I NEED?
    bool loggingEnabledForThisThread() const;

    // Only log annotations from the thread used to initialize the debug annotator
    std::thread::id mAnnotationThread;
};

}  // namespace rx

#endif  // LIBANGLE_RENDERER_VULKAN_DEBUGANNOTATORVK_H_
