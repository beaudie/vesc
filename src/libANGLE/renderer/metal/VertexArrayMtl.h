//
// Copyright 2019 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//

#ifndef LIBANGLE_RENDERER_METAL_VERTEXARRAYMTL_H_
#define LIBANGLE_RENDERER_METAL_VERTEXARRAYMTL_H_

#include "libANGLE/renderer/VertexArrayImpl.h"
#include "libANGLE/renderer/metal/BufferMtl.h"
#include "libANGLE/renderer/metal/mtl_buffer_pool.h"
#include "libANGLE/renderer/metal/mtl_command_buffer.h"
#include "libANGLE/renderer/metal/mtl_format_utils.h"
#include "libANGLE/renderer/metal/mtl_resources.h"

namespace rx
{
class ContextMtl;

class VertexArrayMtl : public VertexArrayImpl
{
  public:
    VertexArrayMtl(const gl::VertexArrayState &state, ContextMtl *context);
    ~VertexArrayMtl() override;

    void destroy(const gl::Context *context) override;

    angle::Result syncState(const gl::Context *context,
                            const gl::VertexArray::DirtyBits &dirtyBits,
                            gl::VertexArray::DirtyAttribBitsArray *attribBits,
                            gl::VertexArray::DirtyBindingBitsArray *bindingBits) override;

    // Feed client side's vertex/index data
    angle::Result updateClientAttribs(const gl::Context *context,
                                      GLint firstVertex,
                                      GLsizei vertexOrIndexCount,
                                      GLsizei instanceCount,
                                      gl::DrawElementsType indexTypeOrInvalid,
                                      const void *indices);

    // vertexDescChanged is both input and output, the input value if is true, will force new
    // mtl::VertexDesc to be returned via vertexDescOut. Otherwise, it is only returned when the
    // vertex array is dirty
    angle::Result setupDraw(const gl::Context *glContext,
                            mtl::RenderCommandEncoder *cmdEncoder,
                            bool *vertexDescChanged,
                            mtl::VertexDesc *vertexDescOut);

    BufferHolderMtl *getElementArrayBuffer() const { return mCurrentElementArrayBuffer; }
    size_t getElementArrayBufferOffset() const { return mCurrentElementArrayBufferOffset; }

    angle::Result convertIndexBuffer(const gl::Context *glContext,
                                     gl::DrawElementsType indexType,
                                     BufferMtl *idxBuffer,
                                     size_t offset);
    angle::Result streamIndexBufferFromClient(const gl::Context *glContext,
                                              gl::DrawElementsType indexType,
                                              size_t indexCount,
                                              const void *sourcePointer);

  private:
    angle::Result syncDirtyAttrib(const gl::Context *glContext,
                                  const gl::VertexAttribute &attrib,
                                  const gl::VertexBinding &binding,
                                  size_t attribIndex);
    angle::Result convertIndexBufferGPU(const gl::Context *glContext,
                                        gl::DrawElementsType indexType,
                                        BufferMtl *idxBuffer,
                                        size_t offset,
                                        size_t indexCount,
                                        ConversionBufferMtl *conversion);
    angle::Result convertIndexBufferCPU(const gl::Context *glContext,
                                        gl::DrawElementsType indexType,
                                        BufferMtl *idxBuffer,
                                        size_t offset,
                                        size_t indexCount,
                                        ConversionBufferMtl *conversion);

    angle::Result convertVertexBuffer(const gl::Context *glContext,
                                      BufferMtl *srcBuffer,
                                      const gl::VertexBinding &binding,
                                      size_t attribIndex,
                                      const mtl::VertexFormat &vertexFormat);

    angle::Result convertVertexBufferCPU(const gl::Context *glContext,
                                         BufferMtl *srcBuffer,
                                         const gl::VertexBinding &binding,
                                         size_t attribIndex,
                                         const mtl::VertexFormat &vertexFormat,
                                         ConversionBufferMtl *conversion);

    gl::AttribArray<BufferHolderMtl *> mCurrentArrayBuffers;
    gl::AttribArray<SimpleWeakBufferHolderMtl> mDynamicArrayBufferHolders;
    gl::AttribArray<size_t> mCurrentArrayBufferOffsets;
    gl::AttribArray<GLuint> mCurrentArrayBufferStrides;
    gl::AttribArray<MTLVertexFormat> mCurrentArrayBufferFormats;

    BufferHolderMtl *mCurrentElementArrayBuffer;
    size_t mCurrentElementArrayBufferOffset = 0;
    SimpleWeakBufferHolderMtl mDynamicElementArrayBufferHolder;

    mtl::BufferPool mDynamicVertexData;
    mtl::BufferPool mDynamicIndexData;

    bool mVertexArrayDirty = true;
};
}  // namespace rx

#endif /* LIBANGLE_RENDERER_METAL_VERTEXARRAYMTL_H_ */
