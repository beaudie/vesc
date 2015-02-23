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

namespace rx
{

StateManagerGL::StateManagerGL(const FunctionsGL *functions)
    : mFunctions(functions),
      mProgram(0)
{
    ASSERT(mFunctions);
}

void StateManagerGL::setProgram(GLuint program)
{
    if (mProgram != program)
    {
        mProgram = program;
        mFunctions->useProgram(mProgram);
    }
}

void StateManagerGL::setVertexAttribute(size_t index, const gl::VertexAttribute &attribute)
{
    if (attribute.buffer.id() != 0)
    {
        UNIMPLEMENTED();
    }

    // Enable or disable the attribute
    if (attribute.enabled)
    {
        mFunctions->enableVertexAttribArray(index);
    }
    else
    {
        mFunctions->disableVertexAttribArray(index);
    }

    mFunctions->vertexAttribPointer(index, attribute.size, attribute.type, attribute.normalized, attribute.stride, attribute.pointer);
}

void StateManagerGL::setDrawState(const gl::State &state)
{
    const gl::VertexArray *vao = state.getVertexArray();
    for (size_t attribIdx = 0; attribIdx < vao->getMaxAttribs(); attribIdx++)
    {
        setVertexAttribute(attribIdx, vao->getVertexAttribute(attribIdx));
        //const gl::VertexAttribute &attribute = vao->getVertexAttribute(attribIdx);
    }

    const gl::Program *program = state.getProgram();
    const ProgramGL *programGL = GetImplAs<ProgramGL>(program); 
    setProgram(programGL->getProgramID());
}

}
