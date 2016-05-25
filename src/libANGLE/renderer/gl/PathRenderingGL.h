//
// Copyright 2016 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//

// PathRenderingGL.h: Class definition for the GL implementation for PathRendering class.

#ifndef LIBANGLE_RENDERER_GL_PATHRENDERING_IMPL_H
#define LIBANGLE_RENDERER_GL_PATHRENDERING_IMPL_H

#include "libANGLE/renderer/PathRenderingImpl.h"

namespace rx
{

class FunctionsGL;
class StateManagerGL;

class PathRenderingGL : public PathRenderingImpl
{
  public:
    PathRenderingGL(const FunctionsGL *functions, StateManagerGL *stateManager);
   ~PathRenderingGL();

    gl::Error createPaths(GLsizei range, GLuint *first) override;

    gl::Error deletePaths(GLuint path, GLsizei range) override;

    gl::Error setCommands(GLuint path, GLsizei numCommands,
                      const GLubyte *commands,
                      GLsizei numCoords,
                      GLenum coordType,
                      const void *coords) override;

    void setPathParameter(GLuint path, GLenum pname, GLfloat value) override;

    void getPathParameter(GLuint path, GLenum pname, GLfloat *value) const override;

    bool isPath(GLuint handle) const override;

    void stencilFillPath(GLuint path, GLenum fillMode, GLuint mask) override;
    void stencilStrokePath(GLuint path, GLint ference, GLuint mask) override;
    void coverFillPath(GLuint path, GLenum coverMode) override;
    void coverStrokePath(GLuint path, GLenum coverMode) override;
    void stencilThenCoverFillPath(GLuint path, GLenum fillMode, GLuint mask, GLenum coverMode) override;
    void stencilThenCoverStrokePath(GLuint path, GLint reference, GLuint mask, GLenum coverMode) override;

  private:
    const FunctionsGL *mFunctions;
    StateManagerGL *mStateManager;

};

} // rx

#endif // LIBANGLE_RENDERER_GL_PATHRENDERING_IMPL_H