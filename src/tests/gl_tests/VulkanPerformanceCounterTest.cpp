//
// Copyright 2020 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//
// VulkanPerformanceCounterTest:
//   Validates specific GL call patterns with ANGLE performance counters.
//   For example we can verify a certain call set doesn't break the RenderPass.
//
// TODO(jmadill): Move to a GL extension. http://anglebug.com/4918

#include "test_utils/ANGLETest.h"
#include "test_utils/angle_test_instantiate.h"
// 'None' is defined as 'struct None {};' in
// third_party/googletest/src/googletest/include/gtest/internal/gtest-type-util.h.
// But 'None' is also defined as a numeric constant 0L in <X11/X.h>.
// So we need to include ANGLETest.h first to avoid this conflict.

#include "libANGLE/Context.h"
#include "libANGLE/angletypes.h"
#include "libANGLE/renderer/vulkan/ContextVk.h"
#include "test_utils/gl_raii.h"

using namespace angle;

namespace
{
class VulkanPerformanceCounterTest : public ANGLETest
{
  protected:
    VulkanPerformanceCounterTest()
    {
        // Depth required for SwapShouldInvalidateDepthAfterClear.
        // Also RGBA8 is required to avoid the clear for emulated alpha.
        setConfigRedBits(8);
        setConfigGreenBits(8);
        setConfigBlueBits(8);
        setConfigAlphaBits(8);
        setConfigDepthBits(24);
    }

    const rx::vk::PerfCounters &hackANGLE() const
    {
        // Hack the angle!
        const gl::Context *context = static_cast<const gl::Context *>(getEGLWindow()->getContext());
        return rx::GetImplAs<const rx::ContextVk>(context)->getPerfCounters();
    }

    void setupClearAndDrawForInvalidateTest(GLProgram *program,
                                            GLFramebuffer *framebuffer,
                                            GLTexture *texture,
                                            GLRenderbuffer *renderbuffer)
    {
        glUseProgram(*program);

        // Setup to draw to color, depth, and stencil
        glBindFramebuffer(GL_FRAMEBUFFER, *framebuffer);
        glBindTexture(GL_TEXTURE_2D, *texture);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, 16, 16, 0, GL_RGBA, GL_UNSIGNED_BYTE, nullptr);
        glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, *texture, 0);
        glBindRenderbuffer(GL_RENDERBUFFER, *renderbuffer);
        glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH24_STENCIL8, 16, 16);
        glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_RENDERBUFFER,
                                  *renderbuffer);
        ASSERT_GL_FRAMEBUFFER_COMPLETE(GL_FRAMEBUFFER);

        // Clear and draw with depth and stencil buffer enabled
        glEnable(GL_DEPTH_TEST);
        glDepthMask(GL_TRUE);
        glDepthFunc(GL_GEQUAL);
        glClearDepthf(0.99f);
        glEnable(GL_STENCIL_TEST);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        drawQuad(*program, essl1_shaders::PositionAttrib(), 0.5f);
        ASSERT_GL_NO_ERROR();
    }

    void invalidateForInvalidateTest()
    {
        const GLenum discards[] = {GL_DEPTH_ATTACHMENT, GL_STENCIL_ATTACHMENT};
        // Note: PUBG uses glDiscardFramebufferEXT() instead of glInvalidateFramebuffer()
        glDiscardFramebufferEXT(GL_FRAMEBUFFER, 2, discards);
        ASSERT_GL_NO_ERROR();
    }

    void enableForInvalidateTest()
    {
        glEnable(GL_DEPTH_TEST);
        glEnable(GL_STENCIL_TEST);
    }

    void disableForInvalidateTest()
    {
        glDisable(GL_DEPTH_TEST);
        glDisable(GL_STENCIL_TEST);
    }

    void drawForInvalidateTest(GLProgram *program)
    {
        drawQuad(*program, essl1_shaders::PositionAttrib(), 0.5f);
        ASSERT_GL_NO_ERROR();
    }
};

class VulkanPerformanceCounterTest_ES31 : public VulkanPerformanceCounterTest
{};

// Tests that texture updates to unused textures don't break the RP.
TEST_P(VulkanPerformanceCounterTest, NewTextureDoesNotBreakRenderPass)
{
    const rx::vk::PerfCounters &counters = hackANGLE();

    GLColor kInitialData[4] = {GLColor::red, GLColor::blue, GLColor::green, GLColor::yellow};

    // Step 1: Set up a simple 2D Texture rendering loop.
    GLTexture texture;
    glBindTexture(GL_TEXTURE_2D, texture);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, 2, 2, 0, GL_RGBA, GL_UNSIGNED_BYTE, kInitialData);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

    auto quadVerts = GetQuadVertices();

    GLBuffer vertexBuffer;
    glBindBuffer(GL_ARRAY_BUFFER, vertexBuffer);
    glBufferData(GL_ARRAY_BUFFER, quadVerts.size() * sizeof(quadVerts[0]), quadVerts.data(),
                 GL_STATIC_DRAW);

    ANGLE_GL_PROGRAM(program, essl1_shaders::vs::Texture2D(), essl1_shaders::fs::Texture2D());
    glUseProgram(program);

    GLint posLoc = glGetAttribLocation(program, essl1_shaders::PositionAttrib());
    ASSERT_NE(-1, posLoc);

    glVertexAttribPointer(posLoc, 3, GL_FLOAT, GL_FALSE, 0, nullptr);
    glEnableVertexAttribArray(posLoc);
    ASSERT_GL_NO_ERROR();

    glDrawArrays(GL_TRIANGLES, 0, 6);
    ASSERT_GL_NO_ERROR();
    uint32_t expectedRenderPassCount = counters.renderPasses;

    // Step 2: Introduce a new 2D Texture with the same Program and Framebuffer.
    GLTexture newTexture;
    glBindTexture(GL_TEXTURE_2D, newTexture);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, 2, 2, 0, GL_RGBA, GL_UNSIGNED_BYTE, kInitialData);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

    glDrawArrays(GL_TRIANGLES, 0, 6);
    ASSERT_GL_NO_ERROR();

    uint32_t actualRenderPassCount = counters.renderPasses;
    EXPECT_EQ(expectedRenderPassCount, actualRenderPassCount);
}

// Tests that RGB texture should not break renderpass.
TEST_P(VulkanPerformanceCounterTest, SampleFromRGBTextureDoesNotBreakRenderPass)
{
    const rx::vk::PerfCounters &counters = hackANGLE();

    ANGLE_GL_PROGRAM(program, essl1_shaders::vs::Texture2D(), essl1_shaders::fs::Texture2D());
    glUseProgram(program);
    GLint textureLoc = glGetUniformLocation(program, essl1_shaders::Texture2DUniform());
    ASSERT_NE(-1, textureLoc);

    GLTexture textureRGBA;
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, textureRGBA);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, 2, 2, 0, GL_RGBA, GL_UNSIGNED_BYTE, nullptr);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, 1, 1, 0, GL_RGBA, GL_UNSIGNED_BYTE, nullptr);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    GLTexture textureRGB;
    glActiveTexture(GL_TEXTURE1);
    glBindTexture(GL_TEXTURE_2D, textureRGB);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, 2, 2, 0, GL_RGB, GL_UNSIGNED_BYTE, nullptr);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, 1, 1, 0, GL_RGB, GL_UNSIGNED_BYTE, nullptr);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    uint32_t expectedRenderPassCount = counters.renderPasses + 1;

    // First draw with textureRGBA which should start the renderpass
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glUniform1i(textureLoc, 0);
    drawQuad(program, essl1_shaders::PositionAttrib(), 0.5f);
    ASSERT_GL_NO_ERROR();

    // Next draw with textureRGB which should not end the renderpass
    glUniform1i(textureLoc, 1);
    drawQuad(program, essl1_shaders::PositionAttrib(), 0.5f);
    ASSERT_GL_NO_ERROR();

    uint32_t actualRenderPassCount = counters.renderPasses;
    EXPECT_EQ(expectedRenderPassCount, actualRenderPassCount);
}

// Tests that RGB texture should not break renderpass.
TEST_P(VulkanPerformanceCounterTest, renderToRGBTextureDoesNotBreakRenderPass)
{
    const rx::vk::PerfCounters &counters = hackANGLE();

    ANGLE_GL_PROGRAM(program, essl1_shaders::vs::Passthrough(), essl1_shaders::fs::UniformColor());
    glUseProgram(program);
    GLint colorUniformLocation =
        glGetUniformLocation(program, angle::essl1_shaders::ColorUniform());
    ASSERT_NE(-1, colorUniformLocation);
    ASSERT_GL_NO_ERROR();

    GLTexture textureRGB;
    glActiveTexture(GL_TEXTURE1);
    glBindTexture(GL_TEXTURE_2D, textureRGB);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, 256, 256, 0, GL_RGB, GL_UNSIGNED_BYTE, nullptr);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    GLFramebuffer framebuffer;
    glBindFramebuffer(GL_FRAMEBUFFER, framebuffer);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, textureRGB, 0);
    ASSERT_GL_FRAMEBUFFER_COMPLETE(GL_FRAMEBUFFER);
    ASSERT_GL_NO_ERROR();

    uint32_t expectedRenderPassCount = counters.renderPasses + 1;

    // Draw into FBO
    glBindFramebuffer(GL_FRAMEBUFFER, framebuffer);
    glClearColor(0.0f, 1.0f, 0.0f, 1.0f);  // clear to green
    glClear(GL_COLOR_BUFFER_BIT);
    glViewport(0, 0, 256, 256);
    glUniform4fv(colorUniformLocation, 1, GLColor::blue.toNormalizedVector().data());
    drawQuad(program, essl1_shaders::PositionAttrib(), 0.5f);

    uint32_t actualRenderPassCount = counters.renderPasses;
    EXPECT_EQ(expectedRenderPassCount, actualRenderPassCount);
}

// Tests that changing a Texture's max level hits the descriptor set cache.
TEST_P(VulkanPerformanceCounterTest, ChangingMaxLevelHitsDescriptorCache)
{
    const rx::vk::PerfCounters &counters = hackANGLE();

    GLColor kInitialData[4] = {GLColor::red, GLColor::blue, GLColor::green, GLColor::yellow};

    // Step 1: Set up a simple mipped 2D Texture rendering loop.
    GLTexture texture;
    glBindTexture(GL_TEXTURE_2D, texture);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, 2, 2, 0, GL_RGBA, GL_UNSIGNED_BYTE, kInitialData);
    glTexImage2D(GL_TEXTURE_2D, 1, GL_RGBA, 1, 1, 0, GL_RGBA, GL_UNSIGNED_BYTE, kInitialData);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST_MIPMAP_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAX_LEVEL, 1);

    auto quadVerts = GetQuadVertices();

    GLBuffer vertexBuffer;
    glBindBuffer(GL_ARRAY_BUFFER, vertexBuffer);
    glBufferData(GL_ARRAY_BUFFER, quadVerts.size() * sizeof(quadVerts[0]), quadVerts.data(),
                 GL_STATIC_DRAW);

    ANGLE_GL_PROGRAM(program, essl1_shaders::vs::Texture2D(), essl1_shaders::fs::Texture2D());
    glUseProgram(program);

    GLint posLoc = glGetAttribLocation(program, essl1_shaders::PositionAttrib());
    ASSERT_NE(-1, posLoc);

    glVertexAttribPointer(posLoc, 3, GL_FLOAT, GL_FALSE, 0, nullptr);
    glEnableVertexAttribArray(posLoc);
    ASSERT_GL_NO_ERROR();

    glDrawArrays(GL_TRIANGLES, 0, 6);
    ASSERT_GL_NO_ERROR();

    // Step 2: Change max level and draw.
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAX_LEVEL, 0);
    glDrawArrays(GL_TRIANGLES, 0, 6);
    ASSERT_GL_NO_ERROR();

    uint32_t expectedWriteDescriptorSetCount = counters.writeDescriptorSets;

    // Step 3: Change max level back to original value and verify we hit the cache.
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAX_LEVEL, 1);
    glDrawArrays(GL_TRIANGLES, 0, 6);
    ASSERT_GL_NO_ERROR();

    uint32_t actualWriteDescriptorSetCount = counters.writeDescriptorSets;
    EXPECT_EQ(expectedWriteDescriptorSetCount, actualWriteDescriptorSetCount);
}

// Tests that two glCopyBufferSubData commands can share a barrier.
TEST_P(VulkanPerformanceCounterTest, IndependentBufferCopiesShareSingleBarrier)
{
    constexpr GLint srcDataA[] = {1, 2, 3, 4};
    constexpr GLint srcDataB[] = {5, 6, 7, 8};

    // Step 1: Set up four buffers for two copies.
    GLBuffer srcA;
    glBindBuffer(GL_COPY_READ_BUFFER, srcA);
    glBufferData(GL_COPY_READ_BUFFER, sizeof(srcDataA), srcDataA, GL_STATIC_COPY);

    GLBuffer dstA;
    glBindBuffer(GL_COPY_WRITE_BUFFER, dstA);
    glBufferData(GL_COPY_WRITE_BUFFER, sizeof(srcDataA[0]) * 2, nullptr, GL_STATIC_COPY);

    GLBuffer srcB;
    glBindBuffer(GL_COPY_READ_BUFFER, srcB);
    glBufferData(GL_COPY_READ_BUFFER, sizeof(srcDataB), srcDataB, GL_STATIC_COPY);

    GLBuffer dstB;
    glBindBuffer(GL_COPY_WRITE_BUFFER, dstB);
    glBufferData(GL_COPY_WRITE_BUFFER, sizeof(srcDataB[0]) * 2, nullptr, GL_STATIC_COPY);

    // We expect that ANGLE generate zero additional command buffers.
    const rx::vk::PerfCounters &counters = hackANGLE();
    uint32_t expectedFlushCount          = counters.flushedOutsideRenderPassCommandBuffers;

    // Step 2: Do the two copies.
    glBindBuffer(GL_COPY_READ_BUFFER, srcA);
    glBindBuffer(GL_COPY_WRITE_BUFFER, dstA);
    glCopyBufferSubData(GL_COPY_READ_BUFFER, GL_COPY_WRITE_BUFFER, sizeof(srcDataB[0]), 0,
                        sizeof(srcDataA[0]) * 2);

    glBindBuffer(GL_COPY_READ_BUFFER, srcB);
    glBindBuffer(GL_COPY_WRITE_BUFFER, dstB);
    glCopyBufferSubData(GL_COPY_READ_BUFFER, GL_COPY_WRITE_BUFFER, sizeof(srcDataB[0]), 0,
                        sizeof(srcDataB[0]) * 2);

    ASSERT_GL_NO_ERROR();

    uint32_t actualFlushCount = counters.flushedOutsideRenderPassCommandBuffers;
    EXPECT_EQ(expectedFlushCount, actualFlushCount);
}

// Test resolving a multisampled texture with blit doesn't break the render pass so a subpass can be
// used
TEST_P(VulkanPerformanceCounterTest_ES31, MultisampleResolveWithBlit)
{
    constexpr int kSize = 16;
    glViewport(0, 0, kSize, kSize);

    GLFramebuffer msaaFBO;
    glBindFramebuffer(GL_FRAMEBUFFER, msaaFBO.get());

    GLTexture texture;
    glBindTexture(GL_TEXTURE_2D_MULTISAMPLE, texture.get());
    glTexStorage2DMultisample(GL_TEXTURE_2D_MULTISAMPLE, 4, GL_RGBA8, kSize, kSize, false);
    ASSERT_GL_NO_ERROR();
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D_MULTISAMPLE,
                           texture.get(), 0);
    ASSERT_GL_FRAMEBUFFER_COMPLETE(GL_FRAMEBUFFER);

    ANGLE_GL_PROGRAM(gradientProgram, essl31_shaders::vs::Passthrough(),
                     essl31_shaders::fs::RedGreenGradient());
    drawQuad(gradientProgram, essl31_shaders::PositionAttrib(), 0.5f, 1.0f, true);
    ASSERT_GL_NO_ERROR();

    // Create another FBO to resolve the multisample buffer into.
    GLTexture resolveTexture;
    GLFramebuffer resolveFBO;
    glBindTexture(GL_TEXTURE_2D, resolveTexture);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, kSize, kSize, 0, GL_RGBA, GL_UNSIGNED_BYTE, nullptr);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glBindFramebuffer(GL_FRAMEBUFFER, resolveFBO);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, resolveTexture, 0);
    EXPECT_GLENUM_EQ(GL_FRAMEBUFFER_COMPLETE, glCheckFramebufferStatus(GL_FRAMEBUFFER));

    glBindFramebuffer(GL_READ_FRAMEBUFFER, msaaFBO);
    glBindFramebuffer(GL_DRAW_FRAMEBUFFER, resolveFBO);
    glBlitFramebuffer(0, 0, kSize, kSize, 0, 0, kSize, kSize, GL_COLOR_BUFFER_BIT, GL_NEAREST);
    ASSERT_GL_NO_ERROR();

    const rx::vk::PerfCounters &counters = hackANGLE();
    EXPECT_EQ(counters.resolveImageCommands, 0u);

    glBindFramebuffer(GL_READ_FRAMEBUFFER, resolveFBO);
    EXPECT_PIXEL_NEAR(0, 0, 0, 0, 0, 255, 1.0);                      // Black
    EXPECT_PIXEL_NEAR(kSize - 1, 1, 239, 0, 0, 255, 1.0);            // Red
    EXPECT_PIXEL_NEAR(0, kSize - 1, 0, 239, 0, 255, 1.0);            // Green
    EXPECT_PIXEL_NEAR(kSize - 1, kSize - 1, 239, 239, 0, 255, 1.0);  // Yellow
}

// Ensures a read-only depth-stencil feedback loop works in a single RenderPass.
TEST_P(VulkanPerformanceCounterTest, ReadOnlyDepthStencilFeedbackLoopUsesSingleRenderPass)
{
    const rx::vk::PerfCounters &counters = hackANGLE();

    constexpr GLsizei kSize = 4;

    ANGLE_GL_PROGRAM(redProgram, essl1_shaders::vs::Simple(), essl1_shaders::fs::Red());
    ANGLE_GL_PROGRAM(texProgram, essl1_shaders::vs::Texture2D(), essl1_shaders::fs::Texture2D());

    GLTexture colorTexture;
    glBindTexture(GL_TEXTURE_2D, colorTexture);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, kSize, kSize, 0, GL_RGBA, GL_UNSIGNED_BYTE, nullptr);

    setupQuadVertexBuffer(0.5f, 1.0f);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, 0);
    glEnableVertexAttribArray(0);

    // Set up a depth texture and fill it with an arbitrary initial value.
    GLTexture depthTexture;
    glBindTexture(GL_TEXTURE_2D, depthTexture);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT16, kSize, kSize, 0, GL_DEPTH_COMPONENT,
                 GL_UNSIGNED_INT, nullptr);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glBindTexture(GL_TEXTURE_2D, 0);

    GLFramebuffer depthAndColorFBO;
    glBindFramebuffer(GL_FRAMEBUFFER, depthAndColorFBO);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, colorTexture, 0);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, depthTexture, 0);
    ASSERT_GL_FRAMEBUFFER_COMPLETE(GL_FRAMEBUFFER);

    GLFramebuffer depthOnlyFBO;
    glBindFramebuffer(GL_FRAMEBUFFER, depthOnlyFBO);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, depthTexture, 0);
    ASSERT_GL_FRAMEBUFFER_COMPLETE(GL_FRAMEBUFFER);

    // Draw to a first FBO to initialize the depth buffer.
    glBindFramebuffer(GL_FRAMEBUFFER, depthOnlyFBO);
    glUseProgram(redProgram);
    glDrawArrays(GL_TRIANGLES, 0, 6);
    ASSERT_GL_NO_ERROR();

    uint32_t expectedRenderPassCount = counters.renderPasses + 1;

    // Start new RenderPass with depth write disabled and no loop.
    glBindFramebuffer(GL_FRAMEBUFFER, depthAndColorFBO);
    glEnable(GL_DEPTH_TEST);
    glDepthMask(false);
    glDrawArrays(GL_TRIANGLES, 0, 6);
    ASSERT_GL_NO_ERROR();

    // Now set up the read-only feedback loop.
    glBindTexture(GL_TEXTURE_2D, depthTexture);
    glUseProgram(texProgram);
    glDrawArrays(GL_TRIANGLES, 0, 6);
    ASSERT_GL_NO_ERROR();

    // Tweak the bits to keep it read-only.
    glEnable(GL_DEPTH_TEST);
    glDrawArrays(GL_TRIANGLES, 0, 6);
    ASSERT_GL_NO_ERROR();

    // Render with just the depth attachment.
    glUseProgram(redProgram);
    glBindTexture(GL_TEXTURE_2D, 0);
    glDrawArrays(GL_TRIANGLES, 0, 6);
    ASSERT_GL_NO_ERROR();

    // Rebind the depth texture.
    glUseProgram(texProgram);
    glDepthMask(GL_FALSE);
    glEnable(GL_DEPTH_TEST);
    glBindTexture(GL_TEXTURE_2D, depthTexture);
    glDrawArrays(GL_TRIANGLES, 0, 6);
    ASSERT_GL_NO_ERROR();

    uint32_t actualRenderPassCount = counters.renderPasses;
    EXPECT_EQ(expectedRenderPassCount, actualRenderPassCount);

    // Do a final write to depth to make sure we can switch out of read-only mode.
    glBindTexture(GL_TEXTURE_2D, 0);
    glDepthMask(GL_TRUE);
    glDrawArrays(GL_TRIANGLES, 0, 6);
    ASSERT_GL_NO_ERROR();
}

// The following macros allow most of the "invalidate" tests to share a lot of code, including
// local-variable declarations and uses
#define DECLARE_INVALIDATE_EXPECTED_INCS(rpCount, dClear, dLoad, dStore, sClear, sLoad, sStore) \
    uint32_t expectedRenderPassCount = counters.renderPasses + rpCount;                         \
    uint32_t expectedDepthClears     = counters.depthClears + dClear;                           \
    uint32_t expectedDepthLoads      = counters.depthLoads + dLoad;                             \
    uint32_t expectedDepthStores     = counters.depthStores + dStore;                           \
    uint32_t expectedStencilClears   = counters.stencilClears + sClear;                         \
    uint32_t expectedStencilLoads    = counters.stencilLoads + sLoad;                           \
    uint32_t expectedStencilStores   = counters.stencilStores + sStore;

#define DECLARE_INVALIDATE_ACTUAL() \
    uint32_t actualDepthClears;     \
    uint32_t actualDepthLoads;      \
    uint32_t actualDepthStores;     \
    uint32_t actualStencilClears;   \
    uint32_t actualStencilLoads;    \
    uint32_t actualStencilStores;

#define SWAP_UPDATE_AND_CHECK_INVALIDATE_ACTUAL()          \
    swapBuffers();                                         \
    actualDepthClears   = counters.depthClears;            \
    actualDepthLoads    = counters.depthLoads;             \
    actualDepthStores   = counters.depthStores;            \
    actualStencilClears = counters.stencilClears;          \
    actualStencilLoads  = counters.stencilLoads;           \
    actualStencilStores = counters.stencilStores;          \
    EXPECT_EQ(expectedDepthClears, actualDepthClears);     \
    EXPECT_EQ(expectedDepthLoads, actualDepthLoads);       \
    EXPECT_EQ(expectedDepthStores, actualDepthStores);     \
    EXPECT_EQ(expectedStencilClears, actualStencilClears); \
    EXPECT_EQ(expectedStencilLoads, actualStencilLoads);   \
    EXPECT_EQ(expectedStencilStores, actualStencilStores);

#define TEST_FUTURE_RP_LOADOP(dLoad, sLoad)               \
    expectedDepthLoads   = counters.depthLoads + dLoad;   \
    expectedStencilLoads = counters.stencilLoads + sLoad; \
    drawForInvalidateTest(&program);                      \
    swapBuffers();                                        \
    actualDepthLoads   = counters.depthLoads;             \
    actualStencilLoads = counters.stencilLoads;           \
    EXPECT_EQ(expectedDepthLoads, actualDepthLoads);      \
    EXPECT_EQ(expectedStencilLoads, actualStencilLoads);

// Tests that common PUBG MOBILE case does not break render pass, and that counts are correct:
//
// - Scenario: invalidate, disable, draw
TEST_P(VulkanPerformanceCounterTest, InvalidateDisableDraw)
{
    const rx::vk::PerfCounters &counters = hackANGLE();

    // Expect rpCount+1, depth(Clears+1, Loads+0, Stores+0), stencil(Clears+0, Load+1, Stores+0)
    DECLARE_INVALIDATE_EXPECTED_INCS(1, 1, 0, 0, 0, 1, 0);
    DECLARE_INVALIDATE_ACTUAL();

    ANGLE_GL_PROGRAM(program, essl1_shaders::vs::Simple(), essl1_shaders::fs::Red());
    GLFramebuffer framebuffer;
    GLTexture texture;
    GLRenderbuffer renderbuffer;
    setupClearAndDrawForInvalidateTest(&program, &framebuffer, &texture, &renderbuffer);

    // Execute the scenario that this test is for:
    //
    // Invalidate (storeOp = DONT_CARE; mContentDefined = false)
    invalidateForInvalidateTest();
    // Disable (shouldn't change result)
    disableForInvalidateTest();
    // Draw (since disabled, shouldn't change result)
    drawForInvalidateTest(&program);

    // Ensure that the render pass wasn't broken
    uint32_t actualRenderPassCount = counters.renderPasses;
    EXPECT_EQ(expectedRenderPassCount, actualRenderPassCount);

    // Use swapBuffers and then check how many loads and stores were actually done
    SWAP_UPDATE_AND_CHECK_INVALIDATE_ACTUAL();

    // Start and end another render pass, to check that the load ops are as expected
    TEST_FUTURE_RP_LOADOP(0, 0);
}

// Tests that alternative PUBG MOBILE case does not break render pass, and that counts are correct:
//
// - Scenario: disable, invalidate, draw
TEST_P(VulkanPerformanceCounterTest, DisableInvalidateDraw)
{
    const rx::vk::PerfCounters &counters = hackANGLE();

    // Expect rpCount+1, depth(Clears+1, Loads+0, Stores+0), stencil(Clears+0, Load+1, Stores+0)
    DECLARE_INVALIDATE_EXPECTED_INCS(1, 1, 0, 0, 0, 1, 0);
    DECLARE_INVALIDATE_ACTUAL();

    ANGLE_GL_PROGRAM(program, essl1_shaders::vs::Simple(), essl1_shaders::fs::Red());
    GLFramebuffer framebuffer;
    GLTexture texture;
    GLRenderbuffer renderbuffer;
    setupClearAndDrawForInvalidateTest(&program, &framebuffer, &texture, &renderbuffer);

    // Execute the scenario that this test is for:
    //
    // Disable (shouldn't change result)
    disableForInvalidateTest();
    // Invalidate (storeOp = DONT_CARE; mContentDefined = false)
    invalidateForInvalidateTest();
    // Draw (since disabled, shouldn't change result)
    drawForInvalidateTest(&program);

    // Ensure that the render pass wasn't broken
    uint32_t actualRenderPassCount = counters.renderPasses;
    EXPECT_EQ(expectedRenderPassCount, actualRenderPassCount);

    // Use swapBuffers and then check how many loads and stores were actually done
    SWAP_UPDATE_AND_CHECK_INVALIDATE_ACTUAL();

    // Start and end another render pass, to check that the load ops are as expected
    TEST_FUTURE_RP_LOADOP(0, 0);
}

// Tests that common TRex case does not break render pass, and that counts are correct:
//
// - Scenario: invalidate
TEST_P(VulkanPerformanceCounterTest, Invalidate)
{
    const rx::vk::PerfCounters &counters = hackANGLE();

    // Expect rpCount+1, depth(Clears+1, Loads+0, Stores+0), stencil(Clears+0, Load+1, Stores+0)
    DECLARE_INVALIDATE_EXPECTED_INCS(1, 1, 0, 0, 0, 1, 0);
    DECLARE_INVALIDATE_ACTUAL();

    ANGLE_GL_PROGRAM(program, essl1_shaders::vs::Simple(), essl1_shaders::fs::Red());
    GLFramebuffer framebuffer;
    GLTexture texture;
    GLRenderbuffer renderbuffer;
    setupClearAndDrawForInvalidateTest(&program, &framebuffer, &texture, &renderbuffer);

    // Execute the scenario that this test is for:
    //
    // Invalidate (storeOp = DONT_CARE; mContentDefined = false)
    invalidateForInvalidateTest();

    // Ensure that the render pass wasn't broken
    uint32_t actualRenderPassCount = counters.renderPasses;
    EXPECT_EQ(expectedRenderPassCount, actualRenderPassCount);

    // Use swapBuffers and then check how many loads and stores were actually done
    SWAP_UPDATE_AND_CHECK_INVALIDATE_ACTUAL();

    // Start and end another render pass, to check that the load ops are as expected
    TEST_FUTURE_RP_LOADOP(0, 0);
}

// Tests that another case does not break render pass, and that counts are correct:
//
// - Scenario: invalidate, draw
TEST_P(VulkanPerformanceCounterTest, InvalidateDraw)
{
    const rx::vk::PerfCounters &counters = hackANGLE();

    // Expect rpCount+1, depth(Clears+1, Loads+0, Stores+1), stencil(Clears+0, Load+1, Stores+1)
    DECLARE_INVALIDATE_EXPECTED_INCS(1, 1, 0, 1, 0, 1, 1);
    DECLARE_INVALIDATE_ACTUAL();

    ANGLE_GL_PROGRAM(program, essl1_shaders::vs::Simple(), essl1_shaders::fs::Red());
    GLFramebuffer framebuffer;
    GLTexture texture;
    GLRenderbuffer renderbuffer;
    setupClearAndDrawForInvalidateTest(&program, &framebuffer, &texture, &renderbuffer);

    // Execute the scenario that this test is for:
    //
    // Invalidate (should result: in storeOp = DONT_CARE; mContentDefined = false)
    invalidateForInvalidateTest();
    // Draw (since enabled, should result: in storeOp = STORE; mContentDefined = true)
    drawForInvalidateTest(&program);

    // Ensure that the render pass wasn't broken
    uint32_t actualRenderPassCount = counters.renderPasses;
    EXPECT_EQ(expectedRenderPassCount, actualRenderPassCount);

    // Use swapBuffers and then check how many loads and stores were actually done
    SWAP_UPDATE_AND_CHECK_INVALIDATE_ACTUAL();

    // Start and end another render pass, to check that the load ops are as expected
    TEST_FUTURE_RP_LOADOP(1, 1);
}

// Tests that another case does not break render pass, and that counts are correct:
//
// - Scenario: invalidate, draw, disable
TEST_P(VulkanPerformanceCounterTest, InvalidateDrawDisable)
{
    const rx::vk::PerfCounters &counters = hackANGLE();

    // Expect rpCount+1, depth(Clears+1, Loads+0, Stores+1), stencil(Clears+0, Load+1, Stores+1)
    DECLARE_INVALIDATE_EXPECTED_INCS(1, 1, 0, 1, 0, 1, 1);
    DECLARE_INVALIDATE_ACTUAL();

    ANGLE_GL_PROGRAM(program, essl1_shaders::vs::Simple(), essl1_shaders::fs::Red());
    GLFramebuffer framebuffer;
    GLTexture texture;
    GLRenderbuffer renderbuffer;
    setupClearAndDrawForInvalidateTest(&program, &framebuffer, &texture, &renderbuffer);

    // Execute the scenario that this test is for:
    //
    // Invalidate (should result: in storeOp = DONT_CARE; mContentDefined = false)
    invalidateForInvalidateTest();
    // Draw (since enabled, should result: in storeOp = STORE; mContentDefined = true)
    drawForInvalidateTest(&program);
    // Disable (shouldn't change result)
    disableForInvalidateTest();
    // Note: this draw is just so that the disable dirty bits will be processed
    drawForInvalidateTest(&program);

    // Ensure that the render pass wasn't broken
    uint32_t actualRenderPassCount = counters.renderPasses;
    EXPECT_EQ(expectedRenderPassCount, actualRenderPassCount);

    // Use swapBuffers and then check how many loads and stores were actually done
    SWAP_UPDATE_AND_CHECK_INVALIDATE_ACTUAL();

    // Start and end another render pass, to check that the load ops are as expected
    TEST_FUTURE_RP_LOADOP(1, 1);
}

// Tests that another case does not break render pass, and that counts are correct:
//
// - Scenario: invalidate, disable, draw, enable
TEST_P(VulkanPerformanceCounterTest, InvalidateDisableDrawEnable)
{
    const rx::vk::PerfCounters &counters = hackANGLE();

    // Expect rpCount+1, depth(Clears+1, Loads+0, Stores+0), stencil(Clears+0, Load+1, Stores+0)
    DECLARE_INVALIDATE_EXPECTED_INCS(1, 1, 0, 0, 0, 1, 0);
    DECLARE_INVALIDATE_ACTUAL();

    ANGLE_GL_PROGRAM(program, essl1_shaders::vs::Simple(), essl1_shaders::fs::Red());
    GLFramebuffer framebuffer;
    GLTexture texture;
    GLRenderbuffer renderbuffer;
    setupClearAndDrawForInvalidateTest(&program, &framebuffer, &texture, &renderbuffer);

    // Execute the scenario that this test is for:
    //
    // Invalidate (should result: in storeOp = DONT_CARE; mContentDefined = false)
    invalidateForInvalidateTest();
    // Disable (shouldn't change result)
    disableForInvalidateTest();
    // Draw (since disabled, shouldn't change result)
    drawForInvalidateTest(&program);
    // Enable (shouldn't change result)
    enableForInvalidateTest();
    // TODO(ianelliott): Need a non-drawing way to force the enable dirty bit to be processed

    // Ensure that the render pass wasn't broken
    uint32_t actualRenderPassCount = counters.renderPasses;
    EXPECT_EQ(expectedRenderPassCount, actualRenderPassCount);

    // Use swapBuffers and then check how many loads and stores were actually done
    SWAP_UPDATE_AND_CHECK_INVALIDATE_ACTUAL();

    // Start and end another render pass, to check that the load ops are as expected
    TEST_FUTURE_RP_LOADOP(0, 0);
}

// Tests that another case does not break render pass, and that counts are correct:
//
// - Scenario: invalidate, disable, draw, enable, draw
TEST_P(VulkanPerformanceCounterTest, InvalidateDisableDrawEnableDraw)
{
    const rx::vk::PerfCounters &counters = hackANGLE();

    // Expect rpCount+1, depth(Clears+1, Loads+0, Stores+1), stencil(Clears+0, Load+1, Stores+1)
    DECLARE_INVALIDATE_EXPECTED_INCS(1, 1, 0, 1, 0, 1, 1);
    DECLARE_INVALIDATE_ACTUAL();

    ANGLE_GL_PROGRAM(program, essl1_shaders::vs::Simple(), essl1_shaders::fs::Red());
    GLFramebuffer framebuffer;
    GLTexture texture;
    GLRenderbuffer renderbuffer;
    setupClearAndDrawForInvalidateTest(&program, &framebuffer, &texture, &renderbuffer);

    // Execute the scenario that this test is for:
    //
    // Invalidate (should result: in storeOp = DONT_CARE; mContentDefined = false)
    invalidateForInvalidateTest();
    // Disable (shouldn't change result)
    disableForInvalidateTest();
    // Draw (since disabled, shouldn't change result)
    drawForInvalidateTest(&program);
    // Enable (shouldn't change result)
    enableForInvalidateTest();
    // Draw (since enabled, should result: in storeOp = STORE; mContentDefined = true)
    drawForInvalidateTest(&program);

    // Ensure that the render pass wasn't broken
    uint32_t actualRenderPassCount = counters.renderPasses;
    EXPECT_EQ(expectedRenderPassCount, actualRenderPassCount);

    // Use swapBuffers and then check how many loads and stores were actually done
    SWAP_UPDATE_AND_CHECK_INVALIDATE_ACTUAL();

    // Start and end another render pass, to check that the load ops are as expected
    TEST_FUTURE_RP_LOADOP(1, 1);
}

// Tests that another case does not break render pass, and that counts are correct:
//
// - Scenario: invalidate, draw, disable, enable
TEST_P(VulkanPerformanceCounterTest, InvalidateDrawDisableEnable)
{
    const rx::vk::PerfCounters &counters = hackANGLE();

    // Expect rpCount+1, depth(Clears+1, Loads+0, Stores+1), stencil(Clears+0, Load+1, Stores+1)
    DECLARE_INVALIDATE_EXPECTED_INCS(1, 1, 0, 1, 0, 1, 1);
    DECLARE_INVALIDATE_ACTUAL();

    ANGLE_GL_PROGRAM(program, essl1_shaders::vs::Simple(), essl1_shaders::fs::Red());
    GLFramebuffer framebuffer;
    GLTexture texture;
    GLRenderbuffer renderbuffer;
    setupClearAndDrawForInvalidateTest(&program, &framebuffer, &texture, &renderbuffer);

    // Execute the scenario that this test is for:
    //
    // Invalidate (should result: in storeOp = DONT_CARE; mContentDefined = false)
    invalidateForInvalidateTest();
    // Draw (since enabled, should result: in storeOp = STORE; mContentDefined = true)
    drawForInvalidateTest(&program);
    // Disable (shouldn't change result)
    disableForInvalidateTest();
    // Note: this draw is just so that the disable dirty bits will be processed
    drawForInvalidateTest(&program);
    // Enable (shouldn't change result)
    enableForInvalidateTest();
    // TODO(ianelliott): Need a non-drawing way to force the enable dirty bit to be processed

    // Ensure that the render pass wasn't broken
    uint32_t actualRenderPassCount = counters.renderPasses;
    EXPECT_EQ(expectedRenderPassCount, actualRenderPassCount);

    // Use swapBuffers and then check how many loads and stores were actually done
    SWAP_UPDATE_AND_CHECK_INVALIDATE_ACTUAL();

    // Start and end another render pass, to check that the load ops are as expected
    TEST_FUTURE_RP_LOADOP(1, 1);
}

// Tests that another case does not break render pass, and that counts are correct:
//
// - Scenario: invalidate, draw, disable, enable, invalidate
TEST_P(VulkanPerformanceCounterTest, InvalidateDrawDisableEnableInvalidate)
{
    const rx::vk::PerfCounters &counters = hackANGLE();

    // Expect rpCount+1, depth(Clears+1, Loads+0, Stores+0), stencil(Clears+0, Load+1, Stores+0)
    DECLARE_INVALIDATE_EXPECTED_INCS(1, 1, 0, 0, 0, 1, 0);
    DECLARE_INVALIDATE_ACTUAL();

    ANGLE_GL_PROGRAM(program, essl1_shaders::vs::Simple(), essl1_shaders::fs::Red());
    GLFramebuffer framebuffer;
    GLTexture texture;
    GLRenderbuffer renderbuffer;
    setupClearAndDrawForInvalidateTest(&program, &framebuffer, &texture, &renderbuffer);

    // Execute the scenario that this test is for:
    //
    // Invalidate (should result: in storeOp = DONT_CARE; mContentDefined = false)
    invalidateForInvalidateTest();
    // Draw (since enabled, should result: in storeOp = STORE; mContentDefined = true)
    drawForInvalidateTest(&program);
    // Disable (shouldn't change result)
    disableForInvalidateTest();
    // Note: this draw is just so that the disable dirty bits will be processed
    drawForInvalidateTest(&program);
    // Enable (shouldn't change result)
    enableForInvalidateTest();
    // Invalidate (should result: in storeOp = DONT_CARE; mContentDefined = false)
    invalidateForInvalidateTest();

    // Ensure that the render pass wasn't broken
    uint32_t actualRenderPassCount = counters.renderPasses;
    EXPECT_EQ(expectedRenderPassCount, actualRenderPassCount);

    // Use swapBuffers and then check how many loads and stores were actually done
    SWAP_UPDATE_AND_CHECK_INVALIDATE_ACTUAL();

    // Start and end another render pass, to check that the load ops are as expected
    TEST_FUTURE_RP_LOADOP(0, 0);
}

// Tests that another case does not break render pass, and that counts are correct:
//
// - Scenario: invalidate, draw, disable, enable, invalidate, draw
TEST_P(VulkanPerformanceCounterTest, InvalidateDrawDisableEnableInvalidateDraw)
{
    const rx::vk::PerfCounters &counters = hackANGLE();

    // Expect rpCount+1, depth(Clears+1, Loads+0, Stores+1), stencil(Clears+0, Load+1, Stores+1)
    DECLARE_INVALIDATE_EXPECTED_INCS(1, 1, 0, 1, 0, 1, 1);
    DECLARE_INVALIDATE_ACTUAL();

    ANGLE_GL_PROGRAM(program, essl1_shaders::vs::Simple(), essl1_shaders::fs::Red());
    GLFramebuffer framebuffer;
    GLTexture texture;
    GLRenderbuffer renderbuffer;
    setupClearAndDrawForInvalidateTest(&program, &framebuffer, &texture, &renderbuffer);

    // Execute the scenario that this test is for:
    //
    // Invalidate (should result: in storeOp = DONT_CARE; mContentDefined = false)
    invalidateForInvalidateTest();
    // Draw (since enabled, should result: in storeOp = STORE; mContentDefined = true)
    drawForInvalidateTest(&program);
    // Disable (shouldn't change result)
    disableForInvalidateTest();
    // Note: this draw is just so that the disable dirty bits will be processed
    drawForInvalidateTest(&program);
    // Enable (shouldn't change result)
    enableForInvalidateTest();
    // Invalidate (should result: in storeOp = DONT_CARE; mContentDefined = false)
    invalidateForInvalidateTest();
    // Draw (since enabled, should result: in storeOp = STORE; mContentDefined = true)
    drawForInvalidateTest(&program);

    // Ensure that the render pass wasn't broken
    uint32_t actualRenderPassCount = counters.renderPasses;
    EXPECT_EQ(expectedRenderPassCount, actualRenderPassCount);

    // Use swapBuffers and then check how many loads and stores were actually done
    SWAP_UPDATE_AND_CHECK_INVALIDATE_ACTUAL();

    // Start and end another render pass, to check that the load ops are as expected
    TEST_FUTURE_RP_LOADOP(1, 1);
}

// Tests that another common (dEQP) case does not break render pass, and that counts are correct:
//
// - Scenario: invalidate, disable, enable, draw
TEST_P(VulkanPerformanceCounterTest, InvalidateDisableEnableDraw)
{
    const rx::vk::PerfCounters &counters = hackANGLE();

    // Expect rpCount+1, depth(Clears+1, Loads+0, Stores+1), stencil(Clears+0, Load+1, Stores+1)
    DECLARE_INVALIDATE_EXPECTED_INCS(1, 1, 0, 1, 0, 1, 1);
    DECLARE_INVALIDATE_ACTUAL();

    ANGLE_GL_PROGRAM(program, essl1_shaders::vs::Simple(), essl1_shaders::fs::Red());
    GLFramebuffer framebuffer;
    GLTexture texture;
    GLRenderbuffer renderbuffer;
    setupClearAndDrawForInvalidateTest(&program, &framebuffer, &texture, &renderbuffer);

    // Execute the scenario that this test is for:
    //
    // Invalidate (should result: in storeOp = DONT_CARE; mContentDefined = false)
    invalidateForInvalidateTest();
    // Disable (shouldn't change result)
    disableForInvalidateTest();
    // Note: this draw is just so that the disable dirty bits will be processed
    drawForInvalidateTest(&program);
    // Enable (shouldn't change result)
    enableForInvalidateTest();
    // Draw (since enabled, should result: in storeOp = STORE; mContentDefined = true)
    drawForInvalidateTest(&program);

    // Ensure that the render pass wasn't broken
    uint32_t actualRenderPassCount = counters.renderPasses;
    EXPECT_EQ(expectedRenderPassCount, actualRenderPassCount);

    // Use swapBuffers and then check how many loads and stores were actually done
    SWAP_UPDATE_AND_CHECK_INVALIDATE_ACTUAL();

    // Start and end another render pass, to check that the load ops are as expected
    TEST_FUTURE_RP_LOADOP(1, 1);
}

// Tests that even if the app clears depth, it should be invalidated if there is no read.
TEST_P(VulkanPerformanceCounterTest, SwapShouldInvalidateDepthAfterClear)
{
    const rx::vk::PerfCounters &counters = hackANGLE();

    ANGLE_GL_PROGRAM(redProgram, essl1_shaders::vs::Simple(), essl1_shaders::fs::Red());

    // Clear depth.
    glClear(GL_DEPTH_BUFFER_BIT);

    // Ensure we never read from depth.
    glDisable(GL_DEPTH_TEST);

    // Do one draw, then swap.
    drawQuad(redProgram, essl1_shaders::PositionAttrib(), 0.5f);
    ASSERT_GL_NO_ERROR();

    uint32_t expectedDepthClears = counters.depthClears;

    swapBuffers();

    uint32_t actualDepthClears = counters.depthClears;
    EXPECT_EQ(expectedDepthClears, actualDepthClears);
}

ANGLE_INSTANTIATE_TEST(VulkanPerformanceCounterTest, ES3_VULKAN());
ANGLE_INSTANTIATE_TEST(VulkanPerformanceCounterTest_ES31, ES31_VULKAN());

}  // anonymous namespace
