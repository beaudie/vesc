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
#include "libANGLE/renderer/vulkan/FramebufferVk.h"
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
    onBeginEnd(context);

    return angle::Result::Continue;
}

angle::Result TransformFeedbackVk::end(const gl::Context *context)
{
    // Remember the calculated number of primitives drawn to be used as query result.
    mPrimitivesDrawn += mState.getPrimitivesDrawn();

    vk::GetImpl(context)->onTransformFeedbackPauseResume();
    onBeginEnd(context);

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
        bufferHelper.addWriteDependency(framebuffer);
        bufferHelper.onWrite(contextVk, VK_ACCESS_SHADER_WRITE_BIT);
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

    const VkDeviceSize offsetAlignment = contextVk->getRenderer()
                                             ->getPhysicalDeviceProperties()
                                             .limits.minStorageBufferOffsetAlignment;

    std::array<VkDescriptorBufferInfo, kMaxSeparateAttributes> descriptorBufferInfo;

    // Write default uniforms for each shader type.
    for (size_t bufferIndex = 0; bufferIndex < xfbBufferCount; ++bufferIndex)
    {
        VkDescriptorBufferInfo &bufferInfo = descriptorBufferInfo[bufferIndex];

        const gl::OffsetBindingPointer<gl::Buffer> &bufferBinding = xfbBuffers[bufferIndex];
        gl::Buffer *buffer                                        = bufferBinding.get();
        GLintptr offset                                           = bufferBinding.getOffset();
        GLsizeiptr size = gl::GetBoundBufferAvailableSize(bufferBinding);
        ASSERT(buffer != nullptr);

        // Make sure there's no possible under/overflow with binding size.
        static_assert(sizeof(VkDeviceSize) >= sizeof(bufferBinding.getSize()),
                      "VkDeviceSize too small");

        vk::BufferHelper &bufferHelper = vk::GetImpl(buffer)->getBuffer();

        // Set the offset as close as possible to the requested offset while remaining aligned.
        VkDeviceSize alignedOffset = (offset / offsetAlignment) * offsetAlignment;

        bufferInfo.buffer = bufferHelper.getBuffer().getHandle();
        bufferInfo.offset = alignedOffset;
        bufferInfo.range  = size + (offset - alignedOffset);
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

void TransformFeedbackVk::updateBufferOffsets(ContextVk *contextVk,
                                              const gl::ProgramState &programState,
                                              GLint drawCallFirstVertex,
                                              int32_t *offsetsOut,
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

    const VkDeviceSize offsetAlignment = contextVk->getRenderer()
                                             ->getPhysicalDeviceProperties()
                                             .limits.minStorageBufferOffsetAlignment;

    for (size_t bufferIndex = 0; bufferIndex < xfbBufferCount; ++bufferIndex)
    {
        const gl::OffsetBindingPointer<gl::Buffer> &bufferBinding = xfbBuffers[bufferIndex];
        ASSERT(bufferBinding.get() != nullptr);

        GLintptr offset = bufferBinding.getOffset();

        // The descriptor offset is set to the same offset or close to it to remain aligned.
        VkDeviceSize descriptorOffset = (offset / offsetAlignment) * offsetAlignment;

        int64_t writeOffset =
            static_cast<int64_t>(offset - descriptorOffset +
                                 (static_cast<GLintptr>(verticesDrawn) - drawCallFirstVertex) *
                                     bufferStrides[bufferIndex]) /
            static_cast<int64_t>(sizeof(uint32_t));

        offsetsOut[bufferIndex] = static_cast<int32_t>(writeOffset);

        // Assert on overflow.  For now, support transform feedback up to 2GB.
        ASSERT(offsetsOut[bufferIndex] == writeOffset);
    }
}

size_t TransformFeedbackVk::getPrimitivesDrawn() const
{
    // Return accumulated primitives drawn plus whatever primitives have been drawn since last
    // begin().
    return mPrimitivesDrawn + mState.getPrimitivesDrawn();
}

void TransformFeedbackVk::onBeginEnd(const gl::Context *context)
{
    // Currently, we don't handle resources switching from read-only to writable and back correctly.
    // In the case of transform feedback, the attached buffers can switch between being written by
    // transform feedback and being read as a vertex array buffer.  For now, we'll end the render
    // pass every time transform feedback is started or ended to work around this issue temporarily.
    //
    // TODO(syoussefi): use a dirty bit on buffer objects to create a new node if necessary every
    // time the buffer usage changes.  http://anglebug.com/3205
    ContextVk *contextVk               = vk::GetImpl(context);
    FramebufferVk *framebufferVk       = vk::GetImpl(context->getState().getDrawFramebuffer());
    vk::FramebufferHelper *framebuffer = framebufferVk->getFramebuffer();

    if (framebuffer->hasStartedRenderPass())
    {
        framebuffer->finishCurrentCommands(contextVk);
    }
}

}  // namespace rx
