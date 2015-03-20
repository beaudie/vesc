//
// Copyright 2015 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//

// BufferGL.h: Defines the class interface for BufferGL.

#ifndef LIBANGLE_RENDERER_GL_BUFFERGL_H_
#define LIBANGLE_RENDERER_GL_BUFFERGL_H_

#include "libANGLE/renderer/BufferImpl.h"
#include "libANGLE/renderer/IndexRangeCache.h"

namespace rx
{

class FunctionsGL;
class StateManagerGL;

class BufferGL : public BufferImpl
{
  public:
    BufferGL(const FunctionsGL *functions, StateManagerGL *stateManager);
    ~BufferGL() override;

    gl::Error setData(size_t size, const uint8_t *data, GLenum usage) override;
    gl::Error setSubData(size_t offset, size_t size, const uint8_t *data) override;
    gl::Error copySubData(const gl::Buffer *source, size_t sourceOffset, size_t destOffset, size_t size) override;
    gl::Error map(GLbitfield access, GLvoid **mapPtr) override;
    gl::Error mapRange(size_t offset, size_t length, GLbitfield access, GLvoid **mapPtr) override;
    gl::Error unmap(GLboolean *result) override;

    gl::Error getIndexRange(GLenum type, size_t offset, size_t count, RangeUI *outRange) const override;

    GLuint getBufferID() const;

  private:
    DISALLOW_COPY_AND_ASSIGN(BufferGL);

    mutable IndexRangeCache mIndexRangeCache;

    const FunctionsGL *mFunctions;
    StateManagerGL *mStateManager;

    GLuint mBufferID;
};

}

#endif // LIBANGLE_RENDERER_GL_BUFFERGL_H_
