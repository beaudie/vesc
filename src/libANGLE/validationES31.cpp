//
// Copyright (c) 2016 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//

// validationES31.cpp: Validation functions for OpenGL ES 3.1 entry point parameters

#include "libANGLE/validationES31.h"

#include "libANGLE/Context.h"
#include "libANGLE/validationES.h"
#include "libANGLE/validationES3.h"
#include "libANGLE/VertexArray.h"

using namespace angle;

namespace gl
{

bool ValidateGetBooleani_v(Context *context, GLenum target, GLuint index, GLboolean *data)
{
    if (context->getClientVersion() < ES_3_1)
    {
        context->handleError(Error(GL_INVALID_OPERATION, "Context does not support GLES3.1"));
        return false;
    }

    if (!ValidateIndexedStateQuery(context, target, index, nullptr))
    {
        return false;
    }

    return true;
}

bool ValidateGetBooleani_vRobustANGLE(Context *context,
                                      GLenum target,
                                      GLuint index,
                                      GLsizei bufSize,
                                      GLsizei *length,
                                      GLboolean *data)
{
    if (context->getClientVersion() < ES_3_1)
    {
        context->handleError(Error(GL_INVALID_OPERATION, "Context does not support GLES3.1"));
        return false;
    }

    if (!ValidateRobustEntryPoint(context, bufSize))
    {
        return false;
    }

    if (!ValidateIndexedStateQuery(context, target, index, length))
    {
        return false;
    }

    if (!ValidateRobustBufferSize(context, bufSize, *length))
    {
        return false;
    }

    return true;
}

bool ValidateDrawIndirectBase(Context *context, GLenum mode, const GLvoid *indirect)
{
    if (context->getClientVersion() < ES_3_1)
    {
        context->handleError(Error(GL_INVALID_OPERATION, "Context does not support GLES3.1"));
        return false;
    }

    // Here the third parameter 1 is only to pass the count validation.
    if (!ValidateDrawBase(context, mode, 1))
    {
        return false;
    }

    const State &state = context->getGLState();

    // An INVALID_OPERATION error is generated if zero is bound to VERTEX_ARRAY_BINDING,
    // DRAW_INDIRECT_BUFFER or to any enabled vertex array.
    if (!state.getVertexArrayId())
    {
        context->handleError(Error(GL_INVALID_OPERATION, "zero is bound to VERTEX_ARRAY_BINDING"));
        return false;
    }

    gl::Buffer *drawIndirectBuffer = state.getDrawIndirectBuffer();
    if (!drawIndirectBuffer)
    {
        context->handleError(Error(GL_INVALID_OPERATION, "zero is bound to DRAW_INDIRECT_BUFFER"));
        return false;
    }

    // An INVALID_VALUE error is generated if indirect is not a multiple of the size, in basic
    // machine units, of uint.
    GLint64 offset = reinterpret_cast<GLint64>(indirect);
    if ((static_cast<GLuint>(offset) % sizeof(GLuint)) != 0)
    {
        context->handleError(
            Error(GL_INVALID_VALUE,
                  "indirect is not a multiple of the size, in basic machine units, of uint"));
        return false;
    }

    return true;
}

bool ValidateDrawArraysIndirect(Context *context, GLenum mode, const GLvoid *indirect)
{
    const State &state                          = context->getGLState();
    gl::TransformFeedback *curTransformFeedback = state.getCurrentTransformFeedback();
    if (curTransformFeedback && curTransformFeedback->isActive() &&
        !curTransformFeedback->isPaused())
    {
        // An INVALID_OPERATION error is generated if transform feedback is active and not paused.
        context->handleError(
            Error(GL_INVALID_OPERATION, "transform feedback is active and not paused."));
        return false;
    }

    if (!ValidateDrawIndirectBase(context, mode, indirect))
        return false;

    gl::Buffer *drawIndirectBuffer = state.getDrawIndirectBuffer();
    CheckedNumeric<size_t> checkedOffset(reinterpret_cast<size_t>(indirect));
    // In OpenGL ES3.1 spec, session 10.5, it defines the struct of DrawArraysIndirectCommand
    // which's size is 4 * sizeof(uint).
    auto checkedSum = checkedOffset + 4 * sizeof(GLuint);
    if (!checkedSum.IsValid() ||
        checkedSum.ValueOrDie() > static_cast<size_t>(drawIndirectBuffer->getSize()))
    {
        context->handleError(
            Error(GL_INVALID_OPERATION,
                  "the  command  would source data beyond the end of the buffer object."));
        return false;
    }

    return true;
}

bool ValidateDrawElementsIndirect(Context *context,
                                  GLenum mode,
                                  GLenum type,
                                  const GLvoid *indirect)
{
    if (!ValidateDrawElementsBase(context, type))
        return false;

    const State &state             = context->getGLState();
    const VertexArray *vao         = state.getVertexArray();
    gl::Buffer *elementArrayBuffer = vao->getElementArrayBuffer().get();
    if (!elementArrayBuffer)
    {
        context->handleError(Error(GL_INVALID_OPERATION, "zero is bound to ELEMENT_ARRAY_BUFFER"));
        return false;
    }

    if (!ValidateDrawIndirectBase(context, mode, indirect))
        return false;

    gl::Buffer *drawIndirectBuffer = state.getDrawIndirectBuffer();
    CheckedNumeric<size_t> checkedOffset(reinterpret_cast<size_t>(indirect));
    // In OpenGL ES3.1 spec, session 10.5, it defines the struct of DrawElementsIndirectCommand
    // which's size is 5 * sizeof(uint).
    auto checkedSum = checkedOffset + 5 * sizeof(GLuint);
    if (!checkedSum.IsValid() ||
        checkedSum.ValueOrDie() > static_cast<size_t>(drawIndirectBuffer->getSize()))
    {
        context->handleError(
            Error(GL_INVALID_OPERATION,
                  "the  command  would source data beyond the end of the buffer object."));
        return false;
    }

    return true;
}

bool ValidateTexStorageMultiSample2DParameters(Context *context,
                                               GLenum target,
                                               GLsizei samples,
                                               GLint internalFormat,
                                               GLsizei width,
                                               GLsizei height,
                                               GLboolean fixedSampleLocations)
{
    if (context->getClientVersion() < ES_3_1)
    {
        context->handleError(Error(GL_INVALID_OPERATION, "Context does not support GLES3.1."));
        return false;
    }

    if (target != GL_TEXTURE_2D_MULTISAMPLE)
    {
        context->handleError(Error(GL_INVALID_ENUM, "Target is not TEXTURE_2D_MULTISAMPLE."));
        return false;
    }

    if (width < 1 || height < 1)
    {
        context->handleError(Error(GL_INVALID_VALUE, "Width or height is less than 1."));
    }

    const Caps &caps = context->getCaps();
    if (static_cast<GLuint>(width) > caps.max2DTextureSize ||
        static_cast<GLuint>(height) > caps.max2DTextureSize)
    {
        context->handleError(
            Error(GL_INVALID_VALUE, "Width or height is greater than GL_MAX_TEXTURE_SIZE."));
    }

    if (samples == 0)
    {
        context->handleError(Error(GL_INVALID_VALUE, "Samples is zero."));
    }

    const TextureCaps &formatCaps = context->getTextureCaps().get(internalFormat);
    if (!formatCaps.renderable)
    {
        context->handleError(Error(GL_INVALID_ENUM,
                                   "SizedInternalformat is not color-renderable, depth-renderable, "
                                   "or stencil-renderable."));
        return false;
    }

    // The ES3.1 spec(section 8.8) states that an INVALID_ENUM error is generated if internalformat
    // is one of the unsized base internalformats listed in table 8.11.
    switch (internalFormat)
    {
        case GL_DEPTH_COMPONENT:
        case GL_DEPTH_STENCIL:
        case GL_LUMINANCE:
        case GL_ALPHA:
        case GL_LUMINANCE_ALPHA:
        case GL_RED:
        case GL_RG:
        case GL_RGB:
        case GL_RGBA:
        case GL_STENCIL_INDEX:
            context->handleError(Error(
                GL_INVALID_ENUM,
                "Internalformat is one of the unsupported unsized base internalformats."));
            return false;
    }

    if (static_cast<GLuint>(samples) > formatCaps.getMaxSamples())
    {
        context->handleError(
            Error(GL_INVALID_OPERATION,
                  "Samples must not be greater than maximum supported value for the format."));
        return false;
    }

    Texture *texture = context->getTargetTexture(target);
    if (!texture)
    {
        context->handleError(Error(GL_INVALID_OPERATION, "Zero is bound to target."));
    }

    if (texture->getImmutableFormat())
    {
        context->handleError(
            Error(GL_INVALID_OPERATION,
                  "The value of TEXTURE_IMMUTABLE_FORMAT for the texture "
                  "currently bound to target on the active texture unit is true."));
        return false;
    }

    return true;
}

bool ValidateGetMultisample(Context *context, GLenum pname, GLuint index, GLfloat *val)
{
    if (context->getClientVersion() < ES_3_1)
    {
        context->handleError(Error(GL_INVALID_OPERATION, "Context does not support GLES3.1."));
        return false;
    }

    if (pname != GL_SAMPLE_POSITION)
    {
        context->handleError(Error(GL_INVALID_ENUM, "Pname is not SAMPLE_POSITION."));
        return false;
    }

    if (index >= GL_SAMPLES)
    {
        context->handleError(
            Error(GL_INVALID_VALUE, "Index is greater than or equal to the value of SAMPLES."));
        return false;
    }

    return true;
}

}  // namespace gl
