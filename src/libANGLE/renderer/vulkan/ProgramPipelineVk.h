//
// Copyright 2017 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//
// ProgramPipelineVk.h:
//    Defines the class interface for ProgramPipelineVk, implementing ProgramPipelineImpl.
//

#ifndef LIBANGLE_RENDERER_VULKAN_PROGRAMPIPELINEVK_H_
#define LIBANGLE_RENDERER_VULKAN_PROGRAMPIPELINEVK_H_

#include "libANGLE/renderer/ProgramPipelineImpl.h"

#include "common/utilities.h"
#include "libANGLE/renderer/vulkan/ContextVk.h"
#include "libANGLE/renderer/vulkan/ProgramVk.h"
#include "libANGLE/renderer/vulkan/vk_helpers.h"

namespace rx
{

class ProgramPipelineVk : public ProgramPipelineImpl
{
  public:
    ProgramPipelineVk(const gl::ProgramPipelineState &state);
    ~ProgramPipelineVk() override;

    void destroy(const gl::Context *context) override;
    void reset(ContextVk *contextVk);

    void updateShaderStages(const gl::State &glState);

    angle::Result getGraphicsPipeline(ContextVk *contextVk,
                                      gl::PrimitiveMode mode,
                                      const vk::GraphicsPipelineDesc &desc,
                                      const gl::AttributesMask &activeAttribLocations,
                                      const vk::GraphicsPipelineDesc **descPtrOut,
                                      vk::PipelineHelper **pipelineOut);

    const ProgramVk *getShaderProgram(gl::ShaderType shaderType) const
    {
        return mPrograms[shaderType];
    }

    // All Shaders
    bool dirtyUniforms();
    bool hasAtomicCounterBuffers();
    bool hasImages();
    bool hasStorageBuffers();
    bool hasTransformFeedbackOutput();
    bool hasUniformBuffers();
    angle::Result updateShaderResourcesDescriptorSet(ContextVk *contextVk,
                                                     vk::CommandGraphResource *recorder);
    angle::Result updateTransformFeedbackDescriptorSet(ContextVk *contextVk,
                                                       vk::FramebufferHelper *framebuffer);
    angle::Result updateUniforms(ContextVk *contextVk);
    angle::Result updateDescriptorSets(ContextVk *contextVk, vk::CommandBuffer *commandBuffer);
    void invalidateCurrentShaderResources(ContextVk *contextVk);

    // Vertex Shader
    unsigned int getMaxActiveAttribLocation() const;
    const gl::AttributesMask &getActiveAttribLocationsMask() const;
    const gl::AttributesMask &getNonBuiltinAttribLocationsMask() const;

    // Fragment Shader
    bool hasTextures();
    angle::Result updateTexturesDescriptorSet(ContextVk *contextVk);

    GLboolean validate(const gl::Caps &caps, gl::InfoLog *infoLog);

  private:
    // The shader programs for each stage.
    gl::ShaderMap<ProgramVk *> mPrograms;

    vk::ShaderProgramHelper mProgramHelper;
};

}  // namespace rx

#endif  // LIBANGLE_RENDERER_VULKAN_PROGRAMPIPELINEVK_H_
