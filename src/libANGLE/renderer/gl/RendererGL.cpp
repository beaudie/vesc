//
// Copyright (c) 2015 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//

// RendererGL.cpp: Implements a renderer class for OpenGL.

#include "libANGLE/renderer/gl/RendererGL.h"

#include "libANGLE/renderer/gl/BufferGL.h"
#include "libANGLE/renderer/gl/CompilerGL.h"
#include "libANGLE/renderer/gl/FenceGL.h"
#include "libANGLE/renderer/gl/FramebufferGL.h"
#include "libANGLE/renderer/gl/ProgramGL.h"
#include "libANGLE/renderer/gl/QueryGL.h"
#include "libANGLE/renderer/gl/RenderbufferGL.h"
#include "libANGLE/renderer/gl/ShaderGL.h"
#include "libANGLE/renderer/gl/TextureGL.h"
#include "libANGLE/renderer/gl/TransformFeedbackGL.h"
#include "libANGLE/renderer/gl/VertexArrayGL.h"

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
    return new CompilerGL();
}

ShaderImpl *RendererGL::createShader(GLenum type)
{
    return new ShaderGL();
}

ProgramImpl *RendererGL::createProgram()
{
    return new ProgramGL();
}

DefaultAttachmentImpl *RendererGL::createDefaultAttachment(GLenum type, egl::Surface *surface)
{
    return new DefaultAttachmentGL();
}

FramebufferImpl *RendererGL::createFramebuffer()
{
    return new FramebufferGL();
}

TextureImpl *RendererGL::createTexture(GLenum target)
{
    return new TextureGL();
}

RenderbufferImpl *RendererGL::createRenderbuffer()
{
    return new RenderbufferGL();
}

BufferImpl *RendererGL::createBuffer()
{
    return new BufferGL();
}

VertexArrayImpl *RendererGL::createVertexArray()
{
    return new VertexArrayGL();
}

QueryImpl *RendererGL::createQuery(GLenum type)
{
    return new QueryGL(type);
}

FenceNVImpl *RendererGL::createFenceNV()
{
    return new FenceNVGL();
}

FenceSyncImpl *RendererGL::createFenceSync()
{
    return new FenceSyncGL();
}

TransformFeedbackImpl *RendererGL::createTransformFeedback()
{
    return new TransformFeedbackGL();
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
    // Set some minimum GLES2 caps
    outCaps->maxElementIndex = static_cast<GLint64>(std::numeric_limits<unsigned int>::max());
    outCaps->max3DTextureSize = 0;
    outCaps->max2DTextureSize = 1024;
    outCaps->maxCubeMapTextureSize = outCaps->max2DTextureSize;
    outCaps->maxArrayTextureLayers = 1;
    outCaps->maxLODBias = 0.0f;
    outCaps->maxRenderbufferSize = outCaps->max2DTextureSize;
    outCaps->maxDrawBuffers = 1;
    outCaps->maxColorAttachments = 1;
    outCaps->maxViewportWidth = outCaps->max2DTextureSize;
    outCaps->maxViewportHeight = outCaps->maxViewportWidth;
    outCaps->minAliasedPointSize = 1.0f;
    outCaps->maxAliasedPointSize = 1.0f;
    outCaps->minAliasedLineWidth = 1.0f;
    outCaps->maxAliasedLineWidth = 1.0f;
    outCaps->maxElementsIndices = 0;
    outCaps->maxElementsVertices = 0;
    outCaps->maxServerWaitTimeout = 0;
    outCaps->maxVertexAttributes = 16;
    outCaps->maxVertexUniformVectors = 256;
    outCaps->maxVertexUniformVectors = outCaps->maxVertexUniformVectors * 4;
    outCaps->maxVertexUniformBlocks = 0;
    outCaps->maxVertexOutputComponents = 16;
    outCaps->maxVertexTextureImageUnits = outCaps->maxTextureImageUnits;
    outCaps->maxFragmentUniformVectors = 256;
    outCaps->maxFragmentUniformComponents = outCaps->maxFragmentUniformVectors * 4;
    outCaps->maxFragmentUniformBlocks = 0;
    outCaps->maxFragmentInputComponents = outCaps->maxVertexOutputComponents;
    outCaps->maxTextureImageUnits = 16;
    outCaps->minProgramTexelOffset = 0;
    outCaps->maxProgramTexelOffset = 0;
    outCaps->maxUniformBufferBindings = 0;
    outCaps->maxUniformBlockSize = 0;
    outCaps->uniformBufferOffsetAlignment = 0;
    outCaps->maxCombinedUniformBlocks = 0;
    outCaps->maxCombinedVertexUniformComponents = 0;
    outCaps->maxCombinedFragmentUniformComponents = 0;
    outCaps->maxVaryingComponents = 0;
    outCaps->maxVaryingVectors = outCaps->maxVertexOutputComponents / 4;
    outCaps->maxCombinedTextureImageUnits = outCaps->maxVertexTextureImageUnits + outCaps->maxTextureImageUnits;
    outCaps->maxTransformFeedbackInterleavedComponents = 0;
    outCaps->maxTransformFeedbackSeparateAttributes = 0;
    outCaps->maxTransformFeedbackSeparateComponents = 0;

    gl::TextureCaps supportedTextureFormat;
    supportedTextureFormat.texturable = true;
    supportedTextureFormat.filterable = true;
    supportedTextureFormat.renderable = true;

    outTextureCaps->insert(GL_RGB565, supportedTextureFormat);
    outTextureCaps->insert(GL_RGBA4, supportedTextureFormat);
    outTextureCaps->insert(GL_RGB5_A1, supportedTextureFormat);
    outTextureCaps->insert(GL_RGB8_OES, supportedTextureFormat);
    outTextureCaps->insert(GL_RGBA8_OES, supportedTextureFormat);

    outTextureCaps->insert(GL_DEPTH_COMPONENT16, supportedTextureFormat);
    outTextureCaps->insert(GL_STENCIL_INDEX8, supportedTextureFormat);

    outExtensions->setTextureExtensionSupport(*outTextureCaps);
    outExtensions->textureNPOT = true;
    outExtensions->textureStorage = true;
}

Workarounds RendererGL::generateWorkarounds() const
{
    Workarounds workarounds;
    return workarounds;
}

}
