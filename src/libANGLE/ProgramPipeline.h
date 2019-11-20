//
// Copyright 2017 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//

// ProgramPipeline.h: Defines the gl::ProgramPipeline class.
// Implements GL program pipeline objects and related functionality.
// [OpenGL ES 3.1] section 7.4 page 105.

#ifndef LIBANGLE_PROGRAMPIPELINE_H_
#define LIBANGLE_PROGRAMPIPELINE_H_

#include <memory>

#include "Program.h"
#include "angletypes.h"
#include "common/angleutils.h"
#include "common/utilities.h"
#include "libANGLE/Debug.h"
#include "libANGLE/RefCountObject.h"

namespace rx
{
class GLImplFactory;
class ProgramPipelineImpl;
}  // namespace rx

namespace gl
{
class Context;
class ProgramPipeline;

class ProgramPipelineState final : angle::NonCopyable
{
  public:
    ProgramPipelineState();
    ~ProgramPipelineState();

    const std::string &getLabel() const;

    void activeShaderProgram(Program *shaderProgram);
    void useProgramStages(GLbitfield stages, Program *shaderProgram);

    const char *validateDrawStates(const State &state, const gl::Extensions &extensions);

    Program *getActiveShaderProgram() { return mActiveShaderProgram; }

    GLboolean isValid() { return mValid; }

    const Program *getShaderProgram(ShaderType shaderType) const { return mPrograms[shaderType]; }

    bool hasLinkedShaderStage(ShaderType shaderType) const
    {
        return mPrograms[shaderType] && mPrograms[shaderType]->hasLinkedShaderStage(shaderType);
    }

    bool isCompute() const { return hasLinkedShaderStage(ShaderType::Compute); }

  private:
    void useProgramStage(ShaderType shaderType, Program *shaderProgram);

    friend class ProgramPipeline;

    std::string mLabel;

    // The active shader program
    Program *mActiveShaderProgram;
    // The shader programs for each stage.
    ShaderMap<Program *> mPrograms;

    GLboolean mValid;
};

class ProgramPipeline final : public RefCountObject<ProgramPipelineID>, public LabeledObject
{
  public:
    ProgramPipeline(rx::GLImplFactory *factory, ProgramPipelineID handle);
    ~ProgramPipeline() override;

    void onDestroy(const Context *context) override;

    void setLabel(const Context *context, const std::string &label) override;
    const std::string &getLabel() const override;

    rx::ProgramPipelineImpl *getImplementation() const;

    void activeShaderProgram(Program *shaderProgram);
    void useProgramStages(GLbitfield stages, Program *shaderProgram);

    int getInfoLogLength() const;
    void getInfoLog(GLsizei bufSize, GLsizei *length, char *infoLog) const;

    const Program *getShaderProgram(ShaderType shaderType) const
    {
        return mState.mPrograms[shaderType];
    }

    bool hasLinkedShaderStage(ShaderType shaderType) const
    {
        ASSERT(shaderType != ShaderType::InvalidEnum);
        return mState.hasLinkedShaderStage(shaderType);
    }

    bool isCompute() const { return mState.isCompute(); }

    // Vertex Shader
    const AttributesMask &getActiveAttribLocationsMask() const;

    // Fragment Shader
    const gl::ActiveTextureMask &getActiveSamplersMask() const;
    const gl::ActiveTextureArray<gl::TextureType> &getActiveSamplerTypes() const;

    void validate(const Caps &caps);
    const char *validateDrawStates(const State &state, const gl::Extensions &extensions) const;
    bool validateSamplers(InfoLog *infoLog, const Caps &caps);

    bool usesShaderProgram(ShaderProgramID program) const;

    // Program pipeline dirty bits.
    enum DirtyBitType
    {
        DIRTY_BIT_VERTEX_SHADER_STAGE,
        DIRTY_BIT_FRAGMENT_SHADER_STAGE,
        DIRTY_BIT_COMPUTE_SHADER_STAGE,
        DIRTY_BIT_SHADER_STAGE_MAX = DIRTY_BIT_COMPUTE_SHADER_STAGE,

        DIRTY_BIT_COUNT = DIRTY_BIT_SHADER_STAGE_MAX,
    };

    using DirtyBits = angle::BitSet<DIRTY_BIT_COUNT>;

    angle::Result syncState(const Context *context);

  private:
    std::unique_ptr<rx::ProgramPipelineImpl> mProgramPipeline;

    ProgramPipelineState mState;

    InfoLog mInfoLog;

    DirtyBits mDirtyBits;

    // Default AttributesMask to return of there is no vertex shader in the pipeline
    AttributesMask mActiveAttribLocationsMask;
};
}  // namespace gl

#endif  // LIBANGLE_PROGRAMPIPELINE_H_
