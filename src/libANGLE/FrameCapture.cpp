//
// Copyright 2019 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//
// FrameCapture.cpp:
//   ANGLE Frame capture implementation.
//

#include "libANGLE/FrameCapture.h"

#include <string>

#include "libANGLE/Context.h"
#include "libANGLE/Framebuffer.h"
#include "libANGLE/Shader.h"
#include "libANGLE/VertexArray.h"
#include "libANGLE/capture_gles_2_0_autogen.h"

namespace angle
{
#if !ANGLE_CAPTURE_ENABLED
CallCapture::~CallCapture() {}
ParamBuffer::~ParamBuffer() {}
ParamCapture::~ParamCapture() {}

FrameCapture::FrameCapture() {}
FrameCapture::~FrameCapture() {}
void FrameCapture::onEndFrame(const gl::Context *context) {}
#else
namespace
{
std::string GetCaptureFileName(size_t frameIndex, const char *suffix)
{
    std::stringstream fnameStream;
    fnameStream << "angle_capture_frame" << std::setfill('0') << std::setw(3) << frameIndex
                << suffix;
    return fnameStream.str();
}

void WriteParamStaticVarName(const CallCapture &call,
                             const ParamCapture &param,
                             int counter,
                             std::ostream &out)
{
    out << call.name() << "_" << param.name << "_" << counter;
}

template <typename T, typename CastT = T>
void WriteInlineData(const std::vector<uint8_t> &vec, std::ostream &out)
{
    const T *data = reinterpret_cast<const T *>(vec.data());
    size_t count  = vec.size() / sizeof(T);

    out << static_cast<CastT>(data[0]);

    for (size_t dataIndex = 1; dataIndex < count; ++dataIndex)
    {
        out << ", " << static_cast<CastT>(data[dataIndex]);
    }
}

constexpr size_t kInlineDataThreshold = 128;

void WriteStringParamReplay(std::ostream &out, const ParamCapture &param)
{
    const std::vector<uint8_t> &data = param.data[0];
    std::string str(data.begin(), data.end());
    out << "\"" << str << "\"";
}

ProgramSources GetAttachedProgramSources(const gl::Program *program)
{
    ProgramSources sources;
    for (gl::ShaderType shaderType : gl::AllShaderTypes())
    {
        const gl::Shader *shader = program->getAttachedShader(shaderType);
        if (shader)
        {
            sources[shaderType] = shader->getSourceString();
        }
    }
    return sources;
}
}  // anonymous namespace

ParamCapture::ParamCapture() : type(ParamType::TGLenum) {}

ParamCapture::ParamCapture(const char *nameIn, ParamType typeIn) : name(nameIn), type(typeIn) {}

ParamCapture::~ParamCapture() = default;

ParamCapture::ParamCapture(ParamCapture &&other) : type(ParamType::TGLenum)
{
    *this = std::move(other);
}

ParamCapture &ParamCapture::operator=(ParamCapture &&other)
{
    std::swap(name, other.name);
    std::swap(type, other.type);
    std::swap(value, other.value);
    std::swap(data, other.data);
    std::swap(arrayClientPointerIndex, other.arrayClientPointerIndex);
    std::swap(readBufferSize, other.readBufferSize);
    return *this;
}

ParamBuffer::ParamBuffer() {}

ParamBuffer::~ParamBuffer() = default;

ParamBuffer::ParamBuffer(ParamBuffer &&other)
{
    *this = std::move(other);
}

ParamBuffer &ParamBuffer::operator=(ParamBuffer &&other)
{
    std::swap(mParamCaptures, other.mParamCaptures);
    std::swap(mClientArrayDataParam, other.mClientArrayDataParam);
    std::swap(mReadBufferSize, other.mReadBufferSize);
    return *this;
}

ParamCapture &ParamBuffer::getParam(const char *paramName, ParamType paramType, int index)
{
    ParamCapture &capture = mParamCaptures[index];
    ASSERT(capture.name == paramName);
    ASSERT(capture.type == paramType);
    return capture;
}

const ParamCapture &ParamBuffer::getParam(const char *paramName,
                                          ParamType paramType,
                                          int index) const
{
    return const_cast<ParamBuffer *>(this)->getParam(paramName, paramType, index);
}

void ParamBuffer::addParam(ParamCapture &&param)
{
    if (param.arrayClientPointerIndex != -1)
    {
        ASSERT(mClientArrayDataParam == -1);
        mClientArrayDataParam = static_cast<int>(mParamCaptures.size());
    }

    mReadBufferSize = std::max(param.readBufferSize, mReadBufferSize);
    mParamCaptures.emplace_back(std::move(param));
}

void ParamBuffer::addReturnValue(ParamCapture &&returnValue)
{
    mReturnValueCapture = std::move(returnValue);
}

ParamCapture &ParamBuffer::getClientArrayPointerParameter()
{
    ASSERT(hasClientArrayData());
    return mParamCaptures[mClientArrayDataParam];
}

CallCapture::CallCapture(gl::EntryPoint entryPointIn, ParamBuffer &&paramsIn)
    : entryPoint(entryPointIn), params(std::move(paramsIn))
{}

CallCapture::CallCapture(const std::string &customFunctionNameIn, ParamBuffer &&paramsIn)
    : entryPoint(gl::EntryPoint::Invalid),
      customFunctionName(customFunctionNameIn),
      params(std::move(paramsIn))
{}

CallCapture::~CallCapture() = default;

CallCapture::CallCapture(CallCapture &&other)
{
    *this = std::move(other);
}

CallCapture &CallCapture::operator=(CallCapture &&other)
{
    std::swap(entryPoint, other.entryPoint);
    std::swap(customFunctionName, other.customFunctionName);
    std::swap(params, other.params);
    return *this;
}

const char *CallCapture::name() const
{
    if (entryPoint == gl::EntryPoint::Invalid)
    {
        ASSERT(!customFunctionName.empty());
        return customFunctionName.c_str();
    }

    return gl::GetEntryPointName(entryPoint);
}

FrameCapture::FrameCapture() : mFrameIndex(0), mReadBufferSize(0)
{
    reset();
}

FrameCapture::~FrameCapture() = default;

void FrameCapture::maybeCaptureClientData(const gl::Context *context, const CallCapture &call)
{
    switch (call.entryPoint)
    {
        case gl::EntryPoint::VertexAttribPointer:
        {
            // Get array location
            GLuint index = call.params.getParam("index", ParamType::TGLuint, 0).value.GLuintVal;

            if (call.params.hasClientArrayData())
            {
                mClientVertexArrayMap[index] = mFrameCalls.size();
            }
            else
            {
                mClientVertexArrayMap[index] = -1;
            }
            break;
        }

        case gl::EntryPoint::DrawArrays:
        {
            if (context->getStateCache().hasAnyActiveClientAttrib())
            {
                // Get counts from paramBuffer.
                GLint firstVertex =
                    call.params.getParam("first", ParamType::TGLint, 1).value.GLintVal;
                GLsizei drawCount =
                    call.params.getParam("count", ParamType::TGLsizei, 2).value.GLsizeiVal;
                captureClientArraySnapshot(context, firstVertex + drawCount, 1);
            }
            break;
        }

        case gl::EntryPoint::DrawElements:
        {
            if (context->getStateCache().hasAnyActiveClientAttrib())
            {
                GLsizei count =
                    call.params.getParam("count", ParamType::TGLsizei, 1).value.GLsizeiVal;
                gl::DrawElementsType drawElementsType =
                    call.params.getParam("typePacked", ParamType::TDrawElementsType, 2)
                        .value.DrawElementsTypeVal;
                const void *indices =
                    call.params.getParam("indices", ParamType::TvoidConstPointer, 3)
                        .value.voidConstPointerVal;

                gl::IndexRange indexRange;

                bool restart = context->getState().isPrimitiveRestartEnabled();

                gl::Buffer *elementArrayBuffer =
                    context->getState().getVertexArray()->getElementArrayBuffer();
                if (elementArrayBuffer)
                {
                    size_t offset = reinterpret_cast<size_t>(indices);
                    (void)elementArrayBuffer->getIndexRange(context, drawElementsType, offset,
                                                            count, restart, &indexRange);
                }
                else
                {
                    indexRange = gl::ComputeIndexRange(drawElementsType, indices, count, restart);
                }

                captureClientArraySnapshot(context, indexRange.end, 1);
            }
            break;
        }

        case gl::EntryPoint::CompileShader:
        {
            // Refresh the cached shader sources.
            GLuint shaderHandle =
                call.params.getParam("shader", ParamType::TGLuint, 0).value.GLuintVal;
            const gl::Shader *shader           = context->getShader(shaderHandle);
            mCachedShaderSources[shaderHandle] = shader->getSourceString();
            break;
        }

        case gl::EntryPoint::LinkProgram:
        {
            // Refresh the cached program sources.
            GLuint programHandle =
                call.params.getParam("program", ParamType::TGLuint, 0).value.GLuintVal;
            const gl::Program *program           = context->getProgramResolveLink(programHandle);
            mCachedProgramSources[programHandle] = GetAttachedProgramSources(program);
            break;
        }
        
        default:
            break;
    }
}

void FrameCapture::captureCall(const gl::Context *context, CallCapture &&call)
{
    // Process client data snapshots.
    maybeCaptureClientData(context, call);

    mReadBufferSize = std::max(mReadBufferSize, call.params.getReadBufferSize());
    mFrameCalls.emplace_back(std::move(call));

    // Process resource ID updates.
    maybeUpdateResourceIDs(context, mFrameCalls.back());
}

void FrameCapture::maybeUpdateResourceIDs(const gl::Context *context, const CallCapture &call)
{
    switch (call.entryPoint)
    {
        case gl::EntryPoint::GenRenderbuffers:
        case gl::EntryPoint::GenRenderbuffersOES:
        {
            GLsizei n = call.params.getParam("n", ParamType::TGLsizei, 0).value.GLsizeiVal;
            const ParamCapture &renderbuffers =
                call.params.getParam("renderbuffersPacked", ParamType::TRenderbufferIDPointer, 1);
            ASSERT(renderbuffers.data.size() == 1);
            const gl::RenderbufferID *returnedIDs =
                reinterpret_cast<const gl::RenderbufferID *>(renderbuffers.data[0].data());

            for (GLsizei idIndex = 0; idIndex < n; ++idIndex)
            {
                gl::RenderbufferID id    = returnedIDs[idIndex];
                GLsizei readBufferOffset = idIndex * sizeof(gl::RenderbufferID);
                ParamBuffer params;
                params.addValueParam("id", ParamType::TGLuint, id.value);
                params.addValueParam("readBufferOffset", ParamType::TGLsizei, readBufferOffset);
                mFrameCalls.emplace_back("UpdateRenderbufferID", std::move(params));
            }
            break;
        }

        default:
            break;
    }
}

void FrameCapture::captureClientArraySnapshot(const gl::Context *context,
                                              size_t vertexCount,
                                              size_t instanceCount)
{
    const gl::VertexArray *vao = context->getState().getVertexArray();

    // Capture client array data.
    for (size_t attribIndex : context->getStateCache().getActiveClientAttribsMask())
    {
        const gl::VertexAttribute &attrib = vao->getVertexAttribute(attribIndex);
        const gl::VertexBinding &binding  = vao->getVertexBinding(attrib.bindingIndex);

        int callIndex = mClientVertexArrayMap[attribIndex];

        if (callIndex != -1)
        {
            size_t count = vertexCount;

            if (binding.getDivisor() > 0)
            {
                count = rx::UnsignedCeilDivide(instanceCount, binding.getDivisor());
            }

            // The last capture element doesn't take up the full stride.
            size_t bytesToCapture = (count - 1) * binding.getStride() + attrib.format->pixelBytes;

            CallCapture &call   = mFrameCalls[callIndex];
            ParamCapture &param = call.params.getClientArrayPointerParameter();
            ASSERT(param.type == ParamType::TvoidConstPointer);

            ParamBuffer updateParamBuffer;
            updateParamBuffer.addValueParam<GLint>("arrayIndex", ParamType::TGLint, attribIndex);

            ParamCapture updateMemory("pointer", ParamType::TvoidConstPointer);
            CaptureMemory(param.value.voidConstPointerVal, bytesToCapture, &updateMemory);
            updateParamBuffer.addParam(std::move(updateMemory));

            mFrameCalls.emplace_back("UpdateClientArrayPointer", std::move(updateParamBuffer));

            mClientArraySizes[attribIndex] =
                std::max(mClientArraySizes[attribIndex], bytesToCapture);
        }
    }
}

bool FrameCapture::anyClientArray() const
{
    for (size_t size : mClientArraySizes)
    {
        if (size > 0)
            return true;
    }

    return false;
}

void FrameCapture::onEndFrame(const gl::Context *context)
{
    if (!mFrameCalls.empty())
    {
        saveCapturedFrameAsCpp();
    }

    reset();
    mFrameIndex++;

    if (enabled())
    {
        captureMidExecutionSetup(context);
    }
}

void FrameCapture::saveCapturedFrameAsCpp()
{
    bool useClientArrays = anyClientArray();

    std::stringstream out;
    std::stringstream header;
    std::vector<uint8_t> binaryData;

    header << "#include \"util/gles_loader_autogen.h\"\n";
    header << "\n";
    header << "#include <cstdio>\n";
    header << "#include <vector>\n";
    header << "#include <unordered_map>\n";
    header << "\n";
    header << "namespace\n";
    header << "{\n";
    if (mReadBufferSize > 0)
    {
        header << "std::vector<uint8_t> gReadBuffer;\n";
    }
    if (useClientArrays)
    {
        header << "std::vector<uint8_t> gClientArrays[" << gl::MAX_VERTEX_ATTRIBS << "];\n";
        header << "template <size_t N>\n";
        header << "void UpdateClientArrayPointer(int arrayIndex, const uint8_t (&data)[N])\n";
        header << "{\n";
        header << "    memcpy(gClientArrays[arrayIndex].data(), data, N);\n";
        header << "}\n";
    }

    header << "std::unordered_map<GLuint, GLuint> gRenderbufferMap;\n";
    header << "void UpdateRenderbufferID(GLuint id, GLsizei readBufferOffset)\n";
    header << "{\n";
    header << "    GLuint returnedID;\n";
    header << "    memcpy(&returnedID, &gReadBuffer[readBufferOffset], sizeof(GLuint));\n";
    header << "    gRenderbufferMap[id] = returnedID;\n";
    header << "}\n";

    out << "void ReplayFrame" << mFrameIndex << "()\n";
    out << "{\n";
    out << "    LoadBinaryData();\n";

    for (size_t arrayIndex = 0; arrayIndex < mClientArraySizes.size(); ++arrayIndex)
    {
        if (mClientArraySizes[arrayIndex] > 0)
        {
            out << "    gClientArrays[" << arrayIndex << "].resize("
                << mClientArraySizes[arrayIndex] << ");\n";
        }
    }

    if (mReadBufferSize > 0)
    {
        out << "    gReadBuffer.resize(" << mReadBufferSize << ");\n";
    }

    for (const CallCapture &call : mFrameCalls)
    {
        out << "    ";
        writeCallReplay(call, out, header, &binaryData);
        out << ";\n";
    }

    if (!binaryData.empty())
    {
        std::string fname = GetCaptureFileName(mFrameIndex, ".angledata");

        FILE *fp = fopen(fname.c_str(), "wb");
        fwrite(binaryData.data(), 1, binaryData.size(), fp);
        fclose(fp);

        header << "std::vector<uint8_t> gBinaryData;\n";
        header << "void LoadBinaryData()\n";
        header << "{\n";
        header << "    gBinaryData.resize(" << static_cast<int>(binaryData.size()) << ");\n";
        header << "    FILE *fp = fopen(\"" << fname << "\", \"rb\");\n";
        header << "    fread(gBinaryData.data(), 1, " << static_cast<int>(binaryData.size())
               << ", fp);\n";
        header << "    fclose(fp);\n";
        header << "}\n";
    }
    else
    {
        header << "// No binary data.\n";
        header << "void LoadBinaryData() {}\n";
    }

    out << "}\n";

    header << "}  // anonymous namespace\n";

    std::string outString    = out.str();
    std::string headerString = header.str();

    std::string fname = GetCaptureFileName(mFrameIndex, ".cpp");
    FILE *fp          = fopen(fname.c_str(), "w");
    fprintf(fp, "%s\n\n%s", headerString.c_str(), outString.c_str());
    fclose(fp);

    printf("Saved '%s'.\n", fname.c_str());
}

int FrameCapture::getAndIncrementCounter(gl::EntryPoint entryPoint, const std::string &paramName)
{
    auto counterKey = std::tie(entryPoint, paramName);
    return mDataCounters[counterKey]++;
}

void FrameCapture::writeStringPointerParamReplay(std::ostream &out,
                                                 std::ostream &header,
                                                 const CallCapture &call,
                                                 const ParamCapture &param)
{
    int counter = getAndIncrementCounter(call.entryPoint, param.name);

    header << "const char *";
    WriteParamStaticVarName(call, param, counter, header);
    header << "[] = { \n";

    for (const std::vector<uint8_t> &data : param.data)
    {
        std::string str(data.begin(), data.end());
        header << "    R\"(" << str << ")\",\n";
    }

    header << " };\n";
    WriteParamStaticVarName(call, param, counter, out);
}

void FrameCapture::writeRenderbufferIDPointerParamReplay(std::ostream &out,
                                                         std::ostream &header,
                                                         const CallCapture &call,
                                                         const ParamCapture &param)
{
    int counter = getAndIncrementCounter(call.entryPoint, param.name);

    header << "const GLuint ";
    WriteParamStaticVarName(call, param, counter, header);
    header << "[] = { ";

    GLsizei n = call.params.getParam("n", ParamType::TGLsizei, 0).value.GLsizeiVal;
    const ParamCapture &renderbuffers =
        call.params.getParam("renderbuffersPacked", ParamType::TRenderbufferIDConstPointer, 1);
    ASSERT(renderbuffers.data.size() == 1);
    const gl::RenderbufferID *returnedIDs =
        reinterpret_cast<const gl::RenderbufferID *>(renderbuffers.data[0].data());
    for (GLsizei resIndex = 0; resIndex < n; ++resIndex)
    {
        gl::RenderbufferID id = returnedIDs[resIndex];
        if (resIndex > 0)
        {
            header << ", ";
        }
        header << "gRenderbufferMap[" << id.value << "]";
    }

    header << " };\n    ";

    WriteParamStaticVarName(call, param, counter, out);
}

void FrameCapture::writeBinaryParamReplay(std::ostream &out,
                                          std::ostream &header,
                                          const CallCapture &call,
                                          const ParamCapture &param,
                                          std::vector<uint8_t> *binaryData)
{
    int counter = getAndIncrementCounter(call.entryPoint, param.name);

    ASSERT(param.data.size() == 1);
    const std::vector<uint8_t> &data = param.data[0];

    if (data.size() > kInlineDataThreshold)
    {
        size_t offset = binaryData->size();
        binaryData->resize(offset + data.size());
        memcpy(binaryData->data() + offset, data.data(), data.size());
        out << "reinterpret_cast<" << ParamTypeToString(param.type) << ">(&gBinaryData[" << offset
            << "])";
    }
    else
    {
        ParamType overrideType = param.type;
        if (param.type == ParamType::TGLvoidConstPointer ||
            param.type == ParamType::TvoidConstPointer)
        {
            overrideType = ParamType::TGLubyteConstPointer;
        }

        std::string paramTypeString = ParamTypeToString(overrideType);
        header << paramTypeString.substr(0, paramTypeString.length() - 1);
        WriteParamStaticVarName(call, param, counter, header);

        header << "[] = { ";

        switch (overrideType)
        {
            case ParamType::TGLintConstPointer:
                WriteInlineData<GLint>(data, header);
                break;
            case ParamType::TGLshortConstPointer:
                WriteInlineData<GLshort>(data, header);
                break;
            case ParamType::TGLfloatConstPointer:
                WriteInlineData<GLfloat>(data, header);
                break;
            case ParamType::TGLubyteConstPointer:
                WriteInlineData<GLubyte, int>(data, header);
                break;
            case ParamType::TGLuintConstPointer:
            case ParamType::TGLenumConstPointer:
                WriteInlineData<GLuint>(data, header);
                break;
            default:
                UNIMPLEMENTED();
                break;
        }

        header << " };\n";

        WriteParamStaticVarName(call, param, counter, out);
    }
}

void FrameCapture::writeCallReplay(const CallCapture &call,
                                   std::ostream &out,
                                   std::ostream &header,
                                   std::vector<uint8_t> *binaryData)
{
    std::ostringstream callOut;

    callOut << call.name() << "(";

    bool first = true;
    for (const ParamCapture &param : call.params.getParamCaptures())
    {
        if (!first)
        {
            callOut << ", ";
        }

        if (param.arrayClientPointerIndex != -1)
        {
            callOut << "gClientArrays[" << param.arrayClientPointerIndex << "].data()";
        }
        else if (param.readBufferSize > 0)
        {
            callOut << "reinterpret_cast<" << ParamTypeToString(param.type)
                    << ">(gReadBuffer.data())";
        }
        else if (param.data.empty())
        {
            callOut << param;
        }
        else
        {
            switch (param.type)
            {
                case ParamType::TGLcharConstPointer:
                    WriteStringParamReplay(callOut, param);
                    break;
                case ParamType::TGLcharConstPointerPointer:
                    writeStringPointerParamReplay(callOut, header, call, param);
                    break;
                case ParamType::TRenderbufferIDConstPointer:
                    writeRenderbufferIDPointerParamReplay(callOut, out, call, param);
                    break;
                case ParamType::TRenderbufferIDPointer:
                    UNIMPLEMENTED();
                    break;
                default:
                    writeBinaryParamReplay(callOut, header, call, param, binaryData);
                    break;
            }
        }

        first = false;
    }

    callOut << ")";

    out << callOut.str();
}

bool FrameCapture::enabled() const
{
    return mFrameIndex < 20;
}

void FrameCapture::reset()
{
    mFrameCalls.clear();
    mClientVertexArrayMap.fill(-1);
    mClientArraySizes.fill(0);
    mDataCounters.clear();
    mReadBufferSize = 0;
}

void FrameCapture::captureMidExecutionSetup(const gl::Context *context)
{
    const gl::State &glState = context->getState();

    // Small helper function to make the code more readable.
    auto cap = [this](CallCapture &&call) { this->mSetupCalls.emplace_back(std::move(call)); };

    // Currently this code assumes we can use create-on-bind. It does not support 'Gen' usage.
    // TODO(jmadill): Use handle mapping for captured objects. http://anglebug.com/3662

    // Capture Buffer data.
    const gl::BufferManager &buffers       = glState.getBufferManagerForCapture();
    const gl::BoundBufferMap &boundBuffers = glState.getBoundBuffersForCapture();

    bool boundArrayBuffer = false;
    for (const auto &bufferIter : buffers)
    {
        GLuint handle      = bufferIter.first;
        gl::Buffer *buffer = bufferIter.second;

        if (handle == 0)
        {
            continue;
        }

        // glBufferData. Would possibly be better implemented using a getData impl method.
        // Saving buffers that are mapped during a swap is not yet handled.
        if (buffer->getSize() == 0)
        {
            continue;
        }
        ASSERT(!buffer->isMapped());
        (void)buffer->map(context, GL_MAP_READ_BIT);

        // Always use the array buffer binding point to upload data to keep things simple.
        cap(CaptureBindBuffer(context, true, gl::BufferBinding::Array, handle));

        boundArrayBuffer = true;

        cap(CaptureBufferData(context, true, gl::BufferBinding::Array, buffer->getSize(),
                              buffer->getMapPointer(), buffer->getUsage()));

        GLboolean dontCare;
        (void)buffer->unmap(context, &dontCare);
    }

    // Capture Buffer bindings.
    for (gl::BufferBinding binding : angle::AllEnums<gl::BufferBinding>())
    {
        gl::Buffer *buffer = boundBuffers[binding].get();
        GLuint bufferID    = buffer ? buffer->id() : 0;
        cap(CaptureBindBuffer(context, true, binding, bufferID));
    }

    // Capture Texture data.
    const gl::TextureManager &textures         = glState.getTextureManagerForCapture();
    const gl::TextureBindingMap &boundTextures = glState.getBoundTexturesForCapture();
    for (const auto &textureIter : textures)
    {
        GLuint handle              = textureIter.first;
        const gl::Texture *texture = textureIter.second;

        if (handle == 0)
        {
            continue;
        }

        // Bind the Texture temporarily.
        cap(CaptureBindTexture(context, true, texture->getType(), handle));

        // Iterate texture levels and layers.
        gl::ImageIndexIterator imageIter = gl::ImageIndexIterator::MakeGeneric(
            texture->getType(), 0, texture->getMaxLevel(), gl::ImageIndex::kEntireLevel,
            gl::ImageIndex::kEntireLevel);
        while (imageIter.hasNext())
        {
            gl::ImageIndex index = imageIter.next();

            const gl::ImageDesc &desc = texture->getTextureState().getImageDesc(index);

            // Assume 2D for now.
            ASSERT(!index.hasLayer());

            // Use back-end to read back pixel data.
            ASSERT(texture->canGetDataForCapture(index));
            angle::MemoryBuffer data;
            (void)texture->getDataForCapture(context, index, &data);

            const gl::InternalFormat &format = *desc.format.info;
            cap(CaptureTexImage2D(context, true, index.getTarget(), index.getLevelIndex(),
                                  format.internalFormat, desc.size.width, desc.size.height, 0,
                                  format.format, format.type, data.data()));
        }
    }

    // Set Texture bindings.
    for (gl::TextureType textureType : angle::AllEnums<gl::TextureType>())
    {
        const gl::TextureBindingVector bindings = boundTextures[textureType];
        for (size_t bindingIndex = 0; bindingIndex < bindings.size(); ++bindingIndex)
        {
            cap(CaptureActiveTexture(context, true, bindingIndex));
            GLuint textureID = bindings[bindingIndex].get() ? bindings[bindingIndex].id() : 0;
            cap(CaptureBindTexture(context, true, textureType, textureID));
        }
    }

    // Set active Texture.
    cap(CaptureActiveTexture(context, true, glState.getActiveSampler()));

    // Capture Renderbuffers.
    const gl::RenderbufferManager &renderbuffers = glState.getRenderbufferManagerForCapture();
    for (const auto &renderbufIter : renderbuffers)
    {
        GLuint handle = renderbufIter.first;

        cap(CaptureBindRenderbuffer(context, true, 1, {handle}));

        // TODO(jmadill): Capture contents. http://anglebug.com/3662
    }

    // Set Renderbuffer binding.
    cap(CaptureBindRenderbuffer(context, true, GL_RENDERBUFFER, {glState.getRenderbufferId()}));

    // Capture Framebuffers.
    const gl::FramebufferManager &framebuffers = glState.getFramebufferManagerForCapture();

    for (const auto &framebufferIter : framebuffers)
    {
        GLuint handle                      = framebufferIter.first;
        const gl::Framebuffer *framebuffer = framebufferIter.second;

        cap(CaptureBindFramebuffer(context, true, GL_FRAMEBUFFER, handle));

        // Color Attachments.
        for (const gl::FramebufferAttachment &colorAttachment : framebuffer->getColorAttachments())
        {
            if (!colorAttachment.isAttached())
            {
                continue;
            }

            GLuint resourceID = colorAttachment.getResource()->getId();

            // TODO(jmadill): Other texture types. http://anglebug.com/3662
            if (colorAttachment.type() == GL_TEXTURE_2D)
            {
                gl::ImageIndex index = colorAttachment.getTextureImageIndex();

                cap(CaptureFramebufferTexture2D(context, true, GL_FRAMEBUFFER,
                                                colorAttachment.getBinding(), index.getTarget(),
                                                resourceID, index.getLevelIndex()));
            }
            else
            {
                ASSERT(colorAttachment.type() == GL_RENDERBUFFER);
                cap(CaptureFramebufferRenderbuffer(context, true, GL_FRAMEBUFFER,
                                                   colorAttachment.getBinding(), GL_RENDERBUFFER,
                                                   {resourceID}));
            }
        }

        // TODO(jmadill): Draw buffer states. http://anglebug.com/3662
    }

    // Capture Shaders and Programs.
    const gl::ShaderProgramManager &shadersAndPrograms =
        glState.getShaderProgramManagerForCapture();
    const gl::ResourceMap<gl::Shader> &shaders = shadersAndPrograms.getShadersForCapture();

    // We create shaders in filled order. Then we delete unused shaders to replicate program gaps.
    // This would be better implemented by using a resource ID map.
    GLuint maxShader = shaders.maxElement();
    std::vector<GLuint> deleteShaders;

    for (GLuint shaderIndex = 0; shaderIndex < maxShader; ++shaderIndex)
    {
        // Record "filler" shader.
        if (!shaders.contains(shaderIndex))
        {
            deleteShaders.push_back(shaderIndex);
            continue;
        }

        gl::Shader *shader = shaders.query(shaderIndex);
        cap(CaptureCreateShader(context, true, shader->getType(), shaderIndex));

        std::string shaderSource  = shader->getSourceString();
        const char *sourcePointer = shaderSource.empty() ? nullptr : shaderSource.c_str();

        if (shader->isCompiled())
        {
            std::string capturedSource = mCachedShaderSources[shaderIndex];

            if (capturedSource != shaderSource)
            {
                ASSERT(!capturedSource.empty());
                sourcePointer = capturedSource.c_str();
            }

            cap(CaptureShaderSource(context, true, shaderIndex, 1, &sourcePointer, nullptr));
            cap(CaptureCompileShader(context, true, shaderIndex));
        }

        if (sourcePointer && (!shader->isCompiled() || sourcePointer != shaderSource.c_str()))
        {
            cap(CaptureShaderSource(context, true, shaderIndex, 1, &sourcePointer, nullptr));
        }
    }

    // Delete "filler" shaders.
    for (GLuint shader : deleteShaders)
    {
        cap(CaptureDeleteShader(context, true, shader));
    }

    // Use the same tricks for programs.
    const gl::ResourceMap<gl::Program> &programs = shadersAndPrograms.getProgramsForCapture();
    GLuint maxProgram                            = programs.maxElement();
    std::vector<GLuint> deletePrograms;

    for (GLuint programIndex = 0; programIndex < maxProgram; ++programIndex)
    {
        // Record "filler" program.
        if (!programs.contains(programIndex))
        {
            deletePrograms.push_back(programIndex);
            continue;
        }

        gl::Program *program = programs.query(programIndex);

        // Get last compiled shader source.
        ProgramSources linkedSources   = mCachedProgramSources[programIndex];
        ProgramSources attachedSources = GetAttachedProgramSources(program);

        // FIXME
        ASSERT(program->isLinked());

        // Compile with last linked sources.
        for (gl::ShaderType shaderType : gl::AllShaderTypes())
        {
            if (!program->hasLinkedShaderStage(shaderType))
            {
                continue;
            }

            ParamBuffer params;
            params.addValueParam<gl::ShaderType>("shaderType", ParamType::TShaderType, shaderType);

            const std::string &sourceString = linkedSources[shaderType];

            ParamCapture source("source", ParamType::TGLcharConstPointer);
            source.data.resize(sourceString.size());
            memcpy(source.data.data(), sourceString.c_str(), sourceString.size());
            params.addParam(std::move(source));

            cap(CallCapture("InitTemporaryShader", std::move(params)));

            // FIXME
            cap(CaptureAttachShader(context, true, programIndex, 0));
        }

        cap(CaptureLinkProgram(context, true, programIndex));

        for (gl::ShaderType shaderType : gl::AllShaderTypes())
        {
            ParamBuffer params;
            params.addValueParam<gl::ShaderType>("shaderType", ParamType::TShaderType, shaderType);
            cap(CallCapture("DeleteTemporaryShader", std::move(params)));
        }
    }

    for (GLuint deleteProgram : deletePrograms)
    {
        cap(CaptureDeleteProgram(context, true, deleteProgram));
    }
}

std::ostream &operator<<(std::ostream &os, const ParamCapture &capture)
{
    WriteParamTypeToStream(os, capture.type, capture.value);
    return os;
}

void CaptureMemory(const void *source, size_t size, ParamCapture *paramCapture)
{
    std::vector<uint8_t> data(size);
    memcpy(data.data(), source, size);
    paramCapture->data.emplace_back(std::move(data));
}

void CaptureString(const GLchar *str, ParamCapture *paramCapture)
{
    CaptureMemory(str, strlen(str), paramCapture);
}

template <>
void WriteParamValueToStream<ParamType::TGLboolean>(std::ostream &os, GLboolean value)
{
    switch (value)
    {
        case GL_TRUE:
            os << "GL_TRUE";
            break;
        case GL_FALSE:
            os << "GL_FALSE";
            break;
        default:
            os << "GL_INVALID_ENUM";
    }
}

template <>
void WriteParamValueToStream<ParamType::TvoidConstPointer>(std::ostream &os, const void *value)
{
    if (value == 0)
    {
        os << "nullptr";
    }
    else
    {
        os << "reinterpret_cast<const void *>("
           << static_cast<int>(reinterpret_cast<uintptr_t>(value)) << ")";
    }
}

template <>
void WriteParamValueToStream<ParamType::TGLDEBUGPROCKHR>(std::ostream &os, GLDEBUGPROCKHR value)
{}

template <>
void WriteParamValueToStream<ParamType::TGLDEBUGPROC>(std::ostream &os, GLDEBUGPROC value)
{}

template <>
void WriteParamValueToStream<ParamType::TRenderbufferID>(std::ostream &os, gl::RenderbufferID value)
{
    os << "gRenderbufferMap[" << value.value << "]";
}
#endif  // ANGLE_CAPTURE_ENABLED
}  // namespace angle
