//
// Copyright 2015 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//
// angle_unittests_utils.h:
//   Helpers for mocking and unit testing.

#ifndef TESTS_ANGLE_UNITTESTS_UTILS_H_
#define TESTS_ANGLE_UNITTESTS_UTILS_H_

#include "libANGLE/Surface.h"
#include "libANGLE/renderer/ContextImpl.h"
#include "libANGLE/renderer/EGLImplFactory.h"
#include "libANGLE/renderer/GLImplFactory.h"

namespace rx
{

// Useful when mocking a part of the GLImplFactory class
class NullFactory : public GLImplFactory
{
  public:
    NullFactory() {}

    // Shader creation
    CompilerImpl *createCompiler() override { return nullptr; }
    ShaderImpl *createShader(const gl::ShaderState &data) override { return nullptr; }
    ProgramImpl *createProgram(const gl::ProgramState &data) override { return nullptr; }

    // Framebuffer creation
    FramebufferImpl *createFramebuffer(const gl::FramebufferState &data) override
    {
        return nullptr;
    }

    // Texture creation
    TextureImpl *createTexture(const gl::TextureState &data) override { return nullptr; }

    // Renderbuffer creation
    RenderbufferImpl *createRenderbuffer(const gl::RenderbufferState &state) override
    {
        return nullptr;
    }

    // Buffer creation
    BufferImpl *createBuffer(const gl::BufferState &state) override { return nullptr; }

    // Vertex Array creation
    VertexArrayImpl *createVertexArray(const gl::VertexArrayState &data) override
    {
        return nullptr;
    }

    // Query and Fence creation
    QueryImpl *createQuery(gl::QueryType type) override { return nullptr; }
    FenceNVImpl *createFenceNV() override { return nullptr; }
    SyncImpl *createSync() override { return nullptr; }

    // Transform Feedback creation
    TransformFeedbackImpl *createTransformFeedback(const gl::TransformFeedbackState &state) override
    {
        return nullptr;
    }

    // Sampler object creation
    SamplerImpl *createSampler(const gl::SamplerState &state) override { return nullptr; }

    // Program Pipeline creation
    ProgramPipelineImpl *createProgramPipeline(const gl::ProgramPipelineState &data) override
    {
        return nullptr;
    }

    SemaphoreImpl *createSemaphore() override { return nullptr; }

    OverlayImpl *createOverlay(const gl::OverlayState &state) override { return nullptr; }
};

// A class with all the factory methods mocked.
class MockGLFactory : public GLImplFactory
{
  public:
    MOCK_METHOD(ContextImpl *, createContext, (const gl::State &));
    MOCK_METHOD(CompilerImpl *, createCompiler, (), (override));
    MOCK_METHOD(ShaderImpl *, createShader, (const gl::ShaderState &), (override));
    MOCK_METHOD(ProgramImpl *, createProgram, (const gl::ProgramState &), (override));
    MOCK_METHOD(ProgramPipelineImpl *,
                createProgramPipeline,
                (const gl::ProgramPipelineState &),
                (override));
    MOCK_METHOD(FramebufferImpl *, createFramebuffer, (const gl::FramebufferState &), (override));
    MOCK_METHOD(MemoryObjectImpl *, createMemoryObject, (), (override));
    MOCK_METHOD(TextureImpl *, createTexture, (const gl::TextureState &), (override));
    MOCK_METHOD(RenderbufferImpl *,
                createRenderbuffer,
                (const gl::RenderbufferState &),
                (override));
    MOCK_METHOD(BufferImpl *, createBuffer, (const gl::BufferState &), (override));
    MOCK_METHOD(VertexArrayImpl *, createVertexArray, (const gl::VertexArrayState &), (override));
    MOCK_METHOD(QueryImpl *, createQuery, (gl::QueryType type), (override));
    MOCK_METHOD(FenceNVImpl *, createFenceNV, (), (override));
    MOCK_METHOD(SyncImpl *, createSync, (), (override));
    MOCK_METHOD(TransformFeedbackImpl *,
                createTransformFeedback,
                (const gl::TransformFeedbackState &),
                (override));
    MOCK_METHOD(SamplerImpl *, createSampler, (const gl::SamplerState &), (override));
    MOCK_METHOD(SemaphoreImpl *, createSemaphore, (), (override));
    MOCK_METHOD(OverlayImpl *, createOverlay, (const gl::OverlayState &), (override));
};

class MockEGLFactory : public EGLImplFactory
{
  public:
    MOCK_METHOD(SurfaceImpl *,
                createWindowSurface,
                (const egl::SurfaceState &, EGLNativeWindowType, const egl::AttributeMap &),
                (override));
    MOCK_METHOD(SurfaceImpl *,
                createPbufferSurface,
                (const egl::SurfaceState &, const egl::AttributeMap &),
                (override));
    MOCK_METHOD(SurfaceImpl *,
                createPbufferFromClientBuffer,
                (const egl::SurfaceState &, EGLenum, EGLClientBuffer, const egl::AttributeMap &),
                (override));
    MOCK_METHOD(SurfaceImpl *,
                createPixmapSurface,
                (const egl::SurfaceState &, NativePixmapType, const egl::AttributeMap &),
                (override));
    MOCK_METHOD(ImageImpl *,
                createImage,
                (const egl::ImageState &, const gl::Context *, EGLenum, const egl::AttributeMap &),
                (override));
    MOCK_METHOD(ContextImpl *,
                createContext,
                (const gl::State &,
                 gl::ErrorSet *,
                 const egl::Config *,
                 const gl::Context *,
                 const egl::AttributeMap &),
                (override));
    MOCK_METHOD(StreamProducerImpl *,
                createStreamProducerD3DTexture,
                (egl::Stream::ConsumerType, const egl::AttributeMap &),
                (override));
    MOCK_METHOD(ShareGroupImpl *, createShareGroup, (), (override));
};

}  // namespace rx

#endif  // TESTS_ANGLE_UNITTESTS_UTILS_H_
