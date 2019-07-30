
// Copyright 2019 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//
// FrameCapture.h:
//   ANGLE Frame capture inteface.
//

#ifndef LIBANGLE_FRAME_CAPTURE_H_
#define LIBANGLE_FRAME_CAPTURE_H_

#include "common/PackedEnums.h"
#include "libANGLE/Context.h"
#include "libANGLE/angletypes.h"
#include "libANGLE/entry_points_utils.h"
#include "libANGLE/frame_capture_utils_autogen.h"

#include <tuple>

namespace angle
{
struct ParamCapture : angle::NonCopyable
{
    ParamCapture();
    ParamCapture(const char *nameIn, ParamType typeIn);
    ~ParamCapture();

    ParamCapture(ParamCapture &&other);
    ParamCapture &operator=(ParamCapture &&other);

    std::string name;
    ParamType type;
    ParamValue value;
    std::vector<std::vector<uint8_t>> data;
    int arrayClientPointerIndex = -1;
    size_t readBufferSizeBytes  = 0;
};

class ParamBuffer final : angle::NonCopyable
{
  public:
    ParamBuffer();
    ~ParamBuffer();

    ParamBuffer(ParamBuffer &&other);
    ParamBuffer &operator=(ParamBuffer &&other);

    template <typename T>
    void addValueParam(const char *paramName, ParamType paramType, T paramValue);

    ParamCapture &getParam(const char *paramName, ParamType paramType, int index);
    const ParamCapture &getParam(const char *paramName, ParamType paramType, int index) const;
    const ParamCapture &getReturnValue() const { return mReturnValueCapture; }

    void addParam(ParamCapture &&param);
    void addReturnValue(ParamCapture &&returnValue);
    bool hasClientArrayData() const { return mClientArrayDataParam != -1; }
    ParamCapture &getClientArrayPointerParameter();
    size_t getReadBufferSize() const { return mReadBufferSize; }

    const std::vector<ParamCapture> &getParamCaptures() const { return mParamCaptures; }

  private:
    std::vector<ParamCapture> mParamCaptures;
    ParamCapture mReturnValueCapture;
    int mClientArrayDataParam = -1;
    size_t mReadBufferSize    = 0;
};

struct CallCapture
{
    CallCapture(gl::EntryPoint entryPointIn, ParamBuffer &&paramsIn);
    CallCapture(const std::string &customFunctionNameIn, ParamBuffer &&paramsIn);
    ~CallCapture();

    CallCapture(CallCapture &&other);
    CallCapture &operator=(CallCapture &&other);

    const char *name() const;

    gl::EntryPoint entryPoint;
    std::string customFunctionName;
    ParamBuffer params;
};

using ProgramSources = gl::ShaderMap<std::string>;

class FrameCapture final : angle::NonCopyable
{
  public:
    FrameCapture();
    ~FrameCapture();

    void captureCall(const gl::Context *context, CallCapture &&call);
    void onEndFrame(const gl::Context *context);
    bool enabled() const;

  private:
    // <CallName, ParamName>
    using Counter = std::tuple<gl::EntryPoint, std::string>;

    void captureClientArraySnapshot(const gl::Context *context,
                                    size_t vertexCount,
                                    size_t instanceCount);

    void writeCallReplay(const CallCapture &call,
                         std::ostream &out,
                         std::ostream &header,
                         std::vector<uint8_t> *binaryData);
    void reset();
    int getAndIncrementCounter(gl::EntryPoint entryPoint, const std::string &paramName);
    bool anyClientArray() const;
    void saveCapturedFrameAsCpp();
    void writeStringPointerParamReplay(std::ostream &out,
                                       std::ostream &header,
                                       const CallCapture &call,
                                       const ParamCapture &param);
    void writeRenderbufferIDPointerParamReplay(std::ostream &out,
                                               std::ostream &header,
                                               const CallCapture &call,
                                               const ParamCapture &param);
    void writeBinaryParamReplay(std::ostream &out,
                                std::ostream &header,
                                const CallCapture &call,
                                const ParamCapture &param,
                                std::vector<uint8_t> *binaryData);
    void maybeCaptureClientData(const gl::Context *context, const CallCapture &call);
    void maybeUpdateResourceIDs(const gl::Context *context, const CallCapture &call);
    void captureMidExecutionSetup(const gl::Context *context);

    std::vector<CallCapture> mSetupCalls;
    std::vector<CallCapture> mFrameCalls;
    std::vector<CallCapture> mTearDownCalls;
    gl::AttribArray<int> mClientVertexArrayMap;
    size_t mFrameIndex;
    gl::AttribArray<size_t> mClientArraySizes;
    std::map<Counter, int> mDataCounters;
    size_t mReadBufferSize;

    // Cache most recently compiled and linked sources.
    std::unordered_map<GLuint, std::string> mCachedShaderSources;
    std::unordered_map<GLuint, ProgramSources> mCachedProgramSources;
};

template <typename CaptureFuncT, typename... ArgsT>
void CaptureCallToFrameCapture(CaptureFuncT captureFunc,
                               bool isCallValid,
                               gl::Context *context,
                               ArgsT... captureParams)
{
    FrameCapture *frameCapture = context->getFrameCapture();
    if (!frameCapture->enabled())
        return;

    CallCapture call = captureFunc(context, isCallValid, captureParams...);
    frameCapture->captureCall(context, std::move(call));
}

template <typename T>
void ParamBuffer::addValueParam(const char *paramName, ParamType paramType, T paramValue)
{
    ParamCapture capture(paramName, paramType);
    InitParamValue(paramType, paramValue, &capture.value);
    mParamCaptures.emplace_back(std::move(capture));
}

std::ostream &operator<<(std::ostream &os, const ParamCapture &capture);

// Pointer capture helpers.
void CaptureMemory(const void *source, size_t size, ParamCapture *paramCapture);
void CaptureString(const GLchar *str, ParamCapture *paramCapture);

template <ParamType ParamT, typename T>
void WriteParamValueToStream(std::ostream &os, T value);

template <>
void WriteParamValueToStream<ParamType::TGLboolean>(std::ostream &os, GLboolean value);

template <>
void WriteParamValueToStream<ParamType::TvoidConstPointer>(std::ostream &os, const void *value);

template <>
void WriteParamValueToStream<ParamType::TGLDEBUGPROCKHR>(std::ostream &os, GLDEBUGPROCKHR value);

template <>
void WriteParamValueToStream<ParamType::TGLDEBUGPROC>(std::ostream &os, GLDEBUGPROC value);

template <>
void WriteParamValueToStream<ParamType::TBufferID>(std::ostream &os, gl::BufferID value);

template <>
void WriteParamValueToStream<ParamType::TRenderbufferID>(std::ostream &os,
                                                         gl::RenderbufferID value);

// General fallback for any unspecific type.
template <ParamType ParamT, typename T>
void WriteParamValueToStream(std::ostream &os, T value)
{
    os << value;
}
}  // namespace angle

#endif  // LIBANGLE_FRAME_CAPTURE_H_
