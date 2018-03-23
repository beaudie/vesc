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
#include "libANGLE/renderer/vulkan/CommandGraph.h"
#include "libANGLE/renderer/vulkan/ContextVk.h"
#include "libANGLE/renderer/vulkan/RendererVk.h"
#include "libANGLE/renderer/vulkan/vk_format_utils.h"

static void br0() {}

namespace
{

template <class T, int Scale>
void ToFloat(float *dst, const uint8_t *src, size_t srcSize, size_t stride, int dimension)
{
    printf("ToFloat item %d stride %d dim %d\n", (int)sizeof(T), (int)stride, (int)dimension);
    const uint8_t *const srcLast = src + srcSize - dimension * sizeof(T);
    int cnt                      = 0;
    while (src <= srcLast)
    {
        printf("%d", cnt);
        for (int i = 0; i < dimension; ++i)
        {
            *dst = (reinterpret_cast<const T *>(src))[i] / Scale;
            printf(" %f", *dst);
            dst++;
        }
        printf("\n");
        src += stride;
        cnt += 1;
    }
    printf("stored %d vertices\n", cnt);
}

void ToFloat(float *dst,
             const uint8_t *src,
             size_t srcSize,
             size_t stride,
             int dimension,
             GLenum type)
{
    switch (type)
    {
        case GL_BYTE:
            return ToFloat<GLbyte, 1>(dst, src, srcSize, stride, dimension);
        case GL_SHORT:
            return ToFloat<GLshort, 1>(dst, src, srcSize, stride, dimension);
        case GL_INT:
            return ToFloat<GLint, 1>(dst, src, srcSize, stride, dimension);
        case GL_UNSIGNED_BYTE:
            return ToFloat<GLubyte, 1>(dst, src, srcSize, stride, dimension);
        case GL_UNSIGNED_SHORT:
            return ToFloat<GLushort, 1>(dst, src, srcSize, stride, dimension);
        case GL_UNSIGNED_INT:
            return ToFloat<GLuint, 1>(dst, src, srcSize, stride, dimension);
        case GL_FLOAT:
            return ToFloat<GLfloat, 1>(dst, src, srcSize, stride, dimension);
        case GL_FIXED:
            return ToFloat<GLint, 0x10000>(dst, src, srcSize, stride, dimension);
    }
    UNREACHABLE();
}
}

namespace rx
{

gl::Error Translation::translate(const gl::Context *context,
                                 BufferVk *src,
                                 size_t offset,
                                 size_t stride,
                                 int dimension,
                                 GLenum type)
{
    ContextVk *contextVk = vk::GetImpl(context);
    RendererVk *renderer = contextVk->getRenderer();
    VkDevice device      = contextVk->getDevice();

    updateQueueSerial(renderer->getCurrentQueueSerial());
    renderer->releaseResource(*this, &mBuffer);
    renderer->releaseResource(*this, &mMemory);

    void *srcMap   = nullptr;
    size_t srcSize = 0;
    ANGLE_TRY(src->map(context, 0, &srcMap, &srcSize));

    VkBufferCreateInfo createInfo;
    createInfo.sType                 = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
    createInfo.pNext                 = nullptr;
    createInfo.flags                 = 0;
    createInfo.size                  = srcSize;
    createInfo.usage                 = VK_BUFFER_USAGE_VERTEX_BUFFER_BIT;
    createInfo.sharingMode           = VK_SHARING_MODE_EXCLUSIVE;
    createInfo.queueFamilyIndexCount = 0;
    createInfo.pQueueFamilyIndices   = nullptr;
    ANGLE_TRY(mBuffer.init(device, createInfo));

    size_t dstSize = 0;
    ANGLE_TRY(vk::AllocateBufferMemory(renderer, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT, &mBuffer,
                                       &mMemory, &dstSize));
    uint8_t *dstMap = nullptr;
    ANGLE_TRY(mMemory.map(device, 0, srcSize, 0, &dstMap));

    // memcpy(dstMap, srcMap, srcSize);
    // XXX either add 'offset' to 'dstMap' or zero out the offset we pass to vulkan
    ToFloat(reinterpret_cast<float *>(dstMap), static_cast<const uint8_t *>(srcMap) + offset,
            srcSize - offset, stride, dimension, type);

    ANGLE_TRY(src->unmap(context, nullptr));
    mMemory.unmap(device);

    return gl::NoError();
}

void Translation::destroy(VkDevice device)
{
    mBuffer.destroy(device);
    mMemory.destroy(device);
}

VertexArrayVk::VertexArrayVk(const gl::VertexArrayState &state)
    : VertexArrayImpl(state),
      mCurrentArrayBufferHandles{},
      mCurrentArrayBufferOffsets{},
      mCurrentArrayBufferResources{},
      mCurrentElementArrayBufferResource(nullptr)
{
    mCurrentArrayBufferHandles.fill(VK_NULL_HANDLE);
    mCurrentArrayBufferOffsets.fill(0);
    mCurrentArrayBufferResources.fill(nullptr);

    mPackedInputBindings.fill({0, 0});
    mPackedInputAttributes.fill({0, 0, 0});
}

VertexArrayVk::~VertexArrayVk()
{
}

void VertexArrayVk::destroy(const gl::Context *context)
{
    VkDevice device = vk::GetImpl(context)->getDevice();
    for (auto &i : mTranslationBuffers)
        i.destroy(device);
}

gl::AttributesMask VertexArrayVk::attribsToStream(ContextVk *context) const
{
    const gl::Program *programGL = context->getGLState().getProgram();
    return mClientMemoryAttribs & programGL->getActiveAttribLocationsMask();
}

gl::Error VertexArrayVk::streamVertexData(ContextVk *context,
                                          StreamingBuffer *stream,
                                          size_t firstVertex,
                                          size_t lastVertex)
{
    const auto &attribs          = mState.getVertexAttributes();
    const auto &bindings         = mState.getVertexBindings();

    // TODO(fjhenigman): When we have a bunch of interleaved attributes, they end up
    // un-interleaved, wasting space and copying time.  Consider improving on that.
    for (auto attribIndex : attribsToStream(context))
    {
        const gl::VertexAttribute &attrib = attribs[attribIndex];
        const gl::VertexBinding &binding  = bindings[attrib.bindingIndex];
        ASSERT(attrib.enabled && binding.getBuffer().get() == nullptr);

        // Only [firstVertex, lastVertex] is needed by the upcoming draw so that
        // is all we copy, but we allocate space for [0, lastVertex] so indexing
        // will work.  If we don't start at zero all the indices will be off.
        // TODO(fjhenigman): See if we can account for indices being off by adjusting
        // the offset, thus avoiding wasted memory.
        uint8_t *dst = nullptr;
        ANGLE_TRY(stream->allocate(context, (lastVertex + 1) * attrib.size * sizeof(float), &dst,
                                   &mCurrentArrayBufferHandles[attribIndex],
                                   &mCurrentArrayBufferOffsets[attribIndex], nullptr));
        ToFloat(reinterpret_cast<float *>(dst) + firstVertex * attrib.size * sizeof(float),
                static_cast<const uint8_t *>(attrib.pointer) + firstVertex * binding.getStride(),
                (lastVertex - firstVertex) * binding.getStride() +
                    gl::ComputeVertexAttributeTypeSize(attrib),
                binding.getStride(), attrib.size, attrib.type);
    }

    ANGLE_TRY(stream->flush(context));
    return gl::NoError();
}

void VertexArrayVk::syncState(const gl::Context *context,
                              const gl::VertexArray::DirtyBits &dirtyBits)
{
    ASSERT(dirtyBits.any());

    // Invalidate current pipeline.
    ContextVk *contextVk = vk::GetImpl(context);
    contextVk->onVertexArrayChange();

    // Rebuild current attribute buffers cache. This will fail horribly if the buffer changes.
    // TODO(jmadill): Handle buffer storage changes.
    const auto &attribs  = mState.getVertexAttributes();
    const auto &bindings = mState.getVertexBindings();

    for (auto dirtyBit : dirtyBits)
    {
        printf(" VertexArrayVk::syncState dirty bit %d\n", (int)dirtyBit);
        if (dirtyBit > 16)
            continue;
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

        // Invalidate the input description for pipelines.
        mDirtyPackedInputs.set(attribIndex);

        const auto &attrib  = attribs[attribIndex];
        const auto &binding = bindings[attrib.bindingIndex];

        if (attrib.enabled)
        {
            gl::Buffer *bufferGL = binding.getBuffer().get();

            if (bufferGL)
            {
                br0();
                size_t offset = ComputeVertexAttributeOffset(attrib, binding);
                printf("VertexArrayVk::syncState size %d type %d offset %d stride %d\n",
                       (int)attrib.size, (int)attrib.type, (int)offset, (int)binding.getStride());
                Translation *t = &mTranslationBuffers[attribIndex];

                BufferVk *vbuf = vk::GetImpl(bufferGL);
                if (t->translate(context, vbuf, offset, binding.getStride(), attrib.size,
                                 attrib.type)
                        .isError())
                {
                    //XXX return an error
                    printf("YOU LOSE\n");
                }

                mCurrentArrayBufferResources[attribIndex] = vbuf;
                mCurrentArrayBufferHandles[attribIndex]   = vbuf->getVkBuffer().getHandle();
                mCurrentArrayBufferResources[attribIndex] = t;
                mCurrentArrayBufferHandles[attribIndex]   = t->mBuffer.getHandle();
                mClientMemoryAttribs.reset(attribIndex);
            }
            else
            {
                mCurrentArrayBufferResources[attribIndex] = nullptr;
                mCurrentArrayBufferHandles[attribIndex]   = VK_NULL_HANDLE;
                mClientMemoryAttribs.set(attribIndex);
            }
            // TODO(jmadill): Offset handling.  Assume zero for now.
            mCurrentArrayBufferOffsets[attribIndex] = 0;
        }
        else
        {
            mClientMemoryAttribs.reset(attribIndex);
            UNIMPLEMENTED();
        }
    }
}

const gl::AttribArray<VkBuffer> &VertexArrayVk::getCurrentArrayBufferHandles() const
{
    return mCurrentArrayBufferHandles;
}

const gl::AttribArray<VkDeviceSize> &VertexArrayVk::getCurrentArrayBufferOffsets() const
{
    return mCurrentArrayBufferOffsets;
}

void VertexArrayVk::updateDrawDependencies(vk::CommandGraphNode *readNode,
                                           const gl::AttributesMask &activeAttribsMask,
                                           ResourceVk *elementArrayBufferOverride,
                                           Serial serial,
                                           DrawType drawType)
{
    // Handle the bound array buffers.
    for (auto attribIndex : activeAttribsMask)
    {
        if (mCurrentArrayBufferResources[attribIndex])
            mCurrentArrayBufferResources[attribIndex]->onReadResource(readNode, serial);
    }

    // Handle the bound element array buffer.
    if (drawType == DrawType::Elements && mCurrentElementArrayBufferResource)
    {
        if (elementArrayBufferOverride != nullptr)
        {
            elementArrayBufferOverride->onReadResource(readNode, serial);
        }
        else
        {
            ASSERT(mCurrentElementArrayBufferResource);
            mCurrentElementArrayBufferResource->onReadResource(readNode, serial);
        }
    }
}

void VertexArrayVk::getPackedInputDescriptions(vk::PipelineDesc *pipelineDesc)
{
    updatePackedInputDescriptions();
    pipelineDesc->updateVertexInputInfo(mPackedInputBindings, mPackedInputAttributes);
}

void VertexArrayVk::updatePackedInputDescriptions()
{
    if (!mDirtyPackedInputs.any())
    {
        return;
    }

    const auto &attribs  = mState.getVertexAttributes();
    const auto &bindings = mState.getVertexBindings();

    for (auto attribIndex : mDirtyPackedInputs)
    {
        const auto &attrib  = attribs[attribIndex];
        const auto &binding = bindings[attrib.bindingIndex];
        if (attrib.enabled)
        {
            updatePackedInputInfo(static_cast<uint32_t>(attribIndex), binding, attrib);
        }
        else
        {
            UNIMPLEMENTED();
        }
    }

    mDirtyPackedInputs.reset();
}

void VertexArrayVk::updatePackedInputInfo(uint32_t attribIndex,
                                          const gl::VertexBinding &binding,
                                          const gl::VertexAttribute &attrib)
{
    vk::PackedVertexInputBindingDesc &bindingDesc = mPackedInputBindings[attribIndex];

    size_t attribSize = gl::ComputeVertexAttributeTypeSize(attrib);
    ASSERT(attribSize <= std::numeric_limits<uint16_t>::max());

    bindingDesc.stride    = sizeof(float) * attrib.size;  // static_cast<uint16_t>(binding.getStride());
    bindingDesc.inputRate = static_cast<uint16_t>(
        binding.getDivisor() > 0 ? VK_VERTEX_INPUT_RATE_INSTANCE : VK_VERTEX_INPUT_RATE_VERTEX);

    printf(
        "VertexArrayVk::updatePackedInputInfo size %d type %d normalized %d pureInteger %d stride "
        "%d\n",
        (int)attrib.size, (int)attrib.type, (int)attrib.normalized, (int)attrib.pureInteger,
        (int)bindingDesc.stride);
    VkFormat vkFormat = (VkFormat)(VK_FORMAT_R32_SFLOAT + 3 * (attrib.size - 1));
    ASSERT(vkFormat <= std::numeric_limits<uint16_t>::max());

    vk::PackedVertexInputAttributeDesc &attribDesc = mPackedInputAttributes[attribIndex];
    attribDesc.format                              = static_cast<uint16_t>(vkFormat);
    attribDesc.location                            = static_cast<uint16_t>(attribIndex);
    attribDesc.offset                              = 0;  // static_cast<uint32_t>(ComputeVertexAttributeOffset(attrib, binding));
}

}  // namespace rx
