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

#include "angle_gl.h"

struct ANativeWindowBuffer;
struct AHardwareBuffer;

namespace angle
{

namespace android
{

// clang-format off
/**
 * Buffer pixel formats mirrored from Android to avoid unnecessary complications
 * when trying to keep the enums defined, but not redefined, across various build
 * systems and across various releases/branches.
 *
 * Taken from
 * https://android.googlesource.com/platform/hardware/interfaces/+/refs/heads/master/graphics/common/aidl/android/hardware/graphics/common/PixelFormat.aidl
 */
enum {
    /**
     * Corresponding formats:
     *   Android: AHARDWAREBUFFER_FORMAT_R8G8B8A8_UNORM
     *   Vulkan: VK_FORMAT_R8G8B8A8_UNORM
     *   OpenGL ES: GL_RGBA8
     */
    ANGLE_AHB_FORMAT_R8G8B8A8_UNORM           = 1,

    /**
     * 32 bits per pixel, 8 bits per channel format where alpha values are
     * ignored (always opaque).
     *
     * Corresponding formats:
     *   Android: AHARDWAREBUFFER_FORMAT_R8G8B8X8_UNORM
     *   Vulkan: VK_FORMAT_R8G8B8A8_UNORM
     *   OpenGL ES: GL_RGB8
     */
    ANGLE_AHB_FORMAT_R8G8B8X8_UNORM           = 2,

    /**
     * Corresponding formats:
     *   Android: AHARDWAREBUFFER_FORMAT_R8G8B8_UNORM
     *   Vulkan: VK_FORMAT_R8G8B8_UNORM
     *   OpenGL ES: GL_RGB8
     */
    ANGLE_AHB_FORMAT_R8G8B8_UNORM             = 3,

    /**
     * Corresponding formats:
     *   Android: AHARDWAREBUFFER_FORMAT_R5G6B5_UNORM
     *   Vulkan: VK_FORMAT_R5G6B5_UNORM_PACK16
     *   OpenGL ES: GL_RGB565
     */
    ANGLE_AHB_FORMAT_R5G6B5_UNORM             = 4,

    /**
     * Corresponding formats:
     *   Android: AHARDWAREBUFFER_FORMAT_B8G8R8A8_UNORM (deprecated)
     */
    ANGLE_AHB_FORMAT_B8G8R8A8_UNORM           = 5,

    /**
     * Corresponding formats:
     *   Android: AHARDWAREBUFFER_FORMAT_B5G5R5A1_UNORM (deprecated)
     */
    ANGLE_AHB_FORMAT_B5G5R5A1_UNORM           = 6,

    /**
     * Corresponding formats:
     *   Android: AHARDWAREBUFFER_FORMAT_B4G4R4A4_UNORM (deprecated)
     */
    ANGLE_AHB_FORMAT_B4G4R4A4_UNORM           = 7,

    /**
     * Legacy format deprecated in favor of YCBCR_420_888. NV21.
     */
    ANGLE_AHB_FORMAT_YCRCB_420_SP           = 0x11,

    /**
     * Corresponding formats:
     *   Android: AHARDWAREBUFFER_FORMAT_R16G16B16A16_FLOAT
     *   Vulkan: VK_FORMAT_R16G16B16A16_SFLOAT
     *   OpenGL ES: GL_RGBA16F
     */
    ANGLE_AHB_FORMAT_R16G16B16A16_FLOAT       = 0x16,

    /**
     * Corresponding formats:
     *   Android: AHARDWAREBUFFER_FORMAT_R10G10B10A2_UNORM
     *   Vulkan: VK_FORMAT_A2B10G10R10_UNORM_PACK32
     *   OpenGL ES: GL_RGB10_A2
     */
    ANGLE_AHB_FORMAT_R10G10B10A2_UNORM        = 0x2b,

    /**
     * An opaque binary blob format that must have height 1, with width equal to
     * the buffer size in bytes.
     *
     * Corresponding formats:
     *   Android: AHARDWAREBUFFER_FORMAT_BLOB
     */
    ANGLE_AHB_FORMAT_BLOB                     = 0x21,

    /**
     * Corresponding formats:
     *   Android: AHARDWAREBUFFER_FORMAT_D16_UNORM
     *   Vulkan: VK_FORMAT_D16_UNORM
     *   OpenGL ES: GL_DEPTH_COMPONENT16
     */
    ANGLE_AHB_FORMAT_D16_UNORM                = 0x30,

    /**
     * Corresponding formats:
     *   Android: AHARDWAREBUFFER_FORMAT_D24_UNORM
     *   Vulkan: VK_FORMAT_X8_D24_UNORM_PACK32
     *   OpenGL ES: GL_DEPTH_COMPONENT24
     */
    ANGLE_AHB_FORMAT_D24_UNORM                = 0x31,

    /**
     * Corresponding formats:
     *   Android: AHARDWAREBUFFER_FORMAT_D24_UNORM_S8_UINT
     *   Vulkan: VK_FORMAT_D24_UNORM_S8_UINT
     *   OpenGL ES: GL_DEPTH24_STENCIL8
     */
    ANGLE_AHB_FORMAT_D24_UNORM_S8_UINT        = 0x32,

    /**
     * Corresponding formats:
     *   Android: AHARDWAREBUFFER_FORMAT_D32_FLOAT
     *   Vulkan: VK_FORMAT_D32_SFLOAT
     *   OpenGL ES: GL_DEPTH_COMPONENT32F
     */
    ANGLE_AHB_FORMAT_D32_FLOAT                = 0x33,

    /**
     * Corresponding formats:
     *   Android: AHARDWAREBUFFER_FORMAT_D32_FLOAT_S8_UINT
     *   Vulkan: VK_FORMAT_D32_SFLOAT_S8_UINT
     *   OpenGL ES: GL_DEPTH32F_STENCIL8
     */
    ANGLE_AHB_FORMAT_D32_FLOAT_S8_UINT        = 0x34,

    /**
     * Corresponding formats:
     *   Android: AHARDWAREBUFFER_FORMAT_D32_FLOAT_S8_UINT
     *   Vulkan: VK_FORMAT_S8_UINT
     *   OpenGL ES: GL_STENCIL_INDEX8
     */
    ANGLE_AHB_FORMAT_S8_UINT                  = 0x35,

    /**
     * YUV 420 888 format.
     * Must have an even width and height. Can be accessed in OpenGL
     * shaders through an external sampler. Does not support mip-maps
     * cube-maps or multi-layered textures.
     *
     * Corresponding formats:
     *   Android: AHARDWAREBUFFER_FORMAT_Y8Cb8Cr8_420
     */
    ANGLE_AHB_FORMAT_Y8Cb8Cr8_420             = 0x23,

    /**
     * Corresponding formats:
     *   Android: AHARDWAREBUFFER_FORMAT_YV12
     *   Vulkan: VK_FORMAT_S8_UINT
     *   OpenGL ES: GL_STENCIL_INDEX8
     */
    ANGLE_AHB_FORMAT_YV12                     = 0x32315659,

    /**
     * Corresponding formats:
     *   Android: AHARDWAREBUFFER_FORMAT_IMPLEMENTATION_DEFINED
     *   Vulkan: VK_FORMAT_S8_UINT
     *   OpenGL ES: GL_STENCIL_INDEX8
     */
    ANGLE_AHB_FORMAT_IMPLEMENTATION_DEFINED   = 0x22,

    /**
     * Corresponding formats:
     *   Android: AHARDWAREBUFFER_FORMAT_R8_UNORM
     *   Vulkan: VK_FORMAT_R8_UNORM
     *   OpenGL ES: GL_R8
     */
    ANGLE_AHB_FORMAT_R8_UNORM   = 0x38,
};

enum {
    /**
     * The buffer will never be locked for direct CPU reads using the
     * AHardwareBuffer_lock() function. Note that reading the buffer
     * using OpenGL or Vulkan functions or memory mappings is still
     * allowed.
     */
    ANGLE_AHB_USAGE_CPU_READ_NEVER        = 0UL,
    /**
     * The buffer will sometimes be locked for direct CPU reads using
     * the AHardwareBuffer_lock() function. Note that reading the
     * buffer using OpenGL or Vulkan functions or memory mappings
     * does not require the presence of this flag.
     */
    ANGLE_AHB_USAGE_CPU_READ_RARELY       = 2UL,
    /**
     * The buffer will often be locked for direct CPU reads using
     * the AHardwareBuffer_lock() function. Note that reading the
     * buffer using OpenGL or Vulkan functions or memory mappings
     * does not require the presence of this flag.
     */
    ANGLE_AHB_USAGE_CPU_READ_OFTEN        = 3UL,

    /** CPU read value mask. */
    ANGLE_AHB_USAGE_CPU_READ_MASK         = 0xFUL,
    /**
     * The buffer will never be locked for direct CPU writes using the
     * AHardwareBuffer_lock() function. Note that writing the buffer
     * using OpenGL or Vulkan functions or memory mappings is still
     * allowed.
     */
    ANGLE_AHB_USAGE_CPU_WRITE_NEVER       = 0UL << 4,
    /**
     * The buffer will sometimes be locked for direct CPU writes using
     * the AHardwareBuffer_lock() function. Note that writing the
     * buffer using OpenGL or Vulkan functions or memory mappings
     * does not require the presence of this flag.
     */
    ANGLE_AHB_USAGE_CPU_WRITE_RARELY      = 2UL << 4,
    /**
     * The buffer will often be locked for direct CPU writes using
     * the AHardwareBuffer_lock() function. Note that writing the
     * buffer using OpenGL or Vulkan functions or memory mappings
     * does not require the presence of this flag.
     */
    ANGLE_AHB_USAGE_CPU_WRITE_OFTEN       = 3UL << 4,
    /** CPU write value mask. */
    ANGLE_AHB_USAGE_CPU_WRITE_MASK        = 0xFUL << 4,
    /** The buffer will be read from by the GPU as a texture. */
    ANGLE_AHB_USAGE_GPU_SAMPLED_IMAGE     = 1UL << 8,
    /** The buffer will be written to by the GPU as a framebuffer attachment.*/
    ANGLE_AHB_USAGE_GPU_FRAMEBUFFER       = 1UL << 9,
    /**
     * The buffer will be written to by the GPU as a framebuffer
     * attachment.
     *
     * Note that the name of this flag is somewhat misleading: it does
     * not imply that the buffer contains a color format. A buffer with
     * depth or stencil format that will be used as a framebuffer
     * attachment should also have this flag. Use the equivalent flag
     * ANGLE_AHB_USAGE_GPU_FRAMEBUFFER to avoid this confusion.
     */
    ANGLE_AHB_USAGE_GPU_COLOR_OUTPUT      = ANGLE_AHB_USAGE_GPU_FRAMEBUFFER,
    /**
     * The buffer will be used as a composer HAL overlay layer.
     *
     * This flag is currently only needed when using ASurfaceTransaction_setBuffer
     * to set a buffer. In all other cases, the framework adds this flag
     * internally to buffers that could be presented in a composer overlay.
     * ASurfaceTransaction_setBuffer is special because it uses buffers allocated
     * directly through AHardwareBuffer_allocate instead of buffers allocated
     * by the framework.
     */
    ANGLE_AHB_USAGE_COMPOSER_OVERLAY      = 1ULL << 11,
    /**
     * The buffer is protected from direct CPU access or being read by
     * non-secure hardware, such as video encoders.
     *
     * This flag is incompatible with CPU read and write flags. It is
     * mainly used when handling DRM video. Refer to the EGL extension
     * EGL_EXT_protected_content and GL extension
     * GL_EXT_protected_textures for more information on how these
     * buffers are expected to behave.
     */
    ANGLE_AHB_USAGE_PROTECTED_CONTENT     = 1UL << 14,
    /** The buffer will be read by a hardware video encoder. */
    ANGLE_AHB_USAGE_VIDEO_ENCODE          = 1UL << 16,
    /**
     * The buffer will be used for direct writes from sensors.
     * When this flag is present, the format must be AHARDWAREBUFFER_FORMAT_BLOB.
     */
    ANGLE_AHB_USAGE_SENSOR_DIRECT_DATA    = 1UL << 23,
    /**
     * The buffer will be used as a shader storage or uniform buffer object.
     * When this flag is present, the format must be AHARDWAREBUFFER_FORMAT_BLOB.
     */
    ANGLE_AHB_USAGE_GPU_DATA_BUFFER       = 1UL << 24,
    /**
     * The buffer will be used as a cube map texture.
     * When this flag is present, the buffer must have a layer count
     * that is a multiple of 6. Note that buffers with this flag must be
     * bound to OpenGL textures using the extension
     * GL_EXT_EGL_image_storage instead of GL_KHR_EGL_image.
     */
    ANGLE_AHB_USAGE_GPU_CUBE_MAP          = 1UL << 25,
    /**
     * The buffer contains a complete mipmap hierarchy.
     * Note that buffers with this flag must be bound to OpenGL textures using
     * the extension GL_EXT_EGL_image_storage instead of GL_KHR_EGL_image.
     */
    ANGLE_AHB_USAGE_GPU_MIPMAP_COMPLETE   = 1UL << 26,

    /**
     * Usage: The buffer is used for front-buffer rendering. When
     * front-buffering rendering is specified, different usages may adjust their
     * behavior as a result. For example, when used as GPU_COLOR_OUTPUT the buffer
     * will behave similar to a single-buffered window. When used with
     * COMPOSER_OVERLAY, the system will try to prioritize the buffer receiving
     * an overlay plane & avoid caching it in intermediate composition buffers.
     */
    ANGLE_AHB_USAGE_FRONT_BUFFER = 1UL << 32,
};

// clang-format on

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

}  // namespace android
}  // namespace angle

#endif  // COMMON_ANDROIDUTIL_H_
