//
// Copyright 2015 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//

// ProgramGL.cpp: Implements the class methods for ProgramGL.

#include "libANGLE/renderer/gl/ProgramGL.h"

#include "common/angleutils.h"
#include "common/bitset_utils.h"
#include "common/debug.h"
#include "common/string_utils.h"
#include "common/utilities.h"
#include "libANGLE/Context.h"
#include "libANGLE/Uniform.h"
#include "libANGLE/renderer/gl/ContextGL.h"
#include "libANGLE/renderer/gl/FunctionsGL.h"
#include "libANGLE/renderer/gl/ShaderGL.h"
#include "libANGLE/renderer/gl/StateManagerGL.h"
#include "libANGLE/renderer/gl/WorkaroundsGL.h"
#include "platform/Platform.h"

namespace rx
{

ProgramGL::ProgramGL(const gl::ProgramState &data,
                     const FunctionsGL *functions,
                     const WorkaroundsGL &workarounds,
                     StateManagerGL *stateManager,
                     bool enablePathRendering)
    : ProgramImpl(data),
      mFunctions(functions),
      mWorkarounds(workarounds),
      mStateManager(stateManager),
      mEnablePathRendering(enablePathRendering),
      mMultiviewBaseViewLayerIndexUniformLocation(-1),
      mProgramID(0)
{
    ASSERT(mFunctions);
    ASSERT(mStateManager);

    mProgramID = mFunctions->gl.createProgram();
}

ProgramGL::~ProgramGL()
{
    mFunctions->gl.deleteProgram(mProgramID);
    mProgramID = 0;
}

gl::LinkResult ProgramGL::load(const gl::Context *context,
                               gl::InfoLog &infoLog,
                               gl::BinaryInputStream *stream)
{
    preLink();

    // Read the binary format, size and blob
    GLenum binaryFormat   = stream->readInt<GLenum>();
    GLint binaryLength    = stream->readInt<GLint>();
    const uint8_t *binary = stream->data() + stream->offset();
    stream->skip(binaryLength);

    // Load the binary
    mFunctions->gl.programBinary(mProgramID, binaryFormat, binary, binaryLength);

    // Verify that the program linked
    if (!checkLinkStatus(infoLog))
    {
        return false;
    }

    postLink();
    reapplyUBOBindingsIfNeeded(context);

    return true;
}

void ProgramGL::save(const gl::Context *context, gl::BinaryOutputStream *stream)
{
    GLint binaryLength = 0;
    mFunctions->gl.getProgramiv(mProgramID, GL_PROGRAM_BINARY_LENGTH, &binaryLength);

    std::vector<uint8_t> binary(binaryLength);
    GLenum binaryFormat = GL_NONE;
    mFunctions->gl.getProgramBinary(mProgramID, binaryLength, &binaryLength, &binaryFormat,
                                    &binary[0]);

    stream->writeInt(binaryFormat);
    stream->writeInt(binaryLength);
    stream->writeBytes(&binary[0], binaryLength);

    reapplyUBOBindingsIfNeeded(context);
}

void ProgramGL::reapplyUBOBindingsIfNeeded(const gl::Context *context)
{
    // Re-apply UBO bindings to work around driver bugs.
    const WorkaroundsGL &workaroundsGL = GetImplAs<ContextGL>(context)->getWorkaroundsGL();
    if (workaroundsGL.reapplyUBOBindingsAfterUsingBinaryProgram)
    {
        const auto &blocks = mState.getUniformBlocks();
        for (size_t blockIndex : mState.getActiveUniformBlockBindingsMask())
        {
            setUniformBlockBinding(static_cast<GLuint>(blockIndex), blocks[blockIndex].binding);
        }
    }
}

void ProgramGL::setBinaryRetrievableHint(bool retrievable)
{
    // glProgramParameteri isn't always available on ES backends.
    if (mFunctions->gl.programParameteri)
    {
        mFunctions->gl.programParameteri(mProgramID, GL_PROGRAM_BINARY_RETRIEVABLE_HINT,
                                         retrievable ? GL_TRUE : GL_FALSE);
    }
}

void ProgramGL::setSeparable(bool separable)
{
    mFunctions->gl.programParameteri(mProgramID, GL_PROGRAM_SEPARABLE,
                                     separable ? GL_TRUE : GL_FALSE);
}

gl::LinkResult ProgramGL::link(const gl::Context *context,
                               const gl::VaryingPacking &packing,
                               gl::InfoLog &infoLog)
{
    preLink();

    if (mState.getAttachedComputeShader())
    {
        const ShaderGL *computeShaderGL = GetImplAs<ShaderGL>(mState.getAttachedComputeShader());

        mFunctions->gl.attachShader(mProgramID, computeShaderGL->getShaderID());

        // Link and verify
        mFunctions->gl.linkProgram(mProgramID);

        // Detach the shaders
        mFunctions->gl.detachShader(mProgramID, computeShaderGL->getShaderID());
    }
    else
    {
        // Set the transform feedback state
        std::vector<std::string> transformFeedbackVaryingMappedNames;
        for (const auto &tfVarying : mState.getTransformFeedbackVaryingNames())
        {
            std::string tfVaryingMappedName =
                mState.getAttachedVertexShader()->getTransformFeedbackVaryingMappedName(tfVarying,
                                                                                        context);
            transformFeedbackVaryingMappedNames.push_back(tfVaryingMappedName);
        }

        if (transformFeedbackVaryingMappedNames.empty())
        {
            if (mFunctions->gl.transformFeedbackVaryings)
            {
                mFunctions->gl.transformFeedbackVaryings(mProgramID, 0, nullptr,
                                                         mState.getTransformFeedbackBufferMode());
            }
        }
        else
        {
            ASSERT(mFunctions->gl.transformFeedbackVaryings);
            std::vector<const GLchar *> transformFeedbackVaryings;
            for (const auto &varying : transformFeedbackVaryingMappedNames)
            {
                transformFeedbackVaryings.push_back(varying.c_str());
            }
            mFunctions->gl.transformFeedbackVaryings(
                mProgramID, static_cast<GLsizei>(transformFeedbackVaryingMappedNames.size()),
                &transformFeedbackVaryings[0], mState.getTransformFeedbackBufferMode());
        }

        const ShaderGL *vertexShaderGL   = GetImplAs<ShaderGL>(mState.getAttachedVertexShader());
        const ShaderGL *fragmentShaderGL = GetImplAs<ShaderGL>(mState.getAttachedFragmentShader());

        // Attach the shaders
        mFunctions->gl.attachShader(mProgramID, vertexShaderGL->getShaderID());
        mFunctions->gl.attachShader(mProgramID, fragmentShaderGL->getShaderID());

        // Bind attribute locations to match the GL layer.
        for (const sh::Attribute &attribute : mState.getAttributes())
        {
            if (!attribute.staticUse || attribute.isBuiltIn())
            {
                continue;
            }

            mFunctions->gl.bindAttribLocation(mProgramID, attribute.location,
                                              attribute.mappedName.c_str());
        }

        // Link and verify
        mFunctions->gl.linkProgram(mProgramID);

        // Detach the shaders
        mFunctions->gl.detachShader(mProgramID, vertexShaderGL->getShaderID());
        mFunctions->gl.detachShader(mProgramID, fragmentShaderGL->getShaderID());
    }

    // Verify the link
    if (!checkLinkStatus(infoLog))
    {
        return false;
    }

    if (mWorkarounds.alwaysCallUseProgramAfterLink)
    {
        mStateManager->forceUseProgram(mProgramID);
    }

    postLink();

    return true;
}

GLboolean ProgramGL::validate(const gl::Caps & /*caps*/, gl::InfoLog * /*infoLog*/)
{
    // TODO(jmadill): implement validate
    return true;
}

void ProgramGL::setUniform1fv(GLint location, GLsizei count, const GLfloat *v)
{
    if (mFunctions->gl.programUniform1fv != nullptr)
    {
        mFunctions->gl.programUniform1fv(mProgramID, uniLoc(location), count, v);
    }
    else
    {
        mStateManager->useProgram(mProgramID);
        mFunctions->gl.uniform1fv(uniLoc(location), count, v);
    }
}

void ProgramGL::setUniform2fv(GLint location, GLsizei count, const GLfloat *v)
{
    if (mFunctions->gl.programUniform2fv != nullptr)
    {
        mFunctions->gl.programUniform2fv(mProgramID, uniLoc(location), count, v);
    }
    else
    {
        mStateManager->useProgram(mProgramID);
        mFunctions->gl.uniform2fv(uniLoc(location), count, v);
    }
}

void ProgramGL::setUniform3fv(GLint location, GLsizei count, const GLfloat *v)
{
    if (mFunctions->gl.programUniform3fv != nullptr)
    {
        mFunctions->gl.programUniform3fv(mProgramID, uniLoc(location), count, v);
    }
    else
    {
        mStateManager->useProgram(mProgramID);
        mFunctions->gl.uniform3fv(uniLoc(location), count, v);
    }
}

void ProgramGL::setUniform4fv(GLint location, GLsizei count, const GLfloat *v)
{
    if (mFunctions->gl.programUniform4fv != nullptr)
    {
        mFunctions->gl.programUniform4fv(mProgramID, uniLoc(location), count, v);
    }
    else
    {
        mStateManager->useProgram(mProgramID);
        mFunctions->gl.uniform4fv(uniLoc(location), count, v);
    }
}

void ProgramGL::setUniform1iv(GLint location, GLsizei count, const GLint *v)
{
    if (mFunctions->gl.programUniform1iv != nullptr)
    {
        mFunctions->gl.programUniform1iv(mProgramID, uniLoc(location), count, v);
    }
    else
    {
        mStateManager->useProgram(mProgramID);
        mFunctions->gl.uniform1iv(uniLoc(location), count, v);
    }
}

void ProgramGL::setUniform2iv(GLint location, GLsizei count, const GLint *v)
{
    if (mFunctions->gl.programUniform2iv != nullptr)
    {
        mFunctions->gl.programUniform2iv(mProgramID, uniLoc(location), count, v);
    }
    else
    {
        mStateManager->useProgram(mProgramID);
        mFunctions->gl.uniform2iv(uniLoc(location), count, v);
    }
}

void ProgramGL::setUniform3iv(GLint location, GLsizei count, const GLint *v)
{
    if (mFunctions->gl.programUniform3iv != nullptr)
    {
        mFunctions->gl.programUniform3iv(mProgramID, uniLoc(location), count, v);
    }
    else
    {
        mStateManager->useProgram(mProgramID);
        mFunctions->gl.uniform3iv(uniLoc(location), count, v);
    }
}

void ProgramGL::setUniform4iv(GLint location, GLsizei count, const GLint *v)
{
    if (mFunctions->gl.programUniform4iv != nullptr)
    {
        mFunctions->gl.programUniform4iv(mProgramID, uniLoc(location), count, v);
    }
    else
    {
        mStateManager->useProgram(mProgramID);
        mFunctions->gl.uniform4iv(uniLoc(location), count, v);
    }
}

void ProgramGL::setUniform1uiv(GLint location, GLsizei count, const GLuint *v)
{
    if (mFunctions->gl.programUniform1uiv != nullptr)
    {
        mFunctions->gl.programUniform1uiv(mProgramID, uniLoc(location), count, v);
    }
    else
    {
        mStateManager->useProgram(mProgramID);
        mFunctions->gl.uniform1uiv(uniLoc(location), count, v);
    }
}

void ProgramGL::setUniform2uiv(GLint location, GLsizei count, const GLuint *v)
{
    if (mFunctions->gl.programUniform2uiv != nullptr)
    {
        mFunctions->gl.programUniform2uiv(mProgramID, uniLoc(location), count, v);
    }
    else
    {
        mStateManager->useProgram(mProgramID);
        mFunctions->gl.uniform2uiv(uniLoc(location), count, v);
    }
}

void ProgramGL::setUniform3uiv(GLint location, GLsizei count, const GLuint *v)
{
    if (mFunctions->gl.programUniform3uiv != nullptr)
    {
        mFunctions->gl.programUniform3uiv(mProgramID, uniLoc(location), count, v);
    }
    else
    {
        mStateManager->useProgram(mProgramID);
        mFunctions->gl.uniform3uiv(uniLoc(location), count, v);
    }
}

void ProgramGL::setUniform4uiv(GLint location, GLsizei count, const GLuint *v)
{
    if (mFunctions->gl.programUniform4uiv != nullptr)
    {
        mFunctions->gl.programUniform4uiv(mProgramID, uniLoc(location), count, v);
    }
    else
    {
        mStateManager->useProgram(mProgramID);
        mFunctions->gl.uniform4uiv(uniLoc(location), count, v);
    }
}

void ProgramGL::setUniformMatrix2fv(GLint location, GLsizei count, GLboolean transpose, const GLfloat *value)
{
    if (mFunctions->gl.programUniformMatrix2fv != nullptr)
    {
        mFunctions->gl.programUniformMatrix2fv(mProgramID, uniLoc(location), count, transpose,
                                               value);
    }
    else
    {
        mStateManager->useProgram(mProgramID);
        mFunctions->gl.uniformMatrix2fv(uniLoc(location), count, transpose, value);
    }
}

void ProgramGL::setUniformMatrix3fv(GLint location, GLsizei count, GLboolean transpose, const GLfloat *value)
{
    if (mFunctions->gl.programUniformMatrix3fv != nullptr)
    {
        mFunctions->gl.programUniformMatrix3fv(mProgramID, uniLoc(location), count, transpose,
                                               value);
    }
    else
    {
        mStateManager->useProgram(mProgramID);
        mFunctions->gl.uniformMatrix3fv(uniLoc(location), count, transpose, value);
    }
}

void ProgramGL::setUniformMatrix4fv(GLint location, GLsizei count, GLboolean transpose, const GLfloat *value)
{
    if (mFunctions->gl.programUniformMatrix4fv != nullptr)
    {
        mFunctions->gl.programUniformMatrix4fv(mProgramID, uniLoc(location), count, transpose,
                                               value);
    }
    else
    {
        mStateManager->useProgram(mProgramID);
        mFunctions->gl.uniformMatrix4fv(uniLoc(location), count, transpose, value);
    }
}

void ProgramGL::setUniformMatrix2x3fv(GLint location, GLsizei count, GLboolean transpose, const GLfloat *value)
{
    if (mFunctions->gl.programUniformMatrix2x3fv != nullptr)
    {
        mFunctions->gl.programUniformMatrix2x3fv(mProgramID, uniLoc(location), count, transpose,
                                                 value);
    }
    else
    {
        mStateManager->useProgram(mProgramID);
        mFunctions->gl.uniformMatrix2x3fv(uniLoc(location), count, transpose, value);
    }
}

void ProgramGL::setUniformMatrix3x2fv(GLint location, GLsizei count, GLboolean transpose, const GLfloat *value)
{
    if (mFunctions->gl.programUniformMatrix3x2fv != nullptr)
    {
        mFunctions->gl.programUniformMatrix3x2fv(mProgramID, uniLoc(location), count, transpose,
                                                 value);
    }
    else
    {
        mStateManager->useProgram(mProgramID);
        mFunctions->gl.uniformMatrix3x2fv(uniLoc(location), count, transpose, value);
    }
}

void ProgramGL::setUniformMatrix2x4fv(GLint location, GLsizei count, GLboolean transpose, const GLfloat *value)
{
    if (mFunctions->gl.programUniformMatrix2x4fv != nullptr)
    {
        mFunctions->gl.programUniformMatrix2x4fv(mProgramID, uniLoc(location), count, transpose,
                                                 value);
    }
    else
    {
        mStateManager->useProgram(mProgramID);
        mFunctions->gl.uniformMatrix2x4fv(uniLoc(location), count, transpose, value);
    }
}

void ProgramGL::setUniformMatrix4x2fv(GLint location, GLsizei count, GLboolean transpose, const GLfloat *value)
{
    if (mFunctions->gl.programUniformMatrix4x2fv != nullptr)
    {
        mFunctions->gl.programUniformMatrix4x2fv(mProgramID, uniLoc(location), count, transpose,
                                                 value);
    }
    else
    {
        mStateManager->useProgram(mProgramID);
        mFunctions->gl.uniformMatrix4x2fv(uniLoc(location), count, transpose, value);
    }
}

void ProgramGL::setUniformMatrix3x4fv(GLint location, GLsizei count, GLboolean transpose, const GLfloat *value)
{
    if (mFunctions->gl.programUniformMatrix3x4fv != nullptr)
    {
        mFunctions->gl.programUniformMatrix3x4fv(mProgramID, uniLoc(location), count, transpose,
                                                 value);
    }
    else
    {
        mStateManager->useProgram(mProgramID);
        mFunctions->gl.uniformMatrix3x4fv(uniLoc(location), count, transpose, value);
    }
}

void ProgramGL::setUniformMatrix4x3fv(GLint location, GLsizei count, GLboolean transpose, const GLfloat *value)
{
    if (mFunctions->gl.programUniformMatrix4x3fv != nullptr)
    {
        mFunctions->gl.programUniformMatrix4x3fv(mProgramID, uniLoc(location), count, transpose,
                                                 value);
    }
    else
    {
        mStateManager->useProgram(mProgramID);
        mFunctions->gl.uniformMatrix4x3fv(uniLoc(location), count, transpose, value);
    }
}

void ProgramGL::setUniformBlockBinding(GLuint uniformBlockIndex, GLuint uniformBlockBinding)
{
    // Lazy init
    if (mUniformBlockRealLocationMap.empty())
    {
        mUniformBlockRealLocationMap.reserve(mState.getUniformBlocks().size());
        for (const gl::InterfaceBlock &uniformBlock : mState.getUniformBlocks())
        {
            const std::string &mappedNameWithIndex = uniformBlock.mappedNameWithArrayIndex();
            GLuint blockIndex =
                mFunctions->gl.getUniformBlockIndex(mProgramID, mappedNameWithIndex.c_str());
            mUniformBlockRealLocationMap.push_back(blockIndex);
        }
    }

    GLuint realBlockIndex = mUniformBlockRealLocationMap[uniformBlockIndex];
    if (realBlockIndex != GL_INVALID_INDEX)
    {
        mFunctions->gl.uniformBlockBinding(mProgramID, realBlockIndex, uniformBlockBinding);
    }
}

GLuint ProgramGL::getProgramID() const
{
    return mProgramID;
}

bool ProgramGL::getUniformBlockSize(const std::string & /* blockName */,
                                    const std::string &blockMappedName,
                                    size_t *sizeOut) const
{
    ASSERT(mProgramID != 0u);

    GLuint blockIndex = mFunctions->gl.getUniformBlockIndex(mProgramID, blockMappedName.c_str());
    if (blockIndex == GL_INVALID_INDEX)
    {
        *sizeOut = 0;
        return false;
    }

    GLint dataSize = 0;
    mFunctions->gl.getActiveUniformBlockiv(mProgramID, blockIndex, GL_UNIFORM_BLOCK_DATA_SIZE,
                                           &dataSize);
    *sizeOut = static_cast<size_t>(dataSize);
    return true;
}

bool ProgramGL::getUniformBlockMemberInfo(const std::string & /* memberUniformName */,
                                          const std::string &memberUniformMappedName,
                                          sh::BlockMemberInfo *memberInfoOut) const
{
    GLuint uniformIndex;
    const GLchar *memberNameGLStr = memberUniformMappedName.c_str();
    mFunctions->gl.getUniformIndices(mProgramID, 1, &memberNameGLStr, &uniformIndex);

    if (uniformIndex == GL_INVALID_INDEX)
    {
        *memberInfoOut = sh::BlockMemberInfo::getDefaultBlockInfo();
        return false;
    }

    mFunctions->gl.getActiveUniformsiv(mProgramID, 1, &uniformIndex, GL_UNIFORM_OFFSET,
                                       &memberInfoOut->offset);
    mFunctions->gl.getActiveUniformsiv(mProgramID, 1, &uniformIndex, GL_UNIFORM_ARRAY_STRIDE,
                                       &memberInfoOut->arrayStride);
    mFunctions->gl.getActiveUniformsiv(mProgramID, 1, &uniformIndex, GL_UNIFORM_MATRIX_STRIDE,
                                       &memberInfoOut->matrixStride);

    // TODO(jmadill): possibly determine this at the gl::Program level.
    GLint isRowMajorMatrix = 0;
    mFunctions->gl.getActiveUniformsiv(mProgramID, 1, &uniformIndex, GL_UNIFORM_IS_ROW_MAJOR,
                                       &isRowMajorMatrix);
    memberInfoOut->isRowMajorMatrix = isRowMajorMatrix != GL_FALSE;
    return true;
}

void ProgramGL::setPathFragmentInputGen(const std::string &inputName,
                                        GLenum genMode,
                                        GLint components,
                                        const GLfloat *coeffs)
{
    ASSERT(mEnablePathRendering);

    for (const auto &input : mPathRenderingFragmentInputs)
    {
        if (input.mappedName == inputName)
        {
            mFunctions->gl.programPathFragmentInputGenNV(mProgramID, input.location, genMode,
                                                         components, coeffs);
            ASSERT(mFunctions->gl.getError() == GL_NO_ERROR);
            return;
        }
    }

}

void ProgramGL::preLink()
{
    // Reset the program state
    mUniformRealLocationMap.clear();
    mUniformBlockRealLocationMap.clear();
    mPathRenderingFragmentInputs.clear();

    mMultiviewBaseViewLayerIndexUniformLocation = -1;
}

bool ProgramGL::checkLinkStatus(gl::InfoLog &infoLog)
{
    GLint linkStatus = GL_FALSE;
    mFunctions->gl.getProgramiv(mProgramID, GL_LINK_STATUS, &linkStatus);
    if (linkStatus == GL_FALSE)
    {
        // Linking failed, put the error into the info log
        GLint infoLogLength = 0;
        mFunctions->gl.getProgramiv(mProgramID, GL_INFO_LOG_LENGTH, &infoLogLength);

        // Info log length includes the null terminator, so 1 means that the info log is an empty
        // string.
        if (infoLogLength > 1)
        {
            std::vector<char> buf(infoLogLength);
            mFunctions->gl.getProgramInfoLog(mProgramID, infoLogLength, nullptr, &buf[0]);

            mFunctions->gl.deleteProgram(mProgramID);
            mProgramID = 0;

            infoLog << buf.data();

            WARN() << "Program link failed unexpectedly: " << buf.data();
        }
        else
        {
            WARN() << "Program link failed unexpectedly with no info log.";
        }

        // TODO, return GL_OUT_OF_MEMORY or just fail the link? This is an unexpected case
        return false;
    }

    return true;
}

void ProgramGL::postLink()
{
    // Query the uniform information
    ASSERT(mUniformRealLocationMap.empty());
    const auto &uniformLocations = mState.getUniformLocations();
    const auto &uniforms = mState.getUniforms();
    mUniformRealLocationMap.resize(uniformLocations.size(), GL_INVALID_INDEX);
    for (size_t uniformLocation = 0; uniformLocation < uniformLocations.size(); uniformLocation++)
    {
        const auto &entry = uniformLocations[uniformLocation];
        if (!entry.used())
        {
            continue;
        }

        // From the GLES 3.0.5 spec:
        // "Locations for sequential array indices are not required to be sequential."
        const gl::LinkedUniform &uniform = uniforms[entry.index];
        std::stringstream fullNameStr;
        fullNameStr << uniform.mappedName;
        if (uniform.isArray())
        {
            for (auto arrayElementIndexIt = entry.arrayIndices.rbegin();
                 arrayElementIndexIt != entry.arrayIndices.rend(); ++arrayElementIndexIt)
            {
                fullNameStr << "[" << (*arrayElementIndexIt) << "]";
            }
        }
        const std::string &fullName = fullNameStr.str();

        GLint realLocation = mFunctions->gl.getUniformLocation(mProgramID, fullName.c_str());
        mUniformRealLocationMap[uniformLocation] = realLocation;
    }

    if (mState.usesMultiview())
    {
        mMultiviewBaseViewLayerIndexUniformLocation =
            mFunctions->gl.getUniformLocation(mProgramID, "multiviewBaseViewLayerIndex");
        ASSERT(mMultiviewBaseViewLayerIndexUniformLocation != -1);
    }

    // Discover CHROMIUM_path_rendering fragment inputs if enabled.
    if (!mEnablePathRendering)
        return;

    GLint numFragmentInputs = 0;
    mFunctions->gl.getProgramInterfaceiv(mProgramID, GL_FRAGMENT_INPUT_NV, GL_ACTIVE_RESOURCES,
                                         &numFragmentInputs);
    if (numFragmentInputs <= 0)
        return;

    GLint maxNameLength = 0;
    mFunctions->gl.getProgramInterfaceiv(mProgramID, GL_FRAGMENT_INPUT_NV, GL_MAX_NAME_LENGTH,
                                         &maxNameLength);
    ASSERT(maxNameLength);

    for (GLint i = 0; i < numFragmentInputs; ++i)
    {
        std::string mappedName;
        mappedName.resize(maxNameLength);

        GLsizei nameLen = 0;
        mFunctions->gl.getProgramResourceName(mProgramID, GL_FRAGMENT_INPUT_NV, i, maxNameLength,
                                              &nameLen, &mappedName[0]);
        mappedName.resize(nameLen);

        // Ignore built-ins
        if (angle::BeginsWith(mappedName, "gl_"))
            continue;

        const GLenum kQueryProperties[] = {GL_LOCATION, GL_ARRAY_SIZE};
        GLint queryResults[ArraySize(kQueryProperties)];
        GLsizei queryLength = 0;

        mFunctions->gl.getProgramResourceiv(
            mProgramID, GL_FRAGMENT_INPUT_NV, i, static_cast<GLsizei>(ArraySize(kQueryProperties)),
            kQueryProperties, static_cast<GLsizei>(ArraySize(queryResults)), &queryLength,
            queryResults);

        ASSERT(queryLength == static_cast<GLsizei>(ArraySize(kQueryProperties)));

        PathRenderingFragmentInput baseElementInput;
        baseElementInput.mappedName = mappedName;
        baseElementInput.location = queryResults[0];
        mPathRenderingFragmentInputs.push_back(std::move(baseElementInput));

        // If the input is an array it's denoted by [0] suffix on the variable
        // name. We'll then create an entry per each array index where index > 0
        if (angle::EndsWith(mappedName, "[0]"))
        {
            // drop the suffix
            mappedName.resize(mappedName.size() - 3);

            const auto arraySize    = queryResults[1];
            const auto baseLocation = queryResults[0];

            for (GLint arrayIndex = 1; arrayIndex < arraySize; ++arrayIndex)
            {
                PathRenderingFragmentInput arrayElementInput;
                arrayElementInput.mappedName = mappedName + "[" + ToString(arrayIndex) + "]";
                arrayElementInput.location = baseLocation + arrayIndex;
                mPathRenderingFragmentInputs.push_back(std::move(arrayElementInput));
            }
        }
    }
}

void ProgramGL::enableSideBySideRenderingPath() const
{
    ASSERT(mState.usesMultiview());
    ASSERT(mMultiviewBaseViewLayerIndexUniformLocation != -1);

    ASSERT(mFunctions->gl.programUniform1i != nullptr);
    mFunctions->gl.programUniform1i(mProgramID, mMultiviewBaseViewLayerIndexUniformLocation, -1);
}

void ProgramGL::enableLayeredRenderingPath(int baseViewIndex) const
{
    ASSERT(mState.usesMultiview());
    ASSERT(mMultiviewBaseViewLayerIndexUniformLocation != -1);

    ASSERT(mFunctions->gl.programUniform1i != nullptr);
    mFunctions->gl.programUniform1i(mProgramID, mMultiviewBaseViewLayerIndexUniformLocation,
                                    baseViewIndex);
}

void ProgramGL::getUniformfv(const gl::Context *context, GLint location, GLfloat *params) const
{
    mFunctions->gl.getUniformfv(mProgramID, uniLoc(location), params);
}

void ProgramGL::getUniformiv(const gl::Context *context, GLint location, GLint *params) const
{
    mFunctions->gl.getUniformiv(mProgramID, uniLoc(location), params);
}

void ProgramGL::getUniformuiv(const gl::Context *context, GLint location, GLuint *params) const
{
    mFunctions->gl.getUniformuiv(mProgramID, uniLoc(location), params);
}

void ProgramGL::markUnusedUniformLocations(std::vector<gl::VariableLocation> *uniformLocations,
                                           std::vector<gl::SamplerBinding> *samplerBindings)
{
    GLint maxLocation = static_cast<GLint>(uniformLocations->size());
    for (GLint location = 0; location < maxLocation; ++location)
    {
        if (uniLoc(location) == -1)
        {
            auto &locationRef = (*uniformLocations)[location];
            if (mState.isSamplerUniformIndex(locationRef.index))
            {
                GLuint samplerIndex = mState.getSamplerIndexFromUniformIndex(locationRef.index);
                (*samplerBindings)[samplerIndex].unreferenced = true;
            }
            locationRef.markUnused();
        }
    }
}

}  // namespace rx
