#include "precompiled.h"
//
// Copyright (c) 2013 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//
// Implementation of the state class for mananging GLES 3 Vertex Array Objects.
//

#include "libGLESv2/VertexArray.h"
#include "libGLESv2/Buffer.h"
#include "libGLESv2/renderer/Renderer.h"
#include "libGLESv2/renderer/VertexArrayImpl.h"

namespace gl
{

VertexArray::VertexArray(rx::Renderer *renderer, GLuint id)
    : RefCountObject(id)
{
    mVertexArray = renderer->createVertexArray();
}

VertexArray::~VertexArray()
{
    for (int i = 0; i < MAX_VERTEX_ATTRIBS; i++)
    {
        mVertexAttributes[i].buffer.set(NULL);
    }
    mElementArrayBuffer.set(NULL);
}

void VertexArray::detachBuffer(GLuint bufferName)
{
    for (int attribute = 0; attribute < MAX_VERTEX_ATTRIBS; attribute++)
    {
        if (mVertexAttributes[attribute].buffer.id() == bufferName)
        {
            mVertexAttributes[attribute].buffer.set(NULL);
        }
    }

    if (mElementArrayBuffer.id() == bufferName)
    {
        mElementArrayBuffer.set(NULL);
    }
}

const VertexAttribute& VertexArray::getVertexAttribute(unsigned int attributeIndex) const
{
    ASSERT(attributeIndex < MAX_VERTEX_ATTRIBS);
    return mVertexAttributes[attributeIndex];
}

void VertexArray::setVertexAttribDivisor(GLuint index, GLuint divisor)
{
    ASSERT(index < gl::MAX_VERTEX_ATTRIBS);
    mVertexAttributes[index].divisor = divisor;
    mVertexArray->setAttributeDivisor(index, divisor);
}

void VertexArray::enableAttribute(unsigned int attributeIndex, bool enabledState)
{
    ASSERT(attributeIndex < gl::MAX_VERTEX_ATTRIBS);
    mVertexAttributes[attributeIndex].enabled = enabledState;
    mVertexArray->enableAttribute(attributeIndex, enabledState);
}

void VertexArray::setAttributeState(unsigned int attributeIndex, gl::Buffer *boundBuffer, GLint size, GLenum type,
                                    bool normalized, bool pureInteger, GLsizei stride, const void *pointer)
{
    ASSERT(attributeIndex < gl::MAX_VERTEX_ATTRIBS);
    mVertexAttributes[attributeIndex].buffer.set(boundBuffer);
    mVertexAttributes[attributeIndex].size = size;
    mVertexAttributes[attributeIndex].type = type;
    mVertexAttributes[attributeIndex].normalized = normalized;
    mVertexAttributes[attributeIndex].pureInteger = pureInteger;
    mVertexAttributes[attributeIndex].stride = stride;
    mVertexAttributes[attributeIndex].pointer = pointer;
    mVertexArray->setAttribute(attributeIndex, mVertexAttributes[attributeIndex]);
}

void VertexArray::setElementArrayBuffer(Buffer *buffer)
{ 
    mElementArrayBuffer.set(buffer);
    mVertexArray->setElementArrayBuffer(buffer);
}

}