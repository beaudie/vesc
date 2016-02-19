//
// Copyright 2015 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//

// RendererVk.cpp: Implements the class methods for RendererVk.

#include "libANGLE/renderer/vulkan/RendererVk.h"

#include "common/debug.h"

#include "libANGLE/renderer/vulkan/TextureVk.h"

namespace rx
{

RendererVk::RendererVk() : Renderer()
{
}

RendererVk::~RendererVk()
{
}

gl::Error RendererVk::flush()
{
    UNIMPLEMENTED();
    return gl::Error(GL_INVALID_OPERATION);
}

gl::Error RendererVk::finish()
{
    UNIMPLEMENTED();
    return gl::Error(GL_INVALID_OPERATION);
}

gl::Error RendererVk::drawArrays(const gl::Data &data, GLenum mode, GLint first, GLsizei count)
{
    UNIMPLEMENTED();
    return gl::Error(GL_INVALID_OPERATION);
}

gl::Error RendererVk::drawArraysInstanced(const gl::Data &data,
                                          GLenum mode,
                                          GLint first,
                                          GLsizei count,
                                          GLsizei instanceCount)
{
    UNIMPLEMENTED();
    return gl::Error(GL_INVALID_OPERATION);
}

gl::Error RendererVk::drawElements(const gl::Data &data,
                                   GLenum mode,
                                   GLsizei count,
                                   GLenum type,
                                   const GLvoid *indices,
                                   const gl::IndexRange &indexRange)
{
    UNIMPLEMENTED();
    return gl::Error(GL_INVALID_OPERATION);
}

gl::Error RendererVk::drawElementsInstanced(const gl::Data &data,
                                            GLenum mode,
                                            GLsizei count,
                                            GLenum type,
                                            const GLvoid *indices,
                                            GLsizei instances,
                                            const gl::IndexRange &indexRange)
{
    UNIMPLEMENTED();
    return gl::Error(GL_INVALID_OPERATION);
}

gl::Error RendererVk::drawRangeElements(const gl::Data &data,
                                        GLenum mode,
                                        GLuint start,
                                        GLuint end,
                                        GLsizei count,
                                        GLenum type,
                                        const GLvoid *indices,
                                        const gl::IndexRange &indexRange)
{
    UNIMPLEMENTED();
    return gl::Error(GL_INVALID_OPERATION);
}

void RendererVk::notifyDeviceLost()
{
    UNIMPLEMENTED();
}

bool RendererVk::isDeviceLost() const
{
    UNIMPLEMENTED();
    return bool();
}

bool RendererVk::testDeviceLost()
{
    UNIMPLEMENTED();
    return bool();
}

bool RendererVk::testDeviceResettable()
{
    UNIMPLEMENTED();
    return bool();
}

std::string RendererVk::getVendorString() const
{
    UNIMPLEMENTED();
    return std::string();
}

std::string RendererVk::getRendererDescription() const
{
    UNIMPLEMENTED();
    return std::string();
}

void RendererVk::insertEventMarker(GLsizei length, const char *marker)
{
    UNIMPLEMENTED();
}

void RendererVk::pushGroupMarker(GLsizei length, const char *marker)
{
    UNIMPLEMENTED();
}

void RendererVk::popGroupMarker()
{
    UNIMPLEMENTED();
}

void RendererVk::syncState(const gl::State &state, const gl::State::DirtyBits &dirtyBits)
{
    UNIMPLEMENTED();
}

GLint RendererVk::getGPUDisjoint()
{
    UNIMPLEMENTED();
    return GLint();
}

GLint64 RendererVk::getTimestamp()
{
    UNIMPLEMENTED();
    return GLint64();
}

void RendererVk::onMakeCurrent(const gl::Data &data)
{
    UNIMPLEMENTED();
}

CompilerImpl *RendererVk::createCompiler()
{
    UNIMPLEMENTED();
    return static_cast<CompilerImpl *>(0);
}

ShaderImpl *RendererVk::createShader(const gl::Shader::Data &data)
{
    UNIMPLEMENTED();
    return static_cast<ShaderImpl *>(0);
}

ProgramImpl *RendererVk::createProgram(const gl::Program::Data &data)
{
    UNIMPLEMENTED();
    return static_cast<ProgramImpl *>(0);
}

FramebufferImpl *RendererVk::createFramebuffer(const gl::Framebuffer::Data &data)
{
    UNIMPLEMENTED();
    return static_cast<FramebufferImpl *>(0);
}

TextureImpl *RendererVk::createTexture(GLenum target)
{
    return new TextureVk();
}

RenderbufferImpl *RendererVk::createRenderbuffer()
{
    UNIMPLEMENTED();
    return static_cast<RenderbufferImpl *>(0);
}

BufferImpl *RendererVk::createBuffer()
{
    UNIMPLEMENTED();
    return static_cast<BufferImpl *>(0);
}

VertexArrayImpl *RendererVk::createVertexArray(const gl::VertexArray::Data &data)
{
    UNIMPLEMENTED();
    return static_cast<VertexArrayImpl *>(0);
}

QueryImpl *RendererVk::createQuery(GLenum type)
{
    UNIMPLEMENTED();
    return static_cast<QueryImpl *>(0);
}

FenceNVImpl *RendererVk::createFenceNV()
{
    UNIMPLEMENTED();
    return static_cast<FenceNVImpl *>(0);
}

FenceSyncImpl *RendererVk::createFenceSync()
{
    UNIMPLEMENTED();
    return static_cast<FenceSyncImpl *>(0);
}

TransformFeedbackImpl *RendererVk::createTransformFeedback()
{
    UNIMPLEMENTED();
    return static_cast<TransformFeedbackImpl *>(0);
}

SamplerImpl *RendererVk::createSampler()
{
    UNIMPLEMENTED();
    return static_cast<SamplerImpl *>(0);
}

}  // namespace rx
