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

    struct ClearParameters
    {
        union
        {
            uint32_t clearValue[4];
            float clearValuef[4];
        };
        size_t offset[3];
        size_t extent[3];
    };

    struct CopyParameters
    {
        size_t srcOffset[3];
        size_t destOffset[3];
        size_t extent[3];
    };

    angle::Result ClearBuffer(const BufferVk &dest, const ClearParameters &params, vk::CommandBuffer *commandBuffer);
    angle::Result CopyBuffer(const BufferVk &dest, const BufferVk &src, const CopyParameters &params, vk::CommandBuffer *commandBuffer);
    angle::Result ClearTexture(const TextureVk &dest, const ClearParameters &params, vk::CommandBuffer *commandBuffer);
    angle::Result CopyTexture(const TextureVk &dest, const TextureVk &src, const CopyParameters &params, vk::CommandBuffer *commandBuffer);
    // TODO(syoussefi): variations with FramebufferVk, or alternatively take vk::Buffer and vk::Image and let the caller deal with that.

  private:
    // Common function that creates the pipeline for the specified function.  The possible values of `function` is defined internally as it is private to this class.
    angle::Result bindPipeline(vk::Context *context, uint32_t function, vk::CommandBuffer *commandBuffer);

    vk::BindingPointer<vk::DescriptorSetLayout> mBufferClearSetLayout;
    vk::BindingPointer<vk::DescriptorSetLayout> mBufferCopySetLayout;
    vk::BindingPointer<vk::DescriptorSetLayout> mTextureClearSetLayout;
    vk::BindingPointer<vk::DescriptorSetLayout> mTextureCopySetLayout;

    vk::BindingPointer<vk::PipelineLayout> mBufferClearPipelineLayout;
    vk::BindingPointer<vk::PipelineLayout> mBufferCopyPipelineLayout;
    vk::BindingPointer<vk::PipelineLayout> mTextureClearPipelineLayout;
    vk::BindingPointer<vk::PipelineLayout> mTextureCopyPipelineLayout;
};

}  // namespace rx

#endif  // LIBANGLE_RENDERER_VULKAN_DISPATCHUTILSGL_H_
