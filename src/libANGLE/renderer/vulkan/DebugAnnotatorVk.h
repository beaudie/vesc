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

namespace rx
{

class DebugAnnotatorVk : public angle::LoggingAnnotator
{
  public:
    DebugAnnotatorVk();
    ~DebugAnnotatorVk() override;
    void initialize() {}
    void release() {}
    void beginEvent(gl::Context *context, const char *eventName, const char *eventMessage) override;
    void endEvent(const char *eventName) override;
    bool getStatus() override;
};

}  // namespace rx

#endif  // LIBANGLE_RENDERER_VULKAN_DEBUGANNOTATORVK_H_
