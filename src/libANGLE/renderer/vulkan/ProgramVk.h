//
// Copyright 2016 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//
// ProgramVk.h:
//    Defines the class interface for ProgramVk, implementing ProgramImpl.
//

#ifndef LIBANGLE_RENDERER_VULKAN_PROGRAMVK_H_
#define LIBANGLE_RENDERER_VULKAN_PROGRAMVK_H_

#include <array>

#include "common/utilities.h"
#include "libANGLE/renderer/ProgramImpl.h"
#include "libANGLE/renderer/glslang_wrapper_utils.h"
#include "libANGLE/renderer/vulkan/ProgramHelperVk.h"
#include "libANGLE/renderer/vulkan/RendererVk.h"
#include "libANGLE/renderer/vulkan/TransformFeedbackVk.h"
#include "libANGLE/renderer/vulkan/vk_helpers.h"

namespace rx
{

class ContextVk;

class ProgramVk : public ProgramImpl, public ProgramHelperVk
{
  public:
    ProgramVk(const gl::ProgramState &state);
    ~ProgramVk() override;
    void destroy(const gl::Context *context) override;

    std::unique_ptr<LinkEvent> load(const gl::Context *context,
                                    gl::BinaryInputStream *stream,
                                    gl::InfoLog &infoLog) override;
    void save(const gl::Context *context, gl::BinaryOutputStream *stream) override;
    void setBinaryRetrievableHint(bool retrievable) override;
    void setSeparable(bool separable) override;

    std::unique_ptr<LinkEvent> link(const gl::Context *context,
                                    const gl::ProgramLinkedResources &resources,
                                    gl::InfoLog &infoLog) override;
    GLboolean validate(const gl::Caps &caps, gl::InfoLog *infoLog) override;

    void setUniform1fv(GLint location, GLsizei count, const GLfloat *v) override;
    void setUniform2fv(GLint location, GLsizei count, const GLfloat *v) override;
    void setUniform3fv(GLint location, GLsizei count, const GLfloat *v) override;
    void setUniform4fv(GLint location, GLsizei count, const GLfloat *v) override;
    void setUniform1iv(GLint location, GLsizei count, const GLint *v) override;
    void setUniform2iv(GLint location, GLsizei count, const GLint *v) override;
    void setUniform3iv(GLint location, GLsizei count, const GLint *v) override;
    void setUniform4iv(GLint location, GLsizei count, const GLint *v) override;
    void setUniform1uiv(GLint location, GLsizei count, const GLuint *v) override;
    void setUniform2uiv(GLint location, GLsizei count, const GLuint *v) override;
    void setUniform3uiv(GLint location, GLsizei count, const GLuint *v) override;
    void setUniform4uiv(GLint location, GLsizei count, const GLuint *v) override;
    void setUniformMatrix2fv(GLint location,
                             GLsizei count,
                             GLboolean transpose,
                             const GLfloat *value) override;
    void setUniformMatrix3fv(GLint location,
                             GLsizei count,
                             GLboolean transpose,
                             const GLfloat *value) override;
    void setUniformMatrix4fv(GLint location,
                             GLsizei count,
                             GLboolean transpose,
                             const GLfloat *value) override;
    void setUniformMatrix2x3fv(GLint location,
                               GLsizei count,
                               GLboolean transpose,
                               const GLfloat *value) override;
    void setUniformMatrix3x2fv(GLint location,
                               GLsizei count,
                               GLboolean transpose,
                               const GLfloat *value) override;
    void setUniformMatrix2x4fv(GLint location,
                               GLsizei count,
                               GLboolean transpose,
                               const GLfloat *value) override;
    void setUniformMatrix4x2fv(GLint location,
                               GLsizei count,
                               GLboolean transpose,
                               const GLfloat *value) override;
    void setUniformMatrix3x4fv(GLint location,
                               GLsizei count,
                               GLboolean transpose,
                               const GLfloat *value) override;
    void setUniformMatrix4x3fv(GLint location,
                               GLsizei count,
                               GLboolean transpose,
                               const GLfloat *value) override;

    void getUniformfv(const gl::Context *context, GLint location, GLfloat *params) const override;
    void getUniformiv(const gl::Context *context, GLint location, GLint *params) const override;
    void getUniformuiv(const gl::Context *context, GLint location, GLuint *params) const override;

    void setPathFragmentInputGen(const std::string &inputName,
                                 GLenum genMode,
                                 GLint components,
                                 const GLfloat *coeffs) override;

    // For testing only.
    void setDefaultUniformBlocksMinSizeForTesting(size_t minSize);

    // ProgramHelperVk Interface
    void updateBindingOffsets() override;
    bool hasDefaultUniforms() const override;
    bool hasTextures() const override;
    bool hasUniformBuffers() const override;
    bool hasStorageBuffers() const override;
    bool hasAtomicCounterBuffers() const override;
    bool hasImages() const override;
    bool hasTransformFeedbackOutput() const override;
    bool dirtyUniforms() const override;
    angle::Result allocateDescriptorSet(ContextVk *contextVk, uint32_t descriptorSetIndex) override;
    angle::Result allocateDescriptorSetAndGetInfo(ContextVk *contextVk,
                                                  uint32_t descriptorSetIndex,
                                                  bool *newPoolAllocatedOut) override;
    angle::Result updateUniforms(ContextVk *contextVk) override;
    angle::Result updateTexturesDescriptorSet(ContextVk *contextVk) override;
    angle::Result updateShaderResourcesDescriptorSet(ContextVk *contextVk,
                                                     vk::CommandGraphResource *recorder) override;
    angle::Result updateTransformFeedbackDescriptorSet(ContextVk *contextVk,
                                                       vk::FramebufferHelper *framebuffer) override;
    angle::Result updateDescriptorSets(ContextVk *contextVk,
                                       vk::CommandBuffer *commandBuffer) override;
    void updateDefaultUniformsDescriptorSet(ContextVk *contextVk) override;
    void updateTransformFeedbackDescriptorSetImpl(ContextVk *contextVk) override;
    void updateBuffersDescriptorSet(ContextVk *contextVk,
                                    vk::CommandGraphResource *recorder,
                                    const std::vector<gl::InterfaceBlock> &blocks,
                                    VkDescriptorType descriptorType) override;
    void updateAtomicCounterBuffersDescriptorSet(ContextVk *contextVk,
                                                 vk::CommandGraphResource *recorder) override;
    angle::Result updateImagesDescriptorSet(ContextVk *contextVk,
                                            vk::CommandGraphResource *recorder) override;

    angle::Result getGraphicsPipeline(ContextVk *contextVk,
                                      gl::PrimitiveMode mode,
                                      const vk::GraphicsPipelineDesc &desc,
                                      const gl::AttributesMask &activeAttribLocations,
                                      const vk::GraphicsPipelineDesc **descPtrOut,
                                      vk::PipelineHelper **pipelineOut) override;

    angle::Result getComputePipeline(ContextVk *contextVk,
                                     vk::PipelineAndSerial **pipelineOut) override;

    // Used in testing only.
    vk::DynamicDescriptorPool *getDynamicDescriptorPool(uint32_t poolIndex)
    {
        return &mDynamicDescriptorPools[poolIndex];
    }

  private:
    template <int cols, int rows>
    void setUniformMatrixfv(GLint location,
                            GLsizei count,
                            GLboolean transpose,
                            const GLfloat *value);

    void reset(ContextVk *contextVk);

    angle::Result initDefaultUniformBlocks(const gl::Context *glContext);
    void generateUniformLayoutMapping(gl::ShaderMap<sh::BlockLayoutMap> &layoutMap,
                                      gl::ShaderMap<size_t> &requiredBufferSize);
    void initDefaultUniformLayoutMapping(gl::ShaderMap<sh::BlockLayoutMap> &layoutMap);
    angle::Result resizeUniformBlockMemory(ContextVk *contextVk,
                                           gl::ShaderMap<size_t> &requiredBufferSize);

    template <class T>
    void getUniformImpl(GLint location, T *v, GLenum entryPointType) const;

    template <typename T>
    void setUniformImpl(GLint location, GLsizei count, const T *v, GLenum entryPointType);
    angle::Result linkImpl(const gl::Context *glContext, gl::InfoLog &infoLog);
    void linkResources(const gl::ProgramLinkedResources &resources);

    ANGLE_INLINE angle::Result initProgram(ContextVk *contextVk,
                                           bool enableLineRasterEmulation,
                                           ProgramInfo *programInfo,
                                           vk::ShaderProgramHelper **shaderProgramOut)
    {
        // Compile shaders if not already.  This is done only once regardless of specialization
        // constants.
        if (!mShaderInfo.valid())
        {
            ANGLE_TRY(mShaderInfo.initShaders(contextVk, mShaderSources, mVariableInfoMap,
                                              &mShaderInfo.getSpirvBlobs()));
        }
        ASSERT(mShaderInfo.valid());

        // Create the program pipeline.  This is done lazily and once per combination of
        // specialization constants.
        if (!programInfo->valid())
        {
            ANGLE_TRY(programInfo->initProgram(contextVk, mShaderInfo, enableLineRasterEmulation));
        }
        ASSERT(programInfo->valid());

        *shaderProgramOut = programInfo->getShaderProgram();
        return angle::Result::Continue;
    }

    ANGLE_INLINE angle::Result initGraphicsProgram(ContextVk *contextVk,
                                                   bool enableLineRasterEmulation,
                                                   vk::ShaderProgramHelper **shaderProgramOut)
    {
        ProgramInfo &programInfo =
            enableLineRasterEmulation ? mLineRasterProgramInfo : mDefaultProgramInfo;

        return initProgram(contextVk, enableLineRasterEmulation, &programInfo, shaderProgramOut);
    }

    ANGLE_INLINE angle::Result initComputeProgram(ContextVk *contextVk,
                                                  vk::ShaderProgramHelper **shaderProgramOut)
    {
        return initProgram(contextVk, false, &mDefaultProgramInfo, shaderProgramOut);
    }

    // Save and load implementation for GLES Program Binary support.
    angle::Result loadSpirvBlob(ContextVk *contextVk, gl::BinaryInputStream *stream);
    void saveSpirvBlob(gl::BinaryOutputStream *stream);

    // State for the default uniform blocks.
    struct DefaultUniformBlock final : private angle::NonCopyable
    {
        DefaultUniformBlock();
        ~DefaultUniformBlock();

        vk::DynamicBuffer storage;

        // Shadow copies of the shader uniform data.
        angle::MemoryBuffer uniformData;

        // Since the default blocks are laid out in std140, this tells us where to write on a call
        // to a setUniform method. They are arranged in uniform location order.
        std::vector<sh::BlockMemberInfo> uniformLayout;
    };

    gl::ShaderMap<DefaultUniformBlock> mDefaultUniformBlocks;
    gl::ShaderBitSet mDefaultUniformBlocksDirty;

    // We keep the translated linked shader sources and the expected location/set/binding mapping to
    // use with shader draw call compilation.
    // TODO(syoussefi): Remove when shader compilation is done at link time.
    // http://anglebug.com/3394
    gl::ShaderMap<std::string> mShaderSources;
    ShaderInterfaceVariableInfoMap mVariableInfoMap;
    // We keep the SPIR-V code to use for draw call pipeline creation.
    ShaderInfo mShaderInfo;
};

}  // namespace rx

#endif  // LIBANGLE_RENDERER_VULKAN_PROGRAMVK_H_
