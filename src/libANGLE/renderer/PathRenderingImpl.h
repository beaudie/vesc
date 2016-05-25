//
// Copyright 2016 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//

// PathRenderingImpl.h: Defines the abstract interface for path rendering.

#ifndef LIBANGLE_RENDERER_PATHRENDERING_IMPL_H
#define LIBANGLE_RENDERER_PATHRENDERING_IMPL_H

#include "common/angleutils.h"
#include "libANGLE/Error.h"

namespace rx
{

class PathRenderingImpl : angle::NonCopyable
{
  public:
    virtual ~PathRenderingImpl() {}

    virtual gl::Error createPaths(GLsizei range, GLuint *first) = 0;

    virtual gl::Error deletePaths(GLuint path, GLsizei range) = 0;

    virtual gl::Error setCommands(GLuint path, GLsizei numCommands,
                      const GLubyte *commands,
                      GLsizei numCoords,
                      GLenum coordType,
                      const void *coords) = 0;

    virtual void setPathParameter(GLuint path, GLenum pname, GLfloat value) = 0;

    virtual void getPathParameter(GLuint path, GLenum pname, GLfloat *value) const = 0;

    virtual bool isPath(GLuint path) const = 0;

    virtual void stencilFillPath(GLuint path, GLenum fillMode, GLuint mask) = 0;
    virtual void stencilStrokePath(GLuint path, GLint ference, GLuint mask) = 0;
    virtual void coverFillPath(GLuint path, GLenum coverMode) = 0;
    virtual void coverStrokePath(GLuint path, GLenum coverMode) = 0;
    virtual void stencilThenCoverFillPath(GLuint path, GLenum fillMode, GLuint mask, GLenum coverMode) = 0;
    virtual void stencilThenCoverStrokePath(GLuint path, GLint reference, GLuint mask, GLenum coverMode) = 0;
};

} // rx

#endif // LIBANGLE_RENDERER_PATHRENDERING_IMPL_H
