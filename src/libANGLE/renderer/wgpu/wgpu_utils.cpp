//
// Copyright 2024 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//

#include "libANGLE/renderer/wgpu/wgpu_utils.h"

namespace webgpu
{
template <typename LargerInt>
GLint LimitToInt(const LargerInt physicalDeviceValue)
{
    static_assert(sizeof(LargerInt) >= sizeof(int32_t), "Incorrect usage of LimitToInt");

    // Limit to INT_MAX / 2 instead of INT_MAX.  If the limit is queried as float, the imprecision
    // in floating point can cause the value to exceed INT_MAX.  This trips dEQP up.
    return static_cast<GLint>(std::min(
        physicalDeviceValue, static_cast<LargerInt>(std::numeric_limits<int32_t>::max() / 2)));
}

void ensureCapsInitialized(gl::Caps &nativeCaps, const wgpu::Device &device)
{
    wgpu::SupportedLimits limitsWgpu = {};
    device.GetLimits(&limitsWgpu);

    nativeCaps.maxElementIndex       = std::numeric_limits<GLuint>::max() - 1;
    nativeCaps.max3DTextureSize      = LimitToInt(limitsWgpu.limits.maxTextureDimension3D);
    nativeCaps.max2DTextureSize      = LimitToInt(limitsWgpu.limits.maxTextureDimension2D);
    nativeCaps.maxArrayTextureLayers = LimitToInt(limitsWgpu.limits.maxTextureArrayLayers);
    nativeCaps.maxCubeMapTextureSize = LimitToInt(limitsWgpu.limits.maxTextureDimension3D);
    nativeCaps.maxRenderbufferSize   = LimitToInt(limitsWgpu.limits.maxBufferSize);

    nativeCaps.maxDrawBuffers       = LimitToInt(limitsWgpu.limits.maxColorAttachments);
    nativeCaps.maxFramebufferWidth  = LimitToInt(limitsWgpu.limits.maxBufferSize);
    nativeCaps.maxFramebufferHeight = LimitToInt(limitsWgpu.limits.maxBufferSize);
    nativeCaps.maxColorAttachments  = LimitToInt(limitsWgpu.limits.maxColorAttachments);

    nativeCaps.maxVertexAttribStride = LimitToInt(limitsWgpu.limits.maxVertexBufferArrayStride);

    nativeCaps.maxVertexAttributes = LimitToInt(limitsWgpu.limits.maxVertexAttributes);

    nativeCaps.maxTextureBufferSize = LimitToInt(limitsWgpu.limits.maxBufferSize);
}
}  // namespace webgpu

namespace wgpu_gl
{
gl::LevelIndex getLevelIndex(webgpu::LevelIndex levelWgpu, gl::LevelIndex baseLevel)
{
    return gl::LevelIndex(levelWgpu.get() + baseLevel.get());
}

gl::Extents getExtents(wgpu::Extent3D wgpuExtent)
{
    gl::Extents glExtent;
    glExtent.width  = wgpuExtent.width;
    glExtent.height = wgpuExtent.height;
    glExtent.depth  = wgpuExtent.depthOrArrayLayers;
    return glExtent;
}
}  // namespace wgpu_gl

namespace gl_wgpu
{
webgpu::LevelIndex getLevelIndex(gl::LevelIndex levelGl, gl::LevelIndex baseLevel)
{
    ASSERT(baseLevel <= levelGl);
    return webgpu::LevelIndex(levelGl.get() - baseLevel.get());
}

wgpu::Extent3D getExtent3D(const gl::Extents &glExtent)
{
    wgpu::Extent3D wgpuExtent;
    wgpuExtent.width              = glExtent.width;
    wgpuExtent.height             = glExtent.height;
    wgpuExtent.depthOrArrayLayers = glExtent.depth;
    return wgpuExtent;
}

wgpu::Origin3D getOffset3D(const gl::Offset &offsetGL)
{
    wgpu::Origin3D offsetWgpu;
    offsetWgpu.x = offsetGL.x;
    offsetWgpu.y = offsetGL.y;
    offsetWgpu.z = offsetGL.z;
    return offsetWgpu;
}

wgpu::TextureDimension getWgpuTextureDimension(gl::TextureType glTextureType)
{
    wgpu::TextureDimension dimension = {};
    switch (glTextureType)
    {
        case gl::TextureType::_2D:
        case gl::TextureType::_2DMultisample:
        case gl::TextureType::Rectangle:
        case gl::TextureType::External:
        case gl::TextureType::Buffer:
            dimension = wgpu::TextureDimension::e2D;
            break;
        case gl::TextureType::_2DArray:
        case gl::TextureType::_2DMultisampleArray:
        case gl::TextureType::_3D:
        case gl::TextureType::CubeMap:
        case gl::TextureType::CubeMapArray:
        case gl::TextureType::VideoImage:
            dimension = wgpu::TextureDimension::e3D;
            break;
        default:
            break;
    }
    return dimension;
}
}  // namespace gl_wgpu
