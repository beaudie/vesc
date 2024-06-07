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

    const char *vertSrc =
        "#version 320 es\n"
        "\n"
        "in highp vec4 a_position;\n"
        "\n"
        "in mediump vec4 a_color;\n"

        "out mediump vec4 v_color;\n"
        "\n"
        "void main()\n"
        "{\n"
        "    gl_Position = a_position;\n"
        "    v_color = a_color;\n"
        "}\n";

    const char *fragSrc =
        "#version 320 es\n"
        "\n"
        "in mediump vec4 v_color;\n"
        "\n"
        "layout(blend_support_colorburn) out;\n"
        "layout(location = 0) out mediump vec4 o_color;\n"
        "\n"
        "void main()\n"
        "{\n"
        "     o_color = v_color;\n"
        "}\n";

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

    // glBlendEquation(GL_COLORBURN);
    //  TODO check if this is a bug, with glBlendEquation, the subsequent glDisable() after
    //  glClear() doesn't seem to disable the blend. On Android device only. Linux is fine.
    //  check if GL_KHR_blend_equation_advanced_coherent is supported
    if (IsGLExtensionEnabled("GL_KHR_blend_equation_advanced_coherent"))
    {
        glDisable(GL_BLEND_ADVANCED_COHERENT_KHR);
    }

    const uint16_t indices[] = {0, 1, 2, 2, 1, 3};
    glClearColor(0.5, 0.5, 0.5, 1.0);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);
    glDisable(GL_BLEND);
    glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_SHORT, &indices[0]);
    ASSERT_GL_NO_ERROR();
    glEnable(GL_BLEND);
    glBlendEquation(GL_COLORBURN);
    glBlendBarrier();
    std::array<GLfloat, 16> attribColorData2 = {0.5, 0.5, 0, 1, 0.5, 0.5, 0, 1,
                                                0.5, 0.5, 0, 1, 0.5, 0.5, 0, 1};

    glEnableVertexAttribArray(attribColorLoc);
    glVertexAttribPointer(attribColorLoc, 4, GL_FLOAT, GL_FALSE, 0, attribColorData2.data());
    glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_SHORT, &indices[0]);
    glBlendBarrier();
    glFlush();

    EXPECT_PIXEL_COLOR_NEAR(64, 64, GLColor(255, 0, 0, 255), kPixelColorThreshhold);
}

GTEST_ALLOW_UNINSTANTIATED_PARAMETERIZED_TEST(AdvancedBlendTest);
ANGLE_INSTANTIATE_TEST_ES32(AdvancedBlendTest);