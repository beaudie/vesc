//
// Copyright 2014 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//

// ProgramImpl.h: Defines the abstract rx::ProgramImpl class.

#ifndef LIBGLESV2_RENDERER_PROGRAMIMPL_H_
#define LIBGLESV2_RENDERER_PROGRAMIMPL_H_

#include "common/angleutils.h"
#include "libGLESv2/BinaryStream.h"
#include "libGLESv2/Constants.h"
#include "libGLESv2/ProgramBinary.h"
#include "libGLESv2/Shader.h"
#include "libGLESv2/renderer/Renderer.h"

#include <map>

namespace rx
{

class Renderer;

class ProgramImpl
{
public:
    virtual ~ProgramImpl() { }

    virtual bool usesPointSize() const = 0;
    virtual int getShaderVersion() const = 0;
    virtual GLenum getTransformFeedbackBufferMode() const = 0;
    virtual std::vector<gl::LinkedVarying> &getTransformFeedbackLinkedVaryings() = 0;
    virtual sh::Attribute *getShaderAttributes() = 0;
    virtual const std::vector<gl::LinkedUniform*> &getUniforms() const = 0;
    virtual const std::vector<gl::VariableLocation> &getUniformIndices() const = 0;
    virtual const std::vector<gl::UniformBlock*> &getUniformBlocks() const = 0;
    virtual gl::LinkedUniform *getUniformByLocation(GLint location) const = 0;
    virtual GLuint getUniformBlockIndex(std::string name) const = 0;

    virtual GLint getSamplerMapping(gl::SamplerType type, unsigned int samplerIndex, const gl::Caps &caps) const = 0;
    virtual GLenum getSamplerTextureType(gl::SamplerType type, unsigned int samplerIndex) const = 0;
    virtual GLint getUsedSamplerRange(gl::SamplerType type) const = 0;
    virtual void updateSamplerMapping() = 0;
    virtual bool validateSamplers(gl::InfoLog *infoLog, const gl::Caps &caps) = 0;

    virtual GLenum getBinaryFormat() = 0;
    virtual bool load(gl::InfoLog &infoLog, gl::BinaryInputStream *stream) = 0;
    virtual bool save(gl::BinaryOutputStream *stream) = 0;

    virtual bool compileProgramExecutables(gl::InfoLog &infoLog, gl::Shader *fragmentShader, gl::Shader *vertexShader,
                                           int registers) = 0;

    virtual bool link(gl::InfoLog &infoLog, gl::Shader *fragmentShader, gl::Shader *vertexShader,
                      const std::vector<std::string> &transformFeedbackVaryings, GLenum transformFeedbackBufferMode,
                      int *registers, std::vector<gl::LinkedVarying> *linkedVaryings,
                      std::map<int, gl::VariableLocation> *outputVariables, const gl::Caps &caps) = 0;

    virtual bool linkUniforms(gl::InfoLog &infoLog, const gl::Shader &vertexShader, const gl::Shader &fragmentShader,
                              const gl::Caps &caps) = 0;
    virtual bool linkUniformBlocks(gl::InfoLog &infoLog, const gl::Shader &vertexShader, const gl::Shader &fragmentShader,
                              const gl::Caps &caps) = 0;

    virtual gl::Error applyUniforms(const std::vector<gl::LinkedUniform*> &uniforms) = 0;
    virtual gl::Error applyUniformBuffers(const std::vector<gl::Buffer*> boundBuffers, const gl::Caps &caps) = 0;
    virtual bool assignUniformBlockRegister(gl::InfoLog &infoLog, gl::UniformBlock *uniformBlock, GLenum shader,
                                            unsigned int registerIndex, const gl::Caps &caps) = 0;
    
    virtual void setUniform1fv(GLint location, GLsizei count, const GLfloat *v) = 0;
    virtual void setUniform2fv(GLint location, GLsizei count, const GLfloat *v) = 0;
    virtual void setUniform3fv(GLint location, GLsizei count, const GLfloat *v) = 0;
    virtual void setUniform4fv(GLint location, GLsizei count, const GLfloat *v) = 0;
    virtual void setUniform1iv(GLint location, GLsizei count, const GLint *v) = 0;
    virtual void setUniform2iv(GLint location, GLsizei count, const GLint *v) = 0;
    virtual void setUniform3iv(GLint location, GLsizei count, const GLint *v) = 0;
    virtual void setUniform4iv(GLint location, GLsizei count, const GLint *v) = 0;
    virtual void setUniform1uiv(GLint location, GLsizei count, const GLuint *v) = 0;
    virtual void setUniform2uiv(GLint location, GLsizei count, const GLuint *v) = 0;
    virtual void setUniform3uiv(GLint location, GLsizei count, const GLuint *v) = 0;
    virtual void setUniform4uiv(GLint location, GLsizei count, const GLuint *v) = 0;
    virtual void setUniformMatrix2fv(GLint location, GLsizei count, GLboolean transpose, const GLfloat *value) = 0;
    virtual void setUniformMatrix3fv(GLint location, GLsizei count, GLboolean transpose, const GLfloat *value) = 0;
    virtual void setUniformMatrix4fv(GLint location, GLsizei count, GLboolean transpose, const GLfloat *value) = 0;
    virtual void setUniformMatrix2x3fv(GLint location, GLsizei count, GLboolean transpose, const GLfloat *value) = 0;
    virtual void setUniformMatrix3x2fv(GLint location, GLsizei count, GLboolean transpose, const GLfloat *value) = 0;
    virtual void setUniformMatrix2x4fv(GLint location, GLsizei count, GLboolean transpose, const GLfloat *value) = 0;
    virtual void setUniformMatrix4x2fv(GLint location, GLsizei count, GLboolean transpose, const GLfloat *value) = 0;
    virtual void setUniformMatrix3x4fv(GLint location, GLsizei count, GLboolean transpose, const GLfloat *value) = 0;
    virtual void setUniformMatrix4x3fv(GLint location, GLsizei count, GLboolean transpose, const GLfloat *value) = 0;

    virtual void getUniformfv(GLint location, GLfloat *params) = 0;
    virtual void getUniformiv(GLint location, GLint *params) = 0;
    virtual void getUniformuiv(GLint location, GLuint *params) = 0;

    virtual void reset() = 0;
};

}

#endif // LIBGLESV2_RENDERER_PROGRAMIMPL_H_
