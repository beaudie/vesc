//
// Copyright 2022 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//
// trace_interpreter.h:
//   Parser and interpreter for the C-based replays.
//

#ifndef ANGLE_TRACE_INTERPRETER_H_
#define ANGLE_TRACE_INTERPRETER_H_

#include "common/frame_capture_utils.h"
#include "frame_capture_test_utils.h"

namespace angle
{
struct TraceShader
{
    std::vector<std::string> sources;
    std::vector<const char *> pointers;
};
using TraceShaderMap = std::map<std::string, TraceShader>;

class TraceInterpreter : public TraceReplayInterface
{
  public:
    TraceInterpreter(const TraceInfo &traceInfo, const char *testDataDir);
    ~TraceInterpreter() override;

    bool valid() const override;
    void setBinaryDataDir(const char *dataDir) override;
    void setBinaryDataDecompressCallback(DecompressCallback decompressCallback,
                                         DeleteCallback deleteCallback) override;
    void replayFrame(uint32_t frameIndex) override;
    void setupReplay() override;
    void resetReplay() override;
    void finishReplay() override;
    const char *getSerializedContextState(uint32_t frameIndex) override;
    void setValidateSerializedStateCallback(ValidateSerializedStateCallback callback) override;

  private:
    void runTraceFunction(const char *name) const;

    const TraceInfo &mTraceInfo;
    const char *mTestDataDir;
    TraceFunctionMap mTraceFunctions;
    TraceShaderMap mTraceShaders;
};

constexpr size_t kMaxTokenSize  = 100;
constexpr size_t kMaxParameters = 20;
using Token                     = char[kMaxTokenSize];

CallCapture ParseCallCapture(const Token &nameToken,
                             size_t numParamTokens,
                             const Token *paramTokens,
                             const TraceShaderMap &shaders);

template <typename T>
void PackParameter(ParamBuffer &params, const Token &token, const TraceShaderMap &shaders);

template <>
void PackParameter<uint32_t>(ParamBuffer &params,
                             const Token &token,
                             const TraceShaderMap &shaders);

template <>
void PackParameter<int32_t>(ParamBuffer &params, const Token &token, const TraceShaderMap &shaders);

template <>
void PackParameter<void *>(ParamBuffer &params, const Token &token, const TraceShaderMap &shaders);

template <>
void PackParameter<const int32_t *>(ParamBuffer &params,
                                    const Token &token,
                                    const TraceShaderMap &shaders);

template <>
void PackParameter<void **>(ParamBuffer &params, const Token &token, const TraceShaderMap &shaders);

template <>
void PackParameter<int *>(ParamBuffer &params, const Token &token, const TraceShaderMap &shaders);

template <>
void PackParameter<uint64_t>(ParamBuffer &params,
                             const Token &token,
                             const TraceShaderMap &shaders);

template <>
void PackParameter<int64_t>(ParamBuffer &params, const Token &token, const TraceShaderMap &shaders);

template <>
void PackParameter<const int64_t *>(ParamBuffer &params,
                                    const Token &token,
                                    const TraceShaderMap &shaders);

template <>
void PackParameter<int64_t *>(ParamBuffer &params,
                              const Token &token,
                              const TraceShaderMap &shaders);

template <>
void PackParameter<uint64_t *>(ParamBuffer &params,
                               const Token &token,
                               const TraceShaderMap &shaders);

template <>
void PackParameter<const char *>(ParamBuffer &params,
                                 const Token &token,
                                 const TraceShaderMap &shaders);

template <>
void PackParameter<const void *>(ParamBuffer &params,
                                 const Token &token,
                                 const TraceShaderMap &shaders);

template <>
void PackParameter<uint32_t *>(ParamBuffer &params,
                               const Token &token,
                               const TraceShaderMap &shaders);

template <>
void PackParameter<const uint32_t *>(ParamBuffer &params,
                                     const Token &token,
                                     const TraceShaderMap &shaders);

template <>
void PackParameter<float>(ParamBuffer &params, const Token &token, const TraceShaderMap &shaders);

template <>
void PackParameter<uint8_t>(ParamBuffer &params, const Token &token, const TraceShaderMap &shaders);

template <>
void PackParameter<float *>(ParamBuffer &params, const Token &token, const TraceShaderMap &shaders);

template <>
void PackParameter<const float *>(ParamBuffer &params,
                                  const Token &token,
                                  const TraceShaderMap &shaders);

template <>
void PackParameter<GLsync>(ParamBuffer &params, const Token &token, const TraceShaderMap &shaders);

template <>
void PackParameter<const char *const *>(ParamBuffer &params,
                                        const Token &token,
                                        const TraceShaderMap &shaders);

template <>
void PackParameter<const char **>(ParamBuffer &params,
                                  const Token &token,
                                  const TraceShaderMap &shaders);

template <>
void PackParameter<GLDEBUGPROCKHR>(ParamBuffer &params,
                                   const Token &token,
                                   const TraceShaderMap &shaders);

template <>
void PackParameter<EGLDEBUGPROCKHR>(ParamBuffer &params,
                                    const Token &token,
                                    const TraceShaderMap &shaders);

template <>
void PackParameter<const struct AHardwareBuffer *>(ParamBuffer &params,
                                                   const Token &token,
                                                   const TraceShaderMap &shaders);

template <>
void PackParameter<EGLSetBlobFuncANDROID>(ParamBuffer &params,
                                          const Token &token,
                                          const TraceShaderMap &shaders);

template <>
void PackParameter<EGLGetBlobFuncANDROID>(ParamBuffer &params,
                                          const Token &token,
                                          const TraceShaderMap &shaders);

template <>
void PackParameter<int16_t>(ParamBuffer &params, const Token &token, const TraceShaderMap &shaders);

template <>
void PackParameter<const int16_t *>(ParamBuffer &params,
                                    const Token &token,
                                    const TraceShaderMap &shaders);

template <>
void PackParameter<char *>(ParamBuffer &params, const Token &token, const TraceShaderMap &shaders);

template <>
void PackParameter<unsigned char *>(ParamBuffer &params,
                                    const Token &token,
                                    const TraceShaderMap &shaders);

template <>
void PackParameter<const void *const *>(ParamBuffer &params,
                                        const Token &token,
                                        const TraceShaderMap &shaders);

template <>
void PackParameter<const uint64_t *>(ParamBuffer &params,
                                     const Token &token,
                                     const TraceShaderMap &shaders);

#if defined(ANGLE_PLATFORM_WINDOWS)
template <>
void PackParameter<EGLNativeWindowType>(ParamBuffer &params,
                                        const Token &token,
                                        const TraceShaderMap &shaders);

template <>
void PackParameter<EGLNativeDisplayType>(ParamBuffer &params,
                                         const Token &token,
                                         const TraceShaderMap &shaders);

template <>
void PackParameter<EGLNativePixmapType>(ParamBuffer &params,
                                        const Token &token,
                                        const TraceShaderMap &shaders);
#endif  // defined(ANGLE_PLATFORM_WINDOWS)

template <typename T>
void PackParameter(ParamBuffer &params, const Token &token, const TraceShaderMap &shaders)
{
    static_assert(AssertFalse<T>::value, "No specialization for type.");
}

template <typename T>
struct ParameterPacker;

template <typename Ret>
struct ParameterPacker<Ret()>
{
    static void Pack(ParamBuffer &params, const Token *tokens, const TraceShaderMap &shaders) {}
};

template <typename Ret, typename Arg>
struct ParameterPacker<Ret(Arg)>
{
    static void Pack(ParamBuffer &params, const Token *tokens, const TraceShaderMap &shaders)
    {
        PackParameter<Arg>(params, tokens[0], shaders);
    }
};

template <typename Ret, typename Arg, typename... Args>
struct ParameterPacker<Ret(Arg, Args...)>
{
    static void Pack(ParamBuffer &params, const Token *tokens, const TraceShaderMap &shaders)
    {
        PackParameter<Arg>(params, tokens[0], shaders);
        ParameterPacker<Ret(Args...)>::Pack(params, &tokens[1], shaders);
    }
};

template <typename Ret, typename Arg, typename... Args>
struct ParameterPacker<Ret (*)(Arg, Args...)>
{
    static void Pack(ParamBuffer &params, const Token *tokens, const TraceShaderMap &shaders)
    {
        PackParameter<Arg>(params, tokens[0], shaders);
        ParameterPacker<Ret(Args...)>::Pack(params, &tokens[1], shaders);
    }
};

template <typename Func>
ParamBuffer ParseParameters(const Token *paramTokens, const TraceShaderMap &shaders)
{
    ParamBuffer params;
    ParameterPacker<Func>::Pack(params, paramTokens, shaders);
    return params;
}
}  // namespace angle

#endif  // ANGLE_TRACE_INTERPRETER_H_
