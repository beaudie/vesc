//
// Copyright 2015 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//

// RendererVk.h: Defines the class interface for RendererVk.

#ifndef LIBANGLE_RENDERER_VULKAN_RENDERERVK_H_
#define LIBANGLE_RENDERER_VULKAN_RENDERERVK_H_

#include "libANGLE/renderer/Renderer.h"

namespace rx
{

class RendererVk : public Renderer
{
  public:
    RendererVk();
    ~RendererVk() override;

    gl::Error flush() override;
    gl::Error finish() override;

    gl::Error drawArrays(const gl::Data &data, GLenum mode, GLint first, GLsizei count) override;
    gl::Error drawArraysInstanced(const gl::Data &data,
                                  GLenum mode,
                                  GLint first,
                                  GLsizei count,
                                  GLsizei instanceCount) override;

    gl::Error drawElements(const gl::Data &data,
                           GLenum mode,
                           GLsizei count,
                           GLenum type,
                           const GLvoid *indices,
                           const gl::IndexRange &indexRange) override;
    gl::Error drawElementsInstanced(const gl::Data &data,
                                    GLenum mode,
                                    GLsizei count,
                                    GLenum type,
                                    const GLvoid *indices,
                                    GLsizei instances,
                                    const gl::IndexRange &indexRange) override;
    gl::Error drawRangeElements(const gl::Data &data,
                                GLenum mode,
                                GLuint start,
                                GLuint end,
                                GLsizei count,
                                GLenum type,
                                const GLvoid *indices,
                                const gl::IndexRange &indexRange) override;

    // lost device
    // TODO(jmadill): investigate if this stuff is necessary in non-D3D
    void notifyDeviceLost() override;
    bool isDeviceLost() const override;
    bool testDeviceLost() override;
    bool testDeviceResettable() override;

    std::string getVendorString() const override;
    std::string getRendererDescription() const override;

    void insertEventMarker(GLsizei length, const char *marker) override;
    void pushGroupMarker(GLsizei length, const char *marker) override;
    void popGroupMarker() override;

    void syncState(const gl::State &state, const gl::State::DirtyBits &dirtyBits) override;

    // Disjoint timer queries
    GLint getGPUDisjoint() override;
    GLint64 getTimestamp() override;

    // Context switching
    void onMakeCurrent(const gl::Data &data) override;

    // Renderer capabilities

    // Shader creation
    CompilerImpl *createCompiler() override;
    ShaderImpl *createShader(const gl::Shader::Data &data) override;
    ProgramImpl *createProgram(const gl::Program::Data &data) override;

    // Framebuffer creation
    FramebufferImpl *createFramebuffer(const gl::Framebuffer::Data &data) override;

    // Texture creation
    TextureImpl *createTexture(GLenum target) override;

    // Renderbuffer creation
    RenderbufferImpl *createRenderbuffer() override;

    // Buffer creation
    BufferImpl *createBuffer() override;

    // Vertex Array creation
    VertexArrayImpl *createVertexArray(const gl::VertexArray::Data &data) override;

    // Query and Fence creation
    QueryImpl *createQuery(GLenum type) override;
    FenceNVImpl *createFenceNV() override;
    FenceSyncImpl *createFenceSync() override;

    // Transform Feedback creation
    TransformFeedbackImpl *createTransformFeedback() override;

    // Sampler object creation
    SamplerImpl *createSampler() override;
};

}  // namespace rx

#endif  // LIBANGLE_RENDERER_VULKAN_RENDERERVK_H_
