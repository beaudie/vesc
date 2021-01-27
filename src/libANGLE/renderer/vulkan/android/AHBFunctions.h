//
// Copyright 2021 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//

#ifndef LIBANGLE_RENDERER_VULKAN_ANDROID_AHBFUNCTIONS_H_
#define LIBANGLE_RENDERER_VULKAN_ANDROID_AHBFUNCTIONS_H_

#include <android/hardware_buffer.h>

#include <memory>

namespace rx
{

class AHBFunctions
{
  public:
    AHBFunctions();
    ~AHBFunctions();

    void acquire(AHardwareBuffer *buffer) const { mAcquireFn(buffer); }

    void describe(const AHardwareBuffer *buffer, AHardwareBuffer_Desc *outDesc) const
    {
        mDescribeFn(buffer, outDesc);
    }

    void release(AHardwareBuffer *buffer) const { mReleaseFn(buffer); }

    bool valid() const { return mAcquireFn && mDescribeFn && mReleaseFn; }

  private:
    void (*mAcquireFn)(AHardwareBuffer *buffer)                                       = nullptr;
    void (*mDescribeFn)(const AHardwareBuffer *buffer, AHardwareBuffer_Desc *outDesc) = nullptr;
    void (*mReleaseFn)(AHardwareBuffer *buffer)                                       = nullptr;
};

}  // namespace rx

#endif  // LIBANGLE_RENDERER_VULKAN_ANDROID_AHBFUNCTIONS_H_
