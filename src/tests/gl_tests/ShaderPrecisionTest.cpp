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

// Uncomment to test with different in0 and in1 values
// uint32_t deBitMask32(int leastSignificantBitNdx, int numBits)
//{
//     if (numBits < 32 && leastSignificantBitNdx < 32)
//     {
//         return ((1u << numBits) - 1u) << (uint32_t)leastSignificantBitNdx;
//     }
//     else if (numBits == 0 && leastSignificantBitNdx == 32)
//     {
//         return 0u;
//     }
//     else
//     {
//         ASSERT(numBits == 32 && leastSignificantBitNdx == 0);
//         return 0xffffffffu;
//     }
// }
//
// int32_t deSignExtendTo32(int32_t value, int numBits)
//{
//     if (numBits < 32)
//     {
//         bool signSet      = ((uint32_t)value & (1u << (numBits - 1))) != 0;
//         uint32_t signMask = deBitMask32(numBits, 32 - numBits);
//
//         ASSERT(((uint32_t)value & signMask) == 0u);
//
//         return (int32_t)((uint32_t)value | (signSet ? signMask : 0u));
//     }
//     else
//     {
//         return value;
//     }
// }

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
                      std::vector<int> &in0in1Data,
                      EvalFunc evaluationFunction)
{
    glUseProgram(program);

    // Setup Vertex Position Attributes
    const float kPositions[] = {-1.0f, -1.0f, 0.0f, 1.0f, -1.0f, 1.0f, 0.0f, 1.0f,
                                1.0f,  -1.0f, 0.0f, 1.0f, 1.0f,  1.0f, 0.0f, 1.0f};
    GLuint vertexPosBuffer;
    glGenBuffers(1, &vertexPosBuffer);

    // Setup Vertex Attributes in0 and in1
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

    // Uncomment to test with different in0 and in1 values
    // Setup utilities to generate the randrom numbers for vertex attribute inputs
    //    angle::RNG rng;
    //    bool isMaxRangeInt0 = int0Range.x() == (int)0x800000000 && int0Range.y() ==
    //    (int)0x7fffffff; bool isMaxRangeInt1 = int1Range.x() == (int)0x800000000 && int1Range.y()
    //    == (int)0x7fffffff;
    uint32_t mask = bits == 32 ? 0xffffffffu : ((1u << bits) - 1);

    // Arrays to hold vertex attribute in0 and in1 inputs
    int in0Arr[4] = {0};
    int in1Arr[4] = {0};

    // vector to store contents read back from FBO color attachment
    std::vector<int> pixels(32 * 32 * 4);

    // Test with random values for in0 and in1, for 128 iterations
    for (int testIteration = 0; testIteration < 128; testIteration++)
    {
        // uncomment to test with different in0 and in1 values
        //        int randomInt0       = isMaxRangeInt0 ? (int)rng.randomInt()
        //                                              : rng.randomIntBetween(int0Range.x(),
        //                                              int0Range.y());
        //        int randomInt1       = isMaxRangeInt1 ? (int)rng.randomInt()
        //                                              : rng.randomIntBetween(int1Range.x(),
        //                                              int1Range.y());
        //        int maskedRandomInt0 = randomInt0 & mask;
        //        int maskedRandomInt1 = randomInt1 & mask;
        //        int in0              = deSignExtendTo32(maskedRandomInt0, bits);
        //        int in1              = deSignExtendTo32(maskedRandomInt1, bits);

        // comment to test with different in0 and in1 values
        int in0 = in0in1Data[testIteration * 2];
        int in1 = in0in1Data[testIteration * 2 + 1];

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

    // delete resource
    glDeleteBuffers(1, &vertexIn0Buffer);
    glDeleteBuffers(1, &vertexIn1Buffer);
    glDeleteBuffers(1, &indexBuffer);
    glDeleteBuffers(1, &vertexPosBuffer);
}

// Test Precision switch from Mediump to Lowp works
// This reproduces a bug on Pixel7, where running
// out/AndroidDebug/angle_deqp_gles3_tests
// --gtest_filter="dEQP?GLES3.functional.shaders.precision.int*" failed on this test
// dEQP-GLES3.functional.shaders.precision.int.lowp_add_vertex
TEST_P(ShaderPrecisionTest, MediumpToLowp)
{
    // For each sub test, run the test for 128 iterations
    // dEQP-GLES3.functional.shaders.precision.int.mediump_add_vertex
    createShaderAndProgram(kIntMediumpAddVertexVS, kIntMediumpAddVertexFS);
    createFramebufferAndRenderbuffer();
    angle::Vector2U int0Range(-(1 << 15), (1 << 15) - 1);
    angle::Vector2U int1Range(-(1 << 15), (1 << 15) - 1);
    std::vector<int> testData = {
        -21041, -6295,  26911,  17599,  15654,  -10398, 18661,  23157,  12062,  -31909, -20237,
        5262,   14250,  -30818, 26585,  1527,   -1273,  -14136, -615,   2649,   -24026, -9458,
        -16325, 85,     22206,  28805,  17508,  -26771, -1071,  -19859, -14422, 12619,  4006,
        -5432,  16466,  19563,  20631,  8027,   -24772, 17140,  -14835, 20982,  -4308,  -19312,
        -31661, -20838, -29663, -28205, -15095, -29638, 22777,  -5754,  32435,  -7830,  7233,
        -27984, -5870,  -8976,  -11217, 25269,  7309,   25779,  9192,   -6998,  31555,  -364,
        -1484,  29714,  -29467, -6397,  16027,  19783,  32401,  -32699, 18951,  31745,  -24441,
        24789,  26034,  28089,  -30127, 24235,  -5526,  495,    -5724,  -18072, 265,    15691,
        16107,  4646,   29987,  6547,   -6539,  -6804,  16487,  -8764,  -11480, -9222,  -17052,
        -32471, -16150, -24936, -12125, 23390,  19925,  29112,  22824,  10734,  26454,  812,
        -9995,  3286,   -6278,  24085,  -9393,  -335,   30300,  2266,   23002,  17833,  -12149,
        -18306, -16079, 8315,   2342,   -2463,  19154,  -8798,  -18627, 12100,  -22906, 6813,
        17346,  -3336,  26742,  8526,   26540,  -20674, 7080,   21417,  15697,  21826,  -16574,
        9501,   26139,  -32337, 10325,  17359,  -11782, 6117,   227,    10414,  24890,  -3567,
        -18489, 16804,  -3351,  -3630,  -2801,  10848,  4876,   -15581, 13640,  9893,   -19869,
        -20295, -12158, -21837, 7786,   20531,  22095,  15013,  -11956, -1031,  -20803, -32249,
        21790,  2989,   -11286, -11708, -31852, -8696,  -9088,  29958,  18225,  -21529, -27441,
        -5120,  -28004, 31105,  -10480, -8895,  -1659,  20652,  -14357, 8718,   26055,  20206,
        -4759,  -12682, -32247, -11270, -27719, 13440,  28484,  2566,   -13814, -6659,  -29566,
        3584,   22743,  18418,  -11347, 991,    962,    31827,  12771,  7901,   12833,  -24812,
        27032,  -30250, -19051, -12711, -26301, -20339, -23049, 1565,   -13431, -24795, -27016,
        28458,  -3381,  -25710, -7850,  28800,  -28748, -28345, 20312,  -25984, 1280,   30490,
        18863,  22968,  20786,  -14112, 22040,  -2025,  20999,  30971,  -24300, -24244, 26161,
        -31256, 32612,  -17953};
    PrecisionIntTest(mProgram, 16, int0Range, int1Range, testData, &add);
    deleteShaderAndProgram();
    deleteFramebufferAndRenderbuffer();

    // dEQP-GLES3.functional.shaders.precision.int.medium_add_fragment
    createShaderAndProgram(kIntMediumpAddFragmentVS, kIntMediumpAddFragmentFS);
    createFramebufferAndRenderbuffer();
    testData.clear();
    testData = {
        28081,  16405,  -9585,  26752,  28016,  -32536, -7766,  27790,  23566,  25386,  11716,
        11406,  1811,   482,    16200,  30493,  670,    -28280, -18118, 26004,  9377,   -14497,
        -29788, 10187,  -25536, 22955,  -11703, 14011,  -26596, -21823, 982,    4562,   -28226,
        -13008, 29633,  -18098, -17578, -18031, 8663,   -31742, 362,    -29459, 986,    -25265,
        30157,  -16560, -5320,  -18454, 14695,  27057,  19917,  29734,  -5798,  -20093, 20463,
        -6471,  19343,  -8559,  -24499, -26440, -16177, -14935, -30142, -14831, -2680,  -21718,
        28889,  -7034,  17284,  11780,  -27485, 1118,   30942,  -23105, 29188,  7685,   -25716,
        -11091, -12278, 27148,  -9598,  -14538, -3028,  -2219,  20982,  2342,   19270,  30228,
        6479,   15040,  17328,  -9940,  25706,  26215,  -8576,  29588,  21741,  3451,   -22424,
        18990,  10687,  30116,  21467,  -23163, 2990,   -26745, 22425,  20683,  10930,  -12802,
        -20655, -13778, -27526, 6826,   10674,  -23983, 19356,  15879,  -3298,  -12752, 28941,
        -31692, 13355,  -12832, 2950,   -25154, 27717,  -10421, 31596,  -27385, 13192,  23453,
        -13459, 8591,   -10449, 5310,   13853,  15679,  23101,  -3452,  -20533, 639,    -3553,
        17668,  15391,  10589,  19568,  -19058, 15031,  -980,   12946,  -15588, 9149,   -11083,
        23242,  21067,  -8999,  -139,   23616,  27155,  9574,   -17844, 2903,   -4896,  1793,
        -17871, 26827,  -5739,  29104,  -6363,  -26125, 24124,  161,    6756,   -26014, -21096,
        22232,  -30374, -28015, 16575,  7788,   -26257, 22613,  -25941, -28529, -18394, -10862,
        -425,   12833,  32314,  25066,  -15222, 19481,  -22909, -27614, -4032,  -797,   8639,
        29366,  30038,  -2878,  -10080, -32682, -1691,  1297,   -27959, 8777,   9779,   450,
        -6905,  26512,  5471,   8842,   -7542,  -19075, 23657,  31017,  20055,  330,    -19694,
        -5670,  4245,   -3307,  21909,  -9552,  -19368, 32340,  -28887, -23208, -4111,  -2275,
        5091,   -7223,  24172,  -6949,  21604,  -4257,  5258,   14870,  -20535, 27231,  -15837,
        24949,  -26510, 24290,  20636,  23739,  -6641,  -6990,  -30882, 626,    -17556, 32219,
        12719,  22920,  17267};
    PrecisionIntTest(mProgram, 16, int0Range, int1Range, testData, &add);
    deleteShaderAndProgram();
    deleteFramebufferAndRenderbuffer();

    // dEQP-GLES3.functional.shaders.precision.int.mediump_sub_vertex
    createShaderAndProgram(kIntMediumpSubVertexVS, kIntMediumpSubVertexFS);
    createFramebufferAndRenderbuffer();
    testData.clear();
    testData = {
        564,    23539,  -9882,  28438,  25224,  -17378, -28189, -1033,  29409,  -8494,  -17650,
        19013,  11374,  -17330, -12219, -3078,  -18002, 28611,  19700,  19964,  -17441, 31599,
        -4198,  -26178, -16811, -29975, -1776,  24600,  -32148, 29981,  -22810, 15859,  -31080,
        -21961, -8062,  3601,   -25150, 32340,  10230,  14738,  20792,  31085,  -13442, 8187,
        31278,  -21320, -20466, -4420,  -8079,  -41,    20773,  -5243,  -27774, 21383,  -19050,
        17601,  31287,  31292,  -12709, -25453, 8838,   23646,  -28170, -1530,  -27235, 28221,
        27400,  -30521, 19671,  9558,   -15167, 10097,  -3348,  26616,  12988,  -8410,  -4222,
        -23759, 22163,  3048,   28220,  -23150, -14843, 7296,   -12865, 4689,   30085,  31697,
        -2300,  -31480, -23251, -429,   8441,   -20122, 15286,  -7612,  7959,   4804,   -4029,
        -24418, -8253,  11255,  809,    -21706, 8102,   -2978,  -19733, -10529, 32618,  -3705,
        -10988, 14289,  -10157, -31205, 28953,  -8983,  -7285,  1401,   17816,  15438,  -24452,
        -14441, -18536, -26757, -23925, -5823,  -24242, -20774, -15781, -3965,  -17055, -18272,
        -4918,  -4823,  229,    -16838, -17896, 11936,  -16713, 510,    -28044, -25983, 6062,
        13969,  -12784, -7370,  9933,   -28178, -26287, -13506, 22379,  -19043, -22464, -2479,
        4139,   -24752, 22579,  -7134,  32157,  17838,  8992,   -27650, -31282, -1683,  5232,
        -17960, 15731,  1453,   2191,   -19759, 32456,  -24616, -15308, -27347, 21855,  -2841,
        -19956, 16708,  14128,  3060,   3204,   -11998, 14589,  13616,  -1857,  -3089,  -4382,
        18934,  -27957, 6776,   -24712, -18657, -12882, -16897, -15574, 9847,   -7819,  22499,
        13877,  -30622, 14292,  11058,  30966,  30692,  -31674, -27312, 16801,  14410,  -14217,
        -30734, -28798, -14022, 27473,  8153,   -18247, 7153,   18843,  7258,   32550,  29227,
        20876,  22591,  7131,   22876,  20661,  -14839, 29669,  14445,  -9123,  17878,  30282,
        -31137, -19886, -1082,  -24458, 19932,  -15934, -20806, 29060,  20377,  -24365, -6423,
        -14813, 25392,  11733,  -10785, -14192, -15066, -11410, 16627,  -4312,  29530,  -24072,
        -5603,  -22014, 32032};
    PrecisionIntTest(mProgram, 16, int0Range, int1Range, testData, &sub);
    deleteShaderAndProgram();
    deleteFramebufferAndRenderbuffer();

    // dEQP-GLES3.functional.shaders.precision.int.mediump_sub_fragment
    createShaderAndProgram(kIntMediumpSubFragmentVS, kIntMediumpSubFragmentFS);
    createFramebufferAndRenderbuffer();
    testData.clear();
    testData = {
        40,     23155,  9201,   10600,  5395,   6108,   -2894,  -12455, -27109, -7534,  -4154,
        -2030,  -26924, 28572,  7489,   -19584, -2387,  -20925, 67,     -8318,  -29919, 32332,
        27257,  23223,  -11830, -14011, -21926, -4037,  -28979, -26923, -32689, 20076,  -22993,
        30092,  -28841, 7878,   8667,   25166,  10297,  7304,   5395,   10695,  -15901, 12988,
        -14564, -7058,  -2264,  -16274, 32628,  -16933, 32144,  19520,  16653,  -3229,  1214,
        -28053, -21461, -9395,  21435,  -17281, -10453, -15089, 30614,  6185,   31274,  10301,
        -1630,  -12304, -25987, -4652,  3122,   -13065, 23244,  -31826, 11177,  5375,   -18127,
        -28679, -31609, -27943, 18085,  16726,  12291,  -30410, 10660,  9538,   -21741, -31008,
        17787,  3419,   1655,   -27811, 5465,   23098,  10011,  22152,  -11306, 19803,  -19677,
        23480,  -9847,  23079,  -13810, 12591,  -7944,  -13360, -23306, -31294, 16544,  21012,
        25473,  -26790, 11794,  31032,  -5837,  -14017, -68,    4350,   14824,  -19109, -15449,
        -32300, 16754,  -7179,  -7963,  -2564,  9732,   -30976, -20173, 19914,  11112,  -7980,
        19043,  -20828, -10919, 30520,  32689,  -4760,  27788,  16263,  15489,  28301,  -21353,
        -15233, -8241,  -7767,  21546,  -3394,  24014,  30454,  -6655,  14318,  -20823, -31658,
        21208,  1983,   30901,  -772,   2407,   32371,  -19684, 16807,  5257,   -16090, 20532,
        32071,  -15093, 18029,  -14452, 10309,  -31007, 16121,  26108,  14989,  -10804, 16166,
        -24159, -13352, 32011,  -4689,  -4629,  30072,  23654,  21122,  2225,   3489,   -6343,
        -13335, 30970,  3000,   6096,   17186,  -28914, 25223,  7248,   10787,  -2839,  -21915,
        17180,  24764,  32602,  -28496, 14353,  -20778, 2072,   -4374,  -12389, -6571,  24597,
        -23393, 31740,  2253,   -23468, 12146,  -21853, 9719,   -19710, 23341,  -4285,  -5372,
        18973,  -8992,  -18287, 31053,  2236,   -25988, 32048,  20516,  -27900, -8319,  443,
        -8660,  27507,  26830,  8314,   -17687, -9640,  -22223, -9302,  -6760,  23908,  12458,
        -23576, -13859, 9229,   18576,  30344,  -14754, -31182, 2874,   23701,  -19786, -28886,
        1979,   -14406, 1637};
    PrecisionIntTest(mProgram, 16, int0Range, int1Range, testData, &sub);
    deleteShaderAndProgram();
    deleteFramebufferAndRenderbuffer();

    // dEQP-GLES3.functional.shaders.precision.int.mediump_mul_vertex
    createShaderAndProgram(kIntMediumpMulVertexVS, kIntMediumpMulVertexFS);
    createFramebufferAndRenderbuffer();
    testData.clear();
    testData = {
        20609,  -8444,  -12492, 25120,  -17920, 21469,  -3247,  -14823, 6972,   2135,   12789,
        25738,  -23455, -24901, 20065,  -20697, 21584,  -13172, -19018, 6077,   -1635,  24442,
        -21795, 18566,  15031,  25560,  14230,  25348,  30903,  -14089, 5230,   -1538,  -22091,
        20307,  -24451, 2928,   -13072, -8304,  -13510, 11544,  -20416, 24667,  28923,  20608,
        22050,  30974,  -21402, 30353,  -7216,  27375,  9470,   -12300, 25697,  -3326,  -19934,
        3319,   8785,   -32516, 16836,  17613,  8147,   12492,  -31276, 13215,  -13106, 32473,
        -22230, 23895,  17195,  -25576, -3084,  6653,   25646,  -18791, 15200,  -29327, -20306,
        5428,   -29965, 17958,  32670,  277,    -8608,  -17524, 3254,   -29117, -11215, 4063,
        4592,   4681,   -19430, -29334, 27774,  -13466, 21823,  -12451, -381,   -2492,  -13722,
        -5086,  -23144, -13712, 2246,   -1080,  -6463,  13771,  24301,  8079,   -15688, 23242,
        -26306, 16830,  29292,  19151,  9880,   20582,  17857,  -21257, 29447,  -8616,  25094,
        -3413,  28312,  -26489, 12433,  27388,  -969,   3807,   -9490,  32192,  15218,  -1099,
        -14329, -13092, 17352,  -8158,  -7480,  20358,  -16656, 13263,  -5191,  -31203, -23172,
        2079,   8019,   -19636, 28971,  -6435,  32731,  19946,  -20561, -27313, 15905,  -6134,
        22454,  577,    -19050, -6629,  -27312, 1292,   -13991, -25312, 1747,   -3681,  -10196,
        -27673, -27652, -13543, -26116, -25231, 32746,  9244,   1241,   -13139, -543,   -27860,
        27221,  26421,  23298,  -9453,  -26460, -7563,  -32701, -32543, 9199,   2428,   -26741,
        4275,   -17965, -1259,  -28361, -17478, 16435,  27391,  -5645,  1539,   6938,   -9911,
        30859,  24082,  8375,   -7619,  -22193, -23636, 9097,   29132,  -28131, -27744, -6623,
        -32295, 25529,  -3882,  -30024, 30277,  -24130, -5633,  7305,   13306,  21481,  -19053,
        -9807,  -11732, 24981,  -28231, -23167, 17292,  -31273, 7899,   -32674, 872,    -1455,
        1784,   13535,  30446,  -7405,  -13380, -7113,  9087,   18968,  25327,  -28507, -23709,
        -5980,  26417,  -15858, 24091,  23379,  7466,   18797,  -26081, 13334,  -9971,  24284,
        -20859, -7373,  -4623};
    PrecisionIntTest(mProgram, 16, int0Range, int1Range, testData, &mul);
    deleteShaderAndProgram();
    deleteFramebufferAndRenderbuffer();

    // dEQP-GLES3.functional.shaders.precision.int.mediump_mul_fragment
    createShaderAndProgram(kIntMediumpMulFragmentVS, kIntMediumpMulFragmentFS);
    createFramebufferAndRenderbuffer();
    testData.clear();
    testData = {
        31348,  -24437, -7489,  29975,  -22432, -7998,  27605,  19914,  -22176, 24413,  30495,
        -29647, -25544, -2374,  -28075, 31110,  1603,   -4728,  -2753,  -7101,  -7558,  27510,
        -9380,  -21826, 23359,  -32621, 7049,   -12396, 14471,  23244,  -28361, 27415,  9491,
        22664,  -17017, -17388, -16110, 1910,   21212,  25756,  4738,   17978,  -18304, 25950,
        -21977, 10764,  -451,   4486,   27585,  -23996, 20377,  -6312,  18030,  -8047,  -18946,
        -27681, -15698, -18677, 11116,  6374,   -14742, -1643,  -27710, 28018,  -24729, 6532,
        26215,  11685,  -8532,  -21151, 16278,  -27448, -13092, -3167,  -3483,  10621,  12316,
        10409,  -8739,  23110,  -8189,  1424,   30913,  3506,   -17448, 20457,  -31021, 25678,
        28627,  -10609, -6104,  -22747, 10515,  -16,    9753,   -24720, -5468,  -19272, -7345,
        9415,   -17747, -496,   -15088, 21782,  19854,  -4907,  -7522,  6250,   26210,  14129,
        -26992, 20079,  -28897, 25927,  -17640, 20182,  29461,  24553,  -13108, -26496, 29653,
        13605,  23624,  24054,  -4263,  -16969, -2554,  27410,  6801,   23806,  -9294,  5921,
        10718,  -25415, 19882,  -28983, 9066,   18222,  23366,  17289,  -1172,  -25590, -22567,
        -10958, -3062,  -17526, 11267,  15756,  -28734, -15625, -15353, -7484,  12881,  -3889,
        9515,   10467,  4859,   24677,  -15358, -23296, -21974, -296,   6538,   -11079, 7235,
        24822,  -10239, -18671, 17684,  -18639, -32581, -27217, -8563,  1548,   7064,   17925,
        -1370,  -28900, -7433,  -2581,  -7636,  10182,  20678,  -25429, 16742,  -24649, -19311,
        4108,   -5880,  -20524, 10303,  -15031, -8451,  6799,   18234,  -15723, -12041, 9420,
        18684,  -2655,  13284,  8370,   21138,  16975,  -23784, 8536,   -8169,  20401,  -16923,
        13459,  27183,  11138,  -21174, -16967, -22903, -25193, -4077,  -24691, -5117,  12124,
        -14496, 7413,   24704,  28813,  25301,  -19390, -23558, 18626,  -24334, -6771,  26230,
        -30825, -8108,  17761,  -144,   27240,  3678,   28824,  -5756,  643,    16693,  30703,
        16435,  27550,  -31122, 10265,  -11888, -6252,  5207,   8427,   25140,  18997,  -3663,
        -5883,  16361,  -630};
    PrecisionIntTest(mProgram, 16, int0Range, int1Range, testData, &mul);
    deleteShaderAndProgram();
    deleteFramebufferAndRenderbuffer();

    // dEQP-GLES3.functional.shaders.precision.int.mediump_div_vertex
    createShaderAndProgram(kIntMediumpDivVertexVS, kIntMediumpDivVertexFS);
    createFramebufferAndRenderbuffer();
    int1Range.x() = 0;
    int1Range.y() = 1000;
    testData.clear();
    testData = {
        23695,  867, -12269, 703, 25117,  695, -19185, 456, -20599, 122, 12794,  996, -18375, 406,
        28418,  322, 26513,  949, 18368,  787, -8713,  219, -32581, 503, -14107, 894, 20771,  133,
        26592,  835, 8496,   241, 11052,  248, -26632, 858, -6935,  184, -15202, 963, 5599,   672,
        28449,  126, -23477, 524, -31058, 290, -20666, 133, 1895,   792, 30664,  695, -16187, 326,
        31492,  655, 20317,  627, 2266,   268, 14689,  378, -592,   646, 11632,  553, -22131, 409,
        24220,  317, 23689,  859, -517,   67,  14027,  250, 31513,  758, -4882,  903, 19203,  726,
        -29628, 305, -10551, 831, 18495,  928, -5889,  291, 9295,   216, 9662,   469, -29972, 185,
        3965,   215, -390,   41,  -24898, 500, -8859,  440, 9664,   702, 19312,  600, -14166, 194,
        -28218, 352, 29955,  339, 14284,  872, -15058, 749, -5425,  496, 26407,  968, -20452, 392,
        29601,  405, 4556,   52,  22506,  924, -13589, 888, -9476,  448, 8331,   6,   -11527, 482,
        -13602, 851, 4001,   83,  15853,  683, -958,   146, 1709,   796, -15200, 228, -32579, 803,
        29704,  488, -16221, 626, -17757, 541, -29118, 763, -1189,  101, -16375, 563, -28434, 856,
        -7476,  182, 3096,   33,  21119,  466, -12908, 31,  18164,  999, -6896,  247, -23531, 194,
        -9283,  295, -17661, 310, -30066, 465, -626,   150, 31769,  695, -14739, 857, 10955,  667,
        -11709, 795, -10337, 63,  -23397, 204, -245,   547, 17945,  895, -29998, 891, -24086, 951,
        14781,  584, 31124,  712, -7939,  493, -5678,  962, 15120,  357, 2384,   805, -9847,  517,
        10242,  66,  258,    289, 384,    350, -428,   230, 23866,  514, -20513, 931, -2330,  366,
        19707,  144, 26821,  102, 2593,   685, -11988, 95,  -22099, 336, 24098,  362, -27333, 292,
        -8612,  726, 28727,  28};
    PrecisionIntTest(mProgram, 16, int0Range, int1Range, testData, &div);
    deleteShaderAndProgram();
    deleteFramebufferAndRenderbuffer();

    // dEQP-GLES3.functional.shaders.precision.int.mediump_div_fragment
    createShaderAndProgram(kIntMediumpDivFragmentVS, kIntMediumpDivFragmentFS);
    createFramebufferAndRenderbuffer();
    testData.clear();
    testData = {
        25514,  721, 6786,   953, 1348,   660, -4651,  904,  19187,  626, 30139,  342, -20696, 138,
        20756,  901, 23399,  823, 5506,   393, -5093,  984,  -183,   990, -15709, 216, 14628,  324,
        -14871, 749, -21447, 754, -17648, 388, -24671, 903,  -1817,  801, 26283,  945, 10100,  110,
        15728,  483, 29471,  167, -14717, 175, 10853,  321,  -23667, 60,  10533,  833, -17924, 850,
        -25263, 36,  -24941, 687, 22319,  489, 2063,   334,  8532,   360, 14897,  528, -6400,  85,
        -18690, 184, -31622, 78,  -11501, 960, -27302, 716,  -13869, 60,  -29298, 114, -27319, 190,
        -10902, 876, -6146,  410, 22194,  40,  -30230, 32,   27934,  726, -24240, 933, -32229, 224,
        28013,  649, -9470,  415, -10747, 614, 8399,   872,  -27734, 943, 12568,  220, -1126,  59,
        -3234,  334, -3816,  688, -10753, 847, -20468, 668,  -14705, 532, 26560,  285, 11880,  909,
        30603,  857, -27283, 224, -12410, 268, 9647,   12,   -13547, 843, 20803,  964, -14323, 400,
        11941,  997, -15570, 965, -9658,  547, -3612,  351,  -27107, 488, -8425,  611, 26172,  276,
        3352,   712, 29687,  717, -10033, 402, 5720,   576,  -18407, 325, -12943, 338, 5589,   301,
        -26903, 606, -28166, 573, 14240,  923, 5222,   113,  25352,  625, -26223, 321, 11128,  807,
        30883,  169, -9490,  971, 23807,  135, -16555, 877,  26537,  329, 19397,  148, 7003,   548,
        5303,   888, -10380, 762, -584,   217, -20317, 1000, -29056, 930, 15456,  531, -14317, 515,
        -22424, 441, 30770,  836, 23439,  889, 11502,  587,  -10923, 8,   26065,  43,  16331,  621,
        28967,  436, -14485, 30,  -28748, 327, -24689, 239,  -6807,  170, 27290,  157, -25067, 550,
        15304,  416, 32220,  224, 15473,  446, -24211, 748,  2949,   488, 19884,  395, -3648,  477,
        -2700,  760, -31892, 748};
    PrecisionIntTest(mProgram, 16, int0Range, int1Range, testData, &div);
    deleteShaderAndProgram();
    deleteFramebufferAndRenderbuffer();

    // dEQP-GLES3.functional.shaders.precision.int.lowp_add_vertex
    createShaderAndProgram(kIntLowpAddVertexVS, kIntLowpAddVertexFS);
    createFramebufferAndRenderbuffer();
    int0Range.x() = -(1 << 7);
    int0Range.y() = (1 << 7) - 1;
    int1Range.x() = -(1 << 7);
    int1Range.y() = (1 << 7) - 1;
    testData.clear();
    testData = {-33,  23,   117,  95,  3,    -57,  14,  -25,  -56, 3,    69,   33,   -3,   -101,
                112,  114,  -49,  119, -99,  -35,  -33, 114,  99,  65,   -35,  117,  -25,  -66,
                0,    46,   122,  -83, -121, -117, 39,  53,   -96, 62,   51,   46,   -79,  106,
                -61,  124,  91,   -9,  -91,  70,   102, 79,   -70, -89,  -46,  -42,  -87,  -95,
                26,   -107, 106,  41,  105,  -101, 40,  -34,  80,  -21,  -37,  83,   -52,  -69,
                -5,   51,   -47,  -57, -31,  106,  -25, 14,   95,  44,   96,   -91,  65,   30,
                70,   -7,   -97,  25,  87,   -17,  -63, 59,   -31, 72,   -36,  -112, -97,  107,
                88,   -84,  -39,  -74, 68,   -120, 102, -27,  58,  -18,  111,  -38,  -83,  74,
                48,   -48,  17,   123, 126,  -1,   -60, -127, -69, -9,   116,  120,  -120, 101,
                -7,   -114, -114, -91, -65,  51,   117, -101, -34, 74,   86,   119,  31,   17,
                33,   22,   14,   -33, 43,   -3,   20,  -4,   74,  -127, -103, -11,  4,    47,
                -116, -16,  43,   78,  98,   82,   -4,  62,   97,  -38,  -10,  -126, -19,  1,
                -63,  -121, -99,  38,  -37,  -18,  -57, 111,  -38, 71,   -30,  -108, 116,  40,
                65,   -93,  -110, 68,  -102, 106,  19,  -69,  67,  -76,  -123, -115, -59,  -43,
                -48,  10,   62,   82,  41,   125,  1,   105,  13,  -11,  103,  44,   -10,  62,
                -50,  -65,  75,   -31, -99,  14,   31,  -4,   -73, 32,   -96,  -76,  104,  68,
                -17,  -75,  -103, -34, 76,   -94,  119, 123,  -1,  -8,   -19,  -5,   -24,  -84,
                -106, 9,    61,   -97, 48,   -51,  47,  -77,  -24, 65,   19,   -100, -51,  34,
                81,   -39,  15,   82};
    PrecisionIntTest(mProgram, 16, int0Range, int1Range, testData, &add);
    deleteShaderAndProgram();
    deleteFramebufferAndRenderbuffer();
}

ANGLE_INSTANTIATE_TEST_ES3(ShaderPrecisionTest);
}  // namespace