//
// Copyright (c) 2002-2016 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//

// Path.h: Defines the gl::Path class, representing CHROMIUM_path_rendering
// path object.

#ifndef LIBANGLE_PATH_H
#define LIBNAGLE_PATH_H

#include "angle_gl.h"
#include "libANGLE/RefCountObject.h"
#include "libANGLE/Error.h"


namespace rx
{
class PathImpl;
}

namespace gl
{
    class Path final
    {
    public:
        Path(rx::PathImpl *impl);

       ~Path();

        Error setCommands(GLsizei numCommands,
                          const GLubyte *commands,
                          GLsizei numCoords,
                          GLenum coordType,
                          const void *coords);

        void setParameter(GLenum pname, GLfloat value);

        void getParameter(GLenum pname, GLfloat *value) const;

        bool hasPathData() const { return mHasData; }

        const rx::PathImpl* getImplementation() const { return mPath; }

    private:
        rx::PathImpl *mPath;
    private:
        bool mHasData;
    private:
        GLenum  mEndCaps;
        GLenum  mJoinStyle;
        GLfloat mStrokeWidth;
        GLfloat mStrokeBound;
        GLfloat mMiterLimit;
    };

} // gl


#endif // LIBANGLE_PATH_H