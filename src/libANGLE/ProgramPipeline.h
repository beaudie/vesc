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
#include "libANGLE/InfoLog.h"
#include "libANGLE/ProgramHelper.h"
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

  private:
    friend class ProgramPipeline;

    std::string mLabel;
};

class ProgramPipeline final : public RefCountObject<ProgramPipelineID>,
                              public LabeledObject,
                              public ProgramHelper
{
  public:
    ProgramPipeline(rx::GLImplFactory *factory, ProgramPipelineID handle);
    ~ProgramPipeline() override;

    void onDestroy(const Context *context) override;

    void setLabel(const Context *context, const std::string &label) override;
    const std::string &getLabel() const override;

    rx::ProgramPipelineImpl *getImplementation() const;

    // ProgramHelper Interface
    int getInfoLogLength() const override;
    void getInfoLog(GLsizei bufSize, GLsizei *length, char *infoLog) const override;

    bool hasLinkedShaderStage(ShaderType shaderType) const override;
    bool isCompute() const override;

    const AttributesMask &getActiveAttribLocationsMask() const override;

    const ActiveTextureMask &getActiveSamplersMask() const override;
    const ActiveTextureArray<gl::TextureType> &getActiveSamplerTypes() const override;
    const ActiveTextureMask &getActiveImagesMask() const override;
    SamplerFormat getSamplerFormatForTextureUnitIndex(size_t textureUnitIndex) const override;

    ProgramMergedVaryings getMergedVaryings() const override;
    angle::Result link(const gl::Context *context) override;
    bool linkVaryings(InfoLog &infoLog) const override;
    bool linkValidateGlobalNames(InfoLog &infoLog) const override;
    void validate(const Caps &caps) override;
    bool validateSamplers(InfoLog *infoLog, const Caps &caps) override;
    bool hasAnyDirtyBit() const override;

  private:
    std::unique_ptr<rx::ProgramPipelineImpl> mProgramPipeline;

    ProgramPipelineState mState;
};
}  // namespace gl

#endif  // LIBANGLE_PROGRAMPIPELINE_H_
