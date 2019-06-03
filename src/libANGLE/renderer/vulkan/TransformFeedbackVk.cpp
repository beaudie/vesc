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
#include "libANGLE/renderer/vulkan/BufferVk.h"
#include "libANGLE/renderer/vulkan/ContextVk.h"
#include "libANGLE/renderer/vulkan/ProgramVk.h"

#include "common/debug.h"

namespace rx
{

TransformFeedbackVk::TransformFeedbackVk(const gl::TransformFeedbackState &state)
    : TransformFeedbackImpl(state), mPrimitivesDrawn(0)
{}

TransformFeedbackVk::~TransformFeedbackVk() {}

angle::Result TransformFeedbackVk::begin(const gl::Context *context,
                                         gl::PrimitiveMode primitiveMode)
{
    ContextVk *contextVk = vk::GetImpl(context);

    // Make sure the transform feedback buffers are bound to the program descriptor sets.
    contextVk->invalidateCurrentTransformFeedbackBuffers();

    vk::GetImpl(context)->onTransformFeedbackPauseResume();
    return angle::Result::Continue;
}

angle::Result TransformFeedbackVk::end(const gl::Context *context)
{
    // Remember the calculated number of primitives drawn to be used as query result.
    mPrimitivesDrawn += mState.getPrimitivesDrawn();

    vk::GetImpl(context)->onTransformFeedbackPauseResume();
    return angle::Result::Continue;
}

angle::Result TransformFeedbackVk::pause(const gl::Context *context)
{
    vk::GetImpl(context)->onTransformFeedbackPauseResume();
    return angle::Result::Continue;
}

angle::Result TransformFeedbackVk::resume(const gl::Context *context)
{
    vk::GetImpl(context)->onTransformFeedbackPauseResume();
    return angle::Result::Continue;
}

angle::Result TransformFeedbackVk::bindIndexedBuffer(
    const gl::Context *context,
    size_t index,
    const gl::OffsetBindingPointer<gl::Buffer> &binding)
{
    // Nothing to do.
    return angle::Result::Continue;
}

void TransformFeedbackVk::updateDescriptorSetLayout(
    const gl::ProgramState &programState,
    vk::DescriptorSetLayoutDesc *descSetLayoutOut) const
{
    size_t xfbBufferCount = programState.getTransformFeedbackBufferCount();
    if (xfbBufferCount == 0)
    {
        return;
    }

    for (size_t bufferIndex = 0; bufferIndex < xfbBufferCount; ++bufferIndex)
    {
        descSetLayoutOut->update(kXfbBindingIndexStart + bufferIndex,
                                 VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, 1, VK_SHADER_STAGE_VERTEX_BIT);
    }
}

void TransformFeedbackVk::addFramebufferDependency(ContextVk *contextVk,
                                                   const gl::ProgramState &programState,
                                                   vk::FramebufferHelper *framebuffer) const
{
    const std::vector<gl::OffsetBindingPointer<gl::Buffer>> &xfbBuffers =
        mState.getIndexedBuffers();
    size_t xfbBufferCount = programState.getTransformFeedbackBufferCount();

    ASSERT(xfbBufferCount > 0);
    ASSERT(programState.getTransformFeedbackBufferMode() != GL_INTERLEAVED_ATTRIBS ||
           xfbBufferCount == 1);

    // Set framebuffer dependent to the transform feedback buffers.  This is especially done
    // separately from |updateDescriptorSet|, to avoid introducing unnecessary buffer barriers
    // every time the descriptor set is updated (which, as the set is shared with default uniforms,
    // could get updated frequently).
    for (size_t bufferIndex = 0; bufferIndex < xfbBufferCount; ++bufferIndex)
    {
        const gl::OffsetBindingPointer<gl::Buffer> &bufferBinding = xfbBuffers[bufferIndex];
        gl::Buffer *buffer                                        = bufferBinding.get();
        ASSERT(buffer != nullptr);

        vk::BufferHelper &bufferHelper = vk::GetImpl(buffer)->getBuffer();
        bufferHelper.onWrite(contextVk, framebuffer, VK_ACCESS_SHADER_WRITE_BIT);
    }
}

void TransformFeedbackVk::updateDescriptorSet(ContextVk *contextVk,
                                              const gl::ProgramState &programState,
                                              VkDescriptorSet descSet,
                                              uint32_t bindingOffset) const
{
    const std::vector<gl::OffsetBindingPointer<gl::Buffer>> &xfbBuffers =
        mState.getIndexedBuffers();
    size_t xfbBufferCount = programState.getTransformFeedbackBufferCount();

    ASSERT(xfbBufferCount > 0);
    ASSERT(programState.getTransformFeedbackBufferMode() != GL_INTERLEAVED_ATTRIBS ||
           xfbBufferCount == 1);

    std::array<VkDescriptorBufferInfo, kMaxSeparateAttributes> descriptorBufferInfo;

    // Write default uniforms for each shader type.
    for (size_t bufferIndex = 0; bufferIndex < xfbBufferCount; ++bufferIndex)
    {
        VkDescriptorBufferInfo &bufferInfo = descriptorBufferInfo[bufferIndex];

        const gl::OffsetBindingPointer<gl::Buffer> &bufferBinding = xfbBuffers[bufferIndex];
        gl::Buffer *buffer                                        = bufferBinding.get();
        ASSERT(buffer != nullptr);

        // Make sure there's no possible under/overflow with binding size.
        static_assert(sizeof(VkDeviceSize) >= sizeof(bufferBinding.getSize()),
                      "VkDeviceSize too small");

        vk::BufferHelper &bufferHelper = vk::GetImpl(buffer)->getBuffer();

        bufferInfo.buffer = bufferHelper.getBuffer().getHandle();
        bufferInfo.offset = 0;
        bufferInfo.range  = VK_WHOLE_SIZE;
    }

    VkDevice device = contextVk->getDevice();

    VkWriteDescriptorSet writeDescriptorInfo = {};
    writeDescriptorInfo.sType                = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
    writeDescriptorInfo.dstSet               = descSet;
    writeDescriptorInfo.dstBinding           = bindingOffset;
    writeDescriptorInfo.dstArrayElement      = 0;
    writeDescriptorInfo.descriptorCount      = xfbBufferCount;
    writeDescriptorInfo.descriptorType       = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
    writeDescriptorInfo.pImageInfo           = nullptr;
    writeDescriptorInfo.pBufferInfo          = descriptorBufferInfo.data();
    writeDescriptorInfo.pTexelBufferView     = nullptr;

    vkUpdateDescriptorSets(device, 1, &writeDescriptorInfo, 0, nullptr);
}

void TransformFeedbackVk::updateBufferOffsets(const gl::ProgramState &programState,
                                              GLint drawCallFirstVertex,
                                              uint32_t *offsetsOut,
                                              size_t offsetsSize) const
{
    const std::vector<gl::OffsetBindingPointer<gl::Buffer>> &xfbBuffers =
        mState.getIndexedBuffers();
    GLsizeiptr verticesDrawn = mState.getVerticesDrawn();
    const std::vector<GLsizei> &bufferStrides =
        mState.getBoundProgram()->getTransformFeedbackStrides();
    size_t xfbBufferCount = programState.getTransformFeedbackBufferCount();

    ASSERT(xfbBufferCount > 0);

    // The caller should make sure the offsets array has enough space.  The maximum possible number
    // of outputs is kMaxSeparateAttributes.
    ASSERT(offsetsSize >= xfbBufferCount);

    for (size_t bufferIndex = 0; bufferIndex < xfbBufferCount; ++bufferIndex)
    {
        const gl::OffsetBindingPointer<gl::Buffer> &bufferBinding = xfbBuffers[bufferIndex];
        ASSERT(bufferBinding.get() != nullptr);

        GLintptr offset = bufferBinding.getOffset();
        ASSERT(static_cast<size_t>(offset) < sizeof(uint32_t));

        offsetsOut[bufferIndex] = static_cast<uint32_t>(
            (offset + (static_cast<GLintptr>(verticesDrawn) - drawCallFirstVertex) *
                          bufferStrides[bufferIndex]) /
            sizeof(uint32_t));
    }
}

size_t TransformFeedbackVk::getPrimitivesDrawn() const
{
    // Return accumulated primitives drawn plus whatever primitives have been drawn since last
    // begin().
    return mPrimitivesDrawn + mState.getPrimitivesDrawn();
}

}  // namespace rx
