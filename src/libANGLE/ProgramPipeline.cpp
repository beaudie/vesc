//
// Copyright 2017 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//

// ProgramPipeline.cpp: Implements the gl::ProgramPipeline class.
// Implements GL program pipeline objects and related functionality.
// [OpenGL ES 3.1] section 7.4 page 105.

#include "libANGLE/ProgramPipeline.h"

#include <algorithm>

#include "libANGLE/Context.h"
#include "libANGLE/ErrorStrings.h"
#include "libANGLE/Program.h"
#include "libANGLE/angletypes.h"
#include "libANGLE/renderer/GLImplFactory.h"
#include "libANGLE/renderer/ProgramPipelineImpl.h"

namespace gl
{

ProgramPipelineState::ProgramPipelineState()
    : mLabel(),
      mIsCompute(false),
      mActiveShaderProgram(nullptr),
      mValid(false),
      mHasBeenBound(false)
{
    mExecutable.setProgramPipelineState(this);

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

bool ProgramPipelineState::useProgramStage(const ShaderType shaderType, Program *shaderProgram)
{
    // If program refers to a program object with a valid shader attached for the indicated shader
    // stage, glUseProgramStages installs the executable code for that stage in the indicated
    // program pipeline object pipeline.
    if (shaderProgram && (shaderProgram->id().value != 0) &&
        shaderProgram->getExecutable().hasLinkedShaderStage(shaderType))
    {
        mPrograms[shaderType] = shaderProgram;
        return true;
    }

    // If program is zero, or refers to a program object with no valid shader executable for the
    // given stage, it is as if the pipeline object has no programmable stage configured for the
    // indicated shader stage.
    mPrograms[shaderType] = nullptr;
    return false;
}

void ProgramPipelineState::useProgramStages(const Context *context,
                                            GLbitfield stages,
                                            Program *shaderProgram)
{
    // Record if the PPO is already using this Program before we attempt to add it,
    // so we don't ref count it twice for the same PPO.
    bool usingShaderProgram               = shaderProgram && usesShaderProgram(shaderProgram->id());
    bool usingProgramStage                = false;
    ShaderMap<Program *> startingPrograms = mPrograms;

    if (stages == GL_ALL_SHADER_BITS)
    {
        for (const ShaderType shaderType : gl::AllShaderTypes())
        {
            usingProgramStage = usingProgramStage || useProgramStage(shaderType, shaderProgram);
        }
    }
    else
    {
        if (stages & GL_VERTEX_SHADER_BIT)
        {
            usingProgramStage = useProgramStage(ShaderType::Vertex, shaderProgram);
        }

        if (stages & GL_FRAGMENT_SHADER_BIT)
        {
            usingProgramStage = useProgramStage(ShaderType::Fragment, shaderProgram);
        }

        if (stages & GL_COMPUTE_SHADER_BIT)
        {
            usingProgramStage = useProgramStage(ShaderType::Compute, shaderProgram);
        }
    }

    // Only refCount Programs actually in use, but don't refCount programs more than once
    if (usingProgramStage && shaderProgram && !usingShaderProgram)
    {
        shaderProgram->addRef();
    }

    // Need to release the reference to any Programs that this Pipeline is no longer using it
    std::vector<Program *> alreadyReleasedPrograms;
    for (Program *startingProgram : startingPrograms)
    {
        if (startingProgram &&
            (std::find(alreadyReleasedPrograms.begin(), alreadyReleasedPrograms.end(),
                       startingProgram) == alreadyReleasedPrograms.end()))
        {
            bool foundProgram = false;
            for (Program *newProgram : mPrograms)
            {
                if (startingProgram == newProgram)
                {
                    foundProgram = true;
                    break;
                }
            }

            if (!foundProgram)
            {
                ASSERT(startingProgram->getRefCount());
                startingProgram->release(context);
            }
            alreadyReleasedPrograms.emplace_back(startingProgram);
        }
    }
}

bool ProgramPipelineState::usesShaderProgram(ShaderProgramID programId) const
{
    for (const Program *program : mPrograms)
    {
        if (program && (program->id() == programId))
        {
            return true;
        }
    }

    return false;
}

bool ProgramPipelineState::hasDefaultUniforms() const
{
    for (const gl::ShaderType shaderType : mExecutable.getLinkedShaderStages())
    {
        const Program *shaderProgram = getShaderProgram(shaderType);
        if (shaderProgram && shaderProgram->getState().hasDefaultUniforms())
        {
            return true;
        }
    }

    return false;
}

bool ProgramPipelineState::hasTextures() const
{
    for (const gl::ShaderType shaderType : mExecutable.getLinkedShaderStages())
    {
        const Program *shaderProgram = getShaderProgram(shaderType);
        if (shaderProgram && shaderProgram->getState().hasTextures())
        {
            return true;
        }
    }

    return false;
}

bool ProgramPipelineState::hasUniformBuffers() const
{
    for (const gl::ShaderType shaderType : mExecutable.getLinkedShaderStages())
    {
        const Program *shaderProgram = getShaderProgram(shaderType);
        if (shaderProgram && shaderProgram->getState().hasUniformBuffers())
        {
            return true;
        }
    }

    return false;
}

bool ProgramPipelineState::hasStorageBuffers() const
{
    for (const gl::ShaderType shaderType : mExecutable.getLinkedShaderStages())
    {
        const Program *shaderProgram = getShaderProgram(shaderType);
        if (shaderProgram && shaderProgram->getState().hasStorageBuffers())
        {
            return true;
        }
    }

    return false;
}

bool ProgramPipelineState::hasAtomicCounterBuffers() const
{
    for (const gl::ShaderType shaderType : mExecutable.getLinkedShaderStages())
    {
        const Program *shaderProgram = getShaderProgram(shaderType);
        if (shaderProgram && shaderProgram->getState().hasAtomicCounterBuffers())
        {
            return true;
        }
    }

    return false;
}

bool ProgramPipelineState::hasImages() const
{
    for (const gl::ShaderType shaderType : mExecutable.getLinkedShaderStages())
    {
        const Program *shaderProgram = getShaderProgram(shaderType);
        if (shaderProgram && shaderProgram->getState().hasImages())
        {
            return true;
        }
    }

    return false;
}

bool ProgramPipelineState::hasTransformFeedbackOutput() const
{
    for (const gl::ShaderType shaderType : mExecutable.getLinkedShaderStages())
    {
        const Program *shaderProgram = getShaderProgram(shaderType);
        if (shaderProgram && shaderProgram->getState().hasTransformFeedbackOutput())
        {
            return true;
        }
    }

    return false;
}

ProgramPipeline::ProgramPipeline(rx::GLImplFactory *factory, ProgramPipelineID handle)
    : RefCountObject(factory->generateSerial(), handle),
      mProgramPipelineImpl(factory->createProgramPipeline(mState))
{
    ASSERT(mProgramPipelineImpl);
}

ProgramPipeline::~ProgramPipeline()
{
    mProgramPipelineImpl.release();
}

void ProgramPipeline::onDestroy(const Context *context)
{
    std::vector<Program *> alreadyReleasedPrograms;
    for (Program *program : mState.mPrograms)
    {
        if (program && (std::find(alreadyReleasedPrograms.begin(), alreadyReleasedPrograms.end(),
                                  program) == alreadyReleasedPrograms.end()))
        {
            ASSERT(program->getRefCount());
            program->release(context);
            alreadyReleasedPrograms.emplace_back(program);
        }
    }

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

void ProgramPipeline::useProgramStages(const Context *context,
                                       GLbitfield stages,
                                       Program *shaderProgram)
{
    mState.useProgramStages(context, stages, shaderProgram);
    updateLinkedShaderStages();
    updateExecutable();
}

void ProgramPipeline::updateLinkedShaderStages()
{
    mState.mExecutable.mLinkedComputeShaderStages.reset();
    mState.mExecutable.mLinkedGraphicsShaderStages.reset();

    for (const ShaderType shaderType : gl::AllShaderTypes())
    {
        Program *program = mState.mPrograms[shaderType];
        if (program)
        {
            if (shaderType == ShaderType::Compute)
            {
                mState.mExecutable.mLinkedComputeShaderStages.set(shaderType);
            }
            else
            {
                mState.mExecutable.mLinkedGraphicsShaderStages.set(shaderType);
            }
        }
    }
}

void ProgramPipeline::updateExecutableAttributes()
{
    Program *vertexProgram = getShaderProgram(gl::ShaderType::Vertex);

    if (!vertexProgram)
    {
        return;
    }

    const ProgramExecutable &vertexExecutable     = vertexProgram->getExecutable();
    mState.mExecutable.mActiveAttribLocationsMask = vertexExecutable.mActiveAttribLocationsMask;
    mState.mExecutable.mMaxActiveAttribLocation   = vertexExecutable.mMaxActiveAttribLocation;
    mState.mExecutable.mAttributesTypeMask        = vertexExecutable.mAttributesTypeMask;
    mState.mExecutable.mAttributesMask            = vertexExecutable.mAttributesMask;
}

void ProgramPipeline::updateExecutableTextures()
{
    for (const ShaderType shaderType : mState.mExecutable.getLinkedShaderStages())
    {
        const Program *program = getShaderProgram(shaderType);
        if (program)
        {
            mState.mExecutable.mActiveSamplersMask |=
                program->getExecutable().getActiveSamplersMask();
            mState.mExecutable.mActiveImagesMask |= program->getExecutable().getActiveImagesMask();
            // Updates mActiveSamplerRefCounts, mActiveSamplerTypes, and mActiveSamplerFormats
            mState.mExecutable.updateActiveSamplers(program->getState());
        }
    }
}

void ProgramPipeline::updateExecutable()
{
    mState.mExecutable.reset();

    // Vertex Shader ProgramExecutable properties
    updateExecutableAttributes();

    // All Shader ProgramExecutable properties
    updateExecutableTextures();
}

ProgramMergedVaryings ProgramPipeline::getMergedVaryings() const
{
    ASSERT(!mState.mExecutable.isCompute());

    // Varyings are matched between pairs of consecutive stages, by location if assigned or
    // by name otherwise.  Note that it's possible for one stage to specify location and the other
    // not: https://cvs.khronos.org/bugzilla/show_bug.cgi?id=16261

    // Map stages to the previous active stage in the rendering pipeline.  When looking at input
    // varyings of a stage, this is used to find the stage whose output varyings are being linked
    // with them.
    ShaderMap<ShaderType> previousActiveStage;

    // Note that kAllGraphicsShaderTypes is sorted according to the rendering pipeline.
    ShaderType lastActiveStage = ShaderType::InvalidEnum;
    for (ShaderType shaderType : getExecutable().getLinkedShaderStages())
    {
        previousActiveStage[shaderType] = lastActiveStage;

        const Program *program = getShaderProgram(shaderType);
        ASSERT(program);
        lastActiveStage = shaderType;
    }

    // First, go through output varyings and create two maps (one by name, one by location) for
    // faster lookup when matching input varyings.

    ShaderMap<std::map<std::string, size_t>> outputVaryingNameToIndexShaderMap;
    ShaderMap<std::map<int, size_t>> outputVaryingLocationToIndexShaderMap;

    ProgramMergedVaryings merged;

    // Gather output varyings.
    for (ShaderType shaderType : getExecutable().getLinkedShaderStages())
    {
        const Program *program = getShaderProgram(shaderType);
        ASSERT(program);
        Shader *shader = program->getState().getAttachedShader(shaderType);
        ASSERT(shader);

        for (const sh::ShaderVariable &varying : shader->getOutputVaryings())
        {
            merged.push_back({});
            ProgramVaryingRef *ref = &merged.back();

            ref->frontShader      = &varying;
            ref->frontShaderStage = shaderType;

            // Always map by name.  Even if location is provided in this stage, it may not be in the
            // paired stage.
            outputVaryingNameToIndexShaderMap[shaderType][varying.name] = merged.size() - 1;

            // If location is provided, also keep it in a map by location.
            if (varying.location != -1)
            {
                outputVaryingLocationToIndexShaderMap[shaderType][varying.location] =
                    merged.size() - 1;
            }
        }
    }

    // Gather input varyings, and match them with output varyings of the previous stage.
    for (ShaderType shaderType : getExecutable().getLinkedShaderStages())
    {
        const Program *program = getShaderProgram(shaderType);
        ASSERT(program);
        Shader *shader = program->getState().getAttachedShader(shaderType);
        ASSERT(shader);
        ShaderType previousStage = previousActiveStage[shaderType];

        for (const sh::ShaderVariable &varying : shader->getInputVaryings())
        {
            size_t mergedIndex = merged.size();
            if (previousStage != ShaderType::InvalidEnum)
            {
                // If location is provided, see if we can match by location.
                if (varying.location != -1)
                {
                    std::map<int, size_t> outputVaryingLocationToIndex =
                        outputVaryingLocationToIndexShaderMap[previousStage];
                    auto byLocationIter = outputVaryingLocationToIndex.find(varying.location);
                    if (byLocationIter != outputVaryingLocationToIndex.end())
                    {
                        mergedIndex = byLocationIter->second;
                    }
                }

                // If not found, try to match by name.
                if (mergedIndex == merged.size())
                {
                    std::map<std::string, size_t> outputVaryingNameToIndex =
                        outputVaryingNameToIndexShaderMap[previousStage];
                    auto byNameIter = outputVaryingNameToIndex.find(varying.name);
                    if (byNameIter != outputVaryingNameToIndex.end())
                    {
                        mergedIndex = byNameIter->second;
                    }
                }
            }

            // If no previous stage, or not matched by location or name, create a new entry for it.
            if (mergedIndex == merged.size())
            {
                merged.push_back({});
                mergedIndex = merged.size() - 1;
            }

            ProgramVaryingRef *ref = &merged[mergedIndex];

            ref->backShader      = &varying;
            ref->backShaderStage = shaderType;
        }
    }

    return merged;
}

// The attached shaders are checked for linking errors by matching up their variables.
// Uniform, input and output variables get collected.
// The code gets compiled into binaries.
angle::Result ProgramPipeline::link(const Context *context)
{
    if (!getExecutable().isCompute())
    {
        InfoLog &infoLog = mState.mExecutable.getInfoLog();
        infoLog.reset();
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

        if (!linkVaryings(infoLog))
        {
            return angle::Result::Stop;
        }

        if (!mState.mExecutable.linkValidateGlobalNames(infoLog))
        {
            return angle::Result::Stop;
        }

        GLuint maxVaryingVectors =
            static_cast<GLuint>(context->getState().getCaps().maxVaryingVectors);
        VaryingPacking varyingPacking(maxVaryingVectors, packMode);

        const ProgramMergedVaryings &mergedVaryings = getMergedVaryings();
        for (ShaderType shaderType : getExecutable().getLinkedShaderStages())
        {
            Program *program = mState.mPrograms[shaderType];
            ASSERT(program);
            program->getResources().varyingPacking.reset();
            ANGLE_TRY(program->linkMergedVaryings(context, mergedVaryings));
        }
    }

    ANGLE_TRY(getImplementation()->link(context));

    return angle::Result::Continue;
}

bool ProgramPipeline::linkVaryings(InfoLog &infoLog) const
{
    Shader *previousShader = nullptr;
    for (ShaderType shaderType : getExecutable().getLinkedShaderStages())
    {
        Program *currentProgram = mState.mPrograms[shaderType];
        ASSERT(currentProgram);

        Shader *currentShader =
            const_cast<Shader *>(currentProgram->getState().getAttachedShader(shaderType));
        if (!currentShader)
        {
            continue;
        }

        if (previousShader)
        {
            if (!Program::linkValidateShaderInterfaceMatching(
                    previousShader, currentShader, currentProgram->isSeparable(), infoLog))
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
    return Program::linkValidateBuiltInVaryings(vertexShader, fragmentShader, infoLog);
}

void ProgramPipeline::validate(const gl::Context *context)
{
    const Caps &caps = context->getCaps();
    mState.mValid    = true;
    InfoLog &infoLog = mState.mExecutable.getInfoLog();
    infoLog.reset();

    for (const ShaderType shaderType : mState.mExecutable.getLinkedShaderStages())
    {
        Program *shaderProgram = mState.mPrograms[shaderType];
        if (shaderProgram)
        {
            shaderProgram->resolveLink(context);
            shaderProgram->validate(caps);
            std::string shaderInfoString = shaderProgram->getExecutable().getInfoLogString();
            if (shaderInfoString.length())
            {
                mState.mValid = false;
                infoLog << shaderInfoString << "\n";
                return;
            }
            if (!shaderProgram->isSeparable())
            {
                mState.mValid = false;
                infoLog << GetShaderTypeString(shaderType) << " is not marked separable."
                        << "\n";
                return;
            }
        }
    }

    if (!linkVaryings(infoLog))
    {
        mState.mValid = false;

        for (const ShaderType shaderType : mState.mExecutable.getLinkedShaderStages())
        {
            Program *shaderProgram = mState.mPrograms[shaderType];
            if (shaderProgram)
            {
                shaderProgram->validate(caps);
                std::string shaderInfoString = shaderProgram->getExecutable().getInfoLogString();
                if (shaderInfoString.length())
                {
                    infoLog << shaderInfoString << "\n";
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

angle::Result ProgramPipeline::syncState(const Context *context)
{
    if (mDirtyBits.any())
    {
        for (const ShaderType shaderType : mState.mExecutable.getLinkedShaderStages())
        {
            Program *shaderProgram = mState.mPrograms[shaderType];
            ANGLE_TRY(shaderProgram->syncState(context));
        }

        mDirtyBits.reset();
    }

    return angle::Result::Continue;
}

angle::Result ProgramPipeline::prepareForDraw(Context *context)
{
    bool goodResult = link(context) == angle::Result::Continue;
    ANGLE_CHECK(context, goodResult, "Program pipeline link failed", GL_INVALID_OPERATION);

    return angle::Result::Continue;
}

angle::Result ProgramPipeline::prepareForDispatch(Context *context)
{
    // Same work as for Draw.
    return prepareForDraw(context);
}

}  // namespace gl
