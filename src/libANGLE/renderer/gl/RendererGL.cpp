//
// Copyright (c) 2015 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//

// RendererGL.cpp: Implements a renderer class for OpenGL.

#include "libANGLE/renderer/gl/RendererGL.h"

namespace rx
{

RendererGL::RendererGL()
{
}

RendererGL::~RendererGL()
{
}

gl::Error RendererGL::flush()
{
    return gl::Error(GL_NO_ERROR);
}

gl::Error RendererGL::finish()
{
    return gl::Error(GL_NO_ERROR);
}

gl::Error RendererGL::drawArrays(const gl::Data &data, GLenum mode, GLint first, GLsizei count, GLsizei instances)
{
    return gl::Error(GL_NO_ERROR);
}

gl::Error RendererGL::drawElements(const gl::Data &data, GLenum mode, GLsizei count, GLenum type, const GLvoid *indices, GLsizei instances, const RangeUI &indexRange)
{
    return gl::Error(GL_NO_ERROR);
}

CompilerImpl *RendererGL::createCompiler(const gl::Data &data)
{
    return nullptr;
}

ShaderImpl *RendererGL::createShader(GLenum type)
{
    return nullptr;
}

ProgramImpl *RendererGL::createProgram()
{
    return nullptr;
}

DefaultAttachmentImpl *RendererGL::createDefaultAttachment(GLenum type, egl::Surface *surface)
{
    return nullptr;
}

FramebufferImpl *RendererGL::createFramebuffer()
{
    return nullptr;
}

TextureImpl *RendererGL::createTexture(GLenum target)
{
    return nullptr;
}

RenderbufferImpl *RendererGL::createRenderbuffer()
{
    return nullptr;
}

BufferImpl *RendererGL::createBuffer()
{
    return nullptr;
}

VertexArrayImpl *RendererGL::createVertexArray()
{
    return nullptr;
}

QueryImpl *RendererGL::createQuery(GLenum type)
{
    return nullptr;
}

FenceNVImpl *RendererGL::createFenceNV()
{
    return nullptr;
}

FenceSyncImpl *RendererGL::createFenceSync()
{
    return nullptr;
}

TransformFeedbackImpl *RendererGL::createTransformFeedback()
{
    return nullptr;
}

void RendererGL::notifyDeviceLost()
{
}

bool RendererGL::isDeviceLost() const
{
    return false;
}

bool RendererGL::testDeviceLost()
{
    return false;
}

bool RendererGL::testDeviceResettable()
{
    return false;
}

VendorID RendererGL::getVendorId() const
{
    return VendorID(0);
}

std::string RendererGL::getRendererDescription() const
{
    return "";
}

std::string RendererGL::getVendorString() const
{
    return "";
}

void RendererGL::generateCaps(gl::Caps *outCaps, gl::TextureCapsMap* outTextureCaps, gl::Extensions *outExtensions) const
{
}

Workarounds RendererGL::generateWorkarounds() const
{
    Workarounds workarounds;
    return workarounds;
}

}
