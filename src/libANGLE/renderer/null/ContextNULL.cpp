//
// Copyright 2016 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//
// ContextNULL.cpp:
//    Implements the class methods for ContextNULL.
//

#include "libANGLE/renderer/null/ContextNULL.h"

#include "common/debug.h"

#include "libANGLE/renderer/null/BufferNULL.h"
#include "libANGLE/renderer/null/CompilerNULL.h"
#include "libANGLE/renderer/null/DisplayNULL.h"
#include "libANGLE/renderer/null/FenceNVNULL.h"
#include "libANGLE/renderer/null/FenceSyncNULL.h"
#include "libANGLE/renderer/null/FramebufferNULL.h"
#include "libANGLE/renderer/null/ImageNULL.h"
#include "libANGLE/renderer/null/PathNULL.h"
#include "libANGLE/renderer/null/ProgramNULL.h"
#include "libANGLE/renderer/null/QueryNULL.h"
#include "libANGLE/renderer/null/RenderbufferNULL.h"
#include "libANGLE/renderer/null/SamplerNULL.h"
#include "libANGLE/renderer/null/ShaderNULL.h"
#include "libANGLE/renderer/null/TextureNULL.h"
#include "libANGLE/renderer/null/TransformFeedbackNULL.h"
#include "libANGLE/renderer/null/VertexArrayNULL.h"

namespace rx
{

ContextNULL::ContextNULL(const gl::ContextState &state) : ContextImpl(state)
{
    // ES 3.1 minimum caps

    mCaps.maxElementIndex       = (1 << 24) - 1;
    mCaps.max3DTextureSize      = 256;
    mCaps.max2DTextureSize      = 2048;
    mCaps.maxArrayTextureLayers = 256;
    mCaps.maxLODBias            = 2.0f;
    mCaps.maxCubeMapTextureSize = 2048;
    mCaps.maxRenderbufferSize   = 2048;
    mCaps.minAliasedPointSize   = 1;
    mCaps.maxAliasedPointSize   = 1;
    mCaps.minAliasedLineWidth   = 1;
    mCaps.maxAliasedLineWidth   = 1;

    mCaps.maxDrawBuffers         = 4;
    mCaps.maxFramebufferWidth    = 2048;
    mCaps.maxFramebufferHeight   = 2048;
    mCaps.maxFramebufferSamples  = 4;
    mCaps.maxColorAttachments    = 4;
    mCaps.maxViewportWidth       = mCaps.max2DTextureSize;
    mCaps.maxViewportHeight      = mCaps.max2DTextureSize;
    mCaps.maxSampleMaskWords     = 1;
    mCaps.maxColorTextureSamples = 1;
    mCaps.maxDepthTextureSamples = 1;
    mCaps.maxIntegerSamples      = 1;
    mCaps.maxServerWaitTimeout   = 0;

    mCaps.maxVertexAttribRelativeOffset = 2047;
    mCaps.maxVertexAttribBindings       = 16;
    mCaps.maxVertexAttribStride         = 2048;
    mCaps.maxElementsIndices            = 0;
    mCaps.maxElementsVertices           = 0;
    // mCaps.compressedTextureFormats;
    // mCaps.programBinaryFormats;
    // mCaps.shaderBinaryFormats;
    mCaps.vertexHighpFloat.setIEEEFloat();
    mCaps.vertexMediumpFloat.setIEEEFloat();
    mCaps.vertexLowpFloat.setIEEEFloat();
    mCaps.vertexHighpInt.setTwosComplementInt(32);
    mCaps.vertexMediumpInt.setTwosComplementInt(32);
    mCaps.vertexLowpInt.setTwosComplementInt(32);
    mCaps.fragmentHighpFloat.setIEEEFloat();
    mCaps.fragmentMediumpFloat.setIEEEFloat();
    mCaps.fragmentLowpFloat.setIEEEFloat();
    mCaps.fragmentHighpInt.setTwosComplementInt(32);
    mCaps.fragmentMediumpInt.setTwosComplementInt(32);
    mCaps.fragmentLowpInt.setTwosComplementInt(32);

    mCaps.maxVertexAttributes           = 16;
    mCaps.maxVertexUniformComponents    = 1024;
    mCaps.maxVertexUniformVectors       = 256;
    mCaps.maxVertexUniformBlocks        = 12;
    mCaps.maxVertexOutputComponents     = 64;
    mCaps.maxVertexTextureImageUnits    = 16;
    mCaps.maxVertexAtomicCounterBuffers = 0;
    mCaps.maxVertexAtomicCounters       = 0;
    mCaps.maxVertexImageUniforms        = 0;
    mCaps.maxVertexShaderStorageBlocks  = 0;

    mCaps.maxFragmentUniformComponents    = 1024;
    mCaps.maxFragmentUniformVectors       = 256;
    mCaps.maxFragmentUniformBlocks        = 12;
    mCaps.maxFragmentInputComponents      = 60;
    mCaps.maxTextureImageUnits            = 16;
    mCaps.maxFragmentAtomicCounterBuffers = 0;
    mCaps.maxFragmentAtomicCounters       = 0;
    mCaps.maxFragmentImageUniforms        = 0;
    mCaps.maxFragmentShaderStorageBlocks  = 0;
    mCaps.minProgramTextureGatherOffset   = 0;
    mCaps.maxProgramTextureGatherOffset   = 0;
    mCaps.minProgramTexelOffset           = -8;
    mCaps.maxProgramTexelOffset           = 7;

    mCaps.maxComputeWorkGroupCount       = {{65535, 65535, 65535}};
    mCaps.maxComputeWorkGroupSize        = {{128, 128, 64}};
    mCaps.maxComputeWorkGroupInvocations = 12;
    mCaps.maxComputeUniformBlocks        = 12;
    mCaps.maxComputeTextureImageUnits    = 16;
    mCaps.maxComputeSharedMemorySize     = 16384;
    mCaps.maxComputeUniformComponents    = 1024;
    mCaps.maxComputeAtomicCounterBuffers = 1;
    mCaps.maxComputeAtomicCounters       = 8;
    mCaps.maxComputeImageUniforms        = 4;
    mCaps.maxCombinedComputeUniformComponents =
        mCaps.maxComputeUniformBlocks * static_cast<GLuint>(mCaps.maxUniformBlockSize / 4) +
        mCaps.maxComputeUniformComponents;
    mCaps.maxComputeShaderStorageBlocks = 4;

    mCaps.maxUniformBufferBindings     = 36;
    mCaps.maxUniformBlockSize          = 16834;
    mCaps.uniformBufferOffsetAlignment = 256;
    mCaps.maxCombinedUniformBlocks     = 24;
    mCaps.maxCombinedVertexUniformComponents =
        mCaps.maxVertexUniformBlocks * (mCaps.maxUniformBlockSize / 4) +
        mCaps.maxVertexUniformComponents;
    mCaps.maxCombinedFragmentUniformComponents =
        mCaps.maxFragmentUniformBlocks * (mCaps.maxUniformBlockSize / 4) +
        mCaps.maxFragmentUniformComponents;
    mCaps.maxVaryingComponents             = 60;
    mCaps.maxVaryingVectors                = 15;
    mCaps.maxCombinedTextureImageUnits     = 48;
    mCaps.maxCombinedShaderOutputResources = 4;

    mCaps.maxUniformLocations                = 1024;
    mCaps.maxAtomicCounterBufferBindings     = 1;
    mCaps.maxAtomicCounterBufferSize         = 32;
    mCaps.maxCombinedAtomicCounterBuffers    = 1;
    mCaps.maxCombinedAtomicCounters          = 8;
    mCaps.maxImageUnits                      = 4;
    mCaps.maxCombinedImageUniforms           = 4;
    mCaps.maxShaderStorageBufferBindings     = 4;
    mCaps.maxShaderStorageBlockSize          = 1 << 27;
    mCaps.maxCombinedShaderStorageBlocks     = 4;
    mCaps.shaderStorageBufferOffsetAlignment = 256;

    mCaps.maxTransformFeedbackInterleavedComponents = 64;
    mCaps.maxTransformFeedbackSeparateAttributes    = 4;
    mCaps.maxTransformFeedbackSeparateComponents    = 4;

    mCaps.maxSamples = 4;
}

ContextNULL::~ContextNULL()
{
}

gl::Error ContextNULL::initialize()
{
    return gl::NoError();
}

gl::Error ContextNULL::flush()
{
    return gl::NoError();
}

gl::Error ContextNULL::finish()
{
    return gl::NoError();
}

gl::Error ContextNULL::drawArrays(GLenum mode, GLint first, GLsizei count)
{
    return gl::NoError();
}

gl::Error ContextNULL::drawArraysInstanced(GLenum mode,
                                           GLint first,
                                           GLsizei count,
                                           GLsizei instanceCount)
{
    return gl::NoError();
}

gl::Error ContextNULL::drawElements(GLenum mode,
                                    GLsizei count,
                                    GLenum type,
                                    const GLvoid *indices,
                                    const gl::IndexRange &indexRange)
{
    return gl::NoError();
}

gl::Error ContextNULL::drawElementsInstanced(GLenum mode,
                                             GLsizei count,
                                             GLenum type,
                                             const GLvoid *indices,
                                             GLsizei instances,
                                             const gl::IndexRange &indexRange)
{
    return gl::NoError();
}

gl::Error ContextNULL::drawRangeElements(GLenum mode,
                                         GLuint start,
                                         GLuint end,
                                         GLsizei count,
                                         GLenum type,
                                         const GLvoid *indices,
                                         const gl::IndexRange &indexRange)
{
    return gl::NoError();
}

void ContextNULL::stencilFillPath(const gl::Path *path, GLenum fillMode, GLuint mask)
{
}

void ContextNULL::stencilStrokePath(const gl::Path *path, GLint reference, GLuint mask)
{
}

void ContextNULL::coverFillPath(const gl::Path *path, GLenum coverMode)
{
}

void ContextNULL::coverStrokePath(const gl::Path *path, GLenum coverMode)
{
}

void ContextNULL::stencilThenCoverFillPath(const gl::Path *path,
                                           GLenum fillMode,
                                           GLuint mask,
                                           GLenum coverMode)
{
}

void ContextNULL::stencilThenCoverStrokePath(const gl::Path *path,
                                             GLint reference,
                                             GLuint mask,
                                             GLenum coverMode)
{
}

void ContextNULL::coverFillPathInstanced(const std::vector<gl::Path *> &paths,
                                         GLenum coverMode,
                                         GLenum transformType,
                                         const GLfloat *transformValues)
{
}

void ContextNULL::coverStrokePathInstanced(const std::vector<gl::Path *> &paths,
                                           GLenum coverMode,
                                           GLenum transformType,
                                           const GLfloat *transformValues)
{
}

void ContextNULL::stencilFillPathInstanced(const std::vector<gl::Path *> &paths,
                                           GLenum fillMode,
                                           GLuint mask,
                                           GLenum transformType,
                                           const GLfloat *transformValues)
{
}

void ContextNULL::stencilStrokePathInstanced(const std::vector<gl::Path *> &paths,
                                             GLint reference,
                                             GLuint mask,
                                             GLenum transformType,
                                             const GLfloat *transformValues)
{
}

void ContextNULL::stencilThenCoverFillPathInstanced(const std::vector<gl::Path *> &paths,
                                                    GLenum coverMode,
                                                    GLenum fillMode,
                                                    GLuint mask,
                                                    GLenum transformType,
                                                    const GLfloat *transformValues)
{
}

void ContextNULL::stencilThenCoverStrokePathInstanced(const std::vector<gl::Path *> &paths,
                                                      GLenum coverMode,
                                                      GLint reference,
                                                      GLuint mask,
                                                      GLenum transformType,
                                                      const GLfloat *transformValues)
{
}

GLenum ContextNULL::getResetStatus()
{
    return GL_NO_ERROR;
}

std::string ContextNULL::getVendorString() const
{
    return "NULL";
}

std::string ContextNULL::getRendererDescription() const
{
    return "NULL";
}

void ContextNULL::insertEventMarker(GLsizei length, const char *marker)
{
}

void ContextNULL::pushGroupMarker(GLsizei length, const char *marker)
{
}

void ContextNULL::popGroupMarker()
{
}

void ContextNULL::syncState(const gl::State &state, const gl::State::DirtyBits &dirtyBits)
{
}

GLint ContextNULL::getGPUDisjoint()
{
    return 0;
}

GLint64 ContextNULL::getTimestamp()
{
    return 0;
}

void ContextNULL::onMakeCurrent(const gl::ContextState &data)
{
}

const gl::Caps &ContextNULL::getNativeCaps() const
{
    return mCaps;
}

const gl::TextureCapsMap &ContextNULL::getNativeTextureCaps() const
{
    return mTextureCaps;
}

const gl::Extensions &ContextNULL::getNativeExtensions() const
{
    return mExtensions;
}

const gl::Limitations &ContextNULL::getNativeLimitations() const
{
    return mLimitations;
}

CompilerImpl *ContextNULL::createCompiler()
{
    return new CompilerNULL();
}

ShaderImpl *ContextNULL::createShader(const gl::ShaderState &data)
{
    return new ShaderNULL(data);
}

ProgramImpl *ContextNULL::createProgram(const gl::ProgramState &data)
{
    return new ProgramNULL(data);
}

FramebufferImpl *ContextNULL::createFramebuffer(const gl::FramebufferState &data)
{
    return new FramebufferNULL(data);
}

TextureImpl *ContextNULL::createTexture(const gl::TextureState &state)
{
    return new TextureNULL(state);
}

RenderbufferImpl *ContextNULL::createRenderbuffer()
{
    return new RenderbufferNULL();
}

BufferImpl *ContextNULL::createBuffer(const gl::BufferState &state)
{
    return new BufferNULL(state);
}

VertexArrayImpl *ContextNULL::createVertexArray(const gl::VertexArrayState &data)
{
    return new VertexArrayNULL(data);
}

QueryImpl *ContextNULL::createQuery(GLenum type)
{
    return new QueryNULL(type);
}

FenceNVImpl *ContextNULL::createFenceNV()
{
    return new FenceNVNULL();
}

FenceSyncImpl *ContextNULL::createFenceSync()
{
    return new FenceSyncNULL();
}

TransformFeedbackImpl *ContextNULL::createTransformFeedback(const gl::TransformFeedbackState &state)
{
    return new TransformFeedbackNULL(state);
}

SamplerImpl *ContextNULL::createSampler()
{
    return new SamplerNULL();
}

std::vector<PathImpl *> ContextNULL::createPaths(GLsizei range)
{
    std::vector<PathImpl *> result(range);
    for (GLsizei idx = 0; idx < range; idx++)
    {
        result[idx] = new PathNULL();
    }
    return result;
}

}  // namespace rx
