//
// Copyright 2018 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//
// DispatchUtilsVk.h:
//    Defines the DispatchUtilsVk class, a helper for various internal dispatch utilities such as
//    buffer clear and copy, texture mip map generation, etc.
//

#ifndef LIBANGLE_RENDERER_VULKAN_DISPATCHUTILSVK_H_
#define LIBANGLE_RENDERER_VULKAN_DISPATCHUTILSVK_H_

#include "libANGLE/renderer/vulkan/vk_cache_utils.h"
#include "libANGLE/renderer/vulkan/vk_helpers.h"
#include "libANGLE/renderer/vulkan/vk_internal_shaders_autogen.h"

namespace rx
{

class BufferVk;
class RendererVk;

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
        size_t offset;
        size_t size;
    };

    struct CopyParameters
    {
        size_t destOffset;
        size_t srcOffset;
        size_t size;
    };

    angle::Result clearBuffer(vk::Context *context,
                              vk::BufferHelper *dest,
                              const ClearParameters &params);
    angle::Result copyBuffer(vk::Context *context,
                             vk::BufferHelper *dest,
                             vk::BufferHelper *src,
                             const CopyParameters &params);

  private:
    struct ShaderParams
    {
        // Structure matching PushConstants in BufferUtils.comp
        uint32_t destOffset;
        uint32_t size;
        uint32_t srcOffset;
        uint32_t padding;
        VkClearColorValue clearValue;
    };

    // Common function that creates the pipeline for the specified function, binds it and prepares
    // the dispatch call. The possible values of `function` comes from
    // vk::InternalShader::BufferUtils_comp defined in vk_internal_shaders_autogen.h
    angle::Result setupProgram(vk::Context *context,
                               uint32_t function,
                               const VkDescriptorSet &descriptorSet,
                               const ShaderParams &params,
                               vk::CommandBuffer *commandBuffer);

    // Functions implemented by the class:
    enum Function
    {
        kBufferClear = 0,
        kBufferCopy  = 1,

        kFunctionCount = 2,
    };

    std::array<vk::BindingPointer<vk::DescriptorSetLayout>, kFunctionCount> mDescriptorSetLayouts;
    std::array<vk::BindingPointer<vk::PipelineLayout>, kFunctionCount> mPipelineLayouts;
    std::array<vk::DynamicDescriptorPool, kFunctionCount> mDescriptorPools;

    vk::ShaderProgramHelper mPrograms[vk::InternalShader::BufferUtils_comp::kFlagsMask |
                                      vk::InternalShader::BufferUtils_comp::kFunctionMask |
                                      vk::InternalShader::BufferUtils_comp::kFormatMask];
};

}  // namespace rx

#endif  // LIBANGLE_RENDERER_VULKAN_DISPATCHUTILSVK_H_
