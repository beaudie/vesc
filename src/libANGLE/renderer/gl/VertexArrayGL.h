//
// Copyright 2015 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//

// VertexArrayGL.h: Defines the class interface for VertexArrayGL.

#ifndef LIBANGLE_RENDERER_GL_VERTEXARRAYGL_H_
#define LIBANGLE_RENDERER_GL_VERTEXARRAYGL_H_

#include "libANGLE/renderer/VertexArrayImpl.h"

namespace rx
{

class FunctionsGL;
class StateManagerGL;

class VertexArrayGL : public VertexArrayImpl
{
  public:
    VertexArrayGL(const FunctionsGL *functions, StateManagerGL *stateManager);
    ~VertexArrayGL() override;

    void setElementArrayBuffer(const gl::Buffer *buffer) override;
    void setAttribute(size_t idx, const gl::VertexAttribute &attr) override;
    void setAttributeDivisor(size_t idx, GLuint divisor) override;
    void enableAttribute(size_t idx, bool enabledState) override;

    gl::Error syncDrawArraysState(GLint first, GLsizei count) const;
    gl::Error syncDrawElementsState(GLsizei count, GLenum type, const GLvoid *indices, const GLvoid **outIndices) const;

    GLuint getVertexArrayID() const;

  private:
    DISALLOW_COPY_AND_ASSIGN(VertexArrayGL);

    gl::Error syncDrawState(GLint first, GLsizei count, GLenum type, const GLvoid *indices, const GLvoid **outIndices) const;

    const FunctionsGL *mFunctions;
    StateManagerGL *mStateManager;

    GLuint mVertexArrayID;

    BindingPointer<const gl::Buffer> mElementArrayBuffer;
    std::vector<gl::VertexAttribute> mAttributes;

    mutable GLuint mAppliedElementArrayBuffer;
    mutable std::vector<gl::VertexAttribute> mAppliedAttributes;

    mutable size_t mStreamingElementArrayBufferSize;
    mutable GLuint mStreamingElementArrayBuffer;

    mutable size_t mStreamingArrayBufferSize;
    mutable GLuint mStreamingArrayBuffer;
};

}

#endif // LIBANGLE_RENDERER_GL_VERTEXARRAYGL_H_
