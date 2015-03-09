//
// Copyright (c) 2015 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//

// StateManagerGL.h: Defines a class for caching applied OpenGL state

#ifndef LIBANGLE_RENDERER_GL_STATEMANAGERGL_H_
#define LIBANGLE_RENDERER_GL_STATEMANAGERGL_H_

#include "common/debug.h"
#include "libANGLE/renderer/gl/functionsgl_typedefs.h"

#include <map>

namespace gl
{
class State;
}

namespace rx
{

class FunctionsGL;

class StateManagerGL
{
  public:
    StateManagerGL(const FunctionsGL *functions);

    void useProgram(GLuint program);
    void bindVertexArray(GLuint vao);
    void bindBuffer(GLenum type, GLuint buffer);

    void setDrawArraysState(const gl::State &state, GLint first, GLsizei count);
    void setDrawElementsState(const gl::State &state, GLsizei count, GLenum type, const GLvoid *indices,
                              const GLvoid **outIndices);

  private:
    DISALLOW_COPY_AND_ASSIGN(StateManagerGL);

    void setGenericDrawState(const gl::State &state);

    const FunctionsGL *mFunctions;

    GLuint mProgram;
    GLuint mVAO;
    std::map<GLenum, GLuint> mBuffers;
};

}

#endif // LIBANGLE_RENDERER_GL_STATEMANAGERGL_H_
