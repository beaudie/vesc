//
// Copyright 2022 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//
// ShaderPrecisionTest:
//   Tests that lowp, mediump, highp precision work properly
//

#include "test_utils/ANGLETest.h"
#include "test_utils/gl_raii.h"
#include "util/random_utils.h"
#include "util/shader_utils.h"

using namespace angle;

namespace
{

class ShaderPrecisionTest : public ANGLETest<>
{
  protected:
    ShaderPrecisionTest()
    {
        setWindowWidth(128);
        setWindowHeight(128);
        setConfigRedBits(8);
        setConfigGreenBits(8);
        setConfigBlueBits(8);
        setConfigAlphaBits(8);
    }

    void createFramebufferAndRenderbuffer()
    {
        glGenFramebuffers(1, &mFramebuffer);
        glGenRenderbuffers(1, &mRenderbuffer);
        glBindRenderbuffer(GL_RENDERBUFFER, mRenderbuffer);
        glRenderbufferStorage(GL_RENDERBUFFER, GL_R32I, 32, 32);
        glBindFramebuffer(GL_FRAMEBUFFER, mFramebuffer);
        glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_RENDERBUFFER,
                                  mRenderbuffer);
        EXPECT_GL_NO_ERROR();
        GLenum status = glCheckFramebufferStatus(GL_FRAMEBUFFER);
        ASSERT_GLENUM_EQ(GL_FRAMEBUFFER_COMPLETE, status);
    }

    void deleteFramebufferAndRenderbuffer()
    {
        glDeleteFramebuffers(1, &mFramebuffer);
        glDeleteRenderbuffers(1, &mRenderbuffer);
    }

    void createShaderAndProgram(const char *vertexShader, const char *fragmentShader)
    {
        mProgram                               = glCreateProgram();
        mVertexShader                          = glCreateShader(GL_VERTEX_SHADER);
        const char *vertexShaderSourceArray[1] = {vertexShader};
        glShaderSource(mVertexShader, 1, vertexShaderSourceArray, nullptr);
        glCompileShader(mVertexShader);
        GLint compileResult;
        glGetShaderiv(mVertexShader, GL_COMPILE_STATUS, &compileResult);
        ASSERT(compileResult == 1);

        mFragmentShader                          = glCreateShader(GL_FRAGMENT_SHADER);
        const char *fragmentShaderSourceArray[1] = {fragmentShader};
        glShaderSource(mFragmentShader, 1, fragmentShaderSourceArray, nullptr);
        glCompileShader(mFragmentShader);
        glGetShaderiv(mFragmentShader, GL_COMPILE_STATUS, &compileResult);

        ASSERT(compileResult == 1);

        glAttachShader(mProgram, mVertexShader);
        glAttachShader(mProgram, mFragmentShader);
        glLinkProgram(mProgram);
    }

    void deleteShaderAndProgram()
    {
        glDeleteShader(mVertexShader);
        glDeleteShader(mFragmentShader);
        glDeleteProgram(mProgram);
    }

    GLuint mProgram;
    GLuint mVertexShader;
    GLuint mFragmentShader;
    GLuint mFramebuffer;
    GLuint mRenderbuffer;
};

const char kIntMediumpAddVertexVS[] =
    "#version 300 es\n"
    "in highp vec4 a_position;\n"
    "in mediump int a_in0;\n"
    "in mediump int a_in1;\n"
    "flat out mediump int v_out;\n"

    "void main (void)\n"
    "{\n"
    "   gl_Position = a_position;\n"
    "   mediump int in0 = a_in0;\n"
    "   mediump int in1 = a_in1;\n"
    "   v_out = in0 + in1;\n"
    "}\n";

const char kIntMediumpAddVertexFS[] =
    "#version 300 es\n"
    "layout(location = 0) out mediump int o_out;\n"
    "flat in mediump int v_out;\n"

    "void main (void)\n"
    "{\n"
    "   o_out = v_out;\n"
    "}\n";

const char kIntMediumpAddFragmentVS[] =
    "#version 300 es\n"
    "in highp vec4 a_position;\n"
    "in mediump int a_in0;\n"
    "in mediump int a_in1;\n"
    "flat out mediump int v_in0;\n"
    "flat out mediump int v_in1;\n"

    "void main (void)\n"
    "{\n"
    "   gl_Position = a_position;\n"
    "   v_in0 = a_in0;\n"
    "   v_in1 = a_in1;\n"
    "}\n";

const char kIntMediumpAddFragmentFS[] =
    "#version 300 es\n"
    "layout(location = 0) out mediump int o_out;\n"
    "flat in mediump int v_in0;\n"
    "flat in mediump int v_in1;\n"

    "void main (void)\n"
    "{\n"
    "   mediump int in0 = v_in0;\n"
    "   mediump int in1 = v_in1;\n"
    "   o_out = in0 + in1;\n"
    "}\n";

const char kIntMediumpSubVertexVS[] =
    "#version 300 es\n"
    "in highp vec4 a_position;\n"
    "in mediump int a_in0;\n"
    "in mediump int a_in1;\n"
    "flat out mediump int v_out;\n"

    "void main (void)\n"
    "{\n"
    "   gl_Position = a_position;\n"
    "   mediump int in0 = a_in0;\n"
    "   mediump int in1 = a_in1;\n"
    "   v_out = in0 - in1;\n"
    "}\n";

const char kIntMediumpSubVertexFS[] =
    "#version 300 es\n"
    "layout(location = 0) out mediump int o_out;\n"
    "flat in mediump int v_out;\n"

    "void main (void)\n"
    "{\n"
    "   o_out = v_out;\n"
    "}\n";

const char kIntMediumpSubFragmentVS[] =
    "#version 300 es\n"
    "in highp vec4 a_position;\n"
    "in mediump int a_in0;\n"
    "in mediump int a_in1;\n"
    "flat out mediump int v_in0;\n"
    "flat out mediump int v_in1;\n"

    "void main (void)\n"
    "{\n"
    "   gl_Position = a_position;\n"
    "   v_in0 = a_in0;\n"
    "   v_in1 = a_in1;\n"
    "}\n";

const char kIntMediumpSubFragmentFS[] =
    "#version 300 es\n"
    "layout(location = 0) out mediump int o_out;\n"
    "flat in mediump int v_in0;\n"
    "flat in mediump int v_in1;\n"

    "void main (void)\n"
    "{\n"
    "   mediump int in0 = v_in0;\n"
    "   mediump int in1 = v_in1;\n"
    "   o_out = in0 - in1;\n"
    "}\n";

const char kIntMediumpMulVertexVS[] =
    "#version 300 es\n"
    "in highp vec4 a_position;\n"
    "in mediump int a_in0;\n"
    "in mediump int a_in1;\n"
    "flat out mediump int v_out;\n"

    "void main (void)\n"
    "{\n"
    "   gl_Position = a_position;\n"
    "   mediump int in0 = a_in0;\n"
    "   mediump int in1 = a_in1;\n"
    "   v_out = in0 * in1;\n"
    "}\n";

const char kIntMediumpMulVertexFS[] =
    "#version 300 es\n"
    "layout(location = 0) out mediump int o_out;\n"
    "flat in mediump int v_out;\n"

    "void main (void)\n"
    "{\n"
    "   o_out = v_out;\n"
    "}\n";

const char kIntMediumpMulFragmentVS[] =
    "#version 300 es\n"
    "in highp vec4 a_position;\n"
    "in mediump int a_in0;\n"
    "in mediump int a_in1;\n"
    "flat out mediump int v_in0;\n"
    "flat out mediump int v_in1;\n"

    "void main (void)\n"
    "{\n"
    "   gl_Position = a_position;\n"
    "   v_in0 = a_in0;\n"
    "   v_in1 = a_in1;\n"
    "}\n";

const char kIntMediumpMulFragmentFS[] =
    "#version 300 es\n"
    "layout(location = 0) out mediump int o_out;\n"
    "flat in mediump int v_in0;\n"
    "flat in mediump int v_in1;\n"

    "void main (void)\n"
    "{\n"
    "   mediump int in0 = v_in0;\n"
    "   mediump int in1 = v_in1;\n"
    "   o_out = in0 * in1;\n"
    "}\n";

const char kIntMediumpDivVertexVS[] =
    "#version 300 es\n"
    "in highp vec4 a_position;\n"
    "in mediump int a_in0;\n"
    "in mediump int a_in1;\n"
    "flat out mediump int v_out;\n"

    "void main (void)\n"
    "{\n"
    "   gl_Position = a_position;\n"
    "   mediump int in0 = a_in0;\n"
    "   mediump int in1 = a_in1;\n"
    "   v_out = in0 / in1;\n"
    "}\n";

const char kIntMediumpDivVertexFS[] =
    "#version 300 es\n"
    "layout(location = 0) out mediump int o_out;\n"
    "flat in mediump int v_out;\n"

    "void main (void)\n"
    "{\n"
    "   o_out = v_out;\n"
    "}\n";

const char kIntMediumpDivFragmentVS[] =
    "#version 300 es\n"
    "in highp vec4 a_position;\n"
    "in mediump int a_in0;\n"
    "in mediump int a_in1;\n"
    "flat out mediump int v_in0;\n"
    "flat out mediump int v_in1;\n"

    "void main (void)\n"
    "{\n"
    "   gl_Position = a_position;\n"
    "   v_in0 = a_in0;\n"
    "   v_in1 = a_in1;\n"
    "}\n";

const char kIntMediumpDivFragmentFS[] =
    "#version 300 es\n"
    "layout(location = 0) out mediump int o_out;\n"
    "flat in mediump int v_in0;\n"
    "flat in mediump int v_in1;\n"

    "void main (void)\n"
    "{\n"
    "   mediump int in0 = v_in0;\n"
    "   mediump int in1 = v_in1;\n"
    "   o_out = in0 / in1;\n"
    "}\n";

const char kIntLowpAddVertexVS[] =
    "#version 300 es\n"
    "in highp vec4 a_position;\n"
    "in lowp int a_in0;\n"
    "in lowp int a_in1;\n"
    "flat out lowp int v_out;\n"

    "void main (void)\n"
    "{\n"
    "   gl_Position = a_position;\n"
    "   lowp int in0 = a_in0;\n"
    "   lowp int in1 = a_in1;\n"
    "   v_out = in0 + in1;\n"
    "}\n";

const char kIntLowpAddVertexFS[] =
    "#version 300 es\n"
    "layout(location = 0) out lowp int o_out;\n"
    "flat in lowp int v_out;\n"

    "void main (void)\n"
    "{\n"
    "   o_out = v_out;\n"
    "}\n";

uint32_t deBitMask32(int leastSignificantBitNdx, int numBits)
{
    if (numBits < 32 && leastSignificantBitNdx < 32)
    {
        return ((1u << numBits) - 1u) << (uint32_t)leastSignificantBitNdx;
    }
    else if (numBits == 0 && leastSignificantBitNdx == 32)
    {
        return 0u;
    }
    else
    {
        ASSERT(numBits == 32 && leastSignificantBitNdx == 0);
        return 0xffffffffu;
    }
}

int32_t deSignExtendTo32(int32_t value, int numBits)
{
    if (numBits < 32)
    {
        bool signSet      = ((uint32_t)value & (1u << (numBits - 1))) != 0;
        uint32_t signMask = deBitMask32(numBits, 32 - numBits);

        ASSERT(((uint32_t)value & signMask) == 0u);

        return (int32_t)((uint32_t)value | (signSet ? signMask : 0u));
    }
    else
    {
        return value;
    }
}

int64_t add(int64_t a, int64_t b)
{
    return a + b;
}

int64_t sub(int64_t a, int64_t b)
{
    return a - b;
}

int64_t mul(int64_t a, int64_t b)
{
    return a * b;
}

int64_t div(int64_t a, int64_t b)
{
    return a / b;
}

typedef int64_t (*EvalFunc)(int64_t a, int64_t b);

void PrecisionIntTest(GLuint &program,
                      int bits,
                      angle::Vector2U int0Range,
                      angle::Vector2U int1Range,
                      EvalFunc evaluationFunction)
{
    glUseProgram(program);

    // Debug code
    //    std::vector<int> debugPixels(32 * 32 * 4);
    //    glReadPixels(0, 0, 32, 32, GL_RGBA_INTEGER, GL_INT, &debugPixels[0]);
    //    INFO() << "Yuxin Debug after Framebuffer initialization, color attachment pixel: "
    //           << debugPixels[0];

    // Setup Vertex Position Attributes
    const float kPositions[] = {-1.0f, -1.0f, 0.0f, 1.0f, -1.0f, 1.0f, 0.0f, 1.0f,
                                1.0f,  -1.0f, 0.0f, 1.0f, 1.0f,  1.0f, 0.0f, 1.0f};
    GLuint vertexPosBuffer;
    glGenBuffers(1, &vertexPosBuffer);

    // Setup other vertex attribute inputs
    GLuint vertexIn0Buffer;
    glGenBuffers(1, &vertexIn0Buffer);

    GLuint vertexIn1Buffer;
    glGenBuffers(1, &vertexIn1Buffer);

    // Setup index buffers
    GLuint indexBuffer;
    glGenBuffers(1, &indexBuffer);
    const uint16_t Indices[] = {0, 1, 2, 2, 1, 3};
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, indexBuffer);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(uint16_t) * 6, Indices, GL_STATIC_DRAW);

    // Setup utilities to generate the randrom numbers for vertex attribute inputs
    bool isMaxRangeInt0 = int0Range.x() == (int)0x800000000 && int0Range.y() == (int)0x7fffffff;
    bool isMaxRangeInt1 = int1Range.x() == (int)0x800000000 && int1Range.y() == (int)0x7fffffff;
    uint32_t mask       = bits == 32 ? 0xffffffffu : ((1u << bits) - 1);
    angle::RNG rng;

    // Arrays to hold vertex attribute inputs
    int in0Arr[4] = {0};
    int in1Arr[4] = {0};

    // Vector to store contents read back from FBO color attachment
    std::vector<int> pixels(32 * 32 * 4);

    // Test with random values for in0 and in1, for 128 iterations
    for (int testIteration = 0; testIteration < 128; testIteration++)
    {
        int randomInt0       = isMaxRangeInt0 ? (int)rng.randomInt()
                                              : rng.randomIntBetween(int0Range.x(), int0Range.y());
        int randomInt1       = isMaxRangeInt1 ? (int)rng.randomInt()
                                              : rng.randomIntBetween(int1Range.x(), int1Range.y());
        int maskedRandomInt0 = randomInt0 & mask;
        int maskedRandomInt1 = randomInt1 & mask;
        int in0              = deSignExtendTo32(maskedRandomInt0, bits);
        int in1              = deSignExtendTo32(maskedRandomInt1, bits);

        int refMasked =
            static_cast<int>(evaluationFunction(in0, in1) & static_cast<uint64_t>(mask));

        std::fill(&in0Arr[0], &in0Arr[0] + 4, in0);
        std::fill(&in1Arr[0], &in1Arr[0] + 4, in1);

        glBindBuffer(GL_ARRAY_BUFFER, vertexIn0Buffer);
        glBufferData(GL_ARRAY_BUFFER, sizeof(in0) * 4, in0Arr, GL_STATIC_DRAW);
        GLuint vertexIn0Position = glGetAttribLocation(program, "a_in0");
        glEnableVertexAttribArray(vertexIn0Position);
        glVertexAttribIPointer(vertexIn0Position, 1, GL_INT, 0, 0);

        glBindBuffer(GL_ARRAY_BUFFER, vertexIn1Buffer);
        glBufferData(GL_ARRAY_BUFFER, sizeof(in1) * 4, in1Arr, GL_STATIC_DRAW);
        GLuint vertexIn1Position = glGetAttribLocation(program, "a_in1");
        glEnableVertexAttribArray(vertexIn1Position);
        glVertexAttribIPointer(vertexIn1Position, 1, GL_INT, 0, 0);

        glBindBuffer(GL_ARRAY_BUFFER, vertexPosBuffer);
        glBufferData(GL_ARRAY_BUFFER, sizeof(GLfloat) * 16, kPositions, GL_STATIC_DRAW);
        GLint positionLocation = glGetAttribLocation(program, "a_position");
        glEnableVertexAttribArray(positionLocation);
        glVertexAttribPointer(positionLocation, 4, GL_FLOAT, GL_FALSE, 0, 0);

        glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_SHORT, 0);

        glDisableVertexAttribArray(vertexIn0Buffer);
        glDisableVertexAttribArray(vertexIn1Buffer);
        glDisableVertexAttribArray(vertexPosBuffer);

        glReadPixels(0, 0, 32, 32, GL_RGBA_INTEGER, GL_INT, &pixels[0]);

        // verify results
        for (int y = 0; y < 32; y++)
        {
            for (int x = 0; x < 32; x++)
            {
                int cmpOut    = pixels[(y * 32 + x) * 4];
                int cmpMasked = cmpOut & mask;
                ASSERT(cmpMasked == refMasked);
            }
        }
    }

    // INFO() << "Yuxin Debug after test, color attachment pixel: " << pixels[0];

    // delete resource
    glDeleteBuffers(1, &vertexIn0Buffer);
    glDeleteBuffers(1, &vertexIn1Buffer);
    glDeleteBuffers(1, &indexBuffer);
    glDeleteBuffers(1, &vertexPosBuffer);
}

// Test Precision switch from Mediump to Lowp works
// This reproduces a bug on Pixel6, where running
// out/AndroidDebug/angle_deqp_gles3_tests
// --gtest_filter="dEQP?GLES3.functional.shaders.precision.int*" failed on this test
// dEQP-GLES3.functional.shaders.precision.int.lowp_add_vertex
TEST_P(ShaderPrecisionTest, MediumpToLowp)
{
    // For each sub test, create a new framebuffer, create a new renderbuffer, run the test for 128
    // iterations
    // dEQP-GLES3.functional.shaders.precision.int.mediump_add_vertex
    createShaderAndProgram(kIntMediumpAddVertexVS, kIntMediumpAddVertexFS);
    createFramebufferAndRenderbuffer();
    angle::Vector2U int0Range(-(1 << 15), (1 << 15) - 1);
    angle::Vector2U int1Range(-(1 << 15), (1 << 15) - 1);
    PrecisionIntTest(mProgram, 16, int0Range, int1Range, &add);
    deleteShaderAndProgram();
    deleteFramebufferAndRenderbuffer();

    // dEQP-GLES3.functional.shaders.precision.int.medium_add_fragment
    createShaderAndProgram(kIntMediumpAddFragmentVS, kIntMediumpAddFragmentFS);
    createFramebufferAndRenderbuffer();
    PrecisionIntTest(mProgram, 16, int0Range, int1Range, &add);
    deleteShaderAndProgram();
    deleteFramebufferAndRenderbuffer();

    // dEQP-GLES3.functional.shaders.precision.int.mediump_sub_vertex
    createShaderAndProgram(kIntMediumpSubVertexVS, kIntMediumpSubVertexFS);
    createFramebufferAndRenderbuffer();
    PrecisionIntTest(mProgram, 16, int0Range, int1Range, &sub);
    deleteShaderAndProgram();
    deleteFramebufferAndRenderbuffer();

    // dEQP-GLES3.functional.shaders.precision.int.mediump_sub_fragment
    createShaderAndProgram(kIntMediumpSubFragmentVS, kIntMediumpSubFragmentFS);
    createFramebufferAndRenderbuffer();
    PrecisionIntTest(mProgram, 16, int0Range, int1Range, &sub);
    deleteShaderAndProgram();
    deleteFramebufferAndRenderbuffer();

    // dEQP-GLES3.functional.shaders.precision.int.mediump_mul_vertex
    createShaderAndProgram(kIntMediumpMulVertexVS, kIntMediumpMulVertexFS);
    createFramebufferAndRenderbuffer();
    PrecisionIntTest(mProgram, 16, int0Range, int1Range, &mul);
    deleteShaderAndProgram();
    deleteFramebufferAndRenderbuffer();

    // dEQP-GLES3.functional.shaders.precision.int.mediump_mul_fragment
    createShaderAndProgram(kIntMediumpMulFragmentVS, kIntMediumpMulFragmentFS);
    createFramebufferAndRenderbuffer();
    PrecisionIntTest(mProgram, 16, int0Range, int1Range, &mul);
    deleteShaderAndProgram();
    deleteFramebufferAndRenderbuffer();

    // dEQP-GLES3.functional.shaders.precision.int.mediump_div_vertex
    createShaderAndProgram(kIntMediumpDivVertexVS, kIntMediumpDivVertexFS);
    createFramebufferAndRenderbuffer();
    int1Range.x() = 0;
    int1Range.y() = 1000;
    PrecisionIntTest(mProgram, 16, int0Range, int1Range, &div);
    deleteShaderAndProgram();
    deleteFramebufferAndRenderbuffer();

    // dEQP-GLES3.functional.shaders.precision.int.mediump_div_fragment
    createShaderAndProgram(kIntMediumpDivFragmentVS, kIntMediumpDivFragmentFS);
    createFramebufferAndRenderbuffer();
    PrecisionIntTest(mProgram, 16, int0Range, int1Range, &div);
    deleteShaderAndProgram();
    deleteFramebufferAndRenderbuffer();

    // dEQP-GLES3.functional.shaders.precision.int.lowp_add_vertex
    createShaderAndProgram(kIntLowpAddVertexVS, kIntLowpAddVertexFS);
    createFramebufferAndRenderbuffer();
    int0Range.x() = -(1 << 7);
    int0Range.y() = (1 << 7) - 1;
    int1Range.x() = -(1 << 7);
    int1Range.y() = (1 << 7) - 1;
    PrecisionIntTest(mProgram, 16, int0Range, int1Range, &add);
    deleteShaderAndProgram();
    deleteFramebufferAndRenderbuffer();
}

ANGLE_INSTANTIATE_TEST_ES3(ShaderPrecisionTest);
}  // namespace