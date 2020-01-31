//
// Copyright 2017 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//

// ProgramPipeline.cpp: Implements the gl::ProgramPipeline class.
// Implements GL program pipeline objects and related functionality.
// [OpenGL ES 3.1] section 7.4 page 105.

#include "libANGLE/ProgramPipeline.h"

#include "libANGLE/angletypes.h"
#include "libANGLE/renderer/GLImplFactory.h"
#include "libANGLE/renderer/ProgramPipelineImpl.h"

namespace gl
{

ProgramPipelineState::ProgramPipelineState() : mLabel() {}

ProgramPipelineState::~ProgramPipelineState() {}

const std::string &ProgramPipelineState::getLabel() const
{
    return mLabel;
}

ProgramPipeline::ProgramPipeline(rx::GLImplFactory *factory, ProgramPipelineID handle)
    : RefCountObject(factory->generateSerial(), handle),
      mProgramPipeline(factory->createProgramPipeline(mState))
{
    ASSERT(mProgramPipeline);
}

ProgramPipeline::~ProgramPipeline()
{
    mProgramPipeline.release();
}

void ProgramPipeline::onDestroy(const Context *context) {}

void ProgramPipeline::setLabel(const Context *context, const std::string &label)
{
    mState.mLabel = label;
}

const std::string &ProgramPipeline::getLabel() const
{
    return mState.mLabel;
}

rx::ProgramPipelineImpl *ProgramPipeline::getImplementation() const
{
    return mProgramPipeline.get();
}

int ProgramPipeline::getInfoLogLength() const
{
    UNIMPLEMENTED();
    return 0;
}

void ProgramPipeline::getInfoLog(GLsizei bufSize, GLsizei *length, char *infoLog) const {}

bool ProgramPipeline::hasLinkedShaderStage(ShaderType shaderType) const
{
    UNIMPLEMENTED();
    return false;
}

bool ProgramPipeline::isCompute() const
{
    UNIMPLEMENTED();
    return false;
}

const AttributesMask &ProgramPipeline::getActiveAttribLocationsMask() const
{
    UNIMPLEMENTED();
    const static AttributesMask attributesMask;
    return attributesMask;
}

const ActiveTextureMask &ProgramPipeline::getActiveSamplersMask() const
{
    UNIMPLEMENTED();
    const static ActiveTextureMask activeTextureMask;
    return activeTextureMask;
}

const ActiveTextureArray<gl::TextureType> &ProgramPipeline::getActiveSamplerTypes() const
{
    UNIMPLEMENTED();
    const static ActiveTextureArray<gl::TextureType> activeTextureArray = {};
    return activeTextureArray;
}

const ActiveTextureMask &ProgramPipeline::getActiveImagesMask() const
{
    UNIMPLEMENTED();
    const static ActiveTextureMask activeTextureMask;
    return activeTextureMask;
}

SamplerFormat ProgramPipeline::getSamplerFormatForTextureUnitIndex(size_t textureUnitIndex) const
{
    UNIMPLEMENTED();
    return SamplerFormat::InvalidEnum;
}

ProgramMergedVaryings ProgramPipeline::getMergedVaryings() const
{
    UNIMPLEMENTED();
    const ProgramMergedVaryings programMergedVaryings;
    return programMergedVaryings;
}

angle::Result ProgramPipeline::link(const gl::Context *context)
{
    UNIMPLEMENTED();
    return angle::Result::Stop;
}

bool ProgramPipeline::linkVaryings(InfoLog &infoLog) const
{
    UNIMPLEMENTED();
    return false;
}

bool ProgramPipeline::linkValidateGlobalNames(InfoLog &infoLog) const
{
    UNIMPLEMENTED();
    return false;
}

void ProgramPipeline::validate(const Caps &caps)
{
    UNIMPLEMENTED();
}

bool ProgramPipeline::validateSamplers(InfoLog *infoLog, const Caps &caps)
{
    UNIMPLEMENTED();
    return false;
}

bool ProgramPipeline::hasAnyDirtyBit() const
{
    UNIMPLEMENTED();
    return false;
}

}  // namespace gl
