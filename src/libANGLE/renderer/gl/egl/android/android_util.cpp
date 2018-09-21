//
// Copyright (c) 2018 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//

// android_util.cpp: Utilities for the using the Android platform

#include "libANGLE/renderer/gl/egl/android/android_util.h"

namespace rx
{

// Taken from system/graphics-base-v1.0.h
// https://android.googlesource.com/platform/system/core/+/master/libsystem/include/system/graphics-base-v1.0.h
typedef enum {
    HAL_PIXEL_FORMAT_RGBA_8888              = 1,
    HAL_PIXEL_FORMAT_RGBX_8888              = 2,
    HAL_PIXEL_FORMAT_RGB_888                = 3,
    HAL_PIXEL_FORMAT_RGB_565                = 4,
    HAL_PIXEL_FORMAT_BGRA_8888              = 5,
    HAL_PIXEL_FORMAT_BGRA_4444              = 6,
    HAL_PIXEL_FORMAT_BGRA_5551              = 7,
    HAL_PIXEL_FORMAT_YCBCR_422_SP           = 16,
    HAL_PIXEL_FORMAT_YCRCB_420_SP           = 17,
    HAL_PIXEL_FORMAT_YCBCR_422_I            = 20,
    HAL_PIXEL_FORMAT_RGBA_FP16              = 22,
    HAL_PIXEL_FORMAT_RAW16                  = 32,
    HAL_PIXEL_FORMAT_BLOB                   = 33,
    HAL_PIXEL_FORMAT_IMPLEMENTATION_DEFINED = 34,
    HAL_PIXEL_FORMAT_YCBCR_420_888          = 35,
    HAL_PIXEL_FORMAT_RAW_OPAQUE             = 36,
    HAL_PIXEL_FORMAT_RAW10                  = 37,
    HAL_PIXEL_FORMAT_RAW12                  = 38,
    HAL_PIXEL_FORMAT_RGBA_1010102           = 43,
    HAL_PIXEL_FORMAT_Y8                     = 538982489,
    HAL_PIXEL_FORMAT_Y16                    = 540422489,
    HAL_PIXEL_FORMAT_YV12                   = 842094169,
} android_pixel_format_t;

// Taken from system/graphics-base-v1.1.h
// https://android.googlesource.com/platform/system/core/+/master/libsystem/include/system/graphics-base-v1.1.h
typedef enum {
    HAL_PIXEL_FORMAT_DEPTH_16            = 48,
    HAL_PIXEL_FORMAT_DEPTH_24            = 49,
    HAL_PIXEL_FORMAT_DEPTH_24_STENCIL_8  = 50,
    HAL_PIXEL_FORMAT_DEPTH_32F           = 51,
    HAL_PIXEL_FORMAT_DEPTH_32F_STENCIL_8 = 52,
    HAL_PIXEL_FORMAT_STENCIL_8           = 53,
    HAL_PIXEL_FORMAT_YCBCR_P010          = 54,
} android_pixel_format_v1_1_t;

namespace android
{
GLenum NativePixelFormatToGLInternalFormat(int pixelFormat)
{
    switch (pixelFormat)
    {
        // 1.0 formats
        case HAL_PIXEL_FORMAT_RGBA_8888:
            return GL_RGBA8;
        case HAL_PIXEL_FORMAT_RGBX_8888:
            return GL_RGB8;
        case HAL_PIXEL_FORMAT_RGB_888:
            return GL_RGB8;
        case HAL_PIXEL_FORMAT_RGB_565:
            return GL_RGB565;
        case HAL_PIXEL_FORMAT_BGRA_8888:
            return GL_BGRA8_EXT;
        case HAL_PIXEL_FORMAT_BGRA_4444:
            return GL_RGBA4;
        case HAL_PIXEL_FORMAT_BGRA_5551:
            return GL_RGB5_A1;

        // No YUV formats supported in ANGLE
        case HAL_PIXEL_FORMAT_YCBCR_422_SP:
            return GL_NONE;
        case HAL_PIXEL_FORMAT_YCRCB_420_SP:
            return GL_NONE;
        case HAL_PIXEL_FORMAT_YCBCR_422_I:
            return GL_NONE;
        case HAL_PIXEL_FORMAT_RGBA_FP16:
            return GL_RGBA16F;
        case HAL_PIXEL_FORMAT_RAW16:
            return GL_NONE;
        case HAL_PIXEL_FORMAT_BLOB:
            return GL_NONE;
        case HAL_PIXEL_FORMAT_IMPLEMENTATION_DEFINED:
            return GL_NONE;
        case HAL_PIXEL_FORMAT_YCBCR_420_888:
            return GL_NONE;
        case HAL_PIXEL_FORMAT_RAW_OPAQUE:
            return GL_NONE;
        case HAL_PIXEL_FORMAT_RAW10:
            return GL_NONE;
        case HAL_PIXEL_FORMAT_RAW12:
            return GL_NONE;
        case HAL_PIXEL_FORMAT_RGBA_1010102:
            return GL_RGB10_A2;
        case HAL_PIXEL_FORMAT_Y8:
            return GL_NONE;
        case HAL_PIXEL_FORMAT_Y16:
            return GL_NONE;
        case HAL_PIXEL_FORMAT_YV12:
            return GL_NONE;

        // 1.1 formats
        case HAL_PIXEL_FORMAT_DEPTH_16:
            return GL_DEPTH_COMPONENT16;
        case HAL_PIXEL_FORMAT_DEPTH_24:
            return GL_DEPTH_COMPONENT24;
        case HAL_PIXEL_FORMAT_DEPTH_24_STENCIL_8:
            return GL_DEPTH24_STENCIL8;
        case HAL_PIXEL_FORMAT_DEPTH_32F:
            return GL_DEPTH_COMPONENT32F;
        case HAL_PIXEL_FORMAT_DEPTH_32F_STENCIL_8:
            return GL_DEPTH32F_STENCIL8;
        case HAL_PIXEL_FORMAT_STENCIL_8:
            return GL_STENCIL_INDEX8;

        // No YUV formats supported in ANGLE
        case HAL_PIXEL_FORMAT_YCBCR_P010:
            return GL_NONE;

        default:
            return GL_NONE;
    }
}
}
}
