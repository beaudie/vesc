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
    : TransformFeedbackImpl(state)
{}

TransformFeedbackVk::~TransformFeedbackVk() {}

angle::Result TransformFeedbackVk::begin(const gl::Context *context,
                                         gl::PrimitiveMode primitiveMode)
{
    ContextVk *contextVk = vk::GetImpl(context);

    // Make sure the transform feedback buffers are bound to the program descriptor sets.
    contextVk->invalidateCurrentTransformFeedbackBuffers();
    contextVk->invalidateCurrentTransformFeedbackBuffers();
    return angle::Result::Continue;
}

angle::Result TransformFeedbackVk::end(const gl::Context *context)
{
    // Nothing to do.
    return angle::Result::Continue;
}

angle::Result TransformFeedbackVk::pause(const gl::Context *context)
{
    // Nothing to do.
    return angle::Result::Continue;
}

angle::Result TransformFeedbackVk::resume(const gl::Context *context)
{
    // Nothing to do.
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
    fprintf(stderr, "%zu\n", xfbBufferCount);
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

angle::Result TransformFeedbackVk::updateDescriptorSet(ContextVk *contextVk,
                                                       const gl::ProgramState &programState,
                                                       VkDescriptorSet descSet,
                                                       uint32_t bindingOffset,
                                                       vk::FramebufferHelper *framebuffer) const
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
        bufferHelper.updateQueueSerial(contextVk->getCurrentQueueSerial());

        // Make sure there's a graph node to create a dependency with.
        // TODO: this kills subpasses. addWriteDependency seems appropriate here, but suffers from
        // a bug in the command graph. If Framebuffer F writes to buffer B, then reads from it,
        // there would be a loop in the graph. I think addWriteDependency should take the context
        // and break off the writer's node if a loop is detected.  That's gonna be more work.
        // I already have a stash doing that with addReadDependency.
        vk::CommandBuffer *commandBuffer;
        ANGLE_TRY(bufferHelper.recordCommands(contextVk, &commandBuffer));

        bufferHelper.onWrite(VK_ACCESS_SHADER_WRITE_BIT);
        framebuffer->addReadDependency(&bufferHelper);

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

    return angle::Result::Continue;
}

void TransformFeedbackVk::updateBufferOffsets(const gl::ProgramState &programState,
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

        // TODO: get mVerticesDrawn from state and multiply it by the size of the varying that's
        // output to this buffer, or the total sum if interleaved.  Otherwise this is always the
        // start offset of the buffer.
        offsetsOut[bufferIndex] =
            static_cast<uint32_t>(offset) + verticesDrawn * bufferStrides[bufferIndex];
    }
}

}  // namespace rx
