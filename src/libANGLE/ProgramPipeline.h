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

#include "common/angleutils.h"
#include "libANGLE/Debug.h"
#include "libANGLE/Program.h"
#include "libANGLE/ProgramExecutable.h"
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

    const ProgramExecutable &getProgramExecutable() const { return mExecutable; }
    ProgramExecutable &getProgramExecutable() { return mExecutable; }

    void activeShaderProgram(Program *shaderProgram);
    void useProgramStages(GLbitfield stages, Program *shaderProgram);

    const char *validateDrawStates(const State &state, const gl::Extensions &extensions);

    Program *getActiveShaderProgram() { return mActiveShaderProgram; }

    GLboolean isValid() { return mValid; }

    const Program *getShaderProgram(ShaderType shaderType) const { return mPrograms[shaderType]; }

    bool hasDefaultUniforms();
    bool hasTextures();
    bool hasUniformBuffers();
    bool hasStorageBuffers();
    bool hasAtomicCounterBuffers();
    bool hasImages();
    bool hasTransformFeedbackOutput();

  private:
    void useProgramStage(ShaderType shaderType, Program *shaderProgram);

    friend class ProgramPipeline;

    std::string mLabel;

    // The active shader program
    Program *mActiveShaderProgram;
    // The shader programs for each stage.
    ShaderMap<Program *> mPrograms;

    GLboolean mValid;

    GLboolean mHasBeenBound;

    ProgramExecutable mExecutable;
};

class ProgramPipeline final : public RefCountObject<ProgramPipelineID>, public LabeledObject
{
  public:
    ProgramPipeline(rx::GLImplFactory *factory, ProgramPipelineID handle);
    ~ProgramPipeline() override;

    void onDestroy(const Context *context) override;

    void setLabel(const Context *context, const std::string &label) override;
    const std::string &getLabel() const override;

    const ProgramPipelineState &getState() const { return mState; }

    const ProgramExecutable &getExecutable() const { return mState.getProgramExecutable(); }
    ProgramExecutable &getExecutable() { return mState.getProgramExecutable(); }

    rx::ProgramPipelineImpl *getImplementation() const;

    Program *getActiveShaderProgram() { return mState.getActiveShaderProgram(); }
    void activeShaderProgram(Program *shaderProgram);
    Program *getLinkedActiveShaderProgram(const Context *context)
    {
        Program *program = mState.getActiveShaderProgram();
        if (program)
        {
            program->resolveLink(context);
        }
        return program;
    }

    void useProgramStages(GLbitfield stages, Program *shaderProgram);

    void updateExecutableAttributes();
    void updateExecutableTextures();
    void updateExecutable();

    Program *getShaderProgram(ShaderType shaderType) const { return mState.mPrograms[shaderType]; }

    ProgramMergedVaryings getMergedVaryings() const;
    angle::Result link(const gl::Context *context);
    bool linkVaryings(InfoLog &infoLog) const;
    void validate(const gl::Context *context);
    const char *validateDrawStates(const State &state, const gl::Extensions &extensions) const;
    bool validateSamplers(InfoLog *infoLog, const Caps &caps);

    bool usesShaderProgram(ShaderProgramID program) const;

    bool hasAnyDirtyBit() const { return mDirtyBits.any(); }

    GLboolean isValid() { return mState.isValid(); }

    void bind() { mState.mHasBeenBound = true; }
    GLboolean hasBeenBound() { return mState.mHasBeenBound; }

    angle::Result prepareForDraw(Context *context);
    angle::Result prepareForDispatch(Context *context);

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
    void updateLinkedShaderStages();

    std::unique_ptr<rx::ProgramPipelineImpl> mProgramPipelineImpl;

    ProgramPipelineState mState;

    DirtyBits mDirtyBits;
};
}  // namespace gl

#endif  // LIBANGLE_PROGRAMPIPELINE_H_
