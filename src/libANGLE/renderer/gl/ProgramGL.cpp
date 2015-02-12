//
// Copyright 2015 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//

// ProgramGL.cpp: Implements the class methods for ProgramGL.

#include "libANGLE/renderer/gl/ProgramGL.h"

#include "common/debug.h"
#include "libANGLE/renderer/gl/FunctionsGL.h"
#include "libANGLE/renderer/gl/RendererGL.h"
#include "libANGLE/renderer/gl/ShaderGL.h"

namespace rx
{

ProgramGL::ProgramGL(RendererGL* renderer, const FunctionsGL *functions)
    : ProgramImpl(),
      mRenderer(renderer),
      mFunctions(functions),
      mProgramID(0)
{
    ASSERT(mFunctions);
}

ProgramGL::~ProgramGL()
{
    if (mProgramID != 0)
    {
        mFunctions->deleteProgram(mProgramID);
    }
}

bool ProgramGL::usesPointSize() const
{
    UNIMPLEMENTED();
    return bool();
}

int ProgramGL::getShaderVersion() const
{
    UNIMPLEMENTED();
    return int();
}

GLenum ProgramGL::getTransformFeedbackBufferMode() const
{
    UNIMPLEMENTED();
    return GLenum();
}

GLenum ProgramGL::getBinaryFormat()
{
    UNIMPLEMENTED();
    return GLenum();
}

LinkResult ProgramGL::load(gl::InfoLog &infoLog, gl::BinaryInputStream *stream)
{
    UNIMPLEMENTED();
    return LinkResult(false, gl::Error(GL_INVALID_OPERATION));
}

gl::Error ProgramGL::save(gl::BinaryOutputStream *stream)
{
    UNIMPLEMENTED();
    return gl::Error(GL_INVALID_OPERATION);
}

LinkResult ProgramGL::link(const gl::Data &data, gl::InfoLog &infoLog,
                           gl::Shader *fragmentShader, gl::Shader *vertexShader,
                           const std::vector<std::string> &transformFeedbackVaryings,
                           GLenum transformFeedbackBufferMode,
                           int *registers, std::vector<gl::LinkedVarying> *linkedVaryings,
                           std::map<int, gl::VariableLocation> *outputVariables)
{
    ShaderGL *vertexShaderGL = GetImplAs<ShaderGL>(vertexShader);
    ShaderGL *fragmentShaderGL = GetImplAs<ShaderGL>(fragmentShader);

    // Generate a new program, make sure one doesn't already exist
    ASSERT(mProgramID == 0);
    mProgramID = mFunctions->createProgram();

    // Attach the shaders
    mFunctions->attachShader(mProgramID, vertexShaderGL->getShaderID());
    mFunctions->attachShader(mProgramID, fragmentShaderGL->getShaderID());

    // TODO: bind attribute locations

    // Link and verify
    mFunctions->linkProgram(mProgramID);

    GLint linkStatus = GL_FALSE;
    mFunctions->getProgramiv(mProgramID, GL_LINK_STATUS, &linkStatus);
    ASSERT(linkStatus == GL_TRUE);
    if (linkStatus == GL_FALSE)
    {
        // Linking failed, put the error into the info log
        GLint infoLogLength = 0;
        mFunctions->getProgramiv(mProgramID, GL_INFO_LOG_LENGTH, &infoLogLength);

        std::vector<char> buf(infoLogLength);
        mFunctions->getProgramInfoLog(mProgramID, infoLogLength, nullptr, &buf[0]);

        mFunctions->deleteProgram(mProgramID);
        mProgramID = 0;

        infoLog.append(&buf[0]);
        TRACE("\n%s", &buf[0]);

        // TODO, return GL_OUT_OF_MEMORY or just fail the link? This is an unexpected case
        return LinkResult(false, gl::Error(GL_NO_ERROR));
    }

    // Query the uniform information
    GLint activeUniformMaxLength = 0;
    mFunctions->getProgramiv(mProgramID, GL_ACTIVE_UNIFORM_MAX_LENGTH, &activeUniformMaxLength);

    std::vector<char> nameBuf(activeUniformMaxLength);

    GLint uniformCount = 0;
    mFunctions->getProgramiv(mProgramID, GL_ACTIVE_UNIFORMS, &uniformCount);
    for (GLint i = 0; i < uniformCount; i++)
    {
        GLsizei uniformNameLength = 0;
        GLint uniformSize = 0;
        GLenum uniformType = GL_NONE;
        mFunctions->getActiveUniform(mProgramID, i, nameBuf.size(), &uniformNameLength, &uniformSize, &uniformType, &nameBuf[0]);

        std::string uniformName(&nameBuf[0], uniformNameLength);

        // TODO: determine uniform precision
        mUniforms.push_back(new gl::LinkedUniform(uniformType, GL_HIGH_FLOAT, uniformName, uniformSize, -1, sh::BlockMemberInfo::getDefaultBlockInfo()));
    }

    return LinkResult(true, gl::Error(GL_NO_ERROR));
}

void ProgramGL::setUniform1fv(GLint location, GLsizei count, const GLfloat *v)
{
    mRenderer->setCurrentProgram(mProgramID);
    mFunctions->uniform1fv(location, count, v);
}

void ProgramGL::setUniform2fv(GLint location, GLsizei count, const GLfloat *v)
{
    mRenderer->setCurrentProgram(mProgramID);
    mFunctions->uniform2fv(location, count, v);
}

void ProgramGL::setUniform3fv(GLint location, GLsizei count, const GLfloat *v)
{
    mRenderer->setCurrentProgram(mProgramID);
    mFunctions->uniform3fv(location, count, v);
}

void ProgramGL::setUniform4fv(GLint location, GLsizei count, const GLfloat *v)
{
    mRenderer->setCurrentProgram(mProgramID);
    mFunctions->uniform4fv(location, count, v);
}

void ProgramGL::setUniform1iv(GLint location, GLsizei count, const GLint *v)
{
    mRenderer->setCurrentProgram(mProgramID);
    mFunctions->uniform1iv(location, count, v);
}

void ProgramGL::setUniform2iv(GLint location, GLsizei count, const GLint *v)
{
    mRenderer->setCurrentProgram(mProgramID);
    mFunctions->uniform2iv(location, count, v);
}

void ProgramGL::setUniform3iv(GLint location, GLsizei count, const GLint *v)
{
    mRenderer->setCurrentProgram(mProgramID);
    mFunctions->uniform3iv(location, count, v);
}

void ProgramGL::setUniform4iv(GLint location, GLsizei count, const GLint *v)
{
    mRenderer->setCurrentProgram(mProgramID);
    mFunctions->uniform4iv(location, count, v);
}

void ProgramGL::setUniform1uiv(GLint location, GLsizei count, const GLuint *v)
{
    mRenderer->setCurrentProgram(mProgramID);
    mFunctions->uniform1uiv(location, count, v);
}

void ProgramGL::setUniform2uiv(GLint location, GLsizei count, const GLuint *v)
{
    mRenderer->setCurrentProgram(mProgramID);
    mFunctions->uniform2uiv(location, count, v);
}

void ProgramGL::setUniform3uiv(GLint location, GLsizei count, const GLuint *v)
{
    mRenderer->setCurrentProgram(mProgramID);
    mFunctions->uniform3uiv(location, count, v);
}

void ProgramGL::setUniform4uiv(GLint location, GLsizei count, const GLuint *v)
{
    mRenderer->setCurrentProgram(mProgramID);
    mFunctions->uniform4uiv(location, count, v);
}

void ProgramGL::setUniformMatrix2fv(GLint location, GLsizei count, GLboolean transpose, const GLfloat *value)
{
    mRenderer->setCurrentProgram(mProgramID);
    mFunctions->uniformMatrix2fv(location, count, transpose, value);
}

void ProgramGL::setUniformMatrix3fv(GLint location, GLsizei count, GLboolean transpose, const GLfloat *value)
{
    mRenderer->setCurrentProgram(mProgramID);
    mFunctions->uniformMatrix3fv(location, count, transpose, value);
}

void ProgramGL::setUniformMatrix4fv(GLint location, GLsizei count, GLboolean transpose, const GLfloat *value)
{
    mRenderer->setCurrentProgram(mProgramID);
    mFunctions->uniformMatrix4fv(location, count, transpose, value);
}

void ProgramGL::setUniformMatrix2x3fv(GLint location, GLsizei count, GLboolean transpose, const GLfloat *value)
{
    mRenderer->setCurrentProgram(mProgramID);
    mFunctions->uniformMatrix2x3fv(location, count, transpose, value);
}

void ProgramGL::setUniformMatrix3x2fv(GLint location, GLsizei count, GLboolean transpose, const GLfloat *value)
{
    mRenderer->setCurrentProgram(mProgramID);
    mFunctions->uniformMatrix3x2fv(location, count, transpose, value);
}

void ProgramGL::setUniformMatrix2x4fv(GLint location, GLsizei count, GLboolean transpose, const GLfloat *value)
{
    mRenderer->setCurrentProgram(mProgramID);
    mFunctions->uniformMatrix2x4fv(location, count, transpose, value);
}

void ProgramGL::setUniformMatrix4x2fv(GLint location, GLsizei count, GLboolean transpose, const GLfloat *value)
{
    mRenderer->setCurrentProgram(mProgramID);
    mFunctions->uniformMatrix4x2fv(location, count, transpose, value);
}

void ProgramGL::setUniformMatrix3x4fv(GLint location, GLsizei count, GLboolean transpose, const GLfloat *value)
{
    mRenderer->setCurrentProgram(mProgramID);
    mFunctions->uniformMatrix3x4fv(location, count, transpose, value);
}

void ProgramGL::setUniformMatrix4x3fv(GLint location, GLsizei count, GLboolean transpose, const GLfloat *value)
{
    mRenderer->setCurrentProgram(mProgramID);
    mFunctions->uniformMatrix4x3fv(location, count, transpose, value);
}

void ProgramGL::getUniformfv(GLint location, GLfloat *params)
{
    mFunctions->getUniformfv(mProgramID, location, params);
}

void ProgramGL::getUniformiv(GLint location, GLint *params)
{
    mFunctions->getUniformiv(mProgramID, location, params);
}

void ProgramGL::getUniformuiv(GLint location, GLuint *params)
{
    mFunctions->getUniformuiv(mProgramID, location, params);
}

GLint ProgramGL::getSamplerMapping(gl::SamplerType type, unsigned int samplerIndex, const gl::Caps &caps) const
{
    UNIMPLEMENTED();
    return GLint();
}

GLenum ProgramGL::getSamplerTextureType(gl::SamplerType type, unsigned int samplerIndex) const
{
    UNIMPLEMENTED();
    return GLenum();
}

GLint ProgramGL::getUsedSamplerRange(gl::SamplerType type) const
{
    UNIMPLEMENTED();
    return GLint();
}

void ProgramGL::updateSamplerMapping()
{
    UNIMPLEMENTED();
}

bool ProgramGL::validateSamplers(gl::InfoLog *infoLog, const gl::Caps &caps)
{
    UNIMPLEMENTED();
    return true;
}

LinkResult ProgramGL::compileProgramExecutables(gl::InfoLog &infoLog, gl::Shader *fragmentShader, gl::Shader *vertexShader,
                                                int registers)
{
    UNIMPLEMENTED();
    return LinkResult(true, gl::Error(GL_NO_ERROR));
}

bool ProgramGL::linkUniforms(gl::InfoLog &infoLog, const gl::Shader &vertexShader, const gl::Shader &fragmentShader,
                             const gl::Caps &caps)
{
    UNIMPLEMENTED();
    return true;
}

bool ProgramGL::defineUniformBlock(gl::InfoLog &infoLog, const gl::Shader &shader, const sh::InterfaceBlock &interfaceBlock,
                                   const gl::Caps &caps)
{
    UNIMPLEMENTED();
    return bool();
}

gl::Error ProgramGL::applyUniforms()
{
    UNIMPLEMENTED();
    return gl::Error(GL_INVALID_OPERATION);
}

gl::Error ProgramGL::applyUniformBuffers(const std::vector<gl::Buffer*> boundBuffers, const gl::Caps &caps)
{
    UNIMPLEMENTED();
    return gl::Error(GL_INVALID_OPERATION);
}

bool ProgramGL::assignUniformBlockRegister(gl::InfoLog &infoLog, gl::UniformBlock *uniformBlock, GLenum shader,
                                           unsigned int registerIndex, const gl::Caps &caps)
{
    UNIMPLEMENTED();
    return bool();
}

}
