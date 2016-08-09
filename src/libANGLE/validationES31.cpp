//
// Copyright (c) 2016 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//

// validationES31.cpp: Validation functions for OpenGL ES 3.1 entry point parameters

#include "libANGLE/validationES3.h"
#include "libANGLE/validationES31.h"

#include "libANGLE/Context.h"

using namespace angle;

namespace gl
{

bool ValidateGetBooleani_v(Context *context, GLenum target, GLuint index, GLboolean *data)
{
    if (!context->getGLVersion().isES31())
    {
        context->handleError(Error(GL_INVALID_OPERATION, "Context does not support GLES3.1"));
        return false;
    }

    if (!ValidateIndexedStateQuery(context, target, index))
    {
        return false;
    }

    return true;
}

bool ValidateBindImageTexture(Context *context,
                              GLuint unit,
                              GLuint texture,
                              GLint level,
                              GLboolean layered,
                              GLint layer,
                              GLenum access,
                              GLenum format)
{

    const gl::Caps &caps = context->getCaps();

    if (unit >= caps.maxImageUnits)
    {
        context->handleError(
            Error(GL_INVALID_VALUE, "Unit cannot be bigger or equal to %u.", caps.maxImageUnits));
        return false;
    }

    if (texture != 0)
    {
        gl::Texture *tex = context->getTexture(texture);

        if (tex == nullptr)
        {
            context->handleError(Error(GL_INVALID_VALUE, "Not an existing texture name."));
            return false;
        }

        if (tex->getImmutableFormat() == false)
        {
            context->handleError(Error(GL_INVALID_OPERATION, "The texture must be immutable."));
            return false;
        }

        if (int(tex->getMaxLevel()) < level)
        {
            context->handleError(
                Error(GL_INVALID_VALUE, "Level is bigger than the max level of the texture."));
            return false;
        }

        if (level < int(tex->getBaseLevel()))
        {
            context->handleError(
                Error(GL_INVALID_VALUE, "Level is smaller than the base level of the texture."));
            return false;
        }

        switch (tex->getTarget())
        {
            case GL_TEXTURE_2D:
            case GL_TEXTURE_2D_ARRAY:
            case GL_TEXTURE_3D:
            case GL_TEXTURE_CUBE_MAP:
                break;
            default:
                context->handleError(Error(GL_INVALID_VALUE, "invalid texture target."));
                return false;
                break;
        }

        if (layer < 0)
        {
            context->handleError(Error(GL_INVALID_VALUE, "Layer cannot be negative"));
            return false;
        }

        switch (access)
        {
            case GL_READ_ONLY:
            case GL_WRITE_ONLY:
            case GL_READ_WRITE:
                break;
            default:
                context->handleError(Error(GL_INVALID_ENUM, "access"));
                return false;
        }

        switch (format)
        {
            case GL_RGBA32F:
            case GL_RGBA16F:
            case GL_R32F:
            case GL_RGBA32UI:
            case GL_RGBA16UI:
            case GL_RGBA8UI:
            case GL_R32UI:
            case GL_RGBA32I:
            case GL_RGBA16I:
            case GL_RGBA8I:
            case GL_R32I:
            case GL_RGBA8:
            case GL_RGBA8_SNORM:
                break;
            default:
                context->handleError(Error(GL_INVALID_ENUM, "format is not supported."));
                return false;
        }
    }

    return true;
}

}  // namespace gl
