#include "test_utils/ANGLETest.h"
#include "test_utils/gl_raii.h"

using namespace angle;

constexpr int kPixelColorThreshhold = 8;

class AdvancedBlendTest : public ANGLETest<>
{
  protected:
    AdvancedBlendTest()
    {
        setWindowWidth(128);
        setWindowHeight(128);
        setConfigRedBits(8);
        setConfigGreenBits(8);
        setConfigBlueBits(8);
        setConfigAlphaBits(8);
    }
};

// Simplified from dEQP-GLES31.functional.blend_equation_advanced.barrier.colorburn
TEST_P(AdvancedBlendTest, colorBurnBlendWithBarrier)
{
    ANGLE_SKIP_TEST_IF(!IsGLExtensionEnabled("GL_KHR_blend_equation_advanced"));

    const char *vertSrc = R"(#version 320 es
        in highp vec4 a_position;
        in mediump vec4 a_color;
        out mediump vec4 v_color;
        void main()
        {
            gl_Position = a_position;
            v_color = a_color;
        }
    )";

    const char *fragSrc = R"(#version 320 es
        in mediump vec4 v_color;
        layout(blend_support_colorburn) out;
        layout(location = 0) out mediump vec4 o_color;
        void main()
        {
            o_color = v_color;
        }
    )";

    ANGLE_GL_PROGRAM(program, vertSrc, fragSrc);
    glUseProgram(program);

    std::array<GLfloat, 16> attribPosData = {1, 1,  0.5, 1, -1, 1,  0.5, 1,
                                             1, -1, 0.5, 1, -1, -1, 0.5, 1};

    GLint attribPosLoc = glGetAttribLocation(1, "a_position");
    ASSERT(attribPosLoc >= 0);
    glEnableVertexAttribArray(attribPosLoc);
    glVertexAttribPointer(attribPosLoc, 4, GL_FLOAT, GL_FALSE, 0, attribPosData.data());

    std::array<GLfloat, 16> attribColorData1 = {1, 0.2, 0.5, 1, 1, 0.2, 0.5, 1,
                                                1, 0.2, 0.5, 1, 1, 0.2, 0.5, 1};
    GLint attribColorLoc                     = glGetAttribLocation(1, "a_color");
    ASSERT(attribColorLoc >= 0);
    glEnableVertexAttribArray(attribColorLoc);
    glVertexAttribPointer(attribColorLoc, 4, GL_FLOAT, GL_FALSE, 0, attribColorData1.data());

    if (IsGLExtensionEnabled("GL_KHR_blend_equation_advanced_coherent"))
    {
        glDisable(GL_BLEND_ADVANCED_COHERENT_KHR);
    }

    const uint16_t indices[] = {0, 1, 2, 2, 1, 3};
    glClearColor(0.5, 0.5, 0.5, 1.0);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);

    // Draw a quad with GL_BLEND disabled. FBO color is expected to be (1, 0.2, 0.5, 1)
    glDisable(GL_BLEND);
    glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_SHORT, &indices[0]);
    ASSERT_GL_NO_ERROR();

    // Draw another quad with GL_BLEND enabled. FBO color is expected to be the blend result
    // between (1, 0.2, 0.5, 1) and (0.5, 0.5, 0, 1), using GL_COLORBURN blend function,
    // which leads to (1, 0, 0, 1)
    glEnable(GL_BLEND);
    glBlendEquation(GL_COLORBURN);
    glBlendBarrier();
    std::array<GLfloat, 16> attribColorData2 = {0.5, 0.5, 0, 1, 0.5, 0.5, 0, 1,
                                                0.5, 0.5, 0, 1, 0.5, 0.5, 0, 1};

    glEnableVertexAttribArray(attribColorLoc);
    glVertexAttribPointer(attribColorLoc, 4, GL_FLOAT, GL_FALSE, 0, attribColorData2.data());
    glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_SHORT, &indices[0]);

    // EXPECT_PIXEL_COLOR_NEAR(64, 64, GLColor(255, 0, 0, 255), kPixelColorThreshhold);

    GLColor color(0, 0, 0, 0);
    glReadPixels(64, 64, 1, 1, GL_RGBA, GL_UNSIGNED_BYTE, &color);
    ASSERT(abs(color.R - 255) <= kPixelColorThreshhold);
    ASSERT(abs(color.G - 0) <= kPixelColorThreshhold);
    ASSERT(abs(color.B - 0) <= kPixelColorThreshhold);
    ASSERT(abs(color.A - 255) <= kPixelColorThreshhold);
}

GTEST_ALLOW_UNINSTANTIATED_PARAMETERIZED_TEST(AdvancedBlendTest);
ANGLE_INSTANTIATE_TEST_ES32(AdvancedBlendTest);