//
// Copyright 2016 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//

// PathGL.cpp: Implements the class methods for PathGL.

#include "libANGLE/renderer/gl/PathRenderingGL.h"

#include "common/debug.h"
#include "libANGLE/renderer/gl/FunctionsGL.h"
#include "libANGLE/renderer/gl/StateManagerGL.h"
#include "libANGLE/renderer/gl/renderergl_utils.h"

// todo: should we check for GL errors from the driver here?

namespace rx
{

PathRenderingGL::PathRenderingGL(const FunctionsGL *functions, StateManagerGL *stateManager)
    : mFunctions(functions),
      mStateManager(stateManager)
{
    ASSERT(mFunctions);
    ASSERT(mStateManager);
}

PathRenderingGL::~PathRenderingGL()
{}

gl::Error PathRenderingGL::createPaths(GLsizei range, GLuint *first)
{
    *first = mFunctions->genPathsNV(range);

    return gl::Error(GL_NO_ERROR);
}

gl::Error PathRenderingGL::deletePaths(GLuint path, GLsizei range)
{
    mFunctions->delPathsNV(path, range);

    return gl::Error(GL_NO_ERROR);
}

gl::Error PathRenderingGL::setCommands(GLuint path, GLsizei numCommands, const GLubyte *commands, GLsizei numCoords, GLenum coordType, const void *coords)
{
    mFunctions->pathCommandsNV(path, numCommands, commands, numCoords, coordType, coords);
    return gl::Error(GL_NO_ERROR);
}

void PathRenderingGL::setPathParameter(GLuint path, GLenum pname, GLfloat value)
{
    mFunctions->setPathParameterfNV(path, pname, value);

    assert(mFunctions->getError() == GL_NO_ERROR);
}

void PathRenderingGL::getPathParameter(GLuint path, GLenum pname, GLfloat *value) const
{
    if (pname == GL_PATH_END_CAPS_CHROMIUM)
        pname = GL_PATH_INITIAL_END_CAP_NV;

    mFunctions->getPathParameterfNV(path, pname, value);

    assert(mFunctions->getError() == GL_NO_ERROR);
}

bool PathRenderingGL::isPath(GLuint path) const
{
    return (bool)mFunctions->isPathNV(path);
}

void PathRenderingGL::stencilFillPath(GLuint path, GLenum fillMode, GLuint mask)
{
    mFunctions->stencilFillPathNV(path, fillMode, mask);

    assert(mFunctions->getError() == GL_NO_ERROR);
}

void PathRenderingGL::stencilStrokePath(GLuint path, GLint reference, GLuint mask)
{
    mFunctions->stencilStrokePathNV(path, reference, mask);

    assert(mFunctions->getError() == GL_NO_ERROR);
}

void PathRenderingGL::coverFillPath(GLuint path, GLenum coverMode)
{
    mFunctions->coverFillPathNV(path, coverMode);

    assert(mFunctions->getError() == GL_NO_ERROR);
}

void PathRenderingGL::coverStrokePath(GLuint path, GLenum coverMode)
{
    mFunctions->coverStrokePathNV(path, coverMode);

    assert(mFunctions->getError() == GL_NO_ERROR);
}

void PathRenderingGL::stencilThenCoverFillPath(GLuint path, GLenum fillMode, GLuint mask, GLenum coverMode)
{
    mFunctions->stencilThenCoverFillPathNV(path, fillMode, mask, coverMode);

    assert(mFunctions->getError() == GL_NO_ERROR);
}

void PathRenderingGL::stencilThenCoverStrokePath(GLuint path, GLint reference, GLuint mask, GLenum coverMode)
{
    mFunctions->stencilThenCoverStrokePathNV(path, reference, mask, coverMode);

    assert(mFunctions->getError() == GL_NO_ERROR);
}



} // rx