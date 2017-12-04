//
// Copyright 2017 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//
// entry_points_gles_3_0_ext.cpp:
//   Defines the GLES 3.0 extention entry points.

#include "libGLESv2/entry_points_gles_3_0_ext.h"

#include "libANGLE/Context.h"
#include "libANGLE/validationES3.h"
#include "libANGLE/validationES31.h"
#include "libGLESv2/global_state.h"

namespace gl
{
void GL_APIENTRY TexStorage2DMultisampleANGLE(GLenum target,
                                              GLsizei samples,
                                              GLenum internalformat,
                                              GLsizei width,
                                              GLsizei height,
                                              GLboolean fixedsamplelocations)
{
    EVENT(
        "(GLenum target = 0x%X, GLsizei samples = %d, GLenum internalformat = 0x%X, GLsizei width "
        "= %d, GLsizei height = %d, GLboolean fixedsamplelocations = %u)",
        target, samples, internalformat, width, height, fixedsamplelocations);

    Context *context = GetValidGlobalContext();
    if (context)
    {
        if (!context->getExtensions().textureMultisample)
        {
            context->handleError(InvalidOperation());
            return;
        }

        if (context->skipValidation() ||
            (context->getClientVersion() < ES_3_1 &&
             !ValidateTexStorage2DMultisampleANGLE(context, target, samples, internalformat, width,
                                                   height, fixedsamplelocations)))
        {
            return;
        }

        if (context->skipValidation() ||
            (context->getClientVersion() >= ES_3_1 &&
             !ValidateTexStorage2DMultisample(context, target, samples, internalformat, width,
                                              height, fixedsamplelocations)))
            return;
    }

    context->texStorage2DMultisample(target, samples, internalformat, width, height,
                                     fixedsamplelocations);
}

void GL_APIENTRY GetMultisamplefvANGLE(GLenum pname, GLuint index, GLfloat *val)
{
    EVENT("(GLenum pname = 0x%X, GLuint index = %u, GLfloat *val = 0x%0.8p)", pname, index, val);

    Context *context = GetValidGlobalContext();

    // TODO(yizhou): implement GetMultisamplefvANGLE.
    if (context)
    {
        if (!context->skipValidation())
        {
            context->handleError(InvalidOperation() << "Entry point not implemented");
        }
        UNIMPLEMENTED();
    }
}

void GL_APIENTRY SampleMaskiANGLE(GLuint maskNumber, GLbitfield mask)
{
    EVENT("(GLuint maskNumber = %u, GLbitfield mask = 0x%X)", maskNumber, mask);

    Context *context = GetValidGlobalContext();

    // TODO(yizhou): implement SampleMaskiANGLE.
    if (context)
    {
        if (!context->skipValidation())
        {
            context->handleError(InvalidOperation() << "Entry point not implemented");
        }
        UNIMPLEMENTED();
    }
}

}  // namespace gl
