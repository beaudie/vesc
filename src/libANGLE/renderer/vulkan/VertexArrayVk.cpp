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
#include "libANGLE/renderer/vulkan/ContextVk.h"
#include "libANGLE/renderer/vulkan/formatutilsvk.h"

namespace rx
{

VertexArrayVk::VertexArrayVk(const gl::VertexArrayState &state)
    : VertexArrayImpl(state),
      //mCurrentVertexBufferHandlesCache(state.getMaxAttribs(), VK_NULL_HANDLE),
      mCurrentVkBuffersCache(state.getMaxAttribs(), nullptr),
      mHandles(state.getMaxAttribs(), VK_NULL_HANDLE),
      mOffsets(state.getMaxAttribs(), 0),
      mCurrentVertexDescsValid(false)
{
    mCurrentVertexBindingDescs.reserve(state.getMaxAttribs());
    mCurrentVertexAttribDescs.reserve(state.getMaxAttribs());
}

void VertexArrayVk::destroy(const gl::Context *context)
{
}

gl::Error VertexArrayVk::streamVertexData(const gl::Context *context, GLsizei count)
{
    const auto &attribs          = mState.getVertexAttributes();
    const auto &bindings         = mState.getVertexBindings();
    const gl::Program *programGL = context->getGLState().getProgram();

    static vk::Buffer buf;
    static vk::DeviceMemory mem;
    static VkDeviceSize off = 0;
    constexpr size_t size = 0x10000;

    ContextVk *contextVk = GetImplAs<ContextVk>(context);
    VkDevice device = contextVk->getDevice();

    if (!buf.valid())
    {

        //release(contextVk->getRenderer());

        // TODO(jmadill): Proper usage bit implementation. Likely will involve multiple backing
        // buffers like in D3D11.
        VkBufferCreateInfo createInfo;
        createInfo.sType                 = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
        createInfo.pNext                 = nullptr;
        createInfo.flags                 = 0;
        createInfo.size                  = size;
        createInfo.usage                 = VK_BUFFER_USAGE_VERTEX_BUFFER_BIT;
        createInfo.sharingMode           = VK_SHARING_MODE_EXCLUSIVE;
        createInfo.queueFamilyIndexCount = 0;
        createInfo.pQueueFamilyIndices   = nullptr;

        ANGLE_TRY(buf.init(device, createInfo));
        size_t reqSize;
        ANGLE_TRY(vk::AllocateBufferMemory(contextVk, size, &buf, &mem, &reqSize));
    }

    uint8_t *ptr;
    ANGLE_TRY(mem.map(device, 0, size, 0, &ptr));

    for (auto attribIndex : programGL->getActiveAttribLocationsMask())
    {
        const auto &attrib   = attribs[attribIndex];
        const auto &binding  = bindings[attrib.bindingIndex];
        gl::Buffer *bufferGL = binding.getBuffer().get();
        printf("index %d   count %d   stride %d\n", (int)attribIndex, (int)count, (int)binding.getStride());
        auto b = mCurrentVkBuffersCache[attribIndex];
        if (b) mHandles[attribIndex] = b->getVkBuffer().getHandle();
        mOffsets[attribIndex] = 0;
        if (attrib.enabled && !bufferGL)
        {
            mHandles[attribIndex]           = buf.getHandle();
            mOffsets[attribIndex]           = off;
            size_t z = count * binding.getStride();
            printf("copying %d bytes to %p + %d\n", (int)z, ptr, (int)off);
            memcpy(ptr+off, attrib.pointer, z);
            off+=z;
            if (off>size)off=0;
        }
    }

    mem.unmap(device);
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
    auto contextVk = vk::GetImpl(context);
    contextVk->invalidateCurrentPipeline();

    // Invalidate the vertex descriptions.
    invalidateVertexDescriptions();

    // Rebuild current attribute buffers cache. This will fail horribly if the buffer changes.
    // TODO(jmadill): Handle buffer storage changes.
    const auto &attribs  = mState.getVertexAttributes();
    const auto &bindings = mState.getVertexBindings();

    for (auto dirtyBit : dirtyBits)
    {
        if (dirtyBit == gl::VertexArray::DIRTY_BIT_ELEMENT_ARRAY_BUFFER)
            continue;

        size_t attribIndex = gl::VertexArray::GetVertexIndexFromDirtyBit(dirtyBit);

        const auto &attrib  = attribs[attribIndex];
        const auto &binding = bindings[attrib.bindingIndex];

        if (attrib.enabled)
        {
            gl::Buffer *bufferGL = binding.getBuffer().get();

            if (bufferGL)
            {
                BufferVk *bufferVk                            = vk::GetImpl(bufferGL);
                mCurrentVkBuffersCache[attribIndex]           = bufferVk;
                //mCurrentVertexBufferHandlesCache[attribIndex] = bufferVk->getVkBuffer().getHandle();
            }
            else
            {
                mCurrentVkBuffersCache[attribIndex]           = nullptr;
                //mCurrentVertexBufferHandlesCache[attribIndex] = VK_NULL_HANDLE;
            }
        }
        else
        {
            UNIMPLEMENTED();
        }
    }
}

/*
const std::vector<VkBuffer> &VertexArrayVk::getCurrentVertexBufferHandlesCache() const
{
    return mCurrentVertexBufferHandlesCache;
}
*/

void VertexArrayVk::updateCurrentBufferSerials(const gl::AttributesMask &activeAttribsMask,
                                               Serial serial)
{
    for (auto attribIndex : activeAttribsMask)
    {
        printf("update %d %p\n", (int)attribIndex, mCurrentVkBuffersCache[attribIndex]);
        if(mCurrentVkBuffersCache[attribIndex])
            mCurrentVkBuffersCache[attribIndex]->setQueueSerial(serial);
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
            //XXX bindingDesc.stride = static_cast<uint32_t>(gl::ComputeVertexAttributeTypeSize(attrib));
            bindingDesc.stride = binding.getStride();
            printf("bindingDesc.stride %d\n", (int)bindingDesc.stride);
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
