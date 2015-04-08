//
// Copyright 2015 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//

// ProgramGL.cpp: Implements the class methods for ProgramGL.

#include "libANGLE/renderer/gl/ProgramGL.h"

#include "common/debug.h"
#include "common/utilities.h"
#include "libANGLE/renderer/gl/FunctionsGL.h"
#include "libANGLE/renderer/gl/ShaderGL.h"
#include "libANGLE/renderer/gl/StateManagerGL.h"

static unsigned int ParseAndStripArrayIndex(std::string* name)
{
    unsigned int subscript = GL_INVALID_INDEX;

    // Strip any trailing array operator and retrieve the subscript
    size_t open = name->find_last_of('[');
    size_t close = name->find_last_of(']');
    if (open != std::string::npos && close == name->length() - 1)
    {
        subscript = atoi(name->substr(open + 1).c_str());
        name->erase(open);
    }

    return subscript;
}

namespace rx
{

ProgramGL::ProgramGL(const FunctionsGL *functions, StateManagerGL *stateManager)
    : ProgramImpl(),
      mFunctions(functions),
      mStateManager(stateManager),
      mProgramID(0)
{
    ASSERT(mFunctions);
    ASSERT(mStateManager);
}

ProgramGL::~ProgramGL()
{
    if (mProgramID != 0)
    {
        GLCall(mFunctions, deleteProgram, mProgramID);
        mProgramID = 0;
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
    // Reset the program state, delete the current program if one exists
    reset();

    ShaderGL *vertexShaderGL = GetImplAs<ShaderGL>(vertexShader);
    ShaderGL *fragmentShaderGL = GetImplAs<ShaderGL>(fragmentShader);

    // Generate a new program, make sure one doesn't already exist
    ASSERT(mProgramID == 0);
    mProgramID = GLCall(mFunctions, createProgram, );

    // Attach the shaders
    GLCall(mFunctions, attachShader, mProgramID, vertexShaderGL->getShaderID());
    GLCall(mFunctions, attachShader, mProgramID, fragmentShaderGL->getShaderID());

    // TODO: bind attribute locations?

    // Link and verify
    GLCall(mFunctions, linkProgram, mProgramID);

    GLint linkStatus = GL_FALSE;
    GLCall(mFunctions, getProgramiv, mProgramID, GL_LINK_STATUS, &linkStatus);
    ASSERT(linkStatus == GL_TRUE);
    if (linkStatus == GL_FALSE)
    {
        // Linking failed, put the error into the info log
        GLint infoLogLength = 0;
        GLCall(mFunctions, getProgramiv, mProgramID, GL_INFO_LOG_LENGTH, &infoLogLength);

        std::vector<char> buf(infoLogLength);
        GLCall(mFunctions, getProgramInfoLog, mProgramID, infoLogLength, nullptr, &buf[0]);

        GLCall(mFunctions, deleteProgram, mProgramID);
        mProgramID = 0;

        infoLog.append(&buf[0]);
        TRACE("\n%s", &buf[0]);

        // TODO, return GL_OUT_OF_MEMORY or just fail the link? This is an unexpected case
        return LinkResult(false, gl::Error(GL_NO_ERROR));
    }

    // Query the uniform information
    // TODO: A lot of this logic should be done at the gl::Program level
    GLint activeUniformMaxLength = 0;
    GLCall(mFunctions, getProgramiv, mProgramID, GL_ACTIVE_UNIFORM_MAX_LENGTH, &activeUniformMaxLength);

    std::vector<GLchar> uniformNameBuffer(activeUniformMaxLength);

    GLint uniformCount = 0;
    GLCall(mFunctions, getProgramiv, mProgramID, GL_ACTIVE_UNIFORMS, &uniformCount);
    for (GLint i = 0; i < uniformCount; i++)
    {
        GLsizei uniformNameLength = 0;
        GLint uniformSize = 0;
        GLenum uniformType = GL_NONE;
        GLCall(mFunctions, getActiveUniform, mProgramID, i, uniformNameBuffer.size(), &uniformNameLength, &uniformSize, &uniformType, &uniformNameBuffer[0]);

        std::string uniformName(&uniformNameBuffer[0], uniformNameLength);

        if (uniformSize > 1)
        {
            ParseAndStripArrayIndex(&uniformName);
        }

        // TODO: determine uniform precision
        for (size_t arrayIndex = 0; arrayIndex < static_cast<size_t>(uniformSize); arrayIndex++)
        {
            std::string locationName = uniformName;
            if (uniformSize > 1)
            {
                locationName += "[" + Str(arrayIndex) + "]";
            }

            GLint location = mFunctions->getUniformLocation(mProgramID, locationName.c_str());
            if (location >= 0)
            {
                if (static_cast<size_t>(location) >= mUniformIndex.size())
                {
                    mUniformIndex.resize(location + 1);
                }

                mUniformIndex[location] = gl::VariableLocation(uniformName, arrayIndex, mUniforms.size());
            }
        }
        mUniforms.push_back(new gl::LinkedUniform(uniformType, GL_NONE, uniformName, uniformSize, -1, sh::BlockMemberInfo::getDefaultBlockInfo()));

        // If the uniform is a sampler, track it in the sampler bindings array
        if (gl::IsSamplerType(uniformType))
        {
            SamplerBindingGL samplerBinding;
            samplerBinding.textureType = gl::SamplerTypeToTextureType(uniformType);
            samplerBinding.boundTextureUnits.resize(uniformSize, 0);

            mSamplerUniformMap[i] = mSamplerBindings.size();
            mSamplerBindings.push_back(samplerBinding);
        }
    }

    // Query the attribute information
    GLint activeAttributeMaxLength = 0;
    GLCall(mFunctions, getProgramiv, mProgramID, GL_ACTIVE_ATTRIBUTE_MAX_LENGTH, &activeAttributeMaxLength);

    std::vector<GLchar> attributeNameBuffer(activeAttributeMaxLength);

    GLint attributeCount = 0;
    GLCall(mFunctions, getProgramiv, mProgramID, GL_ACTIVE_ATTRIBUTES, &attributeCount);
    for (GLint i = 0; i < attributeCount; i++)
    {
        GLsizei attributeNameLength = 0;
        GLint attributeSize = 0;
        GLenum attributeType = GL_NONE;
        GLCall(mFunctions, getActiveAttrib, mProgramID, i, attributeNameBuffer.size(), &attributeNameLength, &attributeSize, &attributeType, &attributeNameBuffer[0]);

        std::string attributeName(&attributeNameBuffer[0], attributeNameLength);

        mShaderAttributes[i].type = attributeType;
        // TODO: determine attribute precision
        mShaderAttributes[i].precision = GL_NONE;
        mShaderAttributes[i].name = attributeName;
        mShaderAttributes[i].arraySize = attributeSize;
        mShaderAttributes[i].location = i;
    }

    return LinkResult(true, gl::Error(GL_NO_ERROR));
}

void ProgramGL::setUniform1fv(GLint location, GLsizei count, const GLfloat *v)
{
    mStateManager->useProgram(mProgramID);
    GLCall(mFunctions, uniform1fv, location, count, v);
}

void ProgramGL::setUniform2fv(GLint location, GLsizei count, const GLfloat *v)
{
    mStateManager->useProgram(mProgramID);
    GLCall(mFunctions, uniform2fv, location, count, v);
}

void ProgramGL::setUniform3fv(GLint location, GLsizei count, const GLfloat *v)
{
    mStateManager->useProgram(mProgramID);
    GLCall(mFunctions, uniform3fv, location, count, v);
}

void ProgramGL::setUniform4fv(GLint location, GLsizei count, const GLfloat *v)
{
    mStateManager->useProgram(mProgramID);
    GLCall(mFunctions, uniform4fv, location, count, v);
}

void ProgramGL::setUniform1iv(GLint location, GLsizei count, const GLint *v)
{
    mStateManager->useProgram(mProgramID);
    GLCall(mFunctions, uniform1iv, location, count, v);

    auto iter = mSamplerUniformMap.find(location);
    if (iter != mSamplerUniformMap.end())
    {
        SamplerBindingGL &binding = mSamplerBindings[iter->second];
        std::copy(v, v + std::min<size_t>(count, binding.boundTextureUnits.size()), binding.boundTextureUnits.begin());
    }
}

void ProgramGL::setUniform2iv(GLint location, GLsizei count, const GLint *v)
{
    mStateManager->useProgram(mProgramID);
    GLCall(mFunctions, uniform2iv, location, count, v);
}

void ProgramGL::setUniform3iv(GLint location, GLsizei count, const GLint *v)
{
    mStateManager->useProgram(mProgramID);
    GLCall(mFunctions, uniform3iv, location, count, v);
}

void ProgramGL::setUniform4iv(GLint location, GLsizei count, const GLint *v)
{
    mStateManager->useProgram(mProgramID);
    GLCall(mFunctions, uniform4iv, location, count, v);
}

void ProgramGL::setUniform1uiv(GLint location, GLsizei count, const GLuint *v)
{
    mStateManager->useProgram(mProgramID);
    GLCall(mFunctions, uniform1uiv, location, count, v);
}

void ProgramGL::setUniform2uiv(GLint location, GLsizei count, const GLuint *v)
{
    mStateManager->useProgram(mProgramID);
    GLCall(mFunctions, uniform2uiv, location, count, v);
}

void ProgramGL::setUniform3uiv(GLint location, GLsizei count, const GLuint *v)
{
    mStateManager->useProgram(mProgramID);
    GLCall(mFunctions, uniform3uiv, location, count, v);
}

void ProgramGL::setUniform4uiv(GLint location, GLsizei count, const GLuint *v)
{
    mStateManager->useProgram(mProgramID);
    GLCall(mFunctions, uniform4uiv, location, count, v);
}

void ProgramGL::setUniformMatrix2fv(GLint location, GLsizei count, GLboolean transpose, const GLfloat *value)
{
    mStateManager->useProgram(mProgramID);
    GLCall(mFunctions, uniformMatrix2fv, location, count, transpose, value);
}

void ProgramGL::setUniformMatrix3fv(GLint location, GLsizei count, GLboolean transpose, const GLfloat *value)
{
    mStateManager->useProgram(mProgramID);
    GLCall(mFunctions, uniformMatrix3fv, location, count, transpose, value);
}

void ProgramGL::setUniformMatrix4fv(GLint location, GLsizei count, GLboolean transpose, const GLfloat *value)
{
    mStateManager->useProgram(mProgramID);
    GLCall(mFunctions, uniformMatrix4fv, location, count, transpose, value);
}

void ProgramGL::setUniformMatrix2x3fv(GLint location, GLsizei count, GLboolean transpose, const GLfloat *value)
{
    mStateManager->useProgram(mProgramID);
    GLCall(mFunctions, uniformMatrix2x3fv, location, count, transpose, value);
}

void ProgramGL::setUniformMatrix3x2fv(GLint location, GLsizei count, GLboolean transpose, const GLfloat *value)
{
    mStateManager->useProgram(mProgramID);
    GLCall(mFunctions, uniformMatrix3x2fv, location, count, transpose, value);
}

void ProgramGL::setUniformMatrix2x4fv(GLint location, GLsizei count, GLboolean transpose, const GLfloat *value)
{
    mStateManager->useProgram(mProgramID);
    GLCall(mFunctions, uniformMatrix2x4fv, location, count, transpose, value);
}

void ProgramGL::setUniformMatrix4x2fv(GLint location, GLsizei count, GLboolean transpose, const GLfloat *value)
{
    mStateManager->useProgram(mProgramID);
    GLCall(mFunctions, uniformMatrix4x2fv, location, count, transpose, value);
}

void ProgramGL::setUniformMatrix3x4fv(GLint location, GLsizei count, GLboolean transpose, const GLfloat *value)
{
    mStateManager->useProgram(mProgramID);
    GLCall(mFunctions, uniformMatrix3x4fv, location, count, transpose, value);
}

void ProgramGL::setUniformMatrix4x3fv(GLint location, GLsizei count, GLboolean transpose, const GLfloat *value)
{
    mStateManager->useProgram(mProgramID);
    GLCall(mFunctions, uniformMatrix4x3fv, location, count, transpose, value);
}

void ProgramGL::getUniformfv(GLint location, GLfloat *params)
{
    GLCall(mFunctions, getUniformfv, mProgramID, location, params);
}

void ProgramGL::getUniformiv(GLint location, GLint *params)
{
    GLCall(mFunctions, getUniformiv, mProgramID, location, params);
}

void ProgramGL::getUniformuiv(GLint location, GLuint *params)
{
    GLCall(mFunctions, getUniformuiv, mProgramID, location, params);
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
    //UNIMPLEMENTED();
    return true;
}

LinkResult ProgramGL::compileProgramExecutables(gl::InfoLog &infoLog, gl::Shader *fragmentShader, gl::Shader *vertexShader,
                                                int registers)
{
    //UNIMPLEMENTED();
    return LinkResult(true, gl::Error(GL_NO_ERROR));
}

bool ProgramGL::linkUniforms(gl::InfoLog &infoLog, const gl::Shader &vertexShader, const gl::Shader &fragmentShader,
                             const gl::Caps &caps)
{
    //UNIMPLEMENTED();
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

gl::Error ProgramGL::applyUniformBuffers(const gl::Data &data, GLuint uniformBlockBindings[])
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

void ProgramGL::reset()
{
    ProgramImpl::reset();

    if (mProgramID)
    {
        GLCall(mFunctions, deleteProgram, mProgramID);
        mProgramID = 0;
    }

    mSamplerUniformMap.clear();
    mSamplerBindings.clear();
}

GLuint ProgramGL::getProgramID() const
{
    return mProgramID;
}

const std::vector<SamplerBindingGL> &ProgramGL::getAppliedSamplerUniforms() const
{
    return mSamplerBindings;
}

}
