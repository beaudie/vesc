//
// Copyright 2018 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//
// DispatchUtilsVk.h:
//    Defines the DispatchUtilsVk class, a helper for various internal dispatch utilities such as
//    buffer and texture clear and copy, mip map generation, etc.
//

#ifndef LIBANGLE_RENDERER_VULKAN_DISPATCHUTILSGL_H_
#define LIBANGLE_RENDERER_VULKAN_DISPATCHUTILSGL_H_

#include "libANGLE/renderer/vulkan/vk_cache_utils.h"
#include "libANGLE/renderer/vulkan/vk_helpers.h"
#include "libANGLE/renderer/vulkan/vk_internal_shaders_autogen.h"

namespace rx
{

class BufferVk;
class FramebufferVk;
class RendererVk;
class TextureVk;

class DispatchUtilsVk : angle::NonCopyable
{
  public:
    DispatchUtilsVk();
    ~DispatchUtilsVk();

    angle::Result initialize(vk::Context *context);
    void destroy(VkDevice device);

    struct ClearParameters
    {
        VkClearColorValue clearValue;
        size_t offset[3];
        size_t extent[3];
    };

    struct CopyParameters
    {
        size_t srcOffset[3];
        size_t destOffset[3];
        size_t extent[3];
    };

    bool canClearTexture(const TextureVk &dest);
    bool canCopyTexture(const TextureVk &dest, const TextureVk &src);

    angle::Result clearBuffer(vk::Context *context, BufferVk &dest, const ClearParameters &params);
    angle::Result copyBuffer(vk::Context *context,
                             const BufferVk &dest,
                             const BufferVk &src,
                             const CopyParameters &params,
                             vk::CommandBuffer *commandBuffer);
    angle::Result clearTexture(vk::Context *context,
                               const TextureVk &dest,
                               const ClearParameters &params,
                               vk::CommandBuffer *commandBuffer);
    angle::Result copyTexture(vk::Context *context,
                              const TextureVk &dest,
                              const TextureVk &src,
                              const CopyParameters &params,
                              vk::CommandBuffer *commandBuffer);
    // TODO(syoussefi): variations with FramebufferVk, or alternatively take vk::Buffer and
    // vk::Image and let the caller deal with that.

  private:
    struct Params
    {
        // Structure matching PushConstants in DispatchUtils.comp
        uint32_t destOffset[3];
        uint32_t padding1;
        uint32_t extents[3];
        uint32_t padding2;
        uint32_t srcOffset[3];
        uint32_t padding3;
        uint32_t clearValue[4];
        float clearValuef[4];
        uint32_t padding[12];
    };
    static_assert(sizeof(Params) == 32 * sizeof(uint32_t), "Unexpected padding");

    // Common function that creates the pipeline for the specified function, binds it and prepares
    // the dispatch call. The possible values of `function` comes from
    // vk::InternalShader::DispatchUtils_comp defined in vk_internal_shaders_autogen.h
    angle::Result setupProgram(vk::Context *context,
                               uint32_t function,
                               const VkDescriptorSet &descriptorSet,
                               const Params &params,
                               vk::CommandBuffer *commandBuffer);

    // Functions implemented by the class:
    enum Function
    {
        kBufferClear  = 0,
        kBufferCopy   = 1,
        kTextureClear = 2,
        kTextureCopy  = 3,

        kFunctionCount = 4,
    };

    std::array<vk::BindingPointer<vk::DescriptorSetLayout>, kFunctionCount> mDescriptorSetLayouts;
    std::array<vk::BindingPointer<vk::PipelineLayout>, kFunctionCount> mPipelineLayouts;
    std::array<vk::DynamicDescriptorPool, kFunctionCount> mDescriptorPools;

    vk::ShaderProgramHelper mPrograms[vk::InternalShader::DispatchUtils_comp::kFlagsMask |
                                      vk::InternalShader::DispatchUtils_comp::kFunctionMask |
                                      vk::InternalShader::DispatchUtils_comp::kFormatMask |
                                      vk::InternalShader::DispatchUtils_comp::kResourceTypeMask];
};

}  // namespace rx

#endif  // LIBANGLE_RENDERER_VULKAN_DISPATCHUTILSGL_H_
