//
// Copyright (c) 2015 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//

// StateManagerGL.h: Defines a class for caching applied OpenGL state

#include "libANGLE/renderer/gl/StateManagerGL.h"

#include "libANGLE/Data.h"
#include "libANGLE/Framebuffer.h"
#include "libANGLE/VertexArray.h"
#include "libANGLE/renderer/gl/FramebufferGL.h"
#include "libANGLE/renderer/gl/FunctionsGL.h"
#include "libANGLE/renderer/gl/ProgramGL.h"
#include "libANGLE/renderer/gl/VertexArrayGL.h"

namespace rx
{

StateManagerGL::StateManagerGL(const FunctionsGL *functions)
    : mFunctions(functions),
      mProgram(0),
      mVAO(0),
      mBuffers(),
      mFramebuffers()
{
    ASSERT(mFunctions);

    mFramebuffers[GL_READ_FRAMEBUFFER] = 0;
    mFramebuffers[GL_DRAW_FRAMEBUFFER] = 0;
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

void StateManagerGL::bindFramebuffer(GLenum type, GLuint framebuffer)
{
    if (mFramebuffers[type] != framebuffer)
    {
        mFramebuffers[type] = framebuffer;
        mFunctions->bindFramebuffer(type, framebuffer);
    }
}

void StateManagerGL::setDrawState(const gl::State &state)
{
    const gl::VertexArray *vao = state.getVertexArray();
    const VertexArrayGL *vaoGL = GetImplAs<VertexArrayGL>(vao);
    bindVertexArray(vaoGL->getVertexArrayID());

    const gl::Program *program = state.getProgram();
    const ProgramGL *programGL = GetImplAs<ProgramGL>(program);
    useProgram(programGL->getProgramID());

    const gl::Framebuffer *framebuffer = state.getDrawFramebuffer();
    const FramebufferGL *framebufferGL = GetImplAs<FramebufferGL>(framebuffer);
    bindFramebuffer(GL_DRAW_FRAMEBUFFER, framebufferGL->getFramebufferID());
}

}
