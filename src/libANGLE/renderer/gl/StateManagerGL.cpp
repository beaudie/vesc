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
      mBuffers(),
      mTextureUnit(GL_TEXTURE0),
      mTextures(),
      mUnpackAlignment(4),
      mUnpackRowLength(0)
{
    ASSERT(mFunctions);

    GLint textureUnits;
    mFunctions->getIntegerv(GL_MAX_TEXTURE_IMAGE_UNITS, &textureUnits);
    mTextures.resize(textureUnits);
}

void StateManagerGL::setProgram(GLuint program)
{
    if (mProgram != program)
    {
        mProgram = program;
        mFunctions->useProgram(mProgram);
    }
}

void StateManagerGL::setVertexArray(GLuint vao)
{
    if (mVAO != vao)
    {
        mVAO = vao;
        mFunctions->bindVertexArray(vao);
    }
}

void StateManagerGL::setBuffer(GLenum type, GLuint buffer)
{
    if (mBuffers[type] == 0)
    {
        mBuffers[type] = buffer;
        mFunctions->bindBuffer(type, buffer);
    }
}

void StateManagerGL::setTextureUnit(GLenum unit)
{
    if (mTextureUnit != unit)
    {
        mTextureUnit = unit;
        mFunctions->activeTexture(unit);
    }
}

void StateManagerGL::setTexture(GLenum type, GLuint texture)
{
    if (mTextures[mTextureUnit] != texture)
    {
        mTextures[mTextureUnit] = texture;
        mFunctions->bindTexture(type, texture);
    }
}

void StateManagerGL::setPixelUnpackState(GLint alignment, GLint rowLength)
{
    if (mUnpackAlignment != alignment)
    {
        mUnpackAlignment = alignment;
        mFunctions->pixelStorei(GL_UNPACK_ALIGNMENT, mUnpackAlignment);
    }

    if (mUnpackRowLength != rowLength)
    {
        mUnpackRowLength = rowLength;
        mFunctions->pixelStorei(GL_UNPACK_ROW_LENGTH, mUnpackRowLength);
    }
}

void StateManagerGL::setDrawState(const gl::State &state)
{
    const gl::VertexArray *vao = state.getVertexArray();
    const VertexArrayGL *vaoGL = GetImplAs<VertexArrayGL>(vao);
    setVertexArray(vaoGL->getVertexArrayID());

    const gl::Program *program = state.getProgram();
    const ProgramGL *programGL = GetImplAs<ProgramGL>(program);
    setProgram(programGL->getProgramID());

    for (size_t textureUnitIndex = 0; textureUnitIndex < mTextures.size(); textureUnitIndex++)
    {
    }
}

}
