//
// Copyright 2024 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//
// VertexArrayWgpu.h:
//    Defines the class interface for VertexArrayWgpu, implementing VertexArrayImpl.
//

#ifndef LIBANGLE_RENDERER_WGPU_VERTEXARRAYWGPU_H_
#define LIBANGLE_RENDERER_WGPU_VERTEXARRAYWGPU_H_

#include "libANGLE/renderer/VertexArrayImpl.h"
#include "libANGLE/renderer/wgpu/BufferWgpu.h"
#include "libANGLE/renderer/wgpu/wgpu_pipeline_state.h"

namespace rx
{

struct VertexBufferUpdate
{
    uint32_t slot;
    webgpu::BufferHelper *buffer;
};

class VertexArrayWgpu : public VertexArrayImpl
{
  public:
    VertexArrayWgpu(const gl::VertexArrayState &data);

    angle::Result syncState(const gl::Context *context,
                            const gl::VertexArray::DirtyBits &dirtyBits,
                            gl::VertexArray::DirtyAttribBitsArray *attribBits,
                            gl::VertexArray::DirtyBindingBitsArray *bindingBits) override;

    // Returns a vector of vertex buffers whose attributes are enabled.
    std::vector<VertexBufferUpdate> getBuffersToSet();

  private:
    angle::Result syncDirtyAttrib(ContextWgpu *contextWgpu,
                                  const gl::VertexAttribute &attrib,
                                  const gl::VertexBinding &binding,
                                  size_t attribIndex);
    gl::AttribArray<webgpu::PackedVertexAttribute> mCurrentAttribs;
    gl::AttribArray<webgpu::BufferHelper *> mCurrentArrayBuffers;
};

}  // namespace rx

#endif  // LIBANGLE_RENDERER_WGPU_VERTEXARRAYWGPU_H_
