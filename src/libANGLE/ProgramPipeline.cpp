//
// Copyright 2017 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//

// ProgramPipeline.cpp: Implements the gl::ProgramPipeline class.
// Implements GL program pipeline objects and related functionality.
// [OpenGL ES 3.1] section 7.4 page 105.

#include "libANGLE/ProgramPipeline.h"

#include <iostream>

#include "libANGLE/Context.h"
#include "libANGLE/angletypes.h"
#include "libANGLE/renderer/GLImplFactory.h"
#include "libANGLE/renderer/ProgramPipelineImpl.h"

namespace gl
{

ProgramPipelineState::ProgramPipelineState()
    : mLabel(), mActiveShaderProgram(nullptr), mValid(false)
{
    for (const ShaderType shaderType : gl::AllShaderTypes())
    {
        mPrograms[shaderType] = nullptr;
    }
}

ProgramPipelineState::~ProgramPipelineState() {}

const std::string &ProgramPipelineState::getLabel() const
{
    return mLabel;
}

void ProgramPipelineState::activeShaderProgram(Program *shaderProgram)
{
    mActiveShaderProgram = shaderProgram;
}

void ProgramPipelineState::useProgramStage(const ShaderType shaderType, Program *shaderProgram)
{
    // If program refers to a program object with a valid shader attached for an indicated shader
    // stage, glUseProgramStages installs the executable code for that stage in the indicated
    // program pipeline object pipeline.
    if ((shaderProgram->id().value != 0) && shaderProgram->hasLinkedShaderStage(shaderType))
    {
        mPrograms[shaderType] = shaderProgram;
    }
    // If program is zero, or refers to a program object with no valid shader executable for a given
    // stage, it is as if the pipeline object has no programmable stage configured for the indicated
    // shader stages.
    else
    {
        mPrograms[shaderType] = nullptr;
    }
}

void ProgramPipelineState::useProgramStages(GLbitfield stages, Program *shaderProgram)
{
    if (stages == GL_ALL_SHADER_BITS)
    {
        for (const ShaderType shaderType : gl::AllShaderTypes())
        {
            useProgramStage(shaderType, shaderProgram);
        }
    }
    else
    {
        if (stages & GL_VERTEX_SHADER_BIT)
        {
            useProgramStage(ShaderType::Vertex, shaderProgram);
        }

        if (stages & GL_FRAGMENT_SHADER_BIT)
        {
            useProgramStage(ShaderType::Fragment, shaderProgram);
        }

        if (stages & GL_COMPUTE_SHADER_BIT)
        {
            useProgramStage(ShaderType::Compute, shaderProgram);
        }
    }
}

ProgramPipeline::ProgramPipeline(rx::GLImplFactory *factory, ProgramPipelineID handle)
    : RefCountObject(factory->generateSerial(), handle),
      mProgramPipelineImpl(factory->createProgramPipeline(mState))
{
    ASSERT(mProgramPipelineImpl);

    mActiveAttribLocationsMask.reset();
}

ProgramPipeline::~ProgramPipeline()
{
    mProgramPipelineImpl.release();
}

void ProgramPipeline::onDestroy(const Context *context)
{
    getImplementation()->destroy(context);
}

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
    return mProgramPipelineImpl.get();
}

void ProgramPipeline::activeShaderProgram(Program *shaderProgram)
{
    mState.activeShaderProgram(shaderProgram);
}

void ProgramPipeline::useProgramStages(GLbitfield stages, Program *shaderProgram)
{
    mState.useProgramStages(stages, shaderProgram);
    updateLinkedShaderStages();
}

void ProgramPipeline::updateLinkedShaderStages()
{
    mState.mLinkedShaderStages.reset();

    for (const ShaderType shaderType : gl::AllShaderTypes())
    {
        if (mState.mPrograms[shaderType])
        {
            mState.mLinkedShaderStages.set(shaderType);
        }
    }
}

int ProgramPipeline::getInfoLogLength() const
{
    return static_cast<int>(mInfoLog.getLength());
}

void ProgramPipeline::getInfoLog(GLsizei bufSize, GLsizei *length, char *infoLog) const
{
    return mInfoLog.getLog(bufSize, length, infoLog);
}

const gl::ActiveTextureMask &ProgramPipeline::getActiveSamplersMask() const
{
    return getShaderProgram(gl::ShaderType::Fragment)->getActiveSamplersMask();
}

const gl::ActiveTextureArray<gl::TextureType> &ProgramPipeline::getActiveSamplerTypes() const
{
    return getShaderProgram(gl::ShaderType::Fragment)->getActiveSamplerTypes();
}

const AttributesMask &ProgramPipeline::getActiveAttribLocationsMask() const
{
    const Program *vertProgram = getShaderProgram(gl::ShaderType::Vertex);
    if (vertProgram)
    {
        return vertProgram->getActiveAttribLocationsMask();
    }

    // Return an empty AttributesMask if there's no vertex shader in the pipeline.
    return mActiveAttribLocationsMask;
}

ProgramMergedVaryings ProgramPipeline::getMergedVaryings() const
{
    ProgramMergedVaryings merged;

    for (ShaderType shaderType : kAllGraphicsShaderTypes)
    {
        const Program *program = getShaderProgram(shaderType);
        if (!program)
        {
            continue;
        }
        Shader *shader = program->getState().getAttachedShader(shaderType);
        if (!shader)
        {
            continue;
        }

        for (const sh::ShaderVariable &varying : shader->getOutputVaryings())
        {
            ProgramVaryingRef *ref = &merged[varying.name];
            ref->frontShader       = &varying;
            ref->frontShaderStage  = shaderType;
        }
        for (const sh::ShaderVariable &varying : shader->getInputVaryings())
        {
            ProgramVaryingRef *ref = &merged[varying.name];
            ref->backShader        = &varying;
            ref->backShaderStage   = shaderType;
        }
    }

    return merged;
}

// The attached shaders are checked for linking errors by matching up their variables.
// Uniform, input and output variables get collected.
// The code gets compiled into binaries.
angle::Result ProgramPipeline::link(const Context *context)
{
    mInfoLog.reset();
    const State &state = context->getState();

    // Map the varyings to the register file
    gl::PackMode packMode = PackMode::ANGLE_RELAXED;
    if (state.getLimitations().noFlexibleVaryingPacking)
    {
        // D3D9 pack mode is strictly more strict than WebGL, so takes priority.
        packMode = PackMode::ANGLE_NON_CONFORMANT_D3D9;
    }
    else if (state.getExtensions().webglCompatibility)
    {
        // In WebGL, we use a slightly different handling for packing variables.
        packMode = PackMode::WEBGL_STRICT;
    }

    if (!linkVaryings(mInfoLog))
    {
        return angle::Result::Stop;
    }

    if (!linkValidateGlobalNames(mInfoLog))
    {
        return angle::Result::Stop;
    }

    GLuint maxVaryingVectors = static_cast<GLuint>(context->getState().getCaps().maxVaryingVectors);
    VaryingPacking varyingPacking(maxVaryingVectors, packMode);

    const ProgramMergedVaryings mergedVaryings = getMergedVaryings();
    for (ShaderType shaderType : kAllGraphicsShaderTypes)
    {
        Program *program = mState.mPrograms[shaderType];
        if (program)
        {
            program->getResources().varyingPacking.reset();
            ANGLE_TRY(program->linkMergedVaryings(context, program->getResources().varyingPacking,
                                                  mergedVaryings));
        }
    }

    ANGLE_TRY(getImplementation()->link(context));

    return angle::Result::Continue;
}

bool ProgramPipeline::linkVaryings(InfoLog &infoLog) const
{
    Shader *previousShader = nullptr;
    for (ShaderType shaderType : kAllGraphicsShaderTypes)
    {
        Program *currentProgram = mState.mPrograms[shaderType];
        if (!currentProgram)
        {
            continue;
        }

        Shader *currentShader =
            const_cast<Shader *>(currentProgram->getState().getAttachedShader(shaderType));
        if (!currentShader)
        {
            continue;
        }

        if (previousShader)
        {
            if (!Program::linkValidateShaderInterfaceMatching(previousShader, currentShader,
                                                              infoLog))
            {
                return false;
            }
        }
        previousShader = currentShader;
    }

    Program *vertexProgram   = mState.mPrograms[ShaderType::Vertex];
    Program *fragmentProgram = mState.mPrograms[ShaderType::Fragment];
    if (!vertexProgram || !fragmentProgram)
    {
        return false;
    }

    Shader *vertexShader =
        const_cast<Shader *>(vertexProgram->getState().getAttachedShader(ShaderType::Vertex));
    Shader *fragmentShader =
        const_cast<Shader *>(fragmentProgram->getState().getAttachedShader(ShaderType::Fragment));
    if (!Program::linkValidateBuiltInVaryings(vertexShader, fragmentShader, infoLog))
    {
        return false;
    }

    if (!fragmentProgram->linkValidateFragmentInputBindings(infoLog))
    {
        return false;
    }

    return true;
}

bool ProgramPipeline::linkValidateGlobalNames(InfoLog &infoLog) const
{
    std::unordered_map<std::string, const sh::ShaderVariable *> uniformMap;
    using BlockAndFieldPair = std::pair<const sh::InterfaceBlock *, const sh::ShaderVariable *>;
    std::unordered_map<std::string, std::vector<BlockAndFieldPair>> uniformBlockFieldMap;

    for (ShaderType shaderType : kAllGraphicsShaderTypes)
    {
        const Program *program = getShaderProgram(shaderType);
        if (!program)
        {
            continue;
        }
        Shader *shader = program->getState().getAttachedShader(shaderType);
        if (!shader)
        {
            continue;
        }

        // Build a map of Uniforms
        const std::vector<sh::ShaderVariable> uniforms = shader->getUniforms();
        for (const auto &uniform : uniforms)
        {
            uniformMap[uniform.name] = &uniform;
        }

        // Build a map of Uniform Blocks
        // This will also detect any field name conflicts between Uniform Blocks without instance
        // names
        const std::vector<sh::InterfaceBlock> &uniformBlocks = shader->getUniformBlocks();
        for (const auto &uniformBlock : uniformBlocks)
        {
            // Only uniform blocks without an instance name can create a conflict with their field
            // names
            if (!uniformBlock.instanceName.empty())
            {
                continue;
            }

            for (const auto &field : uniformBlock.fields)
            {
                if (!uniformBlockFieldMap.count(field.name))
                {
                    // First time we've seen this uniform block field name, so add the
                    // (Uniform Block, Field) pair immediately since there can't be a conflict yet
                    BlockAndFieldPair blockAndFieldPair(&uniformBlock, &field);
                    std::vector<BlockAndFieldPair> newUniformBlockList;
                    newUniformBlockList.push_back(blockAndFieldPair);
                    uniformBlockFieldMap[field.name] = newUniformBlockList;
                    continue;
                }

                // We've seen this name before.
                // We need to check each of the uniform blocks that contain a field with this name
                // to see if there's a conflict or not.
                std::vector<BlockAndFieldPair> prevBlockFieldPairs =
                    uniformBlockFieldMap[field.name];
                for (const auto &prevBlockFieldPair : prevBlockFieldPairs)
                {
                    const sh::InterfaceBlock *prevUniformBlock      = prevBlockFieldPair.first;
                    const sh::ShaderVariable *prevUniformBlockField = prevBlockFieldPair.second;

                    if (uniformBlock.isSameInterfaceBlockAtLinkTime(*prevUniformBlock))
                    {
                        // The same uniform block should, by definition, contain the same field name
                        continue;
                    }

                    // The uniform blocks don't match, so check if the necessary field properties
                    // also match
                    if ((field.name == prevUniformBlockField->name) &&
                        (field.type == prevUniformBlockField->type) &&
                        (field.precision == prevUniformBlockField->precision))
                    {
                        infoLog << "Name conflicts between uniform block field names: "
                                << field.name;
                        return false;
                    }
                }

                // No conflict, so record this pair
                BlockAndFieldPair blockAndFieldPair(&uniformBlock, &field);
                uniformBlockFieldMap[field.name].push_back(blockAndFieldPair);
            }
        }
    }

    // Validate no uniform names conflict with attribute names
    const Program *program = getShaderProgram(ShaderType::Vertex);
    Shader *vertexShader   = program->getState().getAttachedShader(ShaderType::Vertex);
    if (vertexShader)
    {
        for (const auto &attrib : vertexShader->getActiveAttributes())
        {
            if (uniformMap.count(attrib.name))
            {
                infoLog << "Name conflicts between a uniform and an attribute: " << attrib.name;
                return false;
            }
        }
    }

    // Validate no Uniform Block fields conflict with other Uniforms
    for (const auto &uniformBlockField : uniformBlockFieldMap)
    {
        const std::string &fieldName = uniformBlockField.first;
        if (uniformMap.count(fieldName))
        {
            infoLog << "Name conflicts between a uniform and a uniform block field: " << fieldName;
            return false;
        }
    }

    return true;
}

void ProgramPipeline::validate(const Caps &caps)
{
    mState.mValid = true;
    mInfoLog.reset();

    for (const ShaderType shaderType : gl::AllShaderTypes())
    {
        Program *shaderProgram = mState.mPrograms[shaderType];
        if (shaderProgram)
        {
            shaderProgram->validate(caps);
            std::string shaderInfoString = shaderProgram->getInfoLogString();
            if (shaderInfoString.length())
            {
                mState.mValid = false;
                mInfoLog << shaderInfoString << "\n";
                return;
            }
        }
    }

    if (!linkVaryings(mInfoLog))
    {
        mState.mValid = false;

        for (const ShaderType shaderType : gl::AllShaderTypes())
        {
            Program *shaderProgram = mState.mPrograms[shaderType];
            if (shaderProgram)
            {
                shaderProgram->validate(caps);
                std::string shaderInfoString = shaderProgram->getInfoLogString();
                if (shaderInfoString.length())
                {
                    mInfoLog << shaderInfoString << "\n";
                }
            }
        }
    }
}

bool ProgramPipeline::validateSamplers(InfoLog *infoLog, const Caps &caps)
{
    for (const ShaderType shaderType : gl::AllShaderTypes())
    {
        Program *shaderProgram = mState.mPrograms[shaderType];
        if (shaderProgram && !shaderProgram->validateSamplers(infoLog, caps))
        {
            return false;
        }
    }

    return true;
}

const char *ProgramPipeline::validateDrawStates(const State &state,
                                                const Extensions &extensions) const
{
    for (const ShaderType shaderType : gl::AllShaderTypes())
    {
        const Program *program = getShaderProgram(shaderType);
        if (program)
        {
            const char *errorMsg = program->validateDrawStates(state, extensions);
            if (errorMsg)
            {
                return errorMsg;
            }
        }
    }

    return nullptr;
}

angle::Result ProgramPipeline::syncState(const Context *context)
{
    if (mDirtyBits.any())
    {
        for (const ShaderType shaderType : gl::AllShaderTypes())
        {
            Program *shaderProgram = mState.mPrograms[shaderType];
            ANGLE_TRY(shaderProgram->syncState(context));
        }

        mDirtyBits.reset();
    }

    return angle::Result::Continue;
}

bool ProgramPipeline::usesShaderProgram(ShaderProgramID programId) const
{
    for (const Program *program : mState.mPrograms)
    {
        if (program && (program->id() == programId))
        {
            return true;
        }
    }

    return false;
}

angle::Result ProgramPipeline::prepareForDraw(Context *context)
{
    // The default program pipeline only contains monolithic programs, so no extra work is required
    // to prepare for a draw.
    if (isDefaultProgramPipeline())
    {
        return angle::Result::Continue;
    }

    bool goodResult = link(context) == angle::Result::Continue;
    ANGLE_CHECK(context, goodResult, "Program pipeline link failed", GL_INVALID_OPERATION);

    return angle::Result::Continue;
}

}  // namespace gl
