//
// Copyright(c) 2017 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//

// entry_points_gles_3_1_ext.h : Implements the GLES 3.1 extension entry points.

#include "libGLESv2/entry_points_gles_3_1_ext.h"
#include "libGLESv2/global_state.h"

#include "libANGLE/Context.h"
#include "libANGLE/Error.h"

#include "common/debug.h"
#include "common/utilities.h"

namespace gl
{

GL_APICALL void GL_APIENTRY FramebufferTextureEXT(GLenum target,
                                                  GLenum attachment,
                                                  GLuint texture,
                                                  GLint level)
{
    EVENT("(GLenum target = 0x%X, GLenum attachment = 0x%X, GLuint texture = %u, GLint level = %d)",
          target, attachment, texture, level);
    Context *context = GetValidGlobalContext();
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