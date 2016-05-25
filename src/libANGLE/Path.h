//
// Copyright (c) 2002-2016 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//

// Path.h: Defines the gl::Path class, representing CHROMIUM_path_rendering
// path object.

#ifndef LIBANGLE_PATH_H
#define LIBANGLE_PATH_H

#include "angle_gl.h"
#include "common/angleutils.h"
#include "libANGLE/RefCountObject.h"
#include "libANGLE/Error.h"


namespace rx
{
class PathImpl;
}

namespace gl
{
    class Path final : public angle::NonCopyable
    {
    public:
        Path(rx::PathImpl *impl);

       ~Path();

        Error setCommands(GLsizei numCommands,
                          const GLubyte *commands,
                          GLsizei numCoords,
                          GLenum coordType,
                          const void *coords);

        void setStrokeWidth(GLfloat width);
        void setStrokeBound(GLfloat bound);
        void setEndCaps(GLenum type);
        void setJoinStyle(GLenum type);
        void setMiterLimit(GLfloat value);

        GLfloat getStrokeWidth() const { return mStrokeWidth; }
        GLfloat getStrokeBound() const { return mStrokeBound; }
        GLfloat getMiterLimit() const { return mMiterLimit; }
        GLenum getEndCaps() const { return mEndCaps; }
        GLenum getJoinStyle() const { return mJoinStyle; }

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