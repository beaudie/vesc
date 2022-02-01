//
// Copyright 2022 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//
// FramebufferInvalidate tests:
//   Tests various scenarios of framebuffer invalidation.
//

#include "test_utils/ANGLETest.h"
#include "test_utils/gl_raii.h"

using namespace angle;

namespace
{

// Variations:
//
// - bool: whether to use WebGL compatibility mode.
using FramebufferInvalidateTestParams = std::tuple<angle::PlatformParameters, bool>;

std::string FramebufferInvalidateTestPrint(
    const ::testing::TestParamInfo<FramebufferInvalidateTestParams> &paramsInfo)
{
    const FramebufferInvalidateTestParams &params = paramsInfo.param;
    std::ostringstream out;

    out << std::get<0>(params);

    if (std::get<1>(params))
    {
        out << "__WebGLCompatibility";
    }

    return out.str();
}

class FramebufferInvalidateTest : public ANGLETestWithParam<FramebufferInvalidateTestParams>
{
  protected:
    FramebufferInvalidateTest()
    {
        setWindowWidth(kWidth);
        setWindowHeight(kHeight);
        setConfigRedBits(8);
        setConfigGreenBits(8);
        setConfigBlueBits(8);
        setConfigAlphaBits(8);
        if (testing::get<1>(GetParam()))
            setWebGLCompatibilityEnabled(true);
    }

    struct TestResources
    {
        GLTexture colorTexture;
        GLFramebuffer framebuffer;
    };

    TestResources setupResources()
    {
        TestResources resources;

        glBindTexture(GL_TEXTURE_2D, resources.colorTexture);
        glTexStorage2D(GL_TEXTURE_2D, 1, GL_RGBA8, kWidth, kHeight);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        EXPECT_GL_NO_ERROR();

        glBindFramebuffer(GL_FRAMEBUFFER, resources.framebuffer);
        EXPECT_GL_NO_ERROR();
        glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D,
                               resources.colorTexture, 0);
        EXPECT_GL_NO_ERROR();

        return resources;
    }

    // Check the contents of the framebuffer in a small region to
    // ensure no garbage was drawn.
    void checkColor(const GLColor &color)
    {
        for (GLsizei y = 0; y < kHeight; ++y)
        {
            for (GLsizei x = 0; x < kWidth; ++x)
            {
                EXPECT_PIXEL_COLOR_EQ(x, y, color);
            }
        }
    }

    // Use a larger window/framebuffer size than 1x1 to increase the
    // chances that random garbage will appear.
    static constexpr GLsizei kWidth  = 2;
    static constexpr GLsizei kHeight = 2;
};

// Note: tested multiple other combinations:
//
// - Clearing/drawing to the framebuffer after invalidating, without using a
//   secondary FBO
// - Clearing the framebuffer after invalidating, using a secondary FBO
// - Invalidating after clearing/drawing to the FBO, to verify WebGL's behavior
//   that after invalidation, the framebuffer is either unmodified, or cleared
//   to transparent black
//
// This combination, drawing after invalidating plus copying from the drawn-to
// texture, was the only one which provoked the original bug in the Metal
// backend with the following command line arguments:
//
// MTL_DEBUG_LAYER=1 MTL_DEBUG_LAYER_VALIDATE_LOAD_ACTIONS=1 \
//    MTL_DEBUG_LAYER_VALIDATE_STORE_ACTIONS=1 \
//    MTL_DEBUG_LAYER_VALIDATE_UNRETAINED_RESOURCES=4 \
//    angle_end2end_tests ...
//
// See anglebug.com/6923.

TEST_P(FramebufferInvalidateTest, DrawFBOAfterInvalidate)
{
    GLint origFramebuffer = 0;
    glGetIntegerv(GL_DRAW_FRAMEBUFFER_BINDING, &origFramebuffer);
    ANGLE_GL_PROGRAM(drawGreen, essl3_shaders::vs::Simple(), essl3_shaders::fs::Green());

    TestResources resources = setupResources();

    const GLenum attachment = GL_COLOR_ATTACHMENT0;

    glBindFramebuffer(GL_FRAMEBUFFER, resources.framebuffer);
    EXPECT_GLENUM_EQ(GL_FRAMEBUFFER_COMPLETE, glCheckFramebufferStatus(GL_FRAMEBUFFER));

    // Clear to red to start.
    glClearColor(1.0f, 0.0f, 0.0f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT);
    glInvalidateFramebuffer(GL_FRAMEBUFFER, 1, &attachment);
    EXPECT_GL_NO_ERROR();

    // Draw green after invalidate.
    // Important to use a vertex buffer because WebGL doesn't support client-side arrays.
    constexpr bool useVertexBuffer = true;
    drawQuad(drawGreen.get(), essl3_shaders::PositionAttrib(), 0.5f, 1.0f, useVertexBuffer);
    EXPECT_GL_NO_ERROR();

    // Bind original framebuffer.
    glBindFramebuffer(GL_DRAW_FRAMEBUFFER, origFramebuffer);
    glBindFramebuffer(GL_READ_FRAMEBUFFER, resources.framebuffer);
    EXPECT_GLENUM_EQ(GL_FRAMEBUFFER_COMPLETE, glCheckFramebufferStatus(GL_DRAW_FRAMEBUFFER));
    EXPECT_GLENUM_EQ(GL_FRAMEBUFFER_COMPLETE, glCheckFramebufferStatus(GL_READ_FRAMEBUFFER));
    EXPECT_GL_NO_ERROR();

    // Blit from user's framebuffer to the window.
    //
    // This step is crucial to catch bugs in the Metal backend's use of no-op load/store actions.
    glBlitFramebuffer(0, 0, kWidth, kHeight, 0, 0, kWidth, kHeight, GL_COLOR_BUFFER_BIT,
                      GL_NEAREST);
    EXPECT_GL_NO_ERROR();

    // Verify results.
    glBindFramebuffer(GL_READ_FRAMEBUFFER, origFramebuffer);
    EXPECT_GL_NO_ERROR();
    checkColor(GLColor::green);
}

GTEST_ALLOW_UNINSTANTIATED_PARAMETERIZED_TEST(FramebufferInvalidateTest);
ANGLE_INSTANTIATE_TEST_COMBINE_1(FramebufferInvalidateTest,
                                 FramebufferInvalidateTestPrint,
                                 testing::Bool(),
                                 ANGLE_ALL_TEST_PLATFORMS_ES3);

}  // namespace
