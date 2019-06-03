//
// Copyright 2016 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//
// TransformFeedbackVk.h:
//    Defines the class interface for TransformFeedbackVk, implementing TransformFeedbackImpl.
//

#ifndef LIBANGLE_RENDERER_VULKAN_TRANSFORMFEEDBACKVK_H_
#define LIBANGLE_RENDERER_VULKAN_TRANSFORMFEEDBACKVK_H_

#include "libANGLE/renderer/TransformFeedbackImpl.h"
#include "libANGLE/renderer/vulkan/vk_helpers.h"

namespace gl
{
class ProgramState;
}  // namespace gl

namespace rx
{
namespace vk
{
class DescriptorSetLayoutDesc;
}

class TransformFeedbackVk : public TransformFeedbackImpl
{
  public:
    TransformFeedbackVk(const gl::TransformFeedbackState &state);
    ~TransformFeedbackVk() override;

    angle::Result begin(const gl::Context *context, gl::PrimitiveMode primitiveMode) override;
    angle::Result end(const gl::Context *context) override;
    angle::Result pause(const gl::Context *context) override;
    angle::Result resume(const gl::Context *context) override;

    angle::Result bindIndexedBuffer(const gl::Context *context,
                                    size_t index,
                                    const gl::OffsetBindingPointer<gl::Buffer> &binding) override;

    void updateDescriptorSetLayout(const gl::ProgramState &programState,
                                   vk::DescriptorSetLayoutDesc *descSetLayoutOut) const;
    void addFramebufferDependency(ContextVk *contextVk,
                                  const gl::ProgramState &programState,
                                  vk::FramebufferHelper *framebuffer) const;
    void updateDescriptorSet(ContextVk *contextVk,
                             const gl::ProgramState &programState,
                             VkDescriptorSet descSet,
                             uint32_t bindingOffset) const;
    void updateBufferOffsets(const gl::ProgramState &programState,
                             GLint drawCallFirstVertex,
                             uint32_t *offsetsOut,
                             size_t offsetsSize) const;

    void resetPrimitivesDrawn() { mPrimitivesDrawn = 0; }
    size_t getPrimitivesDrawn() const;

    static constexpr uint32_t GetMaxInterleavedComponents() { return kMaxInterleavedComponents; }
    static constexpr uint32_t GetMaxSeparateAttributes() { return kMaxSeparateAttributes; }
    static constexpr uint32_t GetMaxSeparateComponents() { return kMaxSeparateComponents; }

  private:
    // Support 128 interleaved components, as we use 4 32-bit integers as a mask to determine which
    // varyings should find their way to the transform feedback output.
    static constexpr uint32_t kMaxInterleavedComponents = 128;
    // We are required to support at least 4 separate buffers.  We choose the minimum to reduce the
    // impact on driver uniforms size, which is where the offsets are stored.
    static constexpr uint32_t kMaxSeparateAttributes = 4;
    // Support every type, from single-component to mat4.
    static constexpr uint32_t kMaxSeparateComponents = 16;

    size_t mPrimitivesDrawn;
};

}  // namespace rx

#endif  // LIBANGLE_RENDERER_VULKAN_TRANSFORMFEEDBACKVK_H_
