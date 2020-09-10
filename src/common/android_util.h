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
#include <stdint.h>
#include <utility>

#include "angle_gl.h"

struct ANativeWindowBuffer;
struct AHardwareBuffer;

namespace angle
{

namespace android
{

ANativeWindowBuffer *ClientBufferToANativeWindowBuffer(EGLClientBuffer clientBuffer);
EGLClientBuffer AHardwareBufferToClientBuffer(const AHardwareBuffer *hardwareBuffer);

using FormatAndType                           = std::pair<GLenum, GLenum>;
constexpr uint32_t kSupportedFormatTypesCount = 3;
inline const FormatAndType *GetSupportedFormatTypesForNativeClientBuffer()
{
    // TODO(http://anglebug.com/4062): add GL_RGB10_A2 and GL_RGBA16F formats once this
    // issue is resolved
    // We only support these 3 GL internal formats with these 3
    // format types for creating native client buffers, for now.
    static constexpr FormatAndType kSupportedFormatTypes[kSupportedFormatTypesCount] = {
        {GL_RGB8, GL_UNSIGNED_BYTE},
        {GL_RGBA8, GL_UNSIGNED_BYTE},
        {GL_RGB565, GL_UNSIGNED_SHORT_5_6_5}};

    return kSupportedFormatTypes;
}

EGLClientBuffer CreateEGLClientBufferFromAHardwareBuffer(int width,
                                                         int height,
                                                         int depth,
                                                         int androidFormat,
                                                         int usage);

void GetANativeWindowBufferProperties(const ANativeWindowBuffer *buffer,
                                      int *width,
                                      int *height,
                                      int *depth,
                                      int *pixelFormat);
GLenum NativePixelFormatToGLInternalFormat(int pixelFormat);
int GLInternalFormatToNativePixelFormat(GLenum internalFormat);

AHardwareBuffer *ANativeWindowBufferToAHardwareBuffer(ANativeWindowBuffer *windowBuffer);

uint64_t GetAHBUsage(int eglNativeBufferUsage);

}  // namespace android
}  // namespace angle

#endif  // COMMON_ANDROIDUTIL_H_
