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

#include "libANGLE/renderer/ProgramImpl.h"
#include "libANGLE/renderer/vulkan/renderervk_utils.h"

namespace rx
{

class ContextVk;

struct LinkedSampler
{
    LinkedSampler(unsigned int idx) : index(idx), set(0), binding(0) {}

    unsigned int index;
    std::vector<GLuint> elements;
    uint32_t set;
    uint32_t binding;
};

class ProgramVk : public ProgramImpl
{
  public:
    ProgramVk(ContextImpl *context, const gl::ProgramState &state);
    ~ProgramVk() override;

    LinkResult load(const ContextImpl *contextImpl,
                    gl::InfoLog &infoLog,
                    gl::BinaryInputStream *stream) override;
    gl::Error save(gl::BinaryOutputStream *stream) override;
    void setBinaryRetrievableHint(bool retrievable) override;

    LinkResult link(ContextImpl *contextImpl, gl::InfoLog &infoLog) override;
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

    // TODO: synchronize in syncState when dirty bits exist.
    void setUniformBlockBinding(GLuint uniformBlockIndex, GLuint uniformBlockBinding) override;

    // May only be called after a successful link operation.
    // Return false for inactive blocks.
    bool getUniformBlockSize(const std::string &blockName, size_t *sizeOut) const override;

    // May only be called after a successful link operation.
    // Returns false for inactive members.
    bool getUniformBlockMemberInfo(const std::string &memberUniformName,
                                   sh::BlockMemberInfo *memberInfoOut) const override;

    void setPathFragmentInputGen(const std::string &inputName,
                                 GLenum genMode,
                                 GLint components,
                                 const GLfloat *coeffs) override;

    const vk::ShaderModule &getLinkedVertexModule() const;
    const vk::ShaderModule &getLinkedFragmentModule() const;
    gl::ErrorOrResult<vk::PipelineLayout *> getPipelineLayout();
    gl::ErrorOrResult<std::vector<VkDescriptorSet> *> getDescriptorSets(const gl::State &state);

  private:
    uint32_t getSamplerSet(const gl::LinkedUniform &uniform);
    uint32_t getSamplerBinding(const gl::LinkedUniform &uniform);

    ContextVk *mContext;
    vk::ShaderModule mLinkedVertexModule;
    vk::ShaderModule mLinkedFragmentModule;
    vk::PipelineLayout mPipelineLayout;
    std::vector<LinkedSampler> mLinkedSamplers;
    std::vector<VkDescriptorSet> mDescSets;
    std::vector<VkDescriptorSetLayout> mDescSetLayouts;
    VkDescriptorPool mDescPool;
};

}  // namespace rx

#endif  // LIBANGLE_RENDERER_VULKAN_PROGRAMVK_H_
