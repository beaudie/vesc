//
// Copyright 2018 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//
// DrawUtilsVk.h:
//    Defines the DrawUtilsVk class, a helper for various internal fragment-shader based utilities
//    such as image clear and copy, mipmap generation, blit, etc.
//
//    - Image clear: Not yet implemented
//    - Image copy: Not yet implemented
//    - Mipmap generation: Not yet implemented
//

#ifndef LIBANGLE_RENDERER_VULKAN_DRAWUTILSVK_H_
#define LIBANGLE_RENDERER_VULKAN_DRAWUTILSVK_H_

#include "libANGLE/renderer/vulkan/vk_cache_utils.h"
#include "libANGLE/renderer/vulkan/vk_helpers.h"
#include "libANGLE/renderer/vulkan/vk_internal_shaders_autogen.h"

namespace rx
{
class FramebufferVk;

class DrawUtilsVk : angle::NonCopyable
{
  public:
    DrawUtilsVk();
    ~DrawUtilsVk();

    void destroy(VkDevice device);

    struct ClearImageParameters
    {
        VkClearColorValue clearValue;
        VkColorComponentFlags colorMaskFlags;
        GLint renderAreaHeight;
        const gl::DrawBufferMask *alphaMask;
        const vk::RenderPassDesc *renderPassDesc;
    };

    // Note: this function takes a framebuffer instead of image, as that's the only user to avoid
    // recreating a framebuffer.  An overload taking ImageHelper can be added when necessary.
    angle::Result clearImage(ContextVk *context,
                             FramebufferVk *framebuffer,
                             const ClearImageParameters &params);

  private:
    struct ImageClearShaderParams
    {
        // Structure matching PushConstants in ImageClear.frag
        VkClearColorValue clearValue = {};
    };

    // Functions implemented by the class:
    enum class Function
    {
        ImageClear = 0,
        ImageCopy = 1,

        InvalidEnum = 1,
        EnumCount   = 1,
    };

    // Common function that creates the pipeline for the specified function, binds it and prepares
    // the draw call. The possible values of `fsFlags` comes from
    // vk::InternalShader::* defined in vk_internal_shaders_autogen.h.  It's assumed for now that
    // the vertex shaders don't have any variations.
    angle::Result setupProgramCommon(vk::Context *context,
                                     Function function,
                                     vk::RefCounted<vk::ShaderAndSerial> *vertexShader,
                                     vk::RefCounted<vk::ShaderAndSerial> *fragmentShader,
                                     vk::ShaderProgramHelper *program,
                                     const vk::GraphicsPipelineDesc &pipelineDesc,
                                     const VkDescriptorSet descriptorSet,
                                     const void *pushConstants,
                                     size_t pushConstantsSize,
                                     vk::CommandBuffer *commandBuffer);

    using GetShader = angle::Result (vk::ShaderLibrary::*)(vk::Context *,
                                                           uint32_t,
                                                           vk::RefCounted<vk::ShaderAndSerial> **);

    template <GetShader getVS, GetShader getFS, Function function, typename ShaderParams>
    angle::Result setupProgram(vk::Context *context,
                               vk::ShaderProgramHelper *program,
                               uint32_t fsFlags,
                               const vk::GraphicsPipelineDesc &pipelineDesc,
                               const VkDescriptorSet descriptorSet,
                               const ShaderParams &params,
                               vk::CommandBuffer *commandBuffer);

    // Initializes descriptor set layout, pipeline layout and descriptor pool corresponding to given
    // function, if not already initialized.  Uses setSizes to create the layout.
    angle::Result ensureResourcesInitialized(vk::Context *context,
                                             Function function,
                                             VkDescriptorPoolSize *setSizes,
                                             size_t setSizesCount,
                                             size_t pushConstantsSize);

    // Initializers corresponding to functions, calling into ensureResourcesInitialized with the
    // appropriate parameters.
    angle::Result ensureImageClearInitialized(vk::Context *context);
    angle::Result ensureImageCopyInitialized(vk::Context *context);

    angle::PackedEnumMap<Function, vk::DescriptorSetLayoutPointerArray> mDescriptorSetLayouts;
    angle::PackedEnumMap<Function, vk::BindingPointer<vk::PipelineLayout>> mPipelineLayouts;
    angle::PackedEnumMap<Function, vk::DynamicDescriptorPool> mDescriptorPools;

    vk::ShaderProgramHelper mImageClearPrograms[1];
};

}  // namespace rx

#endif  // LIBANGLE_RENDERER_VULKAN_DRAWUTILSVK_H_
