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
#include "libANGLE/renderer/vulkan/ContextVk.h"
#include "libANGLE/renderer/vulkan/RendererVk.h"
#include "libANGLE/renderer/vulkan/TransformFeedbackVk.h"
#include "libANGLE/renderer/vulkan/vk_helpers.h"

namespace rx
{

class ShaderInfo final : angle::NonCopyable
{
  public:
    ShaderInfo();
    ~ShaderInfo();

    angle::Result initShaders(ContextVk *contextVk,
                              const gl::ShaderMap<std::string> &shaderSources,
                              bool enableLineRasterEmulation);
    void release(ContextVk *contextVk);

    ANGLE_INLINE bool valid() const
    {
        return mShaders[gl::ShaderType::Vertex].get().valid() ||
               mShaders[gl::ShaderType::Compute].get().valid();
    }

    vk::ShaderProgramHelper &getShaderProgram() { return mProgramHelper; }

    vk::RefCounted<vk::ShaderAndSerial> &getShader(gl::ShaderType shaderType)
    {
        return mShaders[shaderType];
    }

  private:
    vk::ShaderProgramHelper mProgramHelper;
    gl::ShaderMap<vk::RefCounted<vk::ShaderAndSerial>> mShaders;
};

class ProgramVk : public ProgramImpl
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

    void clearShaderSources();
    void getShaderSource(ContextVk *contextVk, const gl::ProgramLinkedResources &resources);
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

    angle::Result updateUniforms(ContextVk *contextVk,
                                 gl::ShaderType shaderType,
                                 uint32_t *outOffset,
                                 bool *anyNewBufferAllocated);

    // For testing only.
    void setDefaultUniformBlocksMinSizeForTesting(size_t minSize);

    bool hasDefaultUniforms() const { return !mState.getDefaultUniformRange().empty(); }
    bool hasTextures() const { return !mState.getSamplerBindings().empty(); }
    bool hasUniformBuffers() const { return !mState.getUniformBlocks().empty(); }
    bool hasStorageBuffers() const { return !mState.getShaderStorageBlocks().empty(); }
    bool hasAtomicCounterBuffers() const { return !mState.getAtomicCounterBuffers().empty(); }
    bool hasImages() const { return !mState.getImageBindings().empty(); }
    bool hasTransformFeedbackOutput() const
    {
        return !mState.getLinkedTransformFeedbackVaryings().empty();
    }

    bool dirtyUniforms() const { return mDefaultUniformBlocksDirty.any(); }

    ANGLE_INLINE angle::Result initComputeShader(ContextVk *contextVk,
                                                 ShaderInfo &shaderInfo,
                                                 vk::ShaderProgramHelper *shaderProgramOut)
    {
        return initShaders(contextVk, shaderInfo, false, shaderProgramOut);
    }

    ANGLE_INLINE angle::Result initGraphicsShaders(ContextVk *contextVk,
                                                   gl::PrimitiveMode mode,
                                                   ShaderInfo &shaderInfo,
                                                   bool enableLineRasterEmulation,
                                                   vk::ShaderProgramHelper *shaderProgramOut)
    {
        return initShaders(contextVk, shaderInfo, enableLineRasterEmulation, shaderProgramOut);
    }

    vk::BufferHelper *getUniformBlockCurrentBuffer(gl::ShaderType shaderType);

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
    void linkResources(const gl::ProgramLinkedResources &resources);

    angle::Result initShaders(ContextVk *contextVk,
                              ShaderInfo &shaderInfo,
                              const gl::ShaderMap<std::string> &shaderSources,
                              bool enableLineRasterEmulation,
                              vk::ShaderProgramHelper *shaderProgramOut);

    ANGLE_INLINE angle::Result initShaders(ContextVk *contextVk,
                                           ShaderInfo &shaderInfo,
                                           bool enableLineRasterEmulation,
                                           vk::ShaderProgramHelper *shaderProgramOut)
    {
        if (!shaderInfo.valid())
        {
            ANGLE_TRY(initShaders(contextVk, shaderInfo, mShaderSources, enableLineRasterEmulation,
                                  shaderProgramOut));
        }

        ASSERT(shaderInfo.valid());
        return angle::Result::Continue;
    }

    // Save and load implementation for GLES Program Binary support.
    angle::Result loadShaderSource(ContextVk *contextVk, gl::BinaryInputStream *stream);
    void saveShaderSource(gl::BinaryOutputStream *stream);

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

    // We keep the translated linked shader sources to use with shader draw call patching.
    gl::ShaderMap<std::string> mShaderSources;
};

}  // namespace rx

#endif  // LIBANGLE_RENDERER_VULKAN_PROGRAMVK_H_
