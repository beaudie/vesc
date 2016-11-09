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

bool ValidateDispatchCompute(Context *context,
                             GLuint numGroupsX,
                             GLuint numGroupsY,
                             GLuint numGroupsZ)
{

    if (context->getClientVersion() < ES_3_1)
    {
        context->handleError(Error(GL_INVALID_OPERATION, "Context less than ES 3.1."));
        return false;
    }

    const gl::State &state = context->getGLState();

    gl::Program *program = state.getProgram();

    if (program == NULL)
    {
        context->handleError(
            Error(GL_INVALID_OPERATION, "No active program for the compute shader stage."));
        return false;
    }
    else
    {
        if (program->isLinked())
        {
            if (program->getAttachedComputeShader() == NULL)
            {
                context->handleError(Error(GL_INVALID_OPERATION,
                                           "a linked program object contains no compute shaders"));
                return false;
            }
        }
        else
        {
            context->handleError(
                Error(GL_INVALID_OPERATION, "program has not been successfully linked."));
            return false;
        }
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

}  // namespace gl
