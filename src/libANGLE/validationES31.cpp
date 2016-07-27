//
// Copyright (c) 2016 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//

// validationES31.cpp: Validation functions for OpenGL ES 3.1 entry point parameters
#include "libANGLE/validationES.h"
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

bool ValidateDispatchCompute(Context *context,
                             GLuint numGroupsX,
                             GLuint numGroupsY,
                             GLuint numGroupsZ)
{

    if (!context->getGLVersion().isES31())
    {
        context->handleError(Error(GL_INVALID_OPERATION, "Context does not support ES 3.1."));
        return false;
    }

    const gl::State &state = context->getGLState();

    if (!ValidateProgramUniforms(context))
    {
        return false;
    }
    gl::Program *program = state.getProgram();

    if (!program->isComputeProgram())
    {
        context->handleError(
            Error(GL_INVALID_OPERATION, "No active program for the compute shader stage."));
        return false;
    }

    if (numGroupsX < 1)
    {
        context->handleError(Error(GL_INVALID_VALUE, "numGroupsX must be positive."));
        return false;
    }

    if (numGroupsY < 1)
    {
        context->handleError(Error(GL_INVALID_VALUE, "numGroupsY must be positive."));
        return false;
    }

    if (numGroupsZ < 1)
    {
        context->handleError(Error(GL_INVALID_VALUE, "numGroupsZ must be positive."));
        return false;
    }

    const gl::Caps &caps = context->getCaps();

    if (numGroupsX > caps.maxComputeWorkGroupCount[0])
    {
        context->handleError(Error(GL_INVALID_VALUE, "numGroupsX cannot be more than %u.",
                                   caps.maxComputeWorkGroupCount[0]));
        return false;
    }

    if (numGroupsY > caps.maxComputeWorkGroupCount[1])
    {
        context->handleError(Error(GL_INVALID_VALUE, "numGroupsY cannot be more than %u.",
                                   caps.maxComputeWorkGroupCount[1]));
        return false;
    }

    if (numGroupsZ > caps.maxComputeWorkGroupCount[2])
    {
        context->handleError(Error(GL_INVALID_VALUE, "numGroupsZ cannot be more than %u.",
                                   caps.maxComputeWorkGroupCount[2]));
        return false;
    }

    return true;
}

bool ValidateMemoryBarrier(Context *context, GLbitfield barriers)
{

    if ((barriers & ~GL_ALL_BARRIER_BITS) != 0)
    {
        context->handleError(Error(GL_INVALID_VALUE, "Invalid barrier passed to glMemoryBarrier."));
        return false;
    }

    return true;
}

bool ValidateMemoryBarrierByRegion(Context *context, GLbitfield barriers)
{
    static const GLbitfield validBitmask = GL_ATOMIC_COUNTER_BARRIER_BIT |
                                           GL_FRAMEBUFFER_BARRIER_BIT |
                                           GL_SHADER_IMAGE_ACCESS_BARRIER_BIT |
                                           GL_SHADER_STORAGE_BARRIER_BIT |
                                           GL_TEXTURE_FETCH_BARRIER_BIT |
                                           GL_UNIFORM_BARRIER_BIT;
    if (barriers != GL_ALL_BARRIER_BITS && (barriers & ~validBitmask) != 0)
    {
        context->handleError(Error(GL_INVALID_VALUE, "Invalid barrier passed to glMemoryBarrierByRegion."));
        return false;
    }

    return true;
}

}  // namespace gl
