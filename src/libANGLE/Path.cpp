//
// Copyright (c) 2002-2016 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//

// Path.h: Defines the gl::Path class, representing CHROMIUM_path_rendering
// path object.

#include "libANGLE/Path.h"
#include "libANGLE/renderer/PathImpl.h"

#include "common/mathutil.h"
#include "common/debug.h"

namespace gl
{

Path::Path(rx::PathImpl *impl)
   : mPath(impl),
     mHasData(false),
     mEndCaps(GL_FLAT_CHROMIUM),
     mJoinStyle(GL_MITER_REVERT_CHROMIUM),
     mStrokeWidth(1.0f),
     mStrokeBound(0.2f),
     mMiterLimit(4.0f)
{}

Path::~Path()
{
    delete mPath;
}

Error Path::setCommands(GLsizei numCommands,
                        const GLubyte *commands,
                        GLsizei numCoords,
                        GLenum coordType,
                        const void *coords)
{
    Error err = mPath->setCommands(numCommands, commands, numCoords, coordType, coords);
    if (!err.isError())
        mHasData = true;

    return err;
}

void Path::setParameter(GLenum pname, GLfloat value)
{
    switch (pname)
    {
        case GL_PATH_STROKE_WIDTH_CHROMIUM:
            mStrokeWidth = value;
            break;
        case GL_PATH_END_CAPS_CHROMIUM:
            mEndCaps = static_cast<GLenum>(value);
            break;
        case GL_PATH_JOIN_STYLE_CHROMIUM:
            mJoinStyle = static_cast<GLenum>(value);
            break;
        case GL_PATH_MITER_LIMIT_CHROMIUM:
            mMiterLimit = value;
            break;
        case GL_PATH_STROKE_BOUND_CHROMIUM:
            mStrokeBound = clamp(value, 0.0f, 1.0f);
            value = clamp(value, 0.0f, 1.0f);
            break;
        default:
            UNREACHABLE();
            break;
    }
    mPath->setPathParameter(pname, value);
}

void Path::getParameter(GLenum pname, GLfloat *value) const
{
    ASSERT(value);
    switch (pname)
    {
        case GL_PATH_STROKE_WIDTH_CHROMIUM:
            *value = mStrokeWidth;
            break;
        case GL_PATH_END_CAPS_CHROMIUM:
            *value = static_cast<GLfloat>(mEndCaps);
            break;
        case GL_PATH_JOIN_STYLE_CHROMIUM:
            *value = static_cast<GLfloat>(mJoinStyle);
            break;
        case GL_PATH_MITER_LIMIT_CHROMIUM:
            *value = mMiterLimit;
            break;
        case GL_PATH_STROKE_BOUND_CHROMIUM:
            *value = mStrokeBound;
            break;
        default:
            UNREACHABLE();
            break;
    }
}


} // gl

