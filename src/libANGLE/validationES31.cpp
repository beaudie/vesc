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
#include "libANGLE/Framebuffer.h"

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

bool ValidationFramebufferParameteri(Context *context, GLenum target, GLenum pname, GLint param)
{
    if (context->getClientVersion() < ES_3_1)
    {
        context->handleError(Error(GL_INVALID_OPERATION, "Context does not support GLES3.1"));
        return false;
    }

    if (!ValidFramebufferTarget(target))
    {
        context->handleError(Error(GL_INVALID_ENUM));
        return false;
    }

    if (pname != GL_FRAMEBUFFER_DEFAULT_WIDTH && pname != GL_FRAMEBUFFER_DEFAULT_HEIGHT &&
        pname != GL_FRAMEBUFFER_DEFAULT_SAMPLES &&
        pname != GL_FRAMEBUFFER_DEFAULT_FIXED_SAMPLE_LOCATIONS)
    {
        context->handleError(Error(GL_INVALID_ENUM, "Invalid framebuffer pname"));
        return false;
    }

    if (pname == GL_FRAMEBUFFER_DEFAULT_WIDTH)
    {
        if (param < 0 || param > GL_MAX_FRAMEBUFFER_WIDTH)
        {
            context->handleError(Error(
                GL_INVALID_VALUE, "Params less than 0 or greater than GL_MAX_FRAMEBUFFER_WIDTH"));
            return false;
        }
    }

    if (pname == GL_FRAMEBUFFER_DEFAULT_HEIGHT)
    {
        if (param < 0 || param > GL_MAX_FRAMEBUFFER_HEIGHT)
        {
            context->handleError(Error(
                GL_INVALID_VALUE, "Params less than 0 or greater than GL_MAX_FRAMEBUFFER_HEIGHT"));
            return false;
        }
    }

    if (pname == GL_FRAMEBUFFER_DEFAULT_SAMPLES)
    {
        if (param < 0 || param > GL_MAX_FRAMEBUFFER_SAMPLES)
        {
            context->handleError(Error(
                GL_INVALID_VALUE, "Params less than 0 or greater than GL_MAX_FRAMEBUFFER_SAMPLES"));
            return false;
        }
    }

    const Framebuffer *framebuffer = context->getGLState().getTargetFramebuffer(target);
    ASSERT(framebuffer);

    if (framebuffer->id() == 0)
    {
        context->handleError(Error(GL_INVALID_OPERATION));
        return false;
    }
    return true;
}

bool ValidationGetFramebufferParameteri(Context *context,
                                        GLenum target,
                                        GLenum pname,
                                        GLint *params)
{
    if (context->getClientVersion() < ES_3_1)
    {
        context->handleError(Error(GL_INVALID_OPERATION, "Context does not support GLES3.1"));
        return false;
    }

    if (!ValidFramebufferTarget(target))
    {
        context->handleError(Error(GL_INVALID_ENUM));
        return false;
    }

    if (pname != GL_FRAMEBUFFER_DEFAULT_WIDTH && pname != GL_FRAMEBUFFER_DEFAULT_HEIGHT &&
        pname != GL_FRAMEBUFFER_DEFAULT_SAMPLES &&
        pname != GL_FRAMEBUFFER_DEFAULT_FIXED_SAMPLE_LOCATIONS)
    {
        context->handleError(Error(GL_INVALID_ENUM, "Invalid framebuffer pname"));
        return false;
    }

    const Framebuffer *framebuffer = context->getGLState().getTargetFramebuffer(target);
    ASSERT(framebuffer);

    if (framebuffer->id() == 0)
    {
        context->handleError(Error(GL_INVALID_OPERATION));
        return false;
    }
    return true;
}

}  // namespace gl
