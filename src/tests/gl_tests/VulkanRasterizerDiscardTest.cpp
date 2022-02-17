//
// Copyright 2021 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//

// VulkanRasterizerDiscardTest.cpp: Validate the optimization for using a dummy image when
// GL_RASTERIZER_DISCARD is enabled and the default framebuffer is bound.

#include "test_utils/ANGLETest.h"
#include "test_utils/gl_raii.h"

#include "libANGLE/Surface.h"
#include "libANGLE/renderer/vulkan/SurfaceVk.h"

namespace angle
{

class VulkanRasterizerDiscardTest : public ANGLETest
{
  protected:
    static const uint32_t kDefaultFramebuffer = 0;

    static const uint32_t kRboWidth   = 2;
    static const uint32_t kRboHeight  = 2;
    static const uint32_t kRboCenterX = kRboWidth / 2;
    static const uint32_t kRboCenterY = kRboHeight / 2;

    VulkanRasterizerDiscardTest()
    {
        setWindowWidth(128);
        setWindowHeight(128);
        setConfigRedBits(8);
        setConfigGreenBits(8);
        setConfigBlueBits(8);
        setConfigAlphaBits(8);
        setConfigDepthBits(24);
    }

    void testSetUp() override
    {
        mProgram.makeRaster(essl1_shaders::vs::Simple(), essl1_shaders::fs::UniformColor());
        ASSERT_EQ(true, mProgram.valid()) << "shader compilation failed.";

        mColorUniform = glGetUniformLocation(mProgram, essl1_shaders::ColorUniform());

        mCenterX = getWindowWidth() / 2;
        mCenterY = getWindowHeight() / 2;

        // Set the program-related environment
        glUseProgram(mProgram);
        EXPECT_GL_NO_ERROR();

        // Make mNeedToAcquireNextSwapchainImage "true"
        swapBuffers();
        ASSERT_EQ(true, hackWindowSurface()->getDeferredStatusForSwapchainImage());
    }

    void drawFramebufferWithColor(GLColor32F color)
    {
        glUniform4f(mColorUniform, color.R, color.G, color.B, color.A);
        drawQuad(mProgram, essl1_shaders::PositionAttrib(), 0.5f);
        EXPECT_GL_NO_ERROR();
    }

    void clearFramebufferWithColor(GLColor32F color)
    {
        glClearColor(color.R, color.G, color.B, color.A);
        glClear(GL_COLOR_BUFFER_BIT);
        EXPECT_GL_NO_ERROR();
    }

    void initializeRenderToRenderbuffer()
    {
        glBindRenderbuffer(GL_RENDERBUFFER, mRbo);
        glRenderbufferStorage(GL_RENDERBUFFER, GL_RGBA8, kRboWidth, kRboHeight);

        glBindFramebuffer(GL_FRAMEBUFFER, mFbo);
        glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_RENDERBUFFER, mFbo);
        clearFramebufferWithColor(kFloatBlack);
        glBindFramebuffer(GL_FRAMEBUFFER, kDefaultFramebuffer);
    }

    rx::WindowSurfaceVk *hackWindowSurface() const
    {
        // Hack the angle!
        const egl::Surface *surface = static_cast<egl::Surface *>(getEGLWindow()->getSurface());
        return rx::GetImplAs<rx::WindowSurfaceVk>(surface);
    }

    GLProgram mProgram;
    GLint mColorUniform = -1;

    GLint mCenterX = -1;
    GLint mCenterY = -1;

    GLRenderbuffer mRbo;
    GLFramebuffer mFbo;
};

// Check that the difference of the FramebufferVk status between GL_RASTERIZER_DISCARD enabled or
// not.
TEST_P(VulkanRasterizerDiscardTest, SimpleTest)
{
    // Make mNeedToAcquireNextSwapchainImage "true"
    swapBuffers();
    ASSERT_EQ(true, hackWindowSurface()->getDeferredStatusForSwapchainImage());

    glEnable(GL_RASTERIZER_DISCARD);
    EXPECT_GL_NO_ERROR();

    // Clear the default framebuffer
    clearFramebufferWithColor(kFloatBlue);
    EXPECT_EQ(true, hackWindowSurface()->getDeferredStatusForSwapchainImage());

    // Because of GL_RASTERIZER_DISCARD, the dummy VkFramebuffer is used. So, if the result of
    // glReadPixel is a garbage value.
    drawFramebufferWithColor(kFloatRed);
    EXPECT_EQ(true, hackWindowSurface()->getDeferredStatusForSwapchainImage());

    // Because GL_RASTERIZER_DISCARD is enabled, mNeedToAcquireNextSwapchainImage isn't changed.
    swapBuffers();
    EXPECT_EQ(true, hackWindowSurface()->getDeferredStatusForSwapchainImage());

    glDisable(GL_RASTERIZER_DISCARD);
    EXPECT_GL_NO_ERROR();

    // Because GL_RASTERIZER_DISCARD is disabled, when calling a draw function, the color attachment
    // is updated and mNeedToAcquireNextSwapchainImage is changed.
    drawFramebufferWithColor(kFloatGreen);
    ASSERT_EQ(false, hackWindowSurface()->getDeferredStatusForSwapchainImage());
    EXPECT_PIXEL_EQ(mCenterX, mCenterY, GLColor::green.R, GLColor::green.G, GLColor::green.B,
                    GLColor::green.A);
}

// Check that when using a framebuffer object if GL_RASTERIZER_DISCARD is enabled or not, the
// routine for GL_RASTERIZER_DISCARD is correctly operated.
TEST_P(VulkanRasterizerDiscardTest, UsingFBOBetweenDraw)
{
    initializeRenderToRenderbuffer();

    glEnable(GL_RASTERIZER_DISCARD);
    EXPECT_GL_NO_ERROR();

    // Clear the default framebuffer
    clearFramebufferWithColor(kFloatBlue);
    EXPECT_EQ(true, hackWindowSurface()->getDeferredStatusForSwapchainImage());

    // Because of GL_RASTERIZER_DISCARD, the dummy VkFramebuffer is used. So, if the result of
    // glReadPixel is a garbage value.
    drawFramebufferWithColor(kFloatRed);
    EXPECT_EQ(true, hackWindowSurface()->getDeferredStatusForSwapchainImage());

    // Use FBO instead of the default framebuffer. But GL_RASTERIZER_DISCARD is still enabled. And
    // calls related to FBO doesn't affect the status of mNeedToAcquireNextSwapchainImage
    glBindFramebuffer(GL_FRAMEBUFFER, mFbo);
    clearFramebufferWithColor(kFloatGreen);
    EXPECT_EQ(true, hackWindowSurface()->getDeferredStatusForSwapchainImage());

    // Because GL_RASTERIZER_DISCARD is still enabled, the dummy VkFramebuffer is used. So, if the
    // result of glReadPixel is a garbage value.
    drawFramebufferWithColor(kFloatRed);
    EXPECT_EQ(true, hackWindowSurface()->getDeferredStatusForSwapchainImage());
    glBindFramebuffer(GL_FRAMEBUFFER, kDefaultFramebuffer);

    // Because GL_RASTERIZER_DISCARD is enabled, mNeedToAcquireNextSwapchainImage isn't changed.
    swapBuffers();
    EXPECT_EQ(true, hackWindowSurface()->getDeferredStatusForSwapchainImage());

    glDisable(GL_RASTERIZER_DISCARD);
    EXPECT_GL_NO_ERROR();

    // Use FBO instead of the default framebuffer. But GL_RASTERIZER_DISCARD is still enabled. And
    // calls related to FBO doesn't affect the status of mNeedToAcquireNextSwapchainImage
    glBindFramebuffer(GL_FRAMEBUFFER, mFbo);
    clearFramebufferWithColor(kFloatRed);
    EXPECT_EQ(true, hackWindowSurface()->getDeferredStatusForSwapchainImage());
    EXPECT_PIXEL_EQ(kRboCenterX, kRboCenterY, GLColor::red.R, GLColor::red.G, GLColor::red.B,
                    GLColor::red.A);
    glBindFramebuffer(GL_FRAMEBUFFER, kDefaultFramebuffer);

    // Because GL_RASTERIZER_DISCARD is disabled, when calling a draw function, the color attachment
    // is updated and mNeedToAcquireNextSwapchainImage is changed.
    drawFramebufferWithColor(kFloatGreen);
    ASSERT_EQ(false, hackWindowSurface()->getDeferredStatusForSwapchainImage());
    EXPECT_PIXEL_EQ(mCenterX, mCenterY, GLColor::green.R, GLColor::green.G, GLColor::green.B,
                    GLColor::green.A);
}

// Check that if binding a framebuffer object, disabling GL_RASTERIZER_DISCARD and then binding the
// default framebuffer, the color attachment of the default framebuffer is restored. (not using a
// dummy framebuffer)
TEST_P(VulkanRasterizerDiscardTest, DisablingRasterizerDiscardAfterBindingFBO)
{
    initializeRenderToRenderbuffer();

    glEnable(GL_RASTERIZER_DISCARD);
    EXPECT_GL_NO_ERROR();

    // Clear the default framebuffer
    clearFramebufferWithColor(kFloatBlue);
    EXPECT_EQ(true, hackWindowSurface()->getDeferredStatusForSwapchainImage());

    // Because of GL_RASTERIZER_DISCARD, the dummy VkFramebuffer is used. So, if the result of
    // glReadPixel is a garbage value.
    drawFramebufferWithColor(kFloatRed);
    EXPECT_EQ(true, hackWindowSurface()->getDeferredStatusForSwapchainImage());

    glBindFramebuffer(GL_FRAMEBUFFER, mFbo);

    glDisable(GL_RASTERIZER_DISCARD);
    EXPECT_GL_NO_ERROR();

    // Use FBO instead of the default framebuffer and GL_RASTERIZER_DISCARD is disabled. But calls
    // related to FBO doesn't affect the status of mNeedToAcquireNextSwapchainImage in
    // WindowSurfaceVk.
    clearFramebufferWithColor(kFloatGreen);
    EXPECT_EQ(true, hackWindowSurface()->getDeferredStatusForSwapchainImage());
    EXPECT_PIXEL_EQ(kRboCenterX, kRboCenterY, GLColor::green.R, GLColor::green.G, GLColor::green.B,
                    GLColor::green.A);

    drawFramebufferWithColor(kFloatRed);
    EXPECT_EQ(true, hackWindowSurface()->getDeferredStatusForSwapchainImage());
    EXPECT_PIXEL_EQ(kRboCenterX, kRboCenterY, GLColor::red.R, GLColor::red.G, GLColor::red.B,
                    GLColor::red.A);

    // In this case, GL_RATERIZER_DISCARD is disabled when FBO is bound. So, the default framebuffer
    // should acquire the next swapchain image.
    glBindFramebuffer(GL_FRAMEBUFFER, kDefaultFramebuffer);
    clearFramebufferWithColor(kFloatGreen);
    ASSERT_EQ(false, hackWindowSurface()->getDeferredStatusForSwapchainImage());
    EXPECT_PIXEL_EQ(mCenterX, mCenterY, GLColor::green.R, GLColor::green.G, GLColor::green.B,
                    GLColor::green.A);

    swapBuffers();
    EXPECT_EQ(true, hackWindowSurface()->getDeferredStatusForSwapchainImage());
}

ANGLE_INSTANTIATE_TEST(VulkanRasterizerDiscardTest, ES3_VULKAN());

}  // namespace angle