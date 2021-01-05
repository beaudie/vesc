//
// Copyright 2016 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//
// TransformFeedbackVk.cpp:
//    Implements the class methods for TransformFeedbackVk.
//

#include "libANGLE/renderer/vulkan/TransformFeedbackVk.h"

#include "libANGLE/Context.h"
#include "libANGLE/Query.h"
#include "libANGLE/renderer/glslang_wrapper_utils.h"
#include "libANGLE/renderer/vulkan/BufferVk.h"
#include "libANGLE/renderer/vulkan/ContextVk.h"
#include "libANGLE/renderer/vulkan/FramebufferVk.h"
#include "libANGLE/renderer/vulkan/ProgramVk.h"
#include "libANGLE/renderer/vulkan/QueryVk.h"

#include "common/debug.h"

namespace rx
{
namespace
{
void PadEmulationDescriptorBufferInfo(size_t xfbBufferCount,
                                      const vk::BufferHelper &emptyBuffer,
                                      VkDescriptorBufferInfo *bufferInfo)
{
    for (size_t bufferIndex = xfbBufferCount;
         bufferIndex < gl::IMPLEMENTATION_MAX_TRANSFORM_FEEDBACK_BUFFERS; ++bufferIndex)
    {
        VkDescriptorBufferInfo &info = bufferInfo[bufferIndex];
        info.buffer                  = emptyBuffer.getBuffer().getHandle();
        info.offset                  = 0;
        info.range                   = VK_WHOLE_SIZE;
    }
}

}  // anonymous namespace

TransformFeedbackVk::TransformFeedbackVk(const gl::TransformFeedbackState &state)
    : TransformFeedbackImpl(state),
      mRebindTransformFeedbackBuffer(false),
      mBufferHelpers{},
      mBufferHandles{},
      mBufferOffsets{},
      mBufferSizes{},
      mAlignedBufferOffsets{},
      mCounterBufferHandles{}
{}

TransformFeedbackVk::~TransformFeedbackVk() {}

void TransformFeedbackVk::onDestroy(const gl::Context *context)
{
    RendererVk *rendererVk = vk::GetImpl(context)->getRenderer();

    for (vk::BufferHelper &bufferHelper : mCounterBufferHelpers)
    {
        bufferHelper.release(rendererVk);
    }
}

void TransformFeedbackVk::initializeXFBBuffersDesc(ContextVk *contextVk, size_t xfbBufferCount)
{
    mXFBBuffersDesc.reset();
    for (size_t bufferIndex = 0; bufferIndex < xfbBufferCount; ++bufferIndex)
    {
        const gl::OffsetBindingPointer<gl::Buffer> &binding = mState.getIndexedBuffer(bufferIndex);
        ASSERT(binding.get());

        BufferVk *bufferVk = vk::GetImpl(binding.get());

        if (bufferVk->isBufferValid())
        {
            mBufferHelpers[bufferIndex] = &bufferVk->getBuffer();
            mBufferOffsets[bufferIndex] = binding.getOffset();
            mBufferSizes[bufferIndex]   = gl::GetBoundBufferAvailableSize(binding);
        }
        else
        {
            // This can happen in error conditions.
            vk::BufferHelper &nullBuffer = contextVk->getEmptyBuffer();
            mBufferHelpers[bufferIndex]  = &nullBuffer;
            mBufferOffsets[bufferIndex]  = 0;
            mBufferSizes[bufferIndex]    = nullBuffer.getSize();
        }

        mXFBBuffersDesc.updateTransformFeedbackBuffer(
            bufferIndex, mBufferHelpers[bufferIndex]->getBufferSerial());
    }
}

angle::Result TransformFeedbackVk::begin(const gl::Context *context,
                                         gl::PrimitiveMode primitiveMode)
{
    ContextVk *contextVk = vk::GetImpl(context);

    const gl::ProgramExecutable *executable = contextVk->getState().getProgramExecutable();
    ASSERT(executable);
    size_t xfbBufferCount = executable->getTransformFeedbackBufferCount();

    initializeXFBBuffersDesc(contextVk, xfbBufferCount);

    for (size_t bufferIndex = 0; bufferIndex < xfbBufferCount; ++bufferIndex)
    {
        const gl::OffsetBindingPointer<gl::Buffer> &binding = mState.getIndexedBuffer(bufferIndex);
        mBufferHandles[bufferIndex] = mBufferHelpers[bufferIndex]->getBuffer().getHandle();
        if (contextVk->getFeatures().supportsTransformFeedbackExtension.enabled)
        {
            if (mCounterBufferHandles[bufferIndex] == VK_NULL_HANDLE)
            {
                VkBufferCreateInfo createInfo = {};
                createInfo.sType              = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
                createInfo.size               = 16;
                createInfo.usage       = VK_BUFFER_USAGE_TRANSFORM_FEEDBACK_COUNTER_BUFFER_BIT_EXT;
                createInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

                vk::BufferHelper &bufferHelper = mCounterBufferHelpers[bufferIndex];
                ANGLE_TRY(
                    bufferHelper.init(contextVk, createInfo, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT));

                mCounterBufferHandles[bufferIndex] = bufferHelper.getBuffer().getHandle();
            }
        }
        else
        {
            ASSERT(contextVk->getFeatures().emulateTransformFeedback.enabled);
            RendererVk *rendererVk = contextVk->getRenderer();
            const VkDeviceSize offsetAlignment =
                rendererVk->getPhysicalDeviceProperties().limits.minStorageBufferOffsetAlignment;

            // Make sure there's no possible under/overflow with binding size.
            static_assert(sizeof(VkDeviceSize) >= sizeof(binding.getSize()),
                          "VkDeviceSize too small");

            // Set the offset as close as possible to the requested offset while remaining aligned.
            mAlignedBufferOffsets[bufferIndex] =
                (mBufferOffsets[bufferIndex] / offsetAlignment) * offsetAlignment;
        }
    }

    if (contextVk->getFeatures().supportsTransformFeedbackExtension.enabled)
    {
        mRebindTransformFeedbackBuffer = true;
    }

    return contextVk->onBeginTransformFeedback(xfbBufferCount, mBufferHelpers);
}

angle::Result TransformFeedbackVk::end(const gl::Context *context)
{
    ContextVk *contextVk = vk::GetImpl(context);

    // If there's an active transform feedback query, accumulate the primitives drawn.
    const gl::State &glState = context->getState();
    gl::Query *transformFeedbackQuery =
        glState.getActiveQuery(gl::QueryType::TransformFeedbackPrimitivesWritten);

    if (transformFeedbackQuery && contextVk->getFeatures().emulateTransformFeedback.enabled)
    {
        vk::GetImpl(transformFeedbackQuery)->onTransformFeedbackEnd(mState.getPrimitivesDrawn());
    }

    contextVk->onEndTransformFeedback();
    return angle::Result::Continue;
}

angle::Result TransformFeedbackVk::pause(const gl::Context *context)
{
    ContextVk *contextVk = vk::GetImpl(context);

    if (contextVk->getFeatures().emulateTransformFeedback.enabled)
    {
        // Bind the empty buffer until we resume.
        const gl::ProgramExecutable *executable = contextVk->getState().getProgramExecutable();
        ASSERT(executable);
        size_t xfbBufferCount = executable->getTransformFeedbackBufferCount();

        const vk::BufferHelper &emptyBuffer = contextVk->getEmptyBuffer();

        for (size_t xfbIndex = 0; xfbIndex < xfbBufferCount; ++xfbIndex)
        {
            mXFBBuffersDesc.updateTransformFeedbackBuffer(xfbIndex, emptyBuffer.getBufferSerial());
        }
    }

    return contextVk->onPauseTransformFeedback();
}

angle::Result TransformFeedbackVk::resume(const gl::Context *context)
{
    ContextVk *contextVk                    = vk::GetImpl(context);
    const gl::ProgramExecutable *executable = contextVk->getState().getProgramExecutable();
    ASSERT(executable);
    size_t xfbBufferCount = executable->getTransformFeedbackBufferCount();

    if (contextVk->getFeatures().emulateTransformFeedback.enabled)
    {
        initializeXFBBuffersDesc(contextVk, xfbBufferCount);
    }

    return contextVk->onBeginTransformFeedback(xfbBufferCount, mBufferHelpers);
}

angle::Result TransformFeedbackVk::bindIndexedBuffer(
    const gl::Context *context,
    size_t index,
    const gl::OffsetBindingPointer<gl::Buffer> &binding)
{
    ContextVk *contextVk = vk::GetImpl(context);

    // Make sure the transform feedback buffers are bound to the program descriptor sets.
    contextVk->invalidateCurrentTransformFeedbackBuffers();

    return angle::Result::Continue;
}

void TransformFeedbackVk::updateDescriptorSetLayout(
    ContextVk *contextVk,
    const ShaderInterfaceVariableInfoMap &variableInfoMap,
    vk::DescriptorSetLayoutDesc *descSetLayoutOut) const
{
    if (!contextVk->getFeatures().emulateTransformFeedback.enabled)
    {
        return;
    }

    const ShaderInterfaceVariableInfo &info =
        variableInfoMap.get(gl::ShaderType::Vertex, sh::vk::kXfbEmulationBufferBlockName);

    descSetLayoutOut->update(info.binding, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER,
                             gl::IMPLEMENTATION_MAX_TRANSFORM_FEEDBACK_BUFFERS,
                             VK_SHADER_STAGE_VERTEX_BIT, nullptr);
}

void TransformFeedbackVk::initDescriptorSet(ContextVk *contextVk,
                                            const ShaderInterfaceVariableInfoMap &variableInfoMap,
                                            size_t xfbBufferCount,
                                            VkDescriptorSet descSet) const
{
    if (!contextVk->getFeatures().emulateTransformFeedback.enabled)
    {
        return;
    }

    VkDescriptorBufferInfo *descriptorBufferInfo =
        contextVk->allocDescriptorBufferInfos(gl::IMPLEMENTATION_MAX_TRANSFORM_FEEDBACK_BUFFERS);
    vk::BufferHelper *emptyBuffer = &contextVk->getEmptyBuffer();

    for (size_t bufferIndex = 0; bufferIndex < xfbBufferCount; ++bufferIndex)
    {
        VkDescriptorBufferInfo &bufferInfo = descriptorBufferInfo[bufferIndex];
        bufferInfo.buffer                  = emptyBuffer->getBuffer().getHandle();
        bufferInfo.offset                  = 0;
        bufferInfo.range                   = VK_WHOLE_SIZE;
    }

    PadEmulationDescriptorBufferInfo(xfbBufferCount, *emptyBuffer, descriptorBufferInfo);

    writeDescriptorSet(contextVk, variableInfoMap, descriptorBufferInfo, descSet);
}

void TransformFeedbackVk::updateDescriptorSet(ContextVk *contextVk,
                                              const gl::ProgramState &programState,
                                              const ShaderInterfaceVariableInfoMap &variableInfoMap,
                                              VkDescriptorSet descSet) const
{
    if (!contextVk->getFeatures().emulateTransformFeedback.enabled)
    {
        return;
    }

    const gl::ProgramExecutable *executable = contextVk->getState().getProgramExecutable();
    ASSERT(executable);
    size_t xfbBufferCount = executable->getTransformFeedbackBufferCount();

    ASSERT(xfbBufferCount > 0);
    ASSERT(programState.getTransformFeedbackBufferMode() != GL_INTERLEAVED_ATTRIBS ||
           xfbBufferCount == 1);

    VkDescriptorBufferInfo *descriptorBufferInfo =
        contextVk->allocDescriptorBufferInfos(gl::IMPLEMENTATION_MAX_TRANSFORM_FEEDBACK_BUFFERS);

    // Update buffer descriptor binding info for output buffers
    for (size_t bufferIndex = 0; bufferIndex < xfbBufferCount; ++bufferIndex)
    {
        VkDescriptorBufferInfo &bufferInfo = descriptorBufferInfo[bufferIndex];

        bufferInfo.buffer = mBufferHandles[bufferIndex];
        bufferInfo.offset = mAlignedBufferOffsets[bufferIndex];
        bufferInfo.range  = mBufferSizes[bufferIndex] +
                           (mBufferOffsets[bufferIndex] - mAlignedBufferOffsets[bufferIndex]);
        ASSERT(bufferInfo.range != 0);
    }

    PadEmulationDescriptorBufferInfo(xfbBufferCount, contextVk->getEmptyBuffer(),
                                     descriptorBufferInfo);

    writeDescriptorSet(contextVk, variableInfoMap, descriptorBufferInfo, descSet);
}

void TransformFeedbackVk::getBufferOffsets(ContextVk *contextVk,
                                           GLint drawCallFirstVertex,
                                           int32_t *offsetsOut,
                                           size_t offsetsSize) const
{
    if (!contextVk->getFeatures().emulateTransformFeedback.enabled)
    {
        return;
    }

    GLsizeiptr verticesDrawn = mState.getVerticesDrawn();
    const std::vector<GLsizei> &bufferStrides =
        mState.getBoundProgram()->getTransformFeedbackStrides();
    const gl::ProgramExecutable *executable = contextVk->getState().getProgramExecutable();
    ASSERT(executable);
    size_t xfbBufferCount = executable->getTransformFeedbackBufferCount();

    ASSERT(xfbBufferCount > 0);

    // The caller should make sure the offsets array has enough space.  The maximum possible
    // number of outputs is gl::IMPLEMENTATION_MAX_TRANSFORM_FEEDBACK_BUFFERS.
    ASSERT(offsetsSize >= xfbBufferCount);

    for (size_t bufferIndex = 0; bufferIndex < xfbBufferCount; ++bufferIndex)
    {
        int64_t offsetFromDescriptor =
            static_cast<int64_t>(mBufferOffsets[bufferIndex] - mAlignedBufferOffsets[bufferIndex]);
        int64_t drawCallVertexOffset = static_cast<int64_t>(verticesDrawn) - drawCallFirstVertex;

        int64_t writeOffset =
            (offsetFromDescriptor + drawCallVertexOffset * bufferStrides[bufferIndex]) /
            static_cast<int64_t>(sizeof(uint32_t));

        offsetsOut[bufferIndex] = static_cast<int32_t>(writeOffset);

        // Assert on overflow.  For now, support transform feedback up to 2GB.
        ASSERT(offsetsOut[bufferIndex] == writeOffset);
    }
}

void TransformFeedbackVk::writeDescriptorSet(ContextVk *contextVk,
                                             const ShaderInterfaceVariableInfoMap &variableInfoMap,
                                             VkDescriptorBufferInfo *bufferInfo,
                                             VkDescriptorSet descSet) const
{
    ASSERT(contextVk->getFeatures().emulateTransformFeedback.enabled);

    const ShaderInterfaceVariableInfo &info =
        variableInfoMap.get(gl::ShaderType::Vertex, sh::vk::kXfbEmulationBufferBlockName);

    VkWriteDescriptorSet &writeDescriptorInfo = contextVk->allocWriteDescriptorSet();
    writeDescriptorInfo.sType                 = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
    writeDescriptorInfo.dstSet                = descSet;
    writeDescriptorInfo.dstBinding            = info.binding;
    writeDescriptorInfo.dstArrayElement       = 0;
    writeDescriptorInfo.descriptorCount       = gl::IMPLEMENTATION_MAX_TRANSFORM_FEEDBACK_BUFFERS;
    writeDescriptorInfo.descriptorType        = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
    writeDescriptorInfo.pImageInfo            = nullptr;
    writeDescriptorInfo.pBufferInfo           = bufferInfo;
    writeDescriptorInfo.pTexelBufferView      = nullptr;
}

}  // namespace rx
