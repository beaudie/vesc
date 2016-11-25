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

bool ValidateTexStorageMultiSample2DParameters(Context *context,
                                               GLenum target,
                                               GLsizei samples,
                                               GLint internalFormat,
                                               GLsizei width,
                                               GLsizei height,
                                               GLboolean fixedsamplelocations)
{
    if (context->getClientVersion() < ES_3_1)
    {
        context->handleError(Error(GL_INVALID_OPERATION, "Context does not support GLES3.1"));
        return false;
    }

    switch (target)
    {
        case GL_TEXTURE_2D_MULTISAMPLE:
            break;
        case GL_ZERO:
            context->handleError(Error(GL_INVALID_OPERATION, "Zero is bound to target"));
        default:
            context->handleError(Error(GL_INVALID_ENUM, "Target is not TEXTURE_2D_MULTISAMPLE"));
            return false;
    }

    if (width < 1 || height < 1)
    {
        context->handleError(Error(GL_INVALID_VALUE, "Width or height less than 1"));
    }

    if (width > GL_MAX_TEXTURE_SIZE || height > GL_MAX_TEXTURE_SIZE)
    {
        context->handleError(
            Error(GL_INVALID_VALUE, "Width or height is greater than GL_MAX_TEXTURE_SIZE"));
    }

    if (samples == 0)
    {
        context->handleError(Error(GL_INVALID_VALUE, "Samples is zero"));
    }

    const TextureCaps &formatCaps = context->getTextureCaps().get(internalFormat);
    if (!formatCaps.renderable)
    {
        context->handleError(Error(GL_INVALID_ENUM,
                                   "Sizedinternalformat is not color-renderable, depth-renderable, "
                                   "or stencil-renderable"));
        return false;
    }

    // The ES3.1 spec(section 8.8) states that An INVALID_ENUM error is generated if internalformat
    // is one of the unsized base internal formats listed in table 8.11.
    const gl::InternalFormat &formatInfo = gl::GetInternalFormatInfo(internalFormat);
    if (formatInfo.componentType == GL_UNSIGNED_INT)
    {
        context->handleError(Error(
            GL_INVALID_ENUM,
            "Internalformat is one of the unsized base internal formats listed in table 8.11"));
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
    if (texture->getImmutableFormat())
    {
        context->handleError(Error(GL_INVALID_OPERATION,
                                   "The value of TEXTURE_IMMUTABLE_FORMAT for the texture "
                                   "currently bound to target on the active texture unit is true"));
        return false;
    }

    return true;
}

bool ValidateGetMultisample(Context *context, GLenum pname, GLuint index, GLfloat *val)
{
    if (context->getClientVersion() < ES_3_1)
    {
        context->handleError(Error(GL_INVALID_OPERATION, "Context does not support GLES3.1"));
        return false;
    }

    if (pname != GL_SAMPLE_POSITION)
    {
        context->handleError(Error(GL_INVALID_ENUM, "Pname is not SAMPLE_POSITION"));
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
