//
// Copyright 2017 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//

// ProgramHelper.h: Defines the interface that Program and ProgramPipeline must
// implement to help users of those objects interact with them.

#ifndef LIBANGLE_PROGRAMHELPER_H_
#define LIBANGLE_PROGRAMHELPER_H_

#include "libANGLE/Caps.h"
#include "libANGLE/InfoLog.h"
#include "libANGLE/VaryingPacking.h"
#include "libANGLE/angletypes.h"

namespace gl
{

class ProgramHelper
{
  public:
    virtual ~ProgramHelper() = 0;

    virtual int getInfoLogLength() const                                           = 0;
    virtual void getInfoLog(GLsizei bufSize, GLsizei *length, char *infoLog) const = 0;

    virtual bool hasLinkedShaderStage(ShaderType shaderType) const = 0;
    virtual bool isCompute() const                                 = 0;

    // Vertex Shader
    virtual const AttributesMask &getActiveAttribLocationsMask() const = 0;

    virtual const ActiveTextureMask &getActiveSamplersMask() const                           = 0;
    virtual const ActiveTextureArray<gl::TextureType> &getActiveSamplerTypes() const         = 0;
    virtual const ActiveTextureMask &getActiveImagesMask() const                             = 0;
    virtual SamplerFormat getSamplerFormatForTextureUnitIndex(size_t textureUnitIndex) const = 0;

    virtual ProgramMergedVaryings getMergedVaryings() const           = 0;
    virtual angle::Result link(const gl::Context *context)            = 0;
    virtual bool linkVaryings(InfoLog &infoLog) const                 = 0;
    virtual bool linkValidateGlobalNames(InfoLog &infoLog) const      = 0;
    virtual void validate(const Caps &caps)                           = 0;
    virtual bool validateSamplers(InfoLog *infoLog, const Caps &caps) = 0;
    virtual bool hasAnyDirtyBit() const                               = 0;

  protected:
    InfoLog mInfoLog;
};

}  // namespace gl

#endif  // LIBANGLE_PROGRAMHELPER_H_
