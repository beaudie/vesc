//
// Copyright(c) 2016 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//

// entry_points_gles_3_0_ext.cpp : Implements the GLES 3.x extension entry points.

#include "libGLESv2/entry_points_gles_3_0_ext.h"
#include "libGLESv2/global_state.h"

#include "libANGLE/Context.h"
#include "libANGLE/validationES3.h"

#include "common/debug.h"
#include "common/utilities.h"

namespace gl
{

// GL_EXT_blend_func_extended
// This extension is written against GL ES 3.1 but can apply to earlier versions down to 2.0.
// However if GLES 3.0 or higher is not supported these entry points are not available.
// Also GetProgramResourceLocationIndexEXT requires >= 3.1 so it's not implemented (yet).
ANGLE_EXPORT void GL_APIENTRY BindFragDataLocationIndexedEXT(GLuint program,
                                                             GLuint colorNumber,
                                                             GLuint index,
                                                             const GLchar* name)
{
    EVENT("(GLuint program = %u, GLuint colorNumber = %u, GLuint index = %u, const GLchar *name = 0x%0.8p)",
        program, colorNumber, index, name);

    Context* context = GetValidGlobalContext();
    if (context)
    {
        if (!context->skipValidation())
        {
            if (!ValidateBindFragDataLocationIndexed(context, program, colorNumber, index, name))
                return;
        }
        context->bindFragDataLocationIndexed(program, colorNumber, index, name);
    }
}


ANGLE_EXPORT GLint GL_APIENTRY GetFragDataIndexEXT(GLuint program, const GLchar* name)
{
    EVENT("(GLuint program = %u, const GLchar *name = 0x%0.8p)", program, name);

    Context* context = GetValidGlobalContext();
    if (context)
    {
        if (!context->skipValidation())
        {
            if (!ValidateGetFragDataIndex(context, program, name))
                return 0;
        }
        return context->getFragDataIndex(program, name);
    }

    return 0;
}

ANGLE_EXPORT void GL_APIENTRY BindFragDataLocationEXT(GLuint program, GLuint colorNumber, const GLchar* name)
{
    return BindFragDataLocationIndexedEXT(program, colorNumber, 0, name);
}

} // gl