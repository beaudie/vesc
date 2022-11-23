//
// Created by yuxinhu on 11/30/22.
//

#include "Context.h"
#include "Program.h"
#include "capture/FrameCapture.h"
#include "renderer/ProgramImpl.h"

namespace gl
{

void WriteBufferVariable(BinaryOutputStream *stream, const BufferVariable &var)
{
    WriteShaderVar(stream, var);

    stream->writeInt(var.bufferIndex);
    WriteBlockMemberInfo(stream, var.blockInfo);
    stream->writeInt(var.topLevelArraySize);

    for (ShaderType shaderType : AllShaderTypes())
    {
        stream->writeBool(var.isActive(shaderType));
    }
}

void LoadBufferVariable(BinaryInputStream *stream, BufferVariable *var)
{
    LoadShaderVar(stream, var);

    var->bufferIndex = stream->readInt<int>();
    LoadBlockMemberInfo(stream, &var->blockInfo);
    var->topLevelArraySize = stream->readInt<int>();

    for (ShaderType shaderType : AllShaderTypes())
    {
        var->setActive(shaderType, stream->readBool());
    }
}

angle::Result Program::serialize(const Context *context, angle::MemoryBuffer *binaryOut) const
{
    BinaryOutputStream stream;

    stream.writeInt(Context::ANGLE_PROGRAM_VERSION);

    // nullptr context is supported when computing binary length.
    if (context)
    {
        stream.writeInt(context->getClientVersion().major);
        stream.writeInt(context->getClientVersion().minor);
    }
    else
    {
        stream.writeInt(2);
        stream.writeInt(0);
    }

    // Must be before mExecutable->save(), since it uses the value.
    stream.writeBool(mState.mProgramStateData.mSeparable);

    mState.mExecutable->save(mState.mProgramStateData.mSeparable, &stream);

    const auto &computeLocalSize = mState.mProgramStateData.mComputeShaderLocalSize;

    stream.writeInt(computeLocalSize[0]);
    stream.writeInt(computeLocalSize[1]);
    stream.writeInt(computeLocalSize[2]);

    stream.writeInt(mState.mProgramStateData.mNumViews);
    stream.writeInt(mState.mProgramStateData.mSpecConstUsageBits.bits());

    stream.writeInt(mState.mProgramStateData.mUniformLocations.size());
    for (const auto &variable : mState.mProgramStateData.mUniformLocations)
    {
        stream.writeInt(variable.arrayIndex);
        stream.writeIntOrNegOne(variable.index);
        stream.writeBool(variable.ignored);
    }

    stream.writeInt(mState.mProgramStateData.mBufferVariables.size());
    for (const BufferVariable &bufferVariable : mState.mProgramStateData.mBufferVariables)
    {
        WriteBufferVariable(&stream, bufferVariable);
    }

    // Warn the app layer if saving a binary with unsupported transform feedback.
    if (!mState.getLinkedTransformFeedbackVaryings().empty() &&
        context->getFrontendFeatures().disableProgramCachingForTransformFeedback.enabled)
    {
        ANGLE_PERF_WARNING(context->getState().getDebug(), GL_DEBUG_SEVERITY_LOW,
                           "Saving program binary with transform feedback, which is not supported "
                           "on this driver.");
    }

    if (context->getShareGroup()->getFrameCaptureShared()->enabled())
    {
        // Serialize the source for each stage for re-use during capture
        for (ShaderType shaderType : mState.mExecutable->getLinkedShaderStages())
        {
            gl::Shader *shader = getAttachedShader(shaderType);
            if (shader)
            {
                stream.writeString(shader->getSourceString());
            }
            else
            {
                // If we don't have an attached shader, which would occur if this program was
                // created via glProgramBinary, pull from our cached copy
                const angle::ProgramSources &cachedLinkedSources =
                    context->getShareGroup()->getFrameCaptureShared()->getProgramSources(id());
                const std::string &cachedSourceString = cachedLinkedSources[shaderType];
                ASSERT(!cachedSourceString.empty());
                stream.writeString(cachedSourceString.c_str());
            }
        }
    }

    mProgram->save(context, &stream);

    ASSERT(binaryOut);
    if (!binaryOut->resize(stream.length()))
    {
        std::stringstream sstream;
        sstream << "Failed to allocate enough memory to serialize a program. (" << stream.length()
                << " bytes )";
        ANGLE_PERF_WARNING(context->getState().getDebug(), GL_DEBUG_SEVERITY_LOW,
                           sstream.str().c_str());
        return angle::Result::Incomplete;
    }
    memcpy(binaryOut->data(), stream.data(), stream.length());
    return angle::Result::Continue;
}

angle::Result Program::deserialize(const Context *context,
                                   BinaryInputStream &stream,
                                   InfoLog &infoLog)
{
    int angleProgramVersion = stream.readInt<int>();
    if (angleProgramVersion != Context::ANGLE_PROGRAM_VERSION)
    {
        infoLog << "Invalid program binary version.";
        return angle::Result::Stop;
    }

    int majorVersion = stream.readInt<int>();
    int minorVersion = stream.readInt<int>();
    if (majorVersion != context->getClientMajorVersion() ||
        minorVersion != context->getClientMinorVersion())
    {
        infoLog << "Cannot load program binaries across different ES context versions.";
        return angle::Result::Stop;
    }

    // Must be before mExecutable->load(), since it uses the value.
    mState.mProgramStateData.mSeparable = stream.readBool();

    mState.mExecutable->load(mState.mProgramStateData.mSeparable, &stream);

    mState.mProgramStateData.mComputeShaderLocalSize[0] = stream.readInt<int>();
    mState.mProgramStateData.mComputeShaderLocalSize[1] = stream.readInt<int>();
    mState.mProgramStateData.mComputeShaderLocalSize[2] = stream.readInt<int>();

    mState.mProgramStateData.mNumViews = stream.readInt<int>();

    static_assert(sizeof(mState.mProgramStateData.mSpecConstUsageBits.bits()) == sizeof(uint32_t));
    mState.mProgramStateData.mSpecConstUsageBits =
        rx::SpecConstUsageBits(stream.readInt<uint32_t>());

    const size_t uniformIndexCount = stream.readInt<size_t>();
    ASSERT(mState.mProgramStateData.mUniformLocations.empty());
    for (size_t uniformIndexIndex = 0; uniformIndexIndex < uniformIndexCount; ++uniformIndexIndex)
    {
        VariableLocation variable;
        stream.readInt(&variable.arrayIndex);
        stream.readInt(&variable.index);
        stream.readBool(&variable.ignored);

        mState.mProgramStateData.mUniformLocations.push_back(variable);
    }

    size_t bufferVariableCount = stream.readInt<size_t>();
    ASSERT(mState.mProgramStateData.mBufferVariables.empty());
    for (size_t bufferVarIndex = 0; bufferVarIndex < bufferVariableCount; ++bufferVarIndex)
    {
        BufferVariable bufferVariable;
        LoadBufferVariable(&stream, &bufferVariable);
        mState.mProgramStateData.mBufferVariables.push_back(bufferVariable);
    }

    static_assert(static_cast<unsigned long>(ShaderType::EnumCount) <= sizeof(unsigned long) * 8,
                  "Too many shader types");

    // Reject programs that use transform feedback varyings if the hardware cannot support them.
    if (mState.mExecutable->getLinkedTransformFeedbackVaryings().size() > 0 &&
        context->getFrontendFeatures().disableProgramCachingForTransformFeedback.enabled)
    {
        infoLog << "Current driver does not support transform feedback in binary programs.";
        return angle::Result::Stop;
    }

    if (!mState.mAttachedShaders[ShaderType::Compute])
    {
        mState.mExecutable->updateTransformFeedbackStrides();
    }

    postResolveLink(context);
    mState.mExecutable->updateCanDrawWith();

    if (context->getShareGroup()->getFrameCaptureShared()->enabled())
    {
        // Extract the source for each stage from the program binary
        angle::ProgramSources sources;

        for (ShaderType shaderType : mState.mExecutable->getLinkedShaderStages())
        {
            std::string shaderSource = stream.readString();
            ASSERT(shaderSource.length() > 0);
            sources[shaderType] = std::move(shaderSource);
        }

        // Store it for use during mid-execution capture
        context->getShareGroup()->getFrameCaptureShared()->setProgramSources(id(),
                                                                             std::move(sources));
    }

    return angle::Result::Continue;
}
}  // namespace gl
