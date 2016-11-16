//
// Copyright 2016 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//
// ProgramNULL.cpp:
//    Implements the class methods for ProgramNULL.
//

#include "libANGLE/renderer/null/ProgramNULL.h"

#include "common/debug.h"

namespace rx
{

ProgramNULL::ProgramNULL(const gl::ProgramState &state) : ProgramImpl(state)
{
}

ProgramNULL::~ProgramNULL()
{
}

LinkResult ProgramNULL::load(gl::InfoLog &infoLog, gl::BinaryInputStream *stream)
{
    return true;
}

gl::Error ProgramNULL::save(gl::BinaryOutputStream *stream)
{
    return gl::NoError();
}

void ProgramNULL::setBinaryRetrievableHint(bool retrievable)
{
}

LinkResult ProgramNULL::link(const gl::ContextState &data, gl::InfoLog &infoLog)
{
    return true;
}

GLboolean ProgramNULL::validate(const gl::Caps &caps, gl::InfoLog *infoLog)
{
    return GL_TRUE;
}

void ProgramNULL::setProgramUniform1iv(GLint location, GLsizei count, const GLint *v)
{
    UNIMPLEMENTED();
}

void ProgramNULL::setProgramUniform2iv(GLint location, GLsizei count, const GLint *v)
{
    UNIMPLEMENTED();
}

void ProgramNULL::setProgramUniform3iv(GLint location, GLsizei count, const GLint *v)
{
    UNIMPLEMENTED();
}

void ProgramNULL::setProgramUniform4iv(GLint location, GLsizei count, const GLint *v)
{
    UNIMPLEMENTED();
}

void ProgramNULL::setProgramUniform1uiv(GLint location, GLsizei count, const GLuint *v)
{
    UNIMPLEMENTED();
}

void ProgramNULL::setProgramUniform2uiv(GLint location, GLsizei count, const GLuint *v)
{
    UNIMPLEMENTED();
}

void ProgramNULL::setProgramUniform3uiv(GLint location, GLsizei count, const GLuint *v)
{
    UNIMPLEMENTED();
}

void ProgramNULL::setProgramUniform4uiv(GLint location, GLsizei count, const GLuint *v)
{
    UNIMPLEMENTED();
}

void ProgramNULL::setProgramUniform1fv(GLint location, GLsizei count, const GLfloat *v)
{
    UNIMPLEMENTED();
}

void ProgramNULL::setProgramUniform2fv(GLint location, GLsizei count, const GLfloat *v)
{
    UNIMPLEMENTED();
}

void ProgramNULL::setProgramUniform3fv(GLint location, GLsizei count, const GLfloat *v)
{
    UNIMPLEMENTED();
}

void ProgramNULL::setProgramUniform4fv(GLint location, GLsizei count, const GLfloat *v)
{
    UNIMPLEMENTED();
}

void ProgramNULL::setProgramUniformMatrix2fv(GLint location,
                                             GLsizei count,
                                             GLboolean transpose,
                                             const GLfloat *value)
{
    UNIMPLEMENTED();
}

void ProgramNULL::setProgramUniformMatrix3fv(GLint location,
                                             GLsizei count,
                                             GLboolean transpose,
                                             const GLfloat *value)
{
    UNIMPLEMENTED();
}

void ProgramNULL::setProgramUniformMatrix4fv(GLint location,
                                             GLsizei count,
                                             GLboolean transpose,
                                             const GLfloat *value)
{
    UNIMPLEMENTED();
}

void ProgramNULL::setProgramUniformMatrix2x3fv(GLint location,
                                               GLsizei count,
                                               GLboolean transpose,
                                               const GLfloat *value)
{
    UNIMPLEMENTED();
}

void ProgramNULL::setProgramUniformMatrix3x2fv(GLint location,
                                               GLsizei count,
                                               GLboolean transpose,
                                               const GLfloat *value)
{
    UNIMPLEMENTED();
}

void ProgramNULL::setProgramUniformMatrix2x4fv(GLint location,
                                               GLsizei count,
                                               GLboolean transpose,
                                               const GLfloat *value)
{
    UNIMPLEMENTED();
}

void ProgramNULL::setProgramUniformMatrix4x2fv(GLint location,
                                               GLsizei count,
                                               GLboolean transpose,
                                               const GLfloat *value)
{
    UNIMPLEMENTED();
}

void ProgramNULL::setProgramUniformMatrix3x4fv(GLint location,
                                               GLsizei count,
                                               GLboolean transpose,
                                               const GLfloat *value)
{
    UNIMPLEMENTED();
}

void ProgramNULL::setProgramUniformMatrix4x3fv(GLint location,
                                               GLsizei count,
                                               GLboolean transpose,
                                               const GLfloat *value)
{
    UNIMPLEMENTED();
}

void ProgramNULL::setUniform1fv(GLint location, GLsizei count, const GLfloat *v)
{
}

void ProgramNULL::setUniform2fv(GLint location, GLsizei count, const GLfloat *v)
{
}

void ProgramNULL::setUniform3fv(GLint location, GLsizei count, const GLfloat *v)
{
}

void ProgramNULL::setUniform4fv(GLint location, GLsizei count, const GLfloat *v)
{
}

void ProgramNULL::setUniform1iv(GLint location, GLsizei count, const GLint *v)
{
}

void ProgramNULL::setUniform2iv(GLint location, GLsizei count, const GLint *v)
{
}

void ProgramNULL::setUniform3iv(GLint location, GLsizei count, const GLint *v)
{
}

void ProgramNULL::setUniform4iv(GLint location, GLsizei count, const GLint *v)
{
}

void ProgramNULL::setUniform1uiv(GLint location, GLsizei count, const GLuint *v)
{
}

void ProgramNULL::setUniform2uiv(GLint location, GLsizei count, const GLuint *v)
{
}

void ProgramNULL::setUniform3uiv(GLint location, GLsizei count, const GLuint *v)
{
}

void ProgramNULL::setUniform4uiv(GLint location, GLsizei count, const GLuint *v)
{
}

void ProgramNULL::setUniformMatrix2fv(GLint location,
                                      GLsizei count,
                                      GLboolean transpose,
                                      const GLfloat *value)
{
}

void ProgramNULL::setUniformMatrix3fv(GLint location,
                                      GLsizei count,
                                      GLboolean transpose,
                                      const GLfloat *value)
{
}

void ProgramNULL::setUniformMatrix4fv(GLint location,
                                      GLsizei count,
                                      GLboolean transpose,
                                      const GLfloat *value)
{
}

void ProgramNULL::setUniformMatrix2x3fv(GLint location,
                                        GLsizei count,
                                        GLboolean transpose,
                                        const GLfloat *value)
{
}

void ProgramNULL::setUniformMatrix3x2fv(GLint location,
                                        GLsizei count,
                                        GLboolean transpose,
                                        const GLfloat *value)
{
}

void ProgramNULL::setUniformMatrix2x4fv(GLint location,
                                        GLsizei count,
                                        GLboolean transpose,
                                        const GLfloat *value)
{
}

void ProgramNULL::setUniformMatrix4x2fv(GLint location,
                                        GLsizei count,
                                        GLboolean transpose,
                                        const GLfloat *value)
{
}

void ProgramNULL::setUniformMatrix3x4fv(GLint location,
                                        GLsizei count,
                                        GLboolean transpose,
                                        const GLfloat *value)
{
}

void ProgramNULL::setUniformMatrix4x3fv(GLint location,
                                        GLsizei count,
                                        GLboolean transpose,
                                        const GLfloat *value)
{
}

void ProgramNULL::setUniformBlockBinding(GLuint uniformBlockIndex, GLuint uniformBlockBinding)
{
}

bool ProgramNULL::getUniformBlockSize(const std::string &blockName, size_t *sizeOut) const
{
    // TODO(geofflang): Compute reasonable sizes?
    *sizeOut = 0;
    return true;
}

bool ProgramNULL::getUniformBlockMemberInfo(const std::string &memberUniformName,
                                            sh::BlockMemberInfo *memberInfoOut) const
{
    // TODO(geofflang): Compute reasonable values?
    return true;
}

void ProgramNULL::setPathFragmentInputGen(const std::string &inputName,
                                          GLenum genMode,
                                          GLint components,
                                          const GLfloat *coeffs)
{
}

}  // namespace rx
