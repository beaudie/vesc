//
// Copyright 2016 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//
// vk_utils:
//    Helper functions for the Vulkan Caps.
//

#include "libANGLE/renderer/vulkan/vk_caps_utils.h"
namespace rx
{

namespace vk
{

void GenerateCaps(gl::Caps *outCaps,
    gl::TextureCapsMap * /*outTextureCaps*/,
    gl::Extensions *outExtensions,
    gl::Limitations * /* outLimitations */,
    const VkPhysicalDeviceProperties *vkPhysicalDeviceProperties)
{
    // TODO(jmadill): Caps.
    outCaps->maxDrawBuffers = 1;
    outCaps->maxVertexAttributes = gl::MAX_VERTEX_ATTRIBS;
    outCaps->maxVertexAttribBindings = gl::MAX_VERTEX_ATTRIB_BINDINGS;
    outCaps->maxVaryingVectors = 16;
    outCaps->maxTextureImageUnits = 1;
    outCaps->maxCombinedTextureImageUnits = 1;
    outCaps->max2DTextureSize = 1024;
    outCaps->maxFragmentUniformVectors = 8;
    outCaps->maxVertexUniformVectors = 8;
    outCaps->maxColorAttachments = 1;

    // Enable this for simple buffer readback testing, but some functionality is missing.
    // TODO(jmadill): Support full mapBufferRange extension.
    outExtensions->mapBuffer = true;
    outExtensions->mapBufferRange = true;

    // Defaults have been set temporarily. Eventually remove everything at the top.
    outCaps->maxElementIndex = std::numeric_limits<GLuint>::max() - 1;
    outCaps->max3DTextureSize = vkPhysicalDeviceProperties->limits.maxImageDimension3D;
    outCaps->max2DTextureSize = vkPhysicalDeviceProperties->limits.maxImageDimension2D;
}
}
}