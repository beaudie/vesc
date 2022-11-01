//
// Copyright 2018 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//

// android_util.h: Utilities for the using the Android platform

#ifndef COMMON_ANDROIDUTIL_H_
#define COMMON_ANDROIDUTIL_H_

#include <EGL/egl.h>
#include <EGL/eglext.h>

#include <array>
#include <cstdint>
#include <string>
#include <vector>

#include "angle_gl.h"

struct ANativeWindowBuffer;
struct AHardwareBuffer;

namespace angle
{

namespace android
{

constexpr std::array<GLenum, 3> kSupportedSizedInternalFormats = {GL_RGBA8, GL_RGB8, GL_RGB565};

ANativeWindowBuffer *ClientBufferToANativeWindowBuffer(EGLClientBuffer clientBuffer);
EGLClientBuffer AHardwareBufferToClientBuffer(const AHardwareBuffer *hardwareBuffer);
AHardwareBuffer *ClientBufferToAHardwareBuffer(EGLClientBuffer clientBuffer);

EGLClientBuffer CreateEGLClientBufferFromAHardwareBuffer(int width,
                                                         int height,
                                                         int depth,
                                                         int androidFormat,
                                                         int usage);

void GetANativeWindowBufferProperties(const ANativeWindowBuffer *buffer,
                                      int *width,
                                      int *height,
                                      int *depth,
                                      int *pixelFormat,
                                      uint64_t *usage);
GLenum NativePixelFormatToGLInternalFormat(int pixelFormat);
int GLInternalFormatToNativePixelFormat(GLenum internalFormat);

bool NativePixelFormatIsYUV(int pixelFormat);

AHardwareBuffer *ANativeWindowBufferToAHardwareBuffer(ANativeWindowBuffer *windowBuffer);

uint64_t GetAHBUsage(int eglNativeBufferUsage);

// Used to store the backtrace information, such as the stack addresses and symbols.
class UnwindedBacktraceInfo
{
  public:
    UnwindedBacktraceInfo();
    ~UnwindedBacktraceInfo();

    void clear();
    void populateBacktraceInfo(void **stackAddressBuffer, size_t stackAddressCount);
    void printBacktrace();

    [[nodiscard]] std::vector<void *> getStackAddresses() const;
    [[nodiscard]] std::vector<std::string> getStackSymbols() const;

  private:
    std::vector<void *> mStackAddresses;
    std::vector<std::string> mStackSymbols;
};

// Used to obtain the stack addresses and symbols from the device. On unsupported platforms, it
// returns an empty object.
UnwindedBacktraceInfo getBacktraceInfo();

}  // namespace android
}  // namespace angle

#endif  // COMMON_ANDROIDUTIL_H_
