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

BufferImpl *ContextNULL::createBuffer()
{
    return new BufferNULL();
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
