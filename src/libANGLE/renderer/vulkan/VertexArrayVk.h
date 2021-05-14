//
// Copyright 2016 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//
// VertexArrayVk.h:
//    Defines the class interface for VertexArrayVk, implementing VertexArrayImpl.
//

#ifndef LIBANGLE_RENDERER_VULKAN_VERTEXARRAYVK_H_
#define LIBANGLE_RENDERER_VULKAN_VERTEXARRAYVK_H_

#include "libANGLE/renderer/VertexArrayImpl.h"
#include "libANGLE/renderer/vulkan/vk_cache_utils.h"
#include "libANGLE/renderer/vulkan/vk_helpers.h"

namespace rx
{
class BufferVk;
struct ConversionBuffer;

class VertexArrayVk : public VertexArrayImpl
{
  public:
    VertexArrayVk(ContextVk *contextVk, const gl::VertexArrayState &state);
    ~VertexArrayVk() override;

    void destroy(const gl::Context *context) override;

    angle::Result syncState(const gl::Context *context,
                            const gl::VertexArray::DirtyBits &dirtyBits,
                            gl::VertexArray::DirtyAttribBitsArray *attribBits,
                            gl::VertexArray::DirtyBindingBitsArray *bindingBits) override;

    angle::Result updateActiveAttribInfo(ContextVk *contextVk);

    angle::Result updateDefaultAttrib(ContextVk *contextVk,
                                      size_t attribIndex,
                                      VkBuffer bufferHandle,
                                      vk::BufferHelper *buffer,
                                      uint32_t offset);

    angle::Result updateStreamedAttribs(const gl::Context *context,
                                        GLint firstVertex,
                                        GLsizei vertexOrIndexCount,
                                        GLsizei instanceCount,
                                        gl::DrawElementsType indexTypeOrInvalid,
                                        const void *indices);

    angle::Result handleLineLoop(ContextVk *contextVk,
                                 GLint firstVertex,
                                 GLsizei vertexOrIndexCount,
                                 gl::DrawElementsType indexTypeOrInvalid,
                                 const void *indices,
                                 uint32_t *indexCountOut);

    angle::Result handleLineLoopIndexIndirect(ContextVk *contextVk,
                                              gl::DrawElementsType glIndexType,
                                              const vk::BufferAndOffset *indirectBufferAndOffset,
                                              const vk::BufferHelper **indirectBufferAndOffsetOut);

    angle::Result handleLineLoopIndirectDraw(const gl::Context *context,
                                             const vk::BufferAndOffset *indirectBufferAndOffset,
                                             const vk::BufferHelper **indirectBufferAndOffsetOut);

    const gl::AttribArray<VkBuffer> &getCurrentArrayBufferHandles() const
    {
        return mCurrentArrayBufferHandles;
    }

    const gl::AttribArray<const vk::BufferAndOffset *> &getCurrentArrayBuffersAndOffsets() const
    {
        return mCurrentArrayBuffersAndOffsets;
    }

    const vk::BufferAndOffset *getCurrentIndexBufferAndOffset() const
    {
        return mCurrentIndexBufferAndOffset;
    }

    angle::Result convertIndexBufferGPU(ContextVk *contextVk,
                                        BufferVk *bufferVk,
                                        const void *indices);

    angle::Result convertIndexBufferIndirectGPU(ContextVk *contextVk,
                                                const vk::BufferAndOffset *srcIndirectBuf,
                                                const vk::BufferAndOffset **indirectBufferOut);

    angle::Result convertIndexBufferCPU(ContextVk *contextVk,
                                        gl::DrawElementsType indexType,
                                        size_t indexCount,
                                        const void *sourcePointer);

    const gl::AttributesMask &getStreamingVertexAttribsMask() const
    {
        return mStreamingVertexAttribsMask;
    }

  private:
    angle::Result setDefaultPackedInput(ContextVk *contextVk, size_t attribIndex);

    angle::Result convertVertexBufferGPU(ContextVk *contextVk,
                                         BufferVk *srcBuffer,
                                         const gl::VertexBinding &binding,
                                         size_t attribIndex,
                                         const vk::Format &vertexFormat,
                                         ConversionBuffer *conversion,
                                         GLuint relativeOffset,
                                         bool compressed);
    angle::Result convertVertexBufferCPU(ContextVk *contextVk,
                                         BufferVk *srcBuffer,
                                         const gl::VertexBinding &binding,
                                         size_t attribIndex,
                                         const vk::Format &vertexFormat,
                                         ConversionBuffer *conversion,
                                         GLuint relativeOffset,
                                         bool compress);

    angle::Result syncDirtyAttrib(ContextVk *contextVk,
                                  const gl::VertexAttribute &attrib,
                                  const gl::VertexBinding &binding,
                                  size_t attribIndex,
                                  bool bufferOnly);

    // The offset into the buffer to the first attrib
    gl::AttribArray<GLuint> mCurrentArrayBufferRelativeOffsets;
    // |mCurrentArrayBuffersAndOffsets| holds the pointer to where buffer and offset (and its
    // resource use) are stored.  This could be either |mStreamedBuffersAndOffsets| for streamed
    // attributes, |VertexConversionBuffer::bufferAndOffset| if a conversion buffer is used, or
    // |BufferVk::mBufferAndOffset|.
    gl::AttribArray<const vk::BufferAndOffset *> mCurrentArrayBuffersAndOffsets;
    // Cache strides of attributes for a fast pipeline cache update when VAOs are changed
    gl::AttribArray<GLuint> mCurrentArrayBufferStrides;
    gl::AttributesMask mCurrentArrayBufferCompressed;
    // |mCurrentIndexBufferAndOffset| holds the pointer to where buffer and offset for the index
    // buffer are stored.  This could be either |mTranslatedIndexBufferAndOffset| if emulated, or
    // |BufferVk::mBufferAndOffset|.
    const vk::BufferAndOffset *mCurrentIndexBufferAndOffset;

    // Convenience arrays passed to Vulkan functions to bind multiple vertex arrays.
    gl::AttribArray<VkBuffer> mCurrentArrayBufferHandles;
    // TODO: mCurrentArrayBufferOffsets was briefly removed, check with mCurrentArrayBufferHandles
    // and make sure it's always set
    gl::AttribArray<VkDeviceSize> mCurrentArrayBufferOffsets;

    vk::DynamicBuffer mDynamicVertexData;
    vk::DynamicBuffer mDynamicIndexData;
    vk::DynamicBuffer mTranslatedByteIndexData;
    vk::DynamicBuffer mTranslatedByteIndirectData;

    gl::AttribArray<vk::BufferAndOffset> mStreamedBuffersAndOffsets;
    vk::BufferAndOffset mTranslatedIndexBufferAndOffset;
    vk::BufferAndOffset mTranslatedIndirectBufferAndOffset;

    vk::LineLoopHelper mLineLoopHelper;
    Optional<GLint> mLineLoopBufferFirstIndex;
    Optional<size_t> mLineLoopBufferLastIndex;
    bool mDirtyLineLoopTranslation;

    // Track client and/or emulated attribs that we have to stream their buffer contents
    gl::AttributesMask mStreamingVertexAttribsMask;
};
}  // namespace rx

#endif  // LIBANGLE_RENDERER_VULKAN_VERTEXARRAYVK_H_
