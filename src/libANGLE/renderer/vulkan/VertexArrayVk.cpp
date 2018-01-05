//
// Copyright 2016 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//
// VertexArrayVk.cpp:
//    Implements the class methods for VertexArrayVk.
//

#include "libANGLE/renderer/vulkan/VertexArrayVk.h"

#include "common/debug.h"

#include "libANGLE/Context.h"
#include "libANGLE/renderer/vulkan/BufferVk.h"
#include "libANGLE/renderer/vulkan/CommandBufferNode.h"
#include "libANGLE/renderer/vulkan/ContextVk.h"
#include "libANGLE/renderer/vulkan/formatutilsvk.h"

namespace rx
{

VertexArrayVk::VertexArrayVk(const gl::VertexArrayState &state)
    : VertexArrayImpl(state),
      // mCurrentArrayBufferHandles{},
      mCurrentArrayBufferResources{},
      mCurrentElementArrayBufferResource(nullptr),
      mHandles(state.getMaxAttribs(), VK_NULL_HANDLE),
      mOffsets(state.getMaxAttribs(), 0),
      mCurrentVertexDescsValid(false)
{
    // mCurrentArrayBufferHandles.fill(VK_NULL_HANDLE);
    mCurrentArrayBufferResources.fill(nullptr);
    mCurrentVertexBindingDescs.reserve(state.getMaxAttribs());
    mCurrentVertexAttribDescs.reserve(state.getMaxAttribs());
}

VertexArrayVk::~VertexArrayVk()
{
}

void VertexArrayVk::destroy(const gl::Context *context)
{
}

gl::Error VertexArrayVk::streamVertexData(ContextVk *context, GLsizei count)
{
    const auto &attribs          = mState.getVertexAttributes();
    const auto &bindings         = mState.getVertexBindings();
    const gl::Program *programGL = context->getGLState().getProgram();

    auto vertexData = context->getVertexData();
    for (auto attribIndex : programGL->getActiveAttribLocationsMask())
    {
        const auto &attrib   = attribs[attribIndex];
        const auto &binding  = bindings[attrib.bindingIndex];
        gl::Buffer *bufferGL = binding.getBuffer().get();
        auto b               = mCurrentArrayBufferResources[attribIndex];
        if (b)
            mHandles[attribIndex] = ((BufferVk *)b)->getVkBuffer().getHandle();
        mOffsets[attribIndex] = 0;
        if (attrib.enabled && !bufferGL)
        {
            printf("index %d   count %d   addr %p   stride %d   size %d\n", (int)attribIndex,
                   (int)count, attrib.pointer, (int)binding.getStride(),
                   (int)gl::ComputeVertexAttributeTypeSize(attrib));

            const size_t srcStride = binding.getStride();
            const size_t dstStride = gl::ComputeVertexAttributeTypeSize(attrib);
            const uint8_t *src     = static_cast<const uint8_t *>(attrib.pointer);
            uint8_t *dst;
            ANGLE_TRY(vertexData->map(count * dstStride, &dst, &mHandles[attribIndex],
                                      &mOffsets[attribIndex]));

            // Pack data tightly into the destination.  If source has holes we must copy vertices
            // one by one.
            if (srcStride == dstStride)
            {
                memcpy(dst, src, count * dstStride);
            }
            else
            {
                for (int i = 0; i < count; ++i)
                {
                    memcpy(dst, src, dstStride);
                    dst += dstStride;
                    src += srcStride;
                }
            }
        }
        vertexData->unmap();
    }

    return gl::NoError();
}

const VkBuffer *VertexArrayVk::handles() const
{
    return mHandles.data();
}

const VkDeviceSize *VertexArrayVk::offsets() const
{
    return mOffsets.data();
}

void VertexArrayVk::syncState(const gl::Context *context,
                              const gl::VertexArray::DirtyBits &dirtyBits)
{
    ASSERT(dirtyBits.any());

    // Invalidate current pipeline.
    // TODO(jmadill): Use pipeline cache.
    ContextVk *contextVk = vk::GetImpl(context);
    contextVk->onVertexArrayChange();

    // Invalidate the vertex descriptions.
    invalidateVertexDescriptions();

    // Rebuild current attribute buffers cache. This will fail horribly if the buffer changes.
    // TODO(jmadill): Handle buffer storage changes.
    const auto &attribs  = mState.getVertexAttributes();
    const auto &bindings = mState.getVertexBindings();

    for (auto dirtyBit : dirtyBits)
    {
        if (dirtyBit == gl::VertexArray::DIRTY_BIT_ELEMENT_ARRAY_BUFFER)
        {
            gl::Buffer *bufferGL = mState.getElementArrayBuffer().get();
            if (bufferGL)
            {
                mCurrentElementArrayBufferResource = vk::GetImpl(bufferGL);
            }
            else
            {
                mCurrentElementArrayBufferResource = nullptr;
            }
            continue;
        }

        size_t attribIndex = gl::VertexArray::GetVertexIndexFromDirtyBit(dirtyBit);

        const auto &attrib  = attribs[attribIndex];
        const auto &binding = bindings[attrib.bindingIndex];

        if (attrib.enabled)
        {
            gl::Buffer *bufferGL = binding.getBuffer().get();

            if (bufferGL)
            {
                BufferVk *bufferVk                        = vk::GetImpl(bufferGL);
                mCurrentArrayBufferResources[attribIndex] = bufferVk;
                // mCurrentArrayBufferHandles[attribIndex]   = bufferVk->getVkBuffer().getHandle();
            }
            else
            {
                mCurrentArrayBufferResources[attribIndex] = nullptr;
                // mCurrentArrayBufferHandles[attribIndex]   = VK_NULL_HANDLE;
            }
        }
        else
        {
            UNIMPLEMENTED();
        }
    }
}

/*
const gl::AttribArray<VkBuffer> &VertexArrayVk::getCurrentArrayBufferHandles() const
{
    return mCurrentArrayBufferHandles;
}
*/

void VertexArrayVk::updateDrawDependencies(vk::CommandBufferNode *readNode,
                                           const gl::AttributesMask &activeAttribsMask,
                                           Serial serial,
                                           DrawType drawType)
{
    // Handle the bound array buffers.
    for (auto attribIndex : activeAttribsMask)
    {
        if (mCurrentArrayBufferResources[attribIndex])
            mCurrentArrayBufferResources[attribIndex]->updateDependencies(readNode, serial);
    }

    // Handle the bound element array buffer.
    if (drawType == DrawType::Elements)
    {
        ASSERT(mCurrentElementArrayBufferResource);
        mCurrentElementArrayBufferResource->updateDependencies(readNode, serial);
    }
}

void VertexArrayVk::invalidateVertexDescriptions()
{
    mCurrentVertexDescsValid = false;
    mCurrentVertexBindingDescs.clear();
    mCurrentVertexAttribDescs.clear();
}

void VertexArrayVk::updateVertexDescriptions(const gl::Context *context)
{
    if (mCurrentVertexDescsValid)
    {
        return;
    }

    const auto &attribs  = mState.getVertexAttributes();
    const auto &bindings = mState.getVertexBindings();

    const gl::Program *programGL = context->getGLState().getProgram();

    for (auto attribIndex : programGL->getActiveAttribLocationsMask())
    {
        const auto &attrib  = attribs[attribIndex];
        const auto &binding = bindings[attrib.bindingIndex];
        if (attrib.enabled)
        {
            VkVertexInputBindingDescription bindingDesc;
            bindingDesc.binding = static_cast<uint32_t>(mCurrentVertexBindingDescs.size());
            bindingDesc.stride  = static_cast<uint32_t>(gl::ComputeVertexAttributeTypeSize(attrib));
            bindingDesc.inputRate = (binding.getDivisor() > 0 ? VK_VERTEX_INPUT_RATE_INSTANCE
                                                              : VK_VERTEX_INPUT_RATE_VERTEX);

            gl::VertexFormatType vertexFormatType = gl::GetVertexFormatType(attrib);

            VkVertexInputAttributeDescription attribDesc;
            attribDesc.binding  = bindingDesc.binding;
            attribDesc.format   = vk::GetNativeVertexFormat(vertexFormatType);
            attribDesc.location = static_cast<uint32_t>(attribIndex);
            attribDesc.offset =
                static_cast<uint32_t>(ComputeVertexAttributeOffset(attrib, binding));

            mCurrentVertexBindingDescs.push_back(bindingDesc);
            mCurrentVertexAttribDescs.push_back(attribDesc);
        }
        else
        {
            UNIMPLEMENTED();
        }
    }

    mCurrentVertexDescsValid = true;
}

const std::vector<VkVertexInputBindingDescription> &VertexArrayVk::getVertexBindingDescs() const
{
    return mCurrentVertexBindingDescs;
}

const std::vector<VkVertexInputAttributeDescription> &VertexArrayVk::getVertexAttribDescs() const
{
    return mCurrentVertexAttribDescs;
}

}  // namespace rx
