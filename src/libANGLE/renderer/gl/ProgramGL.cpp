//
// Copyright 2015 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//

// ProgramGL.cpp: Implements the ProgramGL class.

#include "libANGLE/renderer/gl/ProgramGL.h"

namespace rx
{

ProgramGL::ProgramGL()
{
}

ProgramGL::~ProgramGL()
{
}

bool ProgramGL::usesPointSize() const
{
    return false;
}

int ProgramGL::getShaderVersion() const
{
    return 0;
}

GLenum ProgramGL::getTransformFeedbackBufferMode() const
{
    return GL_NONE;
}

GLenum ProgramGL::getBinaryFormat()
{
    return GL_NONE;
}

LinkResult ProgramGL::load(gl::InfoLog &infoLog, gl::BinaryInputStream *stream)
{
    return LinkResult(true, gl::Error(GL_NO_ERROR));
}

gl::Error ProgramGL::save(gl::BinaryOutputStream *stream)
{
    return gl::Error(GL_NO_ERROR);
}

LinkResult ProgramGL::link(const gl::Data &data, gl::InfoLog &infoLog, gl::Shader *fragmentShader, gl::Shader *vertexShader, const std::vector<std::string> &transformFeedbackVaryings, GLenum transformFeedbackBufferMode, int *registers, std::vector<gl::LinkedVarying> *linkedVaryings, std::map<int, gl::VariableLocation> *outputVariables)
{
    return LinkResult(true, gl::Error(GL_NO_ERROR));
}

void ProgramGL::setUniform1fv(GLint location, GLsizei count, const GLfloat *v)
{
}

void ProgramGL::setUniform2fv(GLint location, GLsizei count, const GLfloat *v)
{
}

void ProgramGL::setUniform3fv(GLint location, GLsizei count, const GLfloat *v)
{
}

void ProgramGL::setUniform4fv(GLint location, GLsizei count, const GLfloat *v)
{
}

void ProgramGL::setUniform1iv(GLint location, GLsizei count, const GLint *v)
{
}

void ProgramGL::setUniform2iv(GLint location, GLsizei count, const GLint *v)
{
}

void ProgramGL::setUniform3iv(GLint location, GLsizei count, const GLint *v)
{
}

void ProgramGL::setUniform4iv(GLint location, GLsizei count, const GLint *v)
{
}

void ProgramGL::setUniform1uiv(GLint location, GLsizei count, const GLuint *v)
{
}

void ProgramGL::setUniform2uiv(GLint location, GLsizei count, const GLuint *v)
{
}

void ProgramGL::setUniform3uiv(GLint location, GLsizei count, const GLuint *v)
{
}

void ProgramGL::setUniform4uiv(GLint location, GLsizei count, const GLuint *v)
{
}

void ProgramGL::setUniformMatrix2fv(GLint location, GLsizei count, GLboolean transpose, const GLfloat *value)
{
}

void ProgramGL::setUniformMatrix3fv(GLint location, GLsizei count, GLboolean transpose, const GLfloat *value)
{
}

void ProgramGL::setUniformMatrix4fv(GLint location, GLsizei count, GLboolean transpose, const GLfloat *value)
{
}

void ProgramGL::setUniformMatrix2x3fv(GLint location, GLsizei count, GLboolean transpose, const GLfloat *value)
{
}

void ProgramGL::setUniformMatrix3x2fv(GLint location, GLsizei count, GLboolean transpose, const GLfloat *value)
{
}

void ProgramGL::setUniformMatrix2x4fv(GLint location, GLsizei count, GLboolean transpose, const GLfloat *value)
{
}

void ProgramGL::setUniformMatrix4x2fv(GLint location, GLsizei count, GLboolean transpose, const GLfloat *value)
{
}

void ProgramGL::setUniformMatrix3x4fv(GLint location, GLsizei count, GLboolean transpose, const GLfloat *value)
{
}

void ProgramGL::setUniformMatrix4x3fv(GLint location, GLsizei count, GLboolean transpose, const GLfloat *value)
{
}

void ProgramGL::getUniformfv(GLint location, GLfloat *params)
{
}

void ProgramGL::getUniformiv(GLint location, GLint *params)
{
}

void ProgramGL::getUniformuiv(GLint location, GLuint *params)
{
}

GLint ProgramGL::getSamplerMapping(gl::SamplerType type, unsigned int samplerIndex, const gl::Caps &caps) const
{
    return 0;
}

GLenum ProgramGL::getSamplerTextureType(gl::SamplerType type, unsigned int samplerIndex) const
{
    return GL_TEXTURE_2D;
}

GLint ProgramGL::getUsedSamplerRange(gl::SamplerType type) const
{
    return 0;
}

void ProgramGL::updateSamplerMapping()
{
}

bool ProgramGL::validateSamplers(gl::InfoLog *infoLog, const gl::Caps &caps)
{
    return true;
}

LinkResult ProgramGL::compileProgramExecutables(gl::InfoLog &infoLog, gl::Shader *fragmentShader,
                                                gl::Shader *vertexShader, int registers)
{
    return LinkResult(true, gl::Error(GL_NO_ERROR));
}

bool ProgramGL::linkUniforms(gl::InfoLog &infoLog, const gl::Shader &vertexShader, const gl::Shader &fragmentShader,
                             const gl::Caps &caps)
{
    return true;
}

bool ProgramGL::defineUniformBlock(gl::InfoLog &infoLog, const gl::Shader &shader,
                                   const sh::InterfaceBlock &interfaceBlock, const gl::Caps &caps)
{
    throw std::exception("The method or operation is not implemented.");
}

gl::Error ProgramGL::applyUniforms()
{
    return gl::Error(GL_NO_ERROR);
}

gl::Error ProgramGL::applyUniformBuffers(const std::vector<gl::Buffer*> boundBuffers, const gl::Caps &caps)
{
    return gl::Error(GL_NO_ERROR);
}

bool ProgramGL::assignUniformBlockRegister(gl::InfoLog &infoLog, gl::UniformBlock *uniformBlock, GLenum shader,
                                           unsigned int registerIndex, const gl::Caps &caps)
{
    return true;
}

}
