//
// Copyright (c) 2015 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//

// StateManagerGL.h: Defines a class for caching applied OpenGL state

#include "libANGLE/renderer/gl/StateManagerGL.h"

#include "libANGLE/Data.h"
#include "libANGLE/VertexArray.h"
#include "libANGLE/renderer/gl/FunctionsGL.h"
#include "libANGLE/renderer/gl/ProgramGL.h"
#include "libANGLE/renderer/gl/VertexArrayGL.h"

namespace rx
{

StateManagerGL::StateManagerGL(const FunctionsGL *functions)
    : mFunctions(functions),
      mProgram(0),
      mVAO(0),
      mBuffers()
{
    ASSERT(mFunctions);
}

void StateManagerGL::useProgram(GLuint program)
{
    if (mProgram != program)
    {
        mProgram = program;
        mFunctions->useProgram(mProgram);
    }
}

void StateManagerGL::bindVertexArray(GLuint vao)
{
    if (mVAO != vao)
    {
        mVAO = vao;
        mFunctions->bindVertexArray(vao);
    }
}

void StateManagerGL::bindBuffer(GLenum type, GLuint buffer)
{
    if (mBuffers[type] == 0)
    {
        mBuffers[type] = buffer;
        mFunctions->bindBuffer(type, buffer);
    }
}

void StateManagerGL::setDrawArraysState(const gl::State &state, GLint first, GLsizei count)
{
    const gl::VertexArray *vao = state.getVertexArray();
    const VertexArrayGL *vaoGL = GetImplAs<VertexArrayGL>(vao);
    vaoGL->syncDrawArraysState(first, count);
    bindVertexArray(vaoGL->getVertexArrayID());

    setGenericDrawState(state);
}

void StateManagerGL::setDrawElementsState(const gl::State &state, GLsizei count, GLenum type, const GLvoid *indices,
                                          const GLvoid **outIndices)
{
    const gl::VertexArray *vao = state.getVertexArray();
    const VertexArrayGL *vaoGL = GetImplAs<VertexArrayGL>(vao);
    vaoGL->syncDrawElementsState(count, type, indices, outIndices);
    bindVertexArray(vaoGL->getVertexArrayID());

    setGenericDrawState(state);
}

void StateManagerGL::setGenericDrawState(const gl::State &state)
{
    const gl::Program *program = state.getProgram();
    const ProgramGL *programGL = GetImplAs<ProgramGL>(program);
    useProgram(programGL->getProgramID());
}

}
