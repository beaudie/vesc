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

bool ValidateGetBooleaniv(Context *context,
                          GLenum target,
                          GLuint index,
                          GLboolean *data,
                          GLenum *nativeType,
                          unsigned int *numParams)
{
    if (!context->getGLVersion().isES31())
    {
        context->handleError(Error(GL_INVALID_OPERATION, "Context does not support GLES3.1"));
        return false;
    }

    if (!ValidateIndexedStateQuery(context, target, index, nativeType, numParams))
    {
        return false;
    }

    return true;
}

}  // namespace gl
