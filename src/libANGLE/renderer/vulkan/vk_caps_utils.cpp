//
// Copyright 2018 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//
// vk_utils:
//    Helper functions for the Vulkan Caps.
//

#include "libANGLE/renderer/vulkan/vk_caps_utils.h"
#include "libANGLE/Caps.h"

namespace rx
{
namespace vk
{
// Currently we support two shader stages in Vulkan: vertex and fragment.
const int STAGES_SUPPORTED           = 2;
const int UNIFORM_RESERVED_PER_STAGE = 1;
const int MAX_COMPONENTS_PER_VECTOR  = 4;

void GenerateCaps(const VkPhysicalDeviceProperties &physicalDeviceProperties,
                  gl::Caps *outCaps,
                  gl::TextureCapsMap * /*outTextureCaps*/,
                  gl::Extensions *outExtensions,
                  gl::Limitations * /* outLimitations */)
{
    // TODO(jmadill): Caps.
    outCaps->maxVaryingVectors = 16;

    // Enable this for simple buffer readback testing, but some functionality is missing.
    // TODO(jmadill): Support full mapBufferRange extension.
    outExtensions->mapBuffer      = true;
    outExtensions->mapBufferRange = true;

    // TODO(lucferron): Eventually remove everything above this line in this function as the caps
    // get implemented.
    // https://vulkan.lunarg.com/doc/view/1.0.30.0/linux/vkspec.chunked/ch31s02.html
    outCaps->maxElementIndex       = std::numeric_limits<GLuint>::max() - 1;
    outCaps->max3DTextureSize      = physicalDeviceProperties.limits.maxImageDimension3D;
    outCaps->max2DTextureSize      = physicalDeviceProperties.limits.maxImageDimension2D;
    outCaps->maxArrayTextureLayers = physicalDeviceProperties.limits.maxImageArrayLayers;
    outCaps->maxLODBias            = physicalDeviceProperties.limits.maxSamplerLodBias;
    outCaps->maxCubeMapTextureSize = physicalDeviceProperties.limits.maxImageDimensionCube;
    outCaps->maxRenderbufferSize   = outCaps->max2DTextureSize;
    outCaps->minAliasedPointSize   = physicalDeviceProperties.limits.pointSizeRange[0];
    outCaps->maxAliasedPointSize   = physicalDeviceProperties.limits.pointSizeRange[1];
    outCaps->minAliasedLineWidth   = physicalDeviceProperties.limits.lineWidthRange[0];
    outCaps->maxAliasedLineWidth   = physicalDeviceProperties.limits.lineWidthRange[1];
    outCaps->maxDrawBuffers =
        std::min<uint32_t>(physicalDeviceProperties.limits.maxColorAttachments,
                           physicalDeviceProperties.limits.maxFragmentOutputAttachments);
    outCaps->maxFramebufferWidth    = physicalDeviceProperties.limits.maxFramebufferWidth;
    outCaps->maxFramebufferHeight   = physicalDeviceProperties.limits.maxFramebufferHeight;
    outCaps->maxColorAttachments    = physicalDeviceProperties.limits.maxColorAttachments;
    outCaps->maxViewportWidth       = physicalDeviceProperties.limits.maxViewportDimensions[0];
    outCaps->maxViewportHeight      = physicalDeviceProperties.limits.maxViewportDimensions[1];
    outCaps->maxSampleMaskWords     = physicalDeviceProperties.limits.maxSampleMaskWords;
    outCaps->maxColorTextureSamples = physicalDeviceProperties.limits.sampledImageColorSampleCounts;
    outCaps->maxDepthTextureSamples = physicalDeviceProperties.limits.sampledImageDepthSampleCounts;
    outCaps->maxIntegerSamples = physicalDeviceProperties.limits.sampledImageIntegerSampleCounts;

    outCaps->maxVertexAttributes     = physicalDeviceProperties.limits.maxVertexInputAttributes;
    outCaps->maxVertexAttribBindings = physicalDeviceProperties.limits.maxVertexInputBindings;
    outCaps->maxVertexAttribRelativeOffset =
        physicalDeviceProperties.limits.maxVertexInputAttributeOffset;
    outCaps->maxVertexAttribStride = physicalDeviceProperties.limits.maxVertexInputBindingStride;

    outCaps->maxElementsIndices  = std::numeric_limits<GLuint>::max() - 1;
    outCaps->maxElementsVertices = std::numeric_limits<GLuint>::max() - 1;

    // Looks like all floats are IEEE according to the docs here:
    // https://www.khronos.org/registry/vulkan/specs/1.0-wsi_extensions/html/vkspec.html#spirvenv-precision-operation
    outCaps->vertexHighpFloat.setIEEEFloat();
    outCaps->vertexMediumpFloat.setIEEEFloat();
    outCaps->vertexLowpFloat.setIEEEFloat();
    outCaps->fragmentHighpFloat.setIEEEFloat();
    outCaps->fragmentMediumpFloat.setIEEEFloat();
    outCaps->fragmentLowpFloat.setIEEEFloat();

    // Can't find documentation on the int precision in Vulkan.
    outCaps->vertexHighpInt.setTwosComplementInt(32);
    outCaps->vertexMediumpInt.setTwosComplementInt(32);
    outCaps->vertexLowpInt.setTwosComplementInt(32);
    outCaps->fragmentHighpInt.setTwosComplementInt(32);
    outCaps->fragmentMediumpInt.setTwosComplementInt(32);
    outCaps->fragmentLowpInt.setTwosComplementInt(32);

    // TODO(lucferron): This is something we'll need to implement custom in the back-end.
    // Vulkan doesn't do any waiting for you, our back-end code is going to manage sync objects,
    // and we'll have to check that we've exceeded the max wait timeout. Also, this is ES 3.0 so
    // we'll defer the implementation until we tackle the next version.
    // outCaps->maxServerWaitTimeout

    // Uniforms are implemented using a uniform buffer, so the max number of uniforms we can
    // support is the max buffer range divided by the size of a single uniform (4X float).
    outCaps->maxVertexUniformVectors = physicalDeviceProperties.limits.maxUniformBufferRange /
                                       (sizeof(GLfloat) * MAX_COMPONENTS_PER_VECTOR);
    outCaps->maxVertexUniformComponents =
        outCaps->maxVertexUniformVectors * MAX_COMPONENTS_PER_VECTOR;

    // This is maxDescriptorSetUniformBuffers minus the number of uniform buffers we
    // reserve for internal variables. We reserve one per shader stage for default uniforms
    // and likely one per shader stage for ANGLE internal variables.
    outCaps->maxVertexUniformBlocks =
        physicalDeviceProperties.limits.maxDescriptorSetUniformBuffers -
        (STAGES_SUPPORTED * UNIFORM_RESERVED_PER_STAGE);

    outCaps->maxVertexOutputComponents = physicalDeviceProperties.limits.maxVertexOutputComponents;

    // we use the same bindings on each stage, so the limitation is the same combined or not.
    outCaps->maxCombinedTextureImageUnits =
        physicalDeviceProperties.limits.maxPerStageDescriptorSamplers;
    outCaps->maxTextureImageUnits = physicalDeviceProperties.limits.maxPerStageDescriptorSamplers;

    outCaps->maxVaryingVectors = physicalDeviceProperties.limits.maxVertexOutputComponents / 4;

    // Uniforms are implemented using a uniform buffer, so the max number of uniforms we can
    // support is the max buffer range divided by the size of a single uniform (4X float).
    outCaps->maxFragmentUniformVectors = physicalDeviceProperties.limits.maxUniformBufferRange /
                                         (sizeof(GLfloat) * MAX_COMPONENTS_PER_VECTOR);
    outCaps->maxFragmentUniformComponents =
        outCaps->maxFragmentUniformVectors * MAX_COMPONENTS_PER_VECTOR;
}
}  // namespace vk
}  // namespace rx
