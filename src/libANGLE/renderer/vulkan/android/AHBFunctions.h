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

    static std::unique_ptr<AHBFunctions> Create();

    void allocate(const AHardwareBuffer_Desc *desc, AHardwareBuffer **outBuffer) const;
    void acquire(AHardwareBuffer *buffer) const;
    void describe(const AHardwareBuffer *buffer, AHardwareBuffer_Desc *outDesc) const;
    int lock(AHardwareBuffer *buffer,
             uint64_t usage,
             int32_t fence,
             const ARect *rect,
             void **outVirtualAddress) const;
    int recvHandleFromUnixSocket(int socketFd, AHardwareBuffer **outBuffer) const;
    void release(AHardwareBuffer *buffer) const;
    int sendHandleToUnixSocket(const AHardwareBuffer *buffer, int socketFd) const;
    int unlock(AHardwareBuffer *buffer, int32_t *fence) const;

  private:
    bool initialize();

    struct FunctionTable;
    std::unique_ptr<FunctionTable> mFunctionTable;
};

}  // namespace rx

#endif  // LIBANGLE_RENDERER_VULKAN_ANDROID_AHBFUNCTIONS_H_
