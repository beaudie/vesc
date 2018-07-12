//
// Copyright 2015 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//

#include "test_utils/ANGLETest.h"
#include "test_utils/gl_raii.h"

using namespace angle;

namespace
{

GLsizei TypeStride(GLenum attribType)
{
    switch (attribType)
    {
        case GL_UNSIGNED_BYTE:
        case GL_BYTE:
            return 1;
        case GL_UNSIGNED_SHORT:
        case GL_SHORT:
            return 2;
        case GL_UNSIGNED_INT:
        case GL_INT:
        case GL_FLOAT:
            return 4;
        default:
            EXPECT_TRUE(false);
            return 0;
    }
}

template <typename T>
GLfloat Normalize(T value)
{
    static_assert(std::is_integral<T>::value, "Integer required.");
    if (std::is_signed<T>::value)
    {
        typedef typename std::make_unsigned<T>::type unsigned_type;
        return (2.0f * static_cast<GLfloat>(value) + 1.0f) /
               static_cast<GLfloat>(std::numeric_limits<unsigned_type>::max());
    }
    else
    {
        return static_cast<GLfloat>(value) / static_cast<GLfloat>(std::numeric_limits<T>::max());
    }
}

class VertexAttributeTest : public ANGLETest
{
  protected:
    VertexAttributeTest()
        : mProgram(0), mTestAttrib(-1), mExpectedAttrib(-1), mBuffer(0), mQuadBuffer(0)
    {
        setWindowWidth(128);
        setWindowHeight(128);
        setConfigRedBits(8);
        setConfigGreenBits(8);
        setConfigBlueBits(8);
        setConfigAlphaBits(8);
        setConfigDepthBits(24);
    }

    enum class Source
    {
        BUFFER,
        IMMEDIATE,
    };

    struct TestData final : private angle::NonCopyable
    {
        TestData(GLenum typeIn,
                 GLboolean normalizedIn,
                 Source sourceIn,
                 const void *inputDataIn,
                 const GLfloat *expectedDataIn)
            : type(typeIn),
              normalized(normalizedIn),
              bufferOffset(0),
              source(sourceIn),
              inputData(inputDataIn),
              expectedData(expectedDataIn)
        {
        }

        GLenum type;
        GLboolean normalized;
        size_t bufferOffset;
        Source source;

        const void *inputData;
        const GLfloat *expectedData;
    };

    void setupTest(const TestData &test, GLint typeSize)
    {
        if (mProgram == 0)
        {
            initBasicProgram();
        }

        if (test.source == Source::BUFFER)
        {
            GLsizei dataSize = kVertexCount * TypeStride(test.type);
            glBindBuffer(GL_ARRAY_BUFFER, mBuffer);
            glBufferData(GL_ARRAY_BUFFER, dataSize, test.inputData, GL_STATIC_DRAW);
            glVertexAttribPointer(mTestAttrib, typeSize, test.type, test.normalized, 0,
                                  reinterpret_cast<void *>(test.bufferOffset));
            glBindBuffer(GL_ARRAY_BUFFER, 0);
        }
        else
        {
            ASSERT_EQ(Source::IMMEDIATE, test.source);
            glBindBuffer(GL_ARRAY_BUFFER, 0);
            glVertexAttribPointer(mTestAttrib, typeSize, test.type, test.normalized, 0,
                                  test.inputData);
        }

        glVertexAttribPointer(mExpectedAttrib, typeSize, GL_FLOAT, GL_FALSE, 0, test.expectedData);

        glEnableVertexAttribArray(mTestAttrib);
        glEnableVertexAttribArray(mExpectedAttrib);
    }

    void checkPixels()
    {
        GLint viewportSize[4];
        glGetIntegerv(GL_VIEWPORT, viewportSize);

        GLint midPixelX = (viewportSize[0] + viewportSize[2]) / 2;
        GLint midPixelY = (viewportSize[1] + viewportSize[3]) / 2;

        // We need to offset our checks from triangle edges to ensure we don't fall on a single tri
        // Avoid making assumptions of drawQuad with four checks to check the four possible tri
        // regions
        EXPECT_PIXEL_EQ((midPixelX + viewportSize[0]) / 2, midPixelY, 255, 255, 255, 255);
        EXPECT_PIXEL_EQ((midPixelX + viewportSize[2]) / 2, midPixelY, 255, 255, 255, 255);
        EXPECT_PIXEL_EQ(midPixelX, (midPixelY + viewportSize[1]) / 2, 255, 255, 255, 255);
        EXPECT_PIXEL_EQ(midPixelX, (midPixelY + viewportSize[3]) / 2, 255, 255, 255, 255);
    }

    void checkPixelsUnEqual()
    {
        GLint viewportSize[4];
        glGetIntegerv(GL_VIEWPORT, viewportSize);

        GLint midPixelX = (viewportSize[0] + viewportSize[2]) / 2;
        GLint midPixelY = (viewportSize[1] + viewportSize[3]) / 2;

        // We need to offset our checks from triangle edges to ensure we don't fall on a single tri
        // Avoid making assumptions of drawQuad with four checks to check the four possible tri
        // regions
        EXPECT_PIXEL_NE((midPixelX + viewportSize[0]) / 2, midPixelY, 255, 255, 255, 255);
        EXPECT_PIXEL_NE((midPixelX + viewportSize[2]) / 2, midPixelY, 255, 255, 255, 255);
        EXPECT_PIXEL_NE(midPixelX, (midPixelY + viewportSize[1]) / 2, 255, 255, 255, 255);
        EXPECT_PIXEL_NE(midPixelX, (midPixelY + viewportSize[3]) / 2, 255, 255, 255, 255);
    }

    void runTest(const TestData &test) { runTest(test, true); }

    void runTest(const TestData &test, bool checkPixelEqual)
    {
        // TODO(geofflang): Figure out why this is broken on AMD OpenGL
        ANGLE_SKIP_TEST_IF(IsAMD() && IsOpenGL());

        for (GLint i = 0; i < 4; i++)
        {
            GLint typeSize = i + 1;
            setupTest(test, typeSize);

            drawQuad(mProgram, "position", 0.5f);

            glDisableVertexAttribArray(mTestAttrib);
            glDisableVertexAttribArray(mExpectedAttrib);

            if (checkPixelEqual)
            {
                checkPixels();
            }
            else
            {
                checkPixelsUnEqual();
            }
        }
    }

    void SetUp() override
    {
        ANGLETest::SetUp();

        glClearColor(0, 0, 0, 0);
        glClearDepthf(0.0);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        glDisable(GL_DEPTH_TEST);

        glGenBuffers(1, &mBuffer);
    }

    void TearDown() override
    {
        glDeleteProgram(mProgram);
        glDeleteBuffers(1, &mBuffer);
        glDeleteBuffers(1, &mQuadBuffer);

        ANGLETest::TearDown();
    }

    GLuint compileMultiAttribProgram(GLint attribCount)
    {
        std::stringstream shaderStream;

        shaderStream << "attribute mediump vec4 position;" << std::endl;
        for (GLint attribIndex = 0; attribIndex < attribCount; ++attribIndex)
        {
            shaderStream << "attribute float a" << attribIndex << ";" << std::endl;
        }
        shaderStream << "varying mediump float color;" << std::endl
                     << "void main() {" << std::endl
                     << "  gl_Position = position;" << std::endl
                     << "  color = 0.0;" << std::endl;
        for (GLint attribIndex = 0; attribIndex < attribCount; ++attribIndex)
        {
            shaderStream << "  color += a" << attribIndex << ";" << std::endl;
        }
        shaderStream << "}" << std::endl;

        const std::string testFragmentShaderSource =
            "varying mediump float color;\n"
            "void main(void)\n"
            "{\n"
            "    gl_FragColor = vec4(color, 0.0, 0.0, 1.0);\n"
            "}\n";

        return CompileProgram(shaderStream.str(), testFragmentShaderSource);
    }

    void setupMultiAttribs(GLuint program, GLint attribCount, GLfloat value)
    {
        glUseProgram(program);
        for (GLint attribIndex = 0; attribIndex < attribCount; ++attribIndex)
        {
            std::stringstream attribStream;
            attribStream << "a" << attribIndex;
            GLint location = glGetAttribLocation(program, attribStream.str().c_str());
            ASSERT_NE(-1, location);
            glVertexAttrib1f(location, value);
            glDisableVertexAttribArray(location);
        }
    }

    void initBasicProgram()
    {
        const std::string testVertexShaderSource =
            "attribute mediump vec4 position;\n"
            "attribute mediump vec4 test;\n"
            "attribute mediump vec4 expected;\n"
            "varying mediump vec4 color;\n"
            "void main(void)\n"
            "{\n"
            "    gl_Position = position;\n"
            "    vec4 threshold = max(abs(expected) * 0.01, 1.0 / 64.0);\n"
            "    color = vec4(lessThanEqual(abs(test - expected), threshold));\n"
            "}\n";

        const std::string testFragmentShaderSource =
            "varying mediump vec4 color;\n"
            "void main(void)\n"
            "{\n"
            "    gl_FragColor = color;\n"
            "}\n";

        mProgram = CompileProgram(testVertexShaderSource, testFragmentShaderSource);
        ASSERT_NE(0u, mProgram);

        mTestAttrib = glGetAttribLocation(mProgram, "test");
        ASSERT_NE(-1, mTestAttrib);
        mExpectedAttrib = glGetAttribLocation(mProgram, "expected");
        ASSERT_NE(-1, mExpectedAttrib);

        glUseProgram(mProgram);
    }

    static constexpr size_t kVertexCount = 24;

    static void InitTestData(std::array<GLfloat, kVertexCount> &inputData,
                             std::array<GLfloat, kVertexCount> &expectedData)
    {
        for (size_t count = 0; count < kVertexCount; ++count)
        {
            inputData[count]    = static_cast<GLfloat>(count);
            expectedData[count] = inputData[count];
        }
    }

    GLuint mProgram;
    GLint mTestAttrib;
    GLint mExpectedAttrib;
    GLuint mBuffer;
    GLuint mQuadBuffer;
};

TEST_P(VertexAttributeTest, UnsignedByteUnnormalized)
{
    std::array<GLubyte, kVertexCount> inputData = {
        {0, 1, 2, 3, 4, 5, 6, 7, 125, 126, 127, 128, 129, 250, 251, 252, 253, 254, 255}};
    std::array<GLfloat, kVertexCount> expectedData;
    for (size_t i = 0; i < kVertexCount; i++)
    {
        expectedData[i] = inputData[i];
    }

    TestData data(GL_UNSIGNED_BYTE, GL_FALSE, Source::IMMEDIATE, inputData.data(),
                  expectedData.data());
    runTest(data);
}

TEST_P(VertexAttributeTest, UnsignedByteNormalized)
{
    std::array<GLubyte, kVertexCount> inputData = {
        {0, 1, 2, 3, 4, 5, 6, 7, 125, 126, 127, 128, 129, 250, 251, 252, 253, 254, 255}};
    std::array<GLfloat, kVertexCount> expectedData;
    for (size_t i = 0; i < kVertexCount; i++)
    {
        expectedData[i] = Normalize(inputData[i]);
    }

    TestData data(GL_UNSIGNED_BYTE, GL_TRUE, Source::IMMEDIATE, inputData.data(),
                  expectedData.data());
    runTest(data);
}

TEST_P(VertexAttributeTest, ByteUnnormalized)
{
    std::array<GLbyte, kVertexCount> inputData = {
        {0, 1, 2, 3, 4, -1, -2, -3, -4, 125, 126, 127, -128, -127, -126}};
    std::array<GLfloat, kVertexCount> expectedData;
    for (size_t i = 0; i < kVertexCount; i++)
    {
        expectedData[i] = inputData[i];
    }

    TestData data(GL_BYTE, GL_FALSE, Source::IMMEDIATE, inputData.data(), expectedData.data());
    runTest(data);
}

TEST_P(VertexAttributeTest, ByteNormalized)
{
    std::array<GLbyte, kVertexCount> inputData = {
        {0, 1, 2, 3, 4, -1, -2, -3, -4, 125, 126, 127, -128, -127, -126}};
    std::array<GLfloat, kVertexCount> expectedData;
    for (size_t i = 0; i < kVertexCount; i++)
    {
        expectedData[i] = Normalize(inputData[i]);
    }

    TestData data(GL_BYTE, GL_TRUE, Source::IMMEDIATE, inputData.data(), expectedData.data());
    runTest(data);
}

TEST_P(VertexAttributeTest, UnsignedShortUnnormalized)
{
    std::array<GLushort, kVertexCount> inputData = {
        {0, 1, 2, 3, 254, 255, 256, 32766, 32767, 32768, 65533, 65534, 65535}};
    std::array<GLfloat, kVertexCount> expectedData;
    for (size_t i = 0; i < kVertexCount; i++)
    {
        expectedData[i] = inputData[i];
    }

    TestData data(GL_UNSIGNED_SHORT, GL_FALSE, Source::IMMEDIATE, inputData.data(),
                  expectedData.data());
    runTest(data);
}

TEST_P(VertexAttributeTest, UnsignedShortNormalized)
{
    std::array<GLushort, kVertexCount> inputData = {
        {0, 1, 2, 3, 254, 255, 256, 32766, 32767, 32768, 65533, 65534, 65535}};
    std::array<GLfloat, kVertexCount> expectedData;
    for (size_t i = 0; i < kVertexCount; i++)
    {
        expectedData[i] = Normalize(inputData[i]);
    }

    TestData data(GL_UNSIGNED_SHORT, GL_TRUE, Source::IMMEDIATE, inputData.data(),
                  expectedData.data());
    runTest(data);
}

TEST_P(VertexAttributeTest, ShortUnnormalized)
{
    std::array<GLshort, kVertexCount> inputData = {
        {0, 1, 2, 3, -1, -2, -3, -4, 32766, 32767, -32768, -32767, -32766}};
    std::array<GLfloat, kVertexCount> expectedData;
    for (size_t i = 0; i < kVertexCount; i++)
    {
        expectedData[i] = inputData[i];
    }

    TestData data(GL_SHORT, GL_FALSE, Source::IMMEDIATE, inputData.data(), expectedData.data());
    runTest(data);
}

TEST_P(VertexAttributeTest, ShortNormalized)
{
    std::array<GLshort, kVertexCount> inputData = {
        {0, 1, 2, 3, -1, -2, -3, -4, 32766, 32767, -32768, -32767, -32766}};
    std::array<GLfloat, kVertexCount> expectedData;
    for (size_t i = 0; i < kVertexCount; i++)
    {
        expectedData[i] = Normalize(inputData[i]);
    }

    TestData data(GL_SHORT, GL_TRUE, Source::IMMEDIATE, inputData.data(), expectedData.data());
    runTest(data);
}

// Verify that using the same client memory pointer in different format won't mess up the draw.
TEST_P(VertexAttributeTest, UsingDifferentFormatAndSameClientMemoryPointer)
{
    std::array<GLshort, kVertexCount> inputData = {
        {0, 1, 2, 3, -1, -2, -3, -4, 32766, 32767, -32768, -32767, -32766}};

    std::array<GLfloat, kVertexCount> unnormalizedExpectedData;
    for (size_t i = 0; i < kVertexCount; i++)
    {
        unnormalizedExpectedData[i] = inputData[i];
    }

    TestData unnormalizedData(GL_SHORT, GL_FALSE, Source::IMMEDIATE, inputData.data(),
                              unnormalizedExpectedData.data());
    runTest(unnormalizedData);

    std::array<GLfloat, kVertexCount> normalizedExpectedData;
    for (size_t i = 0; i < kVertexCount; i++)
    {
        inputData[i]              = -inputData[i];
        normalizedExpectedData[i] = Normalize(inputData[i]);
    }

    TestData normalizedData(GL_SHORT, GL_TRUE, Source::IMMEDIATE, inputData.data(),
                            normalizedExpectedData.data());
    runTest(normalizedData);
}

// Verify that vertex format is updated correctly when the client memory pointer is same.
TEST_P(VertexAttributeTest, NegativeUsingDifferentFormatAndSameClientMemoryPointer)
{
    std::array<GLshort, kVertexCount> inputData = {
        {0, 1, 2, 3, -1, -2, -3, -4, 32766, 32767, -32768, -32767, -32766}};

    std::array<GLfloat, kVertexCount> unnormalizedExpectedData;
    for (size_t i = 0; i < kVertexCount; i++)
    {
        unnormalizedExpectedData[i] = inputData[i];
    }

    // Use unnormalized short as the format of the data in client memory pointer in the first draw.
    TestData unnormalizedData(GL_SHORT, GL_FALSE, Source::IMMEDIATE, inputData.data(),
                              unnormalizedExpectedData.data());
    runTest(unnormalizedData);

    // Use normalized short as the format of the data in client memory pointer in the second draw,
    // but mExpectedAttrib is the same as the first draw.
    TestData normalizedData(GL_SHORT, GL_TRUE, Source::IMMEDIATE, inputData.data(),
                            unnormalizedExpectedData.data());
    runTest(normalizedData, false);
}

// Verify that using different vertex format and same buffer won't mess up the draw.
TEST_P(VertexAttributeTest, UsingDifferentFormatAndSameBuffer)
{
    std::array<GLshort, kVertexCount> inputData = {
        {0, 1, 2, 3, -1, -2, -3, -4, 32766, 32767, -32768, -32767, -32766}};

    std::array<GLfloat, kVertexCount> unnormalizedExpectedData;
    std::array<GLfloat, kVertexCount> normalizedExpectedData;
    for (size_t i = 0; i < kVertexCount; i++)
    {
        unnormalizedExpectedData[i] = inputData[i];
        normalizedExpectedData[i]   = Normalize(inputData[i]);
    }

    // Use unnormalized short as the format of the data in mBuffer in the first draw.
    TestData unnormalizedData(GL_SHORT, GL_FALSE, Source::BUFFER, inputData.data(),
                              unnormalizedExpectedData.data());
    runTest(unnormalizedData);

    // Use normalized short as the format of the data in mBuffer in the second draw.
    TestData normalizedData(GL_SHORT, GL_TRUE, Source::BUFFER, inputData.data(),
                            normalizedExpectedData.data());
    runTest(normalizedData);
}

// Verify that vertex format is updated correctly when the buffer is same.
TEST_P(VertexAttributeTest, NegativeUsingDifferentFormatAndSameBuffer)
{
    std::array<GLshort, kVertexCount> inputData = {
        {0, 1, 2, 3, -1, -2, -3, -4, 32766, 32767, -32768, -32767, -32766}};

    std::array<GLfloat, kVertexCount> unnormalizedExpectedData;
    for (size_t i = 0; i < kVertexCount; i++)
    {
        unnormalizedExpectedData[i] = inputData[i];
    }

    // Use unnormalized short as the format of the data in mBuffer in the first draw.
    TestData unnormalizedData(GL_SHORT, GL_FALSE, Source::BUFFER, inputData.data(),
                              unnormalizedExpectedData.data());
    runTest(unnormalizedData);

    // Use normalized short as the format of the data in mBuffer in the second draw, but
    // mExpectedAttrib is the same as the first draw.
    TestData normalizedData(GL_SHORT, GL_TRUE, Source::BUFFER, inputData.data(),
                            unnormalizedExpectedData.data());

    // The check should fail because the test data is changed while the expected data is the same.
    runTest(normalizedData, false);
}

// Verify that mixed using buffer and client memory pointer won't mess up the draw.
TEST_P(VertexAttributeTest, MixedUsingBufferAndClientMemoryPointer)
{
    std::array<GLshort, kVertexCount> inputData = {
        {0, 1, 2, 3, -1, -2, -3, -4, 32766, 32767, -32768, -32767, -32766}};

    std::array<GLfloat, kVertexCount> unnormalizedExpectedData;
    std::array<GLfloat, kVertexCount> normalizedExpectedData;
    for (size_t i = 0; i < kVertexCount; i++)
    {
        unnormalizedExpectedData[i] = inputData[i];
        normalizedExpectedData[i]   = Normalize(inputData[i]);
    }

    TestData unnormalizedData(GL_SHORT, GL_FALSE, Source::IMMEDIATE, inputData.data(),
                              unnormalizedExpectedData.data());
    runTest(unnormalizedData);

    TestData unnormalizedBufferData(GL_SHORT, GL_FALSE, Source::BUFFER, inputData.data(),
                                    unnormalizedExpectedData.data());
    runTest(unnormalizedBufferData);

    TestData normalizedData(GL_SHORT, GL_TRUE, Source::IMMEDIATE, inputData.data(),
                            normalizedExpectedData.data());
    runTest(normalizedData);
}

class VertexAttributeTestES3 : public VertexAttributeTest
{
  protected:
    VertexAttributeTestES3() {}
};

TEST_P(VertexAttributeTestES3, IntUnnormalized)
{
    GLint lo = std::numeric_limits<GLint>::min();
    GLint hi = std::numeric_limits<GLint>::max();
    std::array<GLint, kVertexCount> inputData = {
        {0, 1, 2, 3, -1, -2, -3, -4, -1, hi, hi - 1, lo, lo + 1}};
    std::array<GLfloat, kVertexCount> expectedData;
    for (size_t i = 0; i < kVertexCount; i++)
    {
        expectedData[i] = static_cast<GLfloat>(inputData[i]);
    }

    TestData data(GL_INT, GL_FALSE, Source::BUFFER, inputData.data(), expectedData.data());
    runTest(data);
}

TEST_P(VertexAttributeTestES3, IntNormalized)
{
    GLint lo = std::numeric_limits<GLint>::min();
    GLint hi = std::numeric_limits<GLint>::max();
    std::array<GLint, kVertexCount> inputData = {
        {0, 1, 2, 3, -1, -2, -3, -4, -1, hi, hi - 1, lo, lo + 1}};
    std::array<GLfloat, kVertexCount> expectedData;
    for (size_t i = 0; i < kVertexCount; i++)
    {
        expectedData[i] = Normalize(inputData[i]);
    }

    TestData data(GL_INT, GL_TRUE, Source::BUFFER, inputData.data(), expectedData.data());
    runTest(data);
}

TEST_P(VertexAttributeTestES3, UnsignedIntUnnormalized)
{
    GLuint mid = std::numeric_limits<GLuint>::max() >> 1;
    GLuint hi  = std::numeric_limits<GLuint>::max();
    std::array<GLuint, kVertexCount> inputData = {
        {0, 1, 2, 3, 254, 255, 256, mid - 1, mid, mid + 1, hi - 2, hi - 1, hi}};
    std::array<GLfloat, kVertexCount> expectedData;
    for (size_t i = 0; i < kVertexCount; i++)
    {
        expectedData[i] = static_cast<GLfloat>(inputData[i]);
    }

    TestData data(GL_UNSIGNED_INT, GL_FALSE, Source::BUFFER, inputData.data(), expectedData.data());
    runTest(data);
}

TEST_P(VertexAttributeTestES3, UnsignedIntNormalized)
{
    GLuint mid = std::numeric_limits<GLuint>::max() >> 1;
    GLuint hi  = std::numeric_limits<GLuint>::max();
    std::array<GLuint, kVertexCount> inputData = {
        {0, 1, 2, 3, 254, 255, 256, mid - 1, mid, mid + 1, hi - 2, hi - 1, hi}};
    std::array<GLfloat, kVertexCount> expectedData;
    for (size_t i = 0; i < kVertexCount; i++)
    {
        expectedData[i] = Normalize(inputData[i]);
    }

    TestData data(GL_UNSIGNED_INT, GL_TRUE, Source::BUFFER, inputData.data(), expectedData.data());
    runTest(data);
}

void SetupColorsForUnitQuad(GLint location, const GLColor32F &color, GLenum usage, GLBuffer *vbo)
{
    glBindBuffer(GL_ARRAY_BUFFER, *vbo);
    std::vector<GLColor32F> vertices(6, color);
    glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(GLColor32F), vertices.data(), usage);
    glEnableVertexAttribArray(location);
    glVertexAttribPointer(location, 4, GL_FLOAT, GL_FALSE, 0, 0);
}

// Tests that rendering works as expected with VAOs.
TEST_P(VertexAttributeTestES3, VertexArrayObjectRendering)
{
    const std::string kVertexShader =
        "attribute vec4 a_position;\n"
        "attribute vec4 a_color;\n"
        "varying vec4 v_color;\n"
        "void main()\n"
        "{\n"
        "   gl_Position = a_position;\n"
        "   v_color = a_color;\n"
        "}";

    const std::string kFragmentShader =
        "precision mediump float;\n"
        "varying vec4 v_color;\n"
        "void main()\n"
        "{\n"
        "    gl_FragColor = v_color;\n"
        "}";

    ANGLE_GL_PROGRAM(program, kVertexShader, kFragmentShader);

    GLint positionLoc = glGetAttribLocation(program, "a_position");
    ASSERT_NE(-1, positionLoc);
    GLint colorLoc = glGetAttribLocation(program, "a_color");
    ASSERT_NE(-1, colorLoc);

    GLVertexArray vaos[2];
    GLBuffer positionBuffer;
    GLBuffer colorBuffers[2];

    const auto &quadVertices = GetQuadVertices();

    glBindVertexArrayOES(vaos[0]);
    glBindBuffer(GL_ARRAY_BUFFER, positionBuffer);
    glBufferData(GL_ARRAY_BUFFER, quadVertices.size() * sizeof(Vector3), quadVertices.data(),
                 GL_STATIC_DRAW);
    glEnableVertexAttribArray(positionLoc);
    glVertexAttribPointer(positionLoc, 3, GL_FLOAT, GL_FALSE, 0, 0);
    SetupColorsForUnitQuad(colorLoc, kFloatRed, GL_STREAM_DRAW, &colorBuffers[0]);

    glBindVertexArrayOES(vaos[1]);
    glBindBuffer(GL_ARRAY_BUFFER, positionBuffer);
    glEnableVertexAttribArray(positionLoc);
    glVertexAttribPointer(positionLoc, 3, GL_FLOAT, GL_FALSE, 0, 0);
    SetupColorsForUnitQuad(colorLoc, kFloatGreen, GL_STATIC_DRAW, &colorBuffers[1]);

    glUseProgram(program);
    ASSERT_GL_NO_ERROR();

    for (int ii = 0; ii < 2; ++ii)
    {
        glBindVertexArrayOES(vaos[0]);
        glDrawArrays(GL_TRIANGLES, 0, 6);
        EXPECT_PIXEL_COLOR_EQ(0, 0, GLColor::red);

        glBindVertexArrayOES(vaos[1]);
        glDrawArrays(GL_TRIANGLES, 0, 6);
        EXPECT_PIXEL_COLOR_EQ(0, 0, GLColor::green);
    }

    ASSERT_GL_NO_ERROR();
}

// Validate that we can support GL_MAX_ATTRIBS attribs
TEST_P(VertexAttributeTest, MaxAttribs)
{
    // TODO(jmadill): Figure out why we get this error on AMD/OpenGL.
    ANGLE_SKIP_TEST_IF(IsAMD() && IsOpenGL());

    GLint maxAttribs;
    glGetIntegerv(GL_MAX_VERTEX_ATTRIBS, &maxAttribs);
    ASSERT_GL_NO_ERROR();

    // Reserve one attrib for position
    GLint drawAttribs = maxAttribs - 1;

    GLuint program = compileMultiAttribProgram(drawAttribs);
    ASSERT_NE(0u, program);

    setupMultiAttribs(program, drawAttribs, 0.5f / static_cast<float>(drawAttribs));
    drawQuad(program, "position", 0.5f);

    EXPECT_GL_NO_ERROR();
    EXPECT_PIXEL_NEAR(0, 0, 128, 0, 0, 255, 1);
}

// Validate that we cannot support GL_MAX_ATTRIBS+1 attribs
TEST_P(VertexAttributeTest, MaxAttribsPlusOne)
{
    // TODO(jmadill): Figure out why we get this error on AMD/ES2/OpenGL
    ANGLE_SKIP_TEST_IF(IsAMD() && GetParam() == ES2_OPENGL());

    GLint maxAttribs;
    glGetIntegerv(GL_MAX_VERTEX_ATTRIBS, &maxAttribs);
    ASSERT_GL_NO_ERROR();

    // Exceed attrib count by one (counting position)
    GLint drawAttribs = maxAttribs;

    GLuint program = compileMultiAttribProgram(drawAttribs);
    ASSERT_EQ(0u, program);
}

// Simple test for when we use glBindAttribLocation
TEST_P(VertexAttributeTest, SimpleBindAttribLocation)
{
    // Re-use the multi-attrib program, binding attribute 0
    GLuint program = compileMultiAttribProgram(1);
    glBindAttribLocation(program, 2, "position");
    glBindAttribLocation(program, 3, "a0");
    glLinkProgram(program);

    // Setup and draw the quad
    setupMultiAttribs(program, 1, 0.5f);
    drawQuad(program, "position", 0.5f);
    EXPECT_GL_NO_ERROR();
    EXPECT_PIXEL_NEAR(0, 0, 128, 0, 0, 255, 1);
}

// Verify that drawing with a large out-of-range offset generates INVALID_OPERATION.
TEST_P(VertexAttributeTest, DrawArraysBufferTooSmall)
{
    std::array<GLfloat, kVertexCount> inputData;
    std::array<GLfloat, kVertexCount> expectedData;
    InitTestData(inputData, expectedData);

    TestData data(GL_FLOAT, GL_FALSE, Source::BUFFER, inputData.data(), expectedData.data());
    data.bufferOffset = kVertexCount * TypeStride(GL_FLOAT);

    setupTest(data, 1);
    drawQuad(mProgram, "position", 0.5f);
    EXPECT_GL_ERROR(GL_INVALID_OPERATION);
}

// Verify that index draw with an out-of-range offset generates INVALID_OPERATION.
TEST_P(VertexAttributeTest, DrawElementsBufferTooSmall)
{
    // Test skipped due to supporting GL_KHR_robust_buffer_access_behavior
    ANGLE_SKIP_TEST_IF(extensionEnabled("GL_KHR_robust_buffer_access_behavior"));

    std::array<GLfloat, kVertexCount> inputData;
    std::array<GLfloat, kVertexCount> expectedData;
    InitTestData(inputData, expectedData);

    TestData data(GL_FLOAT, GL_FALSE, Source::BUFFER, inputData.data(), expectedData.data());
    data.bufferOffset = (kVertexCount - 3) * TypeStride(GL_FLOAT);

    setupTest(data, 1);
    drawIndexedQuad(mProgram, "position", 0.5f);
    EXPECT_GL_ERROR(GL_INVALID_OPERATION);
}

// Verify that using a different start vertex doesn't mess up the draw.
TEST_P(VertexAttributeTest, DrawArraysWithBufferOffset)
{
    // TODO(jmadill): Diagnose this failure.
    ANGLE_SKIP_TEST_IF(IsD3D11_FL93());

    // TODO(geofflang): Figure out why this is broken on AMD OpenGL
    ANGLE_SKIP_TEST_IF(IsAMD() && IsOpenGL());

    initBasicProgram();
    glUseProgram(mProgram);

    std::array<GLfloat, kVertexCount> inputData;
    std::array<GLfloat, kVertexCount> expectedData;
    InitTestData(inputData, expectedData);

    auto quadVertices        = GetQuadVertices();
    GLsizei quadVerticesSize = static_cast<GLsizei>(quadVertices.size() * sizeof(quadVertices[0]));

    glGenBuffers(1, &mQuadBuffer);
    glBindBuffer(GL_ARRAY_BUFFER, mQuadBuffer);
    glBufferData(GL_ARRAY_BUFFER, quadVerticesSize + sizeof(Vector3), nullptr, GL_STATIC_DRAW);
    glBufferSubData(GL_ARRAY_BUFFER, 0, quadVerticesSize, quadVertices.data());

    GLint positionLocation = glGetAttribLocation(mProgram, "position");
    ASSERT_NE(-1, positionLocation);
    glVertexAttribPointer(positionLocation, 3, GL_FLOAT, GL_FALSE, 0, nullptr);
    glEnableVertexAttribArray(positionLocation);

    GLsizei dataSize = kVertexCount * TypeStride(GL_FLOAT);
    glBindBuffer(GL_ARRAY_BUFFER, mBuffer);
    glBufferData(GL_ARRAY_BUFFER, dataSize + TypeStride(GL_FLOAT), nullptr, GL_STATIC_DRAW);
    glBufferSubData(GL_ARRAY_BUFFER, 0, dataSize, inputData.data());
    glVertexAttribPointer(mTestAttrib, 1, GL_FLOAT, GL_FALSE, 0, nullptr);
    glEnableVertexAttribArray(mTestAttrib);

    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glVertexAttribPointer(mExpectedAttrib, 1, GL_FLOAT, GL_FALSE, 0, expectedData.data());
    glEnableVertexAttribArray(mExpectedAttrib);

    // Vertex draw with no start vertex offset (second argument is zero).
    glDrawArrays(GL_TRIANGLES, 0, 6);
    checkPixels();

    // Draw offset by one vertex.
    glDrawArrays(GL_TRIANGLES, 1, 6);
    checkPixels();

    EXPECT_GL_NO_ERROR();
}

// Verify that when we pass a client memory pointer to a disabled attribute the draw is still
// correct.
TEST_P(VertexAttributeTest, DrawArraysWithDisabledAttribute)
{
    initBasicProgram();

    std::array<GLfloat, kVertexCount> inputData;
    std::array<GLfloat, kVertexCount> expectedData;
    InitTestData(inputData, expectedData);

    auto quadVertices        = GetQuadVertices();
    GLsizei quadVerticesSize = static_cast<GLsizei>(quadVertices.size() * sizeof(quadVertices[0]));

    glGenBuffers(1, &mQuadBuffer);
    glBindBuffer(GL_ARRAY_BUFFER, mQuadBuffer);
    glBufferData(GL_ARRAY_BUFFER, quadVerticesSize, quadVertices.data(), GL_STATIC_DRAW);

    GLint positionLocation = glGetAttribLocation(mProgram, "position");
    ASSERT_NE(-1, positionLocation);
    glVertexAttribPointer(positionLocation, 3, GL_FLOAT, GL_FALSE, 0, nullptr);
    glEnableVertexAttribArray(positionLocation);

    glBindBuffer(GL_ARRAY_BUFFER, mBuffer);
    glBufferData(GL_ARRAY_BUFFER, sizeof(inputData), inputData.data(), GL_STATIC_DRAW);
    glVertexAttribPointer(mTestAttrib, 1, GL_FLOAT, GL_FALSE, 0, nullptr);
    glEnableVertexAttribArray(mTestAttrib);

    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glVertexAttribPointer(mExpectedAttrib, 1, GL_FLOAT, GL_FALSE, 0, expectedData.data());
    glEnableVertexAttribArray(mExpectedAttrib);

    // mProgram2 adds an attribute 'disabled' on the basis of mProgram.
    const std::string testVertexShaderSource2 =
        "attribute mediump vec4 position;\n"
        "attribute mediump vec4 test;\n"
        "attribute mediump vec4 expected;\n"
        "attribute mediump vec4 disabled;\n"
        "varying mediump vec4 color;\n"
        "void main(void)\n"
        "{\n"
        "    gl_Position = position;\n"
        "    vec4 threshold = max(abs(expected + disabled) * 0.005, 1.0 / 64.0);\n"
        "    color = vec4(lessThanEqual(abs(test - expected), threshold));\n"
        "}\n";

    const std::string testFragmentShaderSource =
        "varying mediump vec4 color;\n"
        "void main(void)\n"
        "{\n"
        "    gl_FragColor = color;\n"
        "}\n";

    ANGLE_GL_PROGRAM(program, testVertexShaderSource2, testFragmentShaderSource);
    GLuint mProgram2 = program.get();

    ASSERT_EQ(positionLocation, glGetAttribLocation(mProgram2, "position"));
    ASSERT_EQ(mTestAttrib, glGetAttribLocation(mProgram2, "test"));
    ASSERT_EQ(mExpectedAttrib, glGetAttribLocation(mProgram2, "expected"));

    // Pass a client memory pointer to disabledAttribute and disable it.
    GLint disabledAttribute = glGetAttribLocation(mProgram2, "disabled");
    ASSERT_EQ(-1, glGetAttribLocation(mProgram, "disabled"));
    glVertexAttribPointer(disabledAttribute, 1, GL_FLOAT, GL_FALSE, 0, expectedData.data());
    glDisableVertexAttribArray(disabledAttribute);

    glUseProgram(mProgram);
    glDrawArrays(GL_TRIANGLES, 0, 6);
    checkPixels();

    // Now enable disabledAttribute which should be used in mProgram2.
    glEnableVertexAttribArray(disabledAttribute);
    glUseProgram(mProgram2);
    glDrawArrays(GL_TRIANGLES, 0, 6);
    checkPixels();

    EXPECT_GL_NO_ERROR();
}

// Test based on WebGL Test attribs/gl-disabled-vertex-attrib.html
TEST_P(VertexAttributeTest, DisabledAttribArrays)
{
    // Known failure on Retina MBP: http://crbug.com/635081
    ANGLE_SKIP_TEST_IF(IsOSX() && IsNVIDIA());

    const std::string vsSource =
        "attribute vec4 a_position;\n"
        "attribute vec4 a_color;\n"
        "varying vec4 v_color;\n"
        "bool isCorrectColor(vec4 v) {\n"
        "    return v.x == 0.0 && v.y == 0.0 && v.z == 0.0 && v.w == 1.0;\n"
        "}"
        "void main() {\n"
        "    gl_Position = a_position;\n"
        "    v_color = isCorrectColor(a_color) ? vec4(0, 1, 0, 1) : vec4(1, 0, 0, 1);\n"
        "}";

    const std::string fsSource =
        "varying mediump vec4 v_color;\n"
        "void main() {\n"
        "    gl_FragColor = v_color;\n"
        "}";

    GLint maxVertexAttribs = 0;
    glGetIntegerv(GL_MAX_VERTEX_ATTRIBS, &maxVertexAttribs);

    for (GLint colorIndex = 0; colorIndex < maxVertexAttribs; ++colorIndex)
    {
        GLuint vs = CompileShader(GL_VERTEX_SHADER, vsSource);
        ASSERT_NE(0u, vs);
        GLuint fs = CompileShader(GL_FRAGMENT_SHADER, fsSource);
        ASSERT_NE(0u, fs);

        GLuint program = glCreateProgram();
        glBindAttribLocation(program, colorIndex, "a_color");

        glAttachShader(program, vs);
        glDeleteShader(vs);

        glAttachShader(program, fs);
        glDeleteShader(fs);

        ASSERT_TRUE(LinkAttachedProgram(program));

        drawQuad(program, "a_position", 0.5f);
        ASSERT_GL_NO_ERROR();

        EXPECT_PIXEL_COLOR_EQ(0, 0, GLColor::green);

        glDeleteProgram(program);
    }
}

class VertexAttributeTestES31 : public VertexAttributeTestES3
{
  protected:
    VertexAttributeTestES31() {}

    void initTest()
    {
        initBasicProgram();
        glUseProgram(mProgram);

        glGenVertexArrays(1, &mVAO);
        glBindVertexArray(mVAO);

        auto quadVertices = GetQuadVertices();
        GLsizeiptr quadVerticesSize =
            static_cast<GLsizeiptr>(quadVertices.size() * sizeof(quadVertices[0]));
        glGenBuffers(1, &mQuadBuffer);
        glBindBuffer(GL_ARRAY_BUFFER, mQuadBuffer);
        glBufferData(GL_ARRAY_BUFFER, quadVerticesSize, quadVertices.data(), GL_STATIC_DRAW);

        GLint positionLocation = glGetAttribLocation(mProgram, "position");
        ASSERT_NE(-1, positionLocation);
        glVertexAttribPointer(positionLocation, 3, GL_FLOAT, GL_FALSE, 0, nullptr);
        glEnableVertexAttribArray(positionLocation);

        std::array<GLfloat, kVertexCount> expectedData;
        for (size_t count = 0; count < kVertexCount; ++count)
        {
            expectedData[count] = static_cast<GLfloat>(count);
        }

        const GLsizei kExpectedDataSize = kVertexCount * kFloatStride;
        glGenBuffers(1, &mExpectedBuffer);
        glBindBuffer(GL_ARRAY_BUFFER, mExpectedBuffer);
        glBufferData(GL_ARRAY_BUFFER, kExpectedDataSize, expectedData.data(), GL_STATIC_DRAW);
        glVertexAttribPointer(mExpectedAttrib, 1, GL_FLOAT, GL_FALSE, 0, nullptr);
        glEnableVertexAttribArray(mExpectedAttrib);
    }

    void TearDown() override
    {
        VertexAttributeTestES3::TearDown();

        glDeleteBuffers(1, &mExpectedBuffer);
        glDeleteVertexArrays(1, &mVAO);
    }

    void drawArraysWithStrideAndOffset(GLint stride, GLsizeiptr offset)
    {
        initTest();

        GLint floatStride      = stride ? (stride / kFloatStride) : 1;
        GLsizeiptr floatOffset = offset / kFloatStride;
        size_t floatCount      = static_cast<size_t>(floatOffset) + kVertexCount * floatStride;
        GLsizeiptr inputSize   = static_cast<GLsizeiptr>(floatCount) * kFloatStride;

        std::vector<GLfloat> inputData(floatCount);
        for (size_t count = 0; count < kVertexCount; ++count)
        {
            inputData[floatOffset + count * floatStride] = static_cast<GLfloat>(count);
        }

        // Ensure inputSize, inputStride and inputOffset are multiples of TypeStride(GL_FLOAT).
        GLsizei inputStride    = stride ? floatStride * kFloatStride : 0;
        GLsizeiptr inputOffset = floatOffset * kFloatStride;
        glBindBuffer(GL_ARRAY_BUFFER, mBuffer);
        glBufferData(GL_ARRAY_BUFFER, inputSize, nullptr, GL_STATIC_DRAW);
        glBufferSubData(GL_ARRAY_BUFFER, 0, inputSize, inputData.data());
        glVertexAttribPointer(mTestAttrib, 1, GL_FLOAT, GL_FALSE, inputStride,
                              reinterpret_cast<const void *>(inputOffset));
        glEnableVertexAttribArray(mTestAttrib);

        glDrawArrays(GL_TRIANGLES, 0, 6);
        checkPixels();

        EXPECT_GL_NO_ERROR();
    }

    GLuint mVAO;
    GLuint mExpectedBuffer;

    const GLsizei kFloatStride = TypeStride(GL_FLOAT);

    // Set the maximum value for stride if the stride is too large.
    static constexpr GLint MAX_STRIDE_FOR_TEST = 4095;
};

// Verify that MAX_VERTEX_ATTRIB_STRIDE is no less than the minimum required value (2048) in ES3.1.
TEST_P(VertexAttributeTestES31, MaxVertexAttribStride)
{
    GLint maxStride;
    glGetIntegerv(GL_MAX_VERTEX_ATTRIB_STRIDE, &maxStride);
    ASSERT_GL_NO_ERROR();

    EXPECT_GE(maxStride, 2048);
}

// Verify that GL_MAX_VERTEX_ATTRIB_RELATIVE_OFFSET is no less than the minimum required value
// (2047) in ES3.1.
TEST_P(VertexAttributeTestES31, MaxVertexAttribRelativeOffset)
{
    GLint maxRelativeOffset;
    glGetIntegerv(GL_MAX_VERTEX_ATTRIB_RELATIVE_OFFSET, &maxRelativeOffset);
    ASSERT_GL_NO_ERROR();

    EXPECT_GE(maxRelativeOffset, 2047);
}

// Verify using MAX_VERTEX_ATTRIB_STRIDE as stride doesn't mess up the draw.
// Use default value if the value of MAX_VERTEX_ATTRIB_STRIDE is too large for this test.
TEST_P(VertexAttributeTestES31, DrawArraysWithLargeStride)
{
    GLint maxStride;
    glGetIntegerv(GL_MAX_VERTEX_ATTRIB_STRIDE, &maxStride);
    ASSERT_GL_NO_ERROR();

    GLint largeStride = (maxStride < MAX_STRIDE_FOR_TEST) ? maxStride : MAX_STRIDE_FOR_TEST;
    drawArraysWithStrideAndOffset(largeStride, 0);
}

// TODO(jiawei.shao@intel.com): Merge it into VertexAttributeTestES31 when Vertex Attrib
// Binding is supported on D3D11 back-ends.
class VertexAttributeTestES31_OpenGL : public VertexAttributeTestES31
{
  protected:
    VertexAttributeTestES31_OpenGL() {}
};

// Verify that using VertexAttribBinding after VertexAttribPointer won't mess up the draw.
TEST_P(VertexAttributeTestES31_OpenGL, ChangeAttribBindingAfterVertexAttribPointer)
{
    initTest();

    constexpr GLint kInputStride = 2;
    constexpr GLint kFloatOffset = 10;
    std::array<GLfloat, kVertexCount + kFloatOffset> inputData1;
    std::array<GLfloat, kVertexCount * kInputStride> inputData2;
    for (size_t count = 0; count < kVertexCount; ++count)
    {
        inputData1[kFloatOffset + count] = static_cast<GLfloat>(count);
        inputData2[count * kInputStride] = static_cast<GLfloat>(count);
    }

    GLBuffer mBuffer1;
    glBindBuffer(GL_ARRAY_BUFFER, mBuffer1);
    glBufferData(GL_ARRAY_BUFFER, inputData1.size() * kFloatStride, inputData1.data(),
                 GL_STATIC_DRAW);
    // Update the format indexed mTestAttrib and the binding indexed mTestAttrib by
    // VertexAttribPointer.
    const GLintptr kOffset = static_cast<GLintptr>(kFloatStride * kFloatOffset);
    glVertexAttribPointer(mTestAttrib, 1, GL_FLOAT, GL_FALSE, 0,
                          reinterpret_cast<const GLvoid *>(kOffset));
    glEnableVertexAttribArray(mTestAttrib);

    constexpr GLint kTestBinding = 10;
    ASSERT_NE(mTestAttrib, kTestBinding);

    GLBuffer mBuffer2;
    glBindBuffer(GL_ARRAY_BUFFER, mBuffer2);
    glBufferData(GL_ARRAY_BUFFER, inputData2.size() * kFloatStride, inputData2.data(),
                 GL_STATIC_DRAW);
    glBindVertexBuffer(kTestBinding, mBuffer2, 0, kFloatStride * kInputStride);

    // The attribute indexed mTestAttrib is using the binding indexed kTestBinding in the first
    // draw.
    glVertexAttribBinding(mTestAttrib, kTestBinding);
    glDrawArrays(GL_TRIANGLES, 0, 6);
    checkPixels();
    EXPECT_GL_NO_ERROR();

    // The attribute indexed mTestAttrib is using the binding indexed mTestAttrib which should be
    // set after the call VertexAttribPointer before the first draw.
    glVertexAttribBinding(mTestAttrib, mTestAttrib);
    glDrawArrays(GL_TRIANGLES, 0, 6);
    checkPixels();
    EXPECT_GL_NO_ERROR();
}

// Verify that using VertexAttribFormat after VertexAttribPointer won't mess up the draw.
TEST_P(VertexAttributeTestES31_OpenGL, ChangeAttribFormatAfterVertexAttribPointer)
{
    initTest();

    constexpr GLuint kFloatOffset = 10;
    std::array<GLfloat, kVertexCount + kFloatOffset> inputData;
    for (size_t count = 0; count < kVertexCount; ++count)
    {
        inputData[kFloatOffset + count] = static_cast<GLfloat>(count);
    }

    glBindBuffer(GL_ARRAY_BUFFER, mBuffer);
    glBufferData(GL_ARRAY_BUFFER, inputData.size() * kFloatStride, inputData.data(),
                 GL_STATIC_DRAW);

    // Call VertexAttribPointer on mTestAttrib. Now the relativeOffset of mTestAttrib should be 0.
    const GLuint kOffset = static_cast<GLuint>(kFloatStride * kFloatOffset);
    glVertexAttribPointer(mTestAttrib, 1, GL_FLOAT, GL_FALSE, 0, 0);
    glEnableVertexAttribArray(mTestAttrib);

    // Call VertexAttribFormat on mTestAttrib to modify the relativeOffset to kOffset.
    glVertexAttribFormat(mTestAttrib, 1, GL_FLOAT, GL_FALSE, kOffset);

    glDrawArrays(GL_TRIANGLES, 0, 6);
    checkPixels();
    EXPECT_GL_NO_ERROR();
}

class VertexAttributeCachingTest : public VertexAttributeTest
{
  protected:
    VertexAttributeCachingTest() {}

    void SetUp() override;

    template <typename DestT>
    static std::vector<GLfloat> GetExpectedData(const std::vector<GLubyte> &srcData,
                                                GLenum attribType,
                                                GLboolean normalized);

    void initDoubleAttribProgram()
    {
        const std::string testVertexShaderSource =
            "attribute mediump vec4 position;\n"
            "attribute mediump vec4 test;\n"
            "attribute mediump vec4 expected;\n"
            "attribute mediump vec4 test2;\n"
            "attribute mediump vec4 expected2;\n"
            "varying mediump vec4 color;\n"
            "void main(void)\n"
            "{\n"
            "    gl_Position = position;\n"
            "    vec4 threshold = max(abs(expected) * 0.01, 1.0 / 64.0);\n"
            "    color = vec4(lessThanEqual(abs(test - expected), threshold));\n"
            "    vec4 threshold2 = max(abs(expected2) * 0.01, 1.0 / 64.0);\n"
            "    color += vec4(lessThanEqual(abs(test2 - expected2), threshold2));\n"
            "}\n";

        const std::string testFragmentShaderSource =
            "varying mediump vec4 color;\n"
            "void main(void)\n"
            "{\n"
            "    gl_FragColor = color;\n"
            "}\n";

        mProgram = CompileProgram(testVertexShaderSource, testFragmentShaderSource);
        ASSERT_NE(0u, mProgram);

        mTestAttrib = glGetAttribLocation(mProgram, "test");
        ASSERT_NE(-1, mTestAttrib);
        mExpectedAttrib = glGetAttribLocation(mProgram, "expected");
        ASSERT_NE(-1, mExpectedAttrib);

        glUseProgram(mProgram);
    }

    struct AttribData
    {
        AttribData(GLenum typeIn, GLint sizeIn, GLboolean normalizedIn, GLsizei strideIn);

        GLenum type;
        GLint size;
        GLboolean normalized;
        GLsizei stride;
    };

    std::vector<AttribData> mTestData;
    std::map<GLenum, std::vector<GLfloat>> mExpectedData;
    std::map<GLenum, std::vector<GLfloat>> mNormExpectedData;
};

VertexAttributeCachingTest::AttribData::AttribData(GLenum typeIn,
                                                   GLint sizeIn,
                                                   GLboolean normalizedIn,
                                                   GLsizei strideIn)
    : type(typeIn), size(sizeIn), normalized(normalizedIn), stride(strideIn)
{
}

// static
template <typename DestT>
std::vector<GLfloat> VertexAttributeCachingTest::GetExpectedData(
    const std::vector<GLubyte> &srcData,
    GLenum attribType,
    GLboolean normalized)
{
    std::vector<GLfloat> expectedData;

    const DestT *typedSrcPtr = reinterpret_cast<const DestT *>(srcData.data());
    size_t iterations        = srcData.size() / TypeStride(attribType);

    if (normalized)
    {
        for (size_t index = 0; index < iterations; ++index)
        {
            expectedData.push_back(Normalize(typedSrcPtr[index]));
        }
    }
    else
    {
        for (size_t index = 0; index < iterations; ++index)
        {
            expectedData.push_back(static_cast<GLfloat>(typedSrcPtr[index]));
        }
    }

    return expectedData;
}

void VertexAttributeCachingTest::SetUp()
{
    VertexAttributeTest::SetUp();

    glBindBuffer(GL_ARRAY_BUFFER, mBuffer);

    std::vector<GLubyte> srcData;
    for (size_t count = 0; count < 4; ++count)
    {
        for (GLubyte i = 0; i < std::numeric_limits<GLubyte>::max(); ++i)
        {
            srcData.push_back(i);
        }
    }

    glBufferData(GL_ARRAY_BUFFER, srcData.size(), srcData.data(), GL_STATIC_DRAW);

    GLint viewportSize[4];
    glGetIntegerv(GL_VIEWPORT, viewportSize);

    std::vector<GLenum> attribTypes;
    attribTypes.push_back(GL_BYTE);
    attribTypes.push_back(GL_UNSIGNED_BYTE);
    attribTypes.push_back(GL_SHORT);
    attribTypes.push_back(GL_UNSIGNED_SHORT);

    if (getClientMajorVersion() >= 3)
    {
        attribTypes.push_back(GL_INT);
        attribTypes.push_back(GL_UNSIGNED_INT);
    }

    constexpr GLint kMaxSize     = 4;
    constexpr GLsizei kMaxStride = 4;

    for (GLenum attribType : attribTypes)
    {
        for (GLint attribSize = 1; attribSize <= kMaxSize; ++attribSize)
        {
            for (GLsizei stride = 1; stride <= kMaxStride; ++stride)
            {
                mTestData.push_back(AttribData(attribType, attribSize, GL_FALSE, stride));
                if (attribType != GL_FLOAT)
                {
                    mTestData.push_back(AttribData(attribType, attribSize, GL_TRUE, stride));
                }
            }
        }
    }

    mExpectedData[GL_BYTE]          = GetExpectedData<GLbyte>(srcData, GL_BYTE, GL_FALSE);
    mExpectedData[GL_UNSIGNED_BYTE] = GetExpectedData<GLubyte>(srcData, GL_UNSIGNED_BYTE, GL_FALSE);
    mExpectedData[GL_SHORT]         = GetExpectedData<GLshort>(srcData, GL_SHORT, GL_FALSE);
    mExpectedData[GL_UNSIGNED_SHORT] =
        GetExpectedData<GLushort>(srcData, GL_UNSIGNED_SHORT, GL_FALSE);
    mExpectedData[GL_INT]          = GetExpectedData<GLint>(srcData, GL_INT, GL_FALSE);
    mExpectedData[GL_UNSIGNED_INT] = GetExpectedData<GLuint>(srcData, GL_UNSIGNED_INT, GL_FALSE);

    mNormExpectedData[GL_BYTE] = GetExpectedData<GLbyte>(srcData, GL_BYTE, GL_TRUE);
    mNormExpectedData[GL_UNSIGNED_BYTE] =
        GetExpectedData<GLubyte>(srcData, GL_UNSIGNED_BYTE, GL_TRUE);
    mNormExpectedData[GL_SHORT] = GetExpectedData<GLshort>(srcData, GL_SHORT, GL_TRUE);
    mNormExpectedData[GL_UNSIGNED_SHORT] =
        GetExpectedData<GLushort>(srcData, GL_UNSIGNED_SHORT, GL_TRUE);
    mNormExpectedData[GL_INT]          = GetExpectedData<GLint>(srcData, GL_INT, GL_TRUE);
    mNormExpectedData[GL_UNSIGNED_INT] = GetExpectedData<GLuint>(srcData, GL_UNSIGNED_INT, GL_TRUE);
}

// In D3D11, we must sometimes translate buffer data into static attribute caches. We also use a
// cache management scheme which garbage collects old attributes after we start using too much
// cache data. This test tries to make as many attribute caches from a single buffer as possible
// to stress-test the caching code.
TEST_P(VertexAttributeCachingTest, BufferMulticaching)
{
    ANGLE_SKIP_TEST_IF(IsAMD() && IsDesktopOpenGL());

    initBasicProgram();

    glEnableVertexAttribArray(mTestAttrib);
    glEnableVertexAttribArray(mExpectedAttrib);

    ASSERT_GL_NO_ERROR();

    for (const auto &data : mTestData)
    {
        const auto &expected =
            (data.normalized) ? mNormExpectedData[data.type] : mExpectedData[data.type];

        GLsizei baseStride = static_cast<GLsizei>(data.size) * data.stride;
        GLsizei stride     = TypeStride(data.type) * baseStride;

        glBindBuffer(GL_ARRAY_BUFFER, mBuffer);
        glVertexAttribPointer(mTestAttrib, data.size, data.type, data.normalized, stride, nullptr);
        glBindBuffer(GL_ARRAY_BUFFER, 0);
        glVertexAttribPointer(mExpectedAttrib, data.size, GL_FLOAT, GL_FALSE,
                              sizeof(GLfloat) * baseStride, expected.data());
        drawQuad(mProgram, "position", 0.5f);
        ASSERT_GL_NO_ERROR();
        EXPECT_PIXEL_EQ(getWindowWidth() / 2, getWindowHeight() / 2, 255, 255, 255, 255);
    }
}

// With D3D11 dirty bits for VertxArray11, we can leave vertex state unchanged if there aren't any
// GL calls that affect it. This test targets leaving one vertex attribute unchanged between draw
// calls while changing another vertex attribute enough that it clears the static buffer cache
// after enough iterations. It validates the unchanged attributes don't get deleted incidentally.
TEST_P(VertexAttributeCachingTest, BufferMulticachingWithOneUnchangedAttrib)
{
    ANGLE_SKIP_TEST_IF(IsAMD() && IsDesktopOpenGL());

    initDoubleAttribProgram();

    GLint testAttrib2Location = glGetAttribLocation(mProgram, "test2");
    ASSERT_NE(-1, testAttrib2Location);
    GLint expectedAttrib2Location = glGetAttribLocation(mProgram, "expected2");
    ASSERT_NE(-1, expectedAttrib2Location);

    glEnableVertexAttribArray(mTestAttrib);
    glEnableVertexAttribArray(mExpectedAttrib);
    glEnableVertexAttribArray(testAttrib2Location);
    glEnableVertexAttribArray(expectedAttrib2Location);

    ASSERT_GL_NO_ERROR();

    // Use an attribute that we know must be converted. This is a bit sensitive.
    glBindBuffer(GL_ARRAY_BUFFER, mBuffer);
    glVertexAttribPointer(testAttrib2Location, 3, GL_UNSIGNED_SHORT, GL_FALSE, 6, nullptr);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glVertexAttribPointer(expectedAttrib2Location, 3, GL_FLOAT, GL_FALSE, sizeof(GLfloat) * 3,
                          mExpectedData[GL_UNSIGNED_SHORT].data());

    for (const auto &data : mTestData)
    {
        const auto &expected =
            (data.normalized) ? mNormExpectedData[data.type] : mExpectedData[data.type];

        GLsizei baseStride = static_cast<GLsizei>(data.size) * data.stride;
        GLsizei stride     = TypeStride(data.type) * baseStride;

        glBindBuffer(GL_ARRAY_BUFFER, mBuffer);
        glVertexAttribPointer(mTestAttrib, data.size, data.type, data.normalized, stride, nullptr);
        glBindBuffer(GL_ARRAY_BUFFER, 0);
        glVertexAttribPointer(mExpectedAttrib, data.size, GL_FLOAT, GL_FALSE,
                              sizeof(GLfloat) * baseStride, expected.data());
        drawQuad(mProgram, "position", 0.5f);

        ASSERT_GL_NO_ERROR();
        EXPECT_PIXEL_EQ(getWindowWidth() / 2, getWindowHeight() / 2, 255, 255, 255, 255);
    }
}

// Tests that repeatedly updating a disabled vertex attribute works as expected.
// This covers an ANGLE bug where dirty bits for current values were ignoring repeated updates.
TEST_P(VertexAttributeTest, DisabledAttribUpdates)
{
    constexpr char kVertexShader[] = R"(attribute vec2 position;
attribute float actualValue;
uniform float expectedValue;
varying float result;
void main()
{
    result = (actualValue == expectedValue) ? 1.0 : 0.0;
    gl_Position = vec4(position, 0, 1);
})";

    constexpr char kFragmentShader[] = R"(varying mediump float result;
void main()
{
    gl_FragColor = result > 0.0 ? vec4(0, 1, 0, 1) : vec4(1, 0, 0, 1);
})";

    ANGLE_GL_PROGRAM(program, kVertexShader, kFragmentShader);

    glUseProgram(program);
    GLint attribLoc = glGetAttribLocation(program, "actualValue");
    ASSERT_NE(-1, attribLoc);

    GLint uniLoc = glGetUniformLocation(program, "expectedValue");
    ASSERT_NE(-1, uniLoc);

    glVertexAttribPointer(attribLoc, 1, GL_FLOAT, GL_FALSE, 0, nullptr);

    GLint positionLocation = glGetAttribLocation(program, "position");
    ASSERT_NE(-1, positionLocation);
    setupQuadVertexBuffer(0.5f, 1.0f);
    glVertexAttribPointer(positionLocation, 3, GL_FLOAT, GL_FALSE, 0, 0);
    glEnableVertexAttribArray(positionLocation);

    std::array<GLfloat, 4> testValues = {{1, 2, 3, 4}};
    for (GLfloat testValue : testValues)
    {
        glUniform1f(uniLoc, testValue);
        glVertexAttrib1f(attribLoc, testValue);
        glDrawArrays(GL_TRIANGLES, 0, 6);
        ASSERT_GL_NO_ERROR();
        EXPECT_PIXEL_COLOR_EQ(0, 0, GLColor::green);
    }
}

class VertexAttributeTestMaps : public VertexAttributeTest
{
  protected:
    VertexAttributeTestMaps() {}
};

// Tests vertex attributes as used by Google Maps
TEST_P(VertexAttributeTestMaps, MapsAttributes)
{
    constexpr char kVertexShader[] = R"(
uniform mat4 uMVPMatrix;
uniform mat3 uTextureMatrix;
attribute vec4 aPosition;
attribute vec2 aTextureCoord;
attribute vec4 aColor;
varying vec2 vTextureCoord;
varying vec4 vColor;
void main() {
  gl_Position = uMVPMatrix * aPosition;
  vTextureCoord = (uTextureMatrix * vec3(aTextureCoord, 1.0)).xy;
  vColor = aColor;
}
)";

    constexpr char kFragmentShader[] = R"(
precision mediump float;
varying vec2 vTextureCoord;
varying vec4 vColor;
uniform sampler2D sTexture0;
void main() {
  gl_FragColor = vColor * texture2D(sTexture0, vTextureCoord);
}
)";

    ANGLE_GL_PROGRAM(program, kVertexShader, kFragmentShader);
    ASSERT_GL_NO_ERROR();

    glUseProgram(program);
    GLint uMVPMatrix = glGetUniformLocation(program, "uMVPMatrix");
    ASSERT_NE(-1, uMVPMatrix);

    GLint uTextureMatrix = glGetUniformLocation(program, "uTextureMatrix");
    ASSERT_NE(-1, uTextureMatrix);

    GLint sTexture0 = glGetUniformLocation(program, "sTexture0");
    ASSERT_NE(-1, sTexture0);
    ASSERT_GL_NO_ERROR();

    GLfloat uniformMatrix1[] = {1.0, 0.0, 0.0, 0.0, 0.0, 1.0, 0.0, 0.0,
                                0.0, 0.0, 1.0, 0.0, 0.0, 0.0, 0.0, 1.0};

    glUniformMatrix4fv(uMVPMatrix, 1, false, uniformMatrix1);

    glUniform1i(sTexture0, 0);

    GLfloat uniformMatrix2[] = {1.0, 0.0, 0.0, 0.0, 1.0, 0.0, 0.0, 0.0, 1.0};

    glUniformMatrix3fv(uTextureMatrix, 1, false, uniformMatrix2);

    glBindAttribLocation(program, 0, "aPosition");
    glBindAttribLocation(program, 2, "aColor");
    glBindAttribLocation(program, 4, "aTexureCoord");

    GLfloat aColor[] = {1.0, 1.0, 1.0, 1.0};
    glVertexAttrib4fv(2, aColor);

    ASSERT_GL_NO_ERROR();

    GLint numVertexAttribs;
    glGetIntegerv(GL_MAX_VERTEX_ATTRIBS, &numVertexAttribs);
    for (GLint i = 0; i < numVertexAttribs; i++)
    {
        glDisableVertexAttribArray(i);
    }
    ASSERT_GL_NO_ERROR();

    GLuint textureId;
    GLbyte textureData[] = {0, 0, 255, 255};
    glGenTextures(1, &textureId);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, 1, 1, 0, GL_RGBA, GL_UNSIGNED_BYTE, textureData);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glBindTexture(GL_TEXTURE_2D, textureId);
    ASSERT_GL_NO_ERROR();

    GLuint arrayBuf;
    glGenBuffers(1, &arrayBuf);
    glBindBuffer(GL_ARRAY_BUFFER, arrayBuf);
    GLfloat bufData[] = {
        0.0,         0.0,          0.5,           0.5,           1.0,           0.0,
        1.0,         0.5,          0.9980267,     0.06279052,    0.99901336,    0.46860474,
        0.0,         0.0,          0.5,           0.5,           0.9980267,     0.06279052,
        0.99901336,  0.46860474,   0.9921147,     0.12533323,    0.9960574,     0.4373334,
        0.0,         0.0,          0.5,           0.5,           0.9921147,     0.12533323,
        0.9960574,   0.4373334,    0.9822872,     0.18738133,    0.9911436,     0.40630937,
        0.0,         0.0,          0.5,           0.5,           0.9822872,     0.18738133,
        0.9911436,   0.40630937,   0.96858317,    0.2486899,     0.98429155,    0.37565506,
        0.0,         0.0,          0.5,           0.5,           0.96858317,    0.2486899,
        0.98429155,  0.37565506,   0.95105654,    0.309017,      0.97552824,    0.34549153,
        0.0,         0.0,          0.5,           0.5,           0.95105654,    0.309017,
        0.97552824,  0.34549153,   0.9297765,     0.36812454,    0.9648882,     0.31593776,
        0.0,         0.0,          0.5,           0.5,           0.9297765,     0.36812454,
        0.9648882,   0.31593776,   0.90482706,    0.42577928,    0.95241356,    0.28711033,
        0.0,         0.0,          0.5,           0.5,           0.90482706,    0.42577928,
        0.95241356,  0.28711033,   0.87630665,    0.4817537,     0.9381533,     0.25912315,
        0.0,         0.0,          0.5,           0.5,           0.87630665,    0.4817537,
        0.9381533,   0.25912315,   0.8443279,     0.53582686,    0.92216396,    0.23208654,
        0.0,         0.0,          0.5,           0.5,           0.8443279,     0.53582686,
        0.92216396,  0.23208654,   0.80901694,    0.5877853,     0.9045085,     0.20610738,
        0.0,         0.0,          0.5,           0.5,           0.80901694,    0.5877853,
        0.9045085,   0.20610738,   0.7705132,     0.63742405,    0.8852566,     0.181288,
        0.0,         0.0,          0.5,           0.5,           0.7705132,     0.63742405,
        0.8852566,   0.181288,     0.72896856,    0.6845472,     0.8644843,     0.1577264,
        0.0,         0.0,          0.5,           0.5,           0.72896856,    0.6845472,
        0.8644843,   0.1577264,    0.684547,      0.72896874,    0.8422735,     0.13551563,
        0.0,         0.0,          0.5,           0.5,           0.684547,      0.72896874,
        0.8422735,   0.13551563,   0.6374239,     0.77051336,    0.81871194,    0.11474335,
        0.0,         0.0,          0.5,           0.5,           0.6374239,     0.77051336,
        0.81871194,  0.11474335,   0.58778507,    0.8090171,     0.7938925,     0.09549141,
        0.0,         0.0,          0.5,           0.5,           0.58778507,    0.8090171,
        0.7938925,   0.09549141,   0.5358266,     0.84432805,    0.76791334,    0.07783598,
        0.0,         0.0,          0.5,           0.5,           0.5358266,     0.84432805,
        0.76791334,  0.07783598,   0.4817535,     0.8763068,     0.74087673,    0.061846614,
        0.0,         0.0,          0.5,           0.5,           0.4817535,     0.8763068,
        0.74087673,  0.061846614,  0.4257791,     0.9048271,     0.71288955,    0.04758644,
        0.0,         0.0,          0.5,           0.5,           0.4257791,     0.9048271,
        0.71288955,  0.04758644,   0.36812434,    0.92977655,    0.6840622,     0.035111725,
        0.0,         0.0,          0.5,           0.5,           0.36812434,    0.92977655,
        0.6840622,   0.035111725,  0.30901673,    0.9510566,     0.65450835,    0.0244717,
        0.0,         0.0,          0.5,           0.5,           0.30901673,    0.9510566,
        0.65450835,  0.0244717,    0.24868959,    0.9685832,     0.6243448,     0.015708387,
        0.0,         0.0,          0.5,           0.5,           0.24868959,    0.9685832,
        0.6243448,   0.015708387,  0.187381,      0.9822873,     0.5936905,     0.008856356,
        0.0,         0.0,          0.5,           0.5,           0.187381,      0.9822873,
        0.5936905,   0.008856356,  0.12533289,    0.9921147,     0.5626664,     0.003942609,
        0.0,         0.0,          0.5,           0.5,           0.12533289,    0.9921147,
        0.5626664,   0.003942609,  0.06279015,    0.9980267,     0.5313951,     9.866357E-4,
        0.0,         0.0,          0.5,           0.5,           0.06279015,    0.9980267,
        0.5313951,   9.866357E-4,  -4.0133926E-7, 1.0,           0.4999998,     0.0,
        0.0,         0.0,          0.5,           0.5,           -4.0133926E-7, 1.0,
        0.4999998,   0.0,          -0.062790945,  0.9980267,     0.46860453,    9.866357E-4,
        0.0,         0.0,          0.5,           0.5,           -0.062790945,  0.9980267,
        0.46860453,  9.866357E-4,  -0.12533368,   0.99211466,    0.43733317,    0.0039426684,
        0.0,         0.0,          0.5,           0.5,           -0.12533368,   0.99211466,
        0.43733317,  0.0039426684, -0.18738179,   0.98228717,    0.4063091,     0.008856416,
        0.0,         0.0,          0.5,           0.5,           -0.18738179,   0.98228717,
        0.4063091,   0.008856416,  -0.24869038,   0.96858305,    0.37565482,    0.015708447,
        0.0,         0.0,          0.5,           0.5,           -0.24869038,   0.96858305,
        0.37565482,  0.015708447,  -0.3090175,    0.95105636,    0.34549123,    0.02447182,
        0.0,         0.0,          0.5,           0.5,           -0.3090175,    0.95105636,
        0.34549123,  0.02447182,   -0.36812508,   0.9297763,     0.31593746,    0.035111845,
        0.0,         0.0,          0.5,           0.5,           -0.36812508,   0.9297763,
        0.31593746,  0.035111845,  -0.42577982,   0.9048268,     0.2871101,     0.04758656,
        0.0,         0.0,          0.5,           0.5,           -0.42577982,   0.9048268,
        0.2871101,   0.04758656,   -0.4817542,    0.8763064,     0.2591229,     0.061846793,
        0.0,         0.0,          0.5,           0.5,           -0.4817542,    0.8763064,
        0.2591229,   0.061846793,  -0.53582734,   0.84432757,    0.23208633,    0.077836215,
        0.0,         0.0,          0.5,           0.5,           -0.53582734,   0.84432757,
        0.23208633,  0.077836215,  -0.5877858,    0.8090166,     0.20610711,    0.09549171,
        0.0,         0.0,          0.5,           0.5,           -0.5877858,    0.8090166,
        0.20610711,  0.09549171,   -0.6374245,    0.7705128,     0.18128774,    0.11474359,
        0.0,         0.0,          0.5,           0.5,           -0.6374245,    0.7705128,
        0.18128774,  0.11474359,   -0.6845476,    0.72896814,    0.1577262,     0.13551593,
        0.0,         0.0,          0.5,           0.5,           -0.6845476,    0.72896814,
        0.1577262,   0.13551593,   -0.72896916,   0.6845466,     0.13551542,    0.1577267,
        0.0,         0.0,          0.5,           0.5,           -0.72896916,   0.6845466,
        0.13551542,  0.1577267,    -0.7705137,    0.6374234,     0.11474314,    0.1812883,
        0.0,         0.0,          0.5,           0.5,           -0.7705137,    0.6374234,
        0.11474314,  0.1812883,    -0.8090175,    0.5877846,     0.09549126,    0.20610774,
        0.0,         0.0,          0.5,           0.5,           -0.8090175,    0.5877846,
        0.09549126,  0.20610774,   -0.84432834,   0.5358261,     0.07783583,    0.23208696,
        0.0,         0.0,          0.5,           0.5,           -0.84432834,   0.5358261,
        0.07783583,  0.23208696,   -0.8763071,    0.48175293,    0.061846465,   0.25912356,
        0.0,         0.0,          0.5,           0.5,           -0.8763071,    0.48175293,
        0.061846465, 0.25912356,   -0.9048274,    0.4257785,     0.047586292,   0.28711075,
        0.0,         0.0,          0.5,           0.5,           -0.9048274,    0.4257785,
        0.047586292, 0.28711075,   -0.9297768,    0.36812374,    0.035111606,   0.31593812,
        0.0,         0.0,          0.5,           0.5,           -0.9297768,    0.36812374,
        0.035111606, 0.31593812,   -0.9510568,    0.30901614,    0.02447161,    0.34549195,
        0.0,         0.0,          0.5,           0.5,           -0.9510568,    0.30901614,
        0.02447161,  0.34549195,   -0.9685834,    0.24868898,    0.015708297,   0.37565553,
        0.0,         0.0,          0.5,           0.5,           -0.9685834,    0.24868898,
        0.015708297, 0.37565553,   -0.9822874,    0.18738037,    0.008856297,   0.40630984,
        0.0,         0.0,          0.5,           0.5,           -0.9822874,    0.18738037,
        0.008856297, 0.40630984,   -0.99211484,   0.12533225,    0.003942579,   0.43733388,
        0.0,         0.0,          0.5,           0.5,           -0.99211484,   0.12533225,
        0.003942579, 0.43733388,   -0.9980268,    0.06278951,    9.866059E-4,   0.46860522,
        0.0,         0.0,          0.5,           0.5,           -0.9980268,    0.06278951,
        9.866059E-4, 0.46860522,   -1.0,          -1.0410971E-6, 0.0,           0.5000005,
        0.0,         0.0,          0.5,           0.5,           -1.0,          -1.0410971E-6,
        0.0,         0.5000005,    -0.99802667,   -0.062791586,  9.866655E-4,   0.5313958,
        0.0,         0.0,          0.5,           0.5,           -0.99802667,   -0.062791586,
        9.866655E-4, 0.5313958,    -0.99211454,   -0.12533432,   0.003942728,   0.56266713,
        0.0,         0.0,          0.5,           0.5,           -0.99211454,   -0.12533432,
        0.003942728, 0.56266713,   -0.98228705,   -0.18738241,   0.008856475,   0.59369123,
        0.0,         0.0,          0.5,           0.5,           -0.98228705,   -0.18738241,
        0.008856475, 0.59369123,   -0.96858287,   -0.248691,     0.015708566,   0.6243455,
        0.0,         0.0,          0.5,           0.5,           -0.96858287,   -0.248691,
        0.015708566, 0.6243455,    -0.9510562,    -0.3090181,    0.024471909,   0.65450907,
        0.0,         0.0,          0.5,           0.5,           -0.9510562,    -0.3090181,
        0.024471909, 0.65450907,   -0.9297761,    -0.36812568,   0.035111964,   0.68406284,
        0.0,         0.0,          0.5,           0.5,           -0.9297761,    -0.36812568,
        0.035111964, 0.68406284,   -0.9048265,    -0.4257804,    0.04758674,    0.7128902,
        0.0,         0.0,          0.5,           0.5,           -0.9048265,    -0.4257804,
        0.04758674,  0.7128902,    -0.87630606,   -0.48175478,   0.06184697,    0.7408774,
        0.0,         0.0,          0.5,           0.5,           -0.87630606,   -0.48175478,
        0.06184697,  0.7408774,    -0.8443273,    -0.5358279,    0.077836365,   0.76791394,
        0.0,         0.0,          0.5,           0.5,           -0.8443273,    -0.5358279,
        0.077836365, 0.76791394,   -0.8090162,    -0.5877863,    0.095491886,   0.79389316,
        0.0,         0.0,          0.5,           0.5,           -0.8090162,    -0.5877863,
        0.095491886, 0.79389316,   -0.7705124,    -0.637425,     0.1147438,     0.8187125,
        0.0,         0.0,          0.5,           0.5,           -0.7705124,    -0.637425,
        0.1147438,   0.8187125,    -0.7289677,    -0.6845481,    0.13551614,    0.84227407,
        0.0,         0.0,          0.5,           0.5,           -0.7289677,    -0.6845481,
        0.13551614,  0.84227407,   -0.6845461,    -0.7289696,    0.15772694,    0.8644848,
        0.0,         0.0,          0.5,           0.5,           -0.6845461,    -0.7289696,
        0.15772694,  0.8644848,    -0.6374231,    -0.770514,     0.18128845,    0.885257,
        0.0,         0.0,          0.5,           0.5,           -0.6374231,    -0.770514,
        0.18128845,  0.885257,     -0.5877843,    -0.8090177,    0.20610785,    0.9045088,
        0.0,         0.0,          0.5,           0.5,           -0.5877843,    -0.8090177,
        0.20610785,  0.9045088,    -0.5358258,    -0.8443286,    0.2320871,     0.9221643,
        0.0,         0.0,          0.5,           0.5,           -0.5358258,    -0.8443286,
        0.2320871,   0.9221643,    -0.4817526,    -0.87630725,   0.25912368,    0.9381536,
        0.0,         0.0,          0.5,           0.5,           -0.4817526,    -0.87630725,
        0.25912368,  0.9381536,    -0.42577815,   -0.9048276,    0.28711092,    0.9524138,
        0.0,         0.0,          0.5,           0.5,           -0.42577815,   -0.9048276,
        0.28711092,  0.9524138,    -0.36812335,   -0.92977697,   0.31593832,    0.96488845,
        0.0,         0.0,          0.5,           0.5,           -0.36812335,   -0.92977697,
        0.31593832,  0.96488845,   -0.30901575,   -0.9510569,    0.34549212,    0.9755285,
        0.0,         0.0,          0.5,           0.5,           -0.30901575,   -0.9510569,
        0.34549212,  0.9755285,    -0.2486886,    -0.96858346,   0.3756557,     0.98429173,
        0.0,         0.0,          0.5,           0.5,           -0.2486886,    -0.96858346,
        0.3756557,   0.98429173,   -0.18737997,   -0.9822875,    0.40631002,    0.99114376,
        0.0,         0.0,          0.5,           0.5,           -0.18737997,   -0.9822875,
        0.40631002,  0.99114376,   -0.12533185,   -0.9921149,    0.43733406,    0.99605745,
        0.0,         0.0,          0.5,           0.5,           -0.12533185,   -0.9921149,
        0.43733406,  0.99605745,   -0.062789105,  -0.9980268,    0.46860546,    0.9990134,
        0.0,         0.0,          0.5,           0.5,           -0.062789105,  -0.9980268,
        0.46860546,  0.9990134,    1.4424363E-6,  -1.0,          0.5000007,     1.0,
        0.0,         0.0,          0.5,           0.5,           1.4424363E-6,  -1.0,
        0.5000007,   1.0,          0.06279199,    -0.9980266,    0.531396,      0.9990133,
        0.0,         0.0,          0.5,           0.5,           0.06279199,    -0.9980266,
        0.531396,    0.9990133,    0.12533471,    -0.99211454,   0.56266737,    0.9960573,
        0.0,         0.0,          0.5,           0.5,           0.12533471,    -0.99211454,
        0.56266737,  0.9960573,    0.1873828,     -0.982287,     0.5936914,     0.99114347,
        0.0,         0.0,          0.5,           0.5,           0.1873828,     -0.982287,
        0.5936914,   0.99114347,   0.24869138,    -0.96858275,   0.62434566,    0.9842914,
        0.0,         0.0,          0.5,           0.5,           0.24869138,    -0.96858275,
        0.62434566,  0.9842914,    0.3090185,     -0.951056,     0.65450925,    0.975528,
        0.0,         0.0,          0.5,           0.5,           0.3090185,     -0.951056,
        0.65450925,  0.975528,     0.36812603,    -0.9297759,    0.684063,      0.964888,
        0.0,         0.0,          0.5,           0.5,           0.36812603,    -0.9297759,
        0.684063,    0.964888,     0.42578077,    -0.90482634,   0.7128904,     0.9524132,
        0.0,         0.0,          0.5,           0.5,           0.42578077,    -0.90482634,
        0.7128904,   0.9524132,    0.4817551,     -0.8763059,    0.74087757,    0.9381529,
        0.0,         0.0,          0.5,           0.5,           0.4817551,     -0.8763059,
        0.74087757,  0.9381529,    0.53582823,    -0.84432703,   0.7679141,     0.9221635,
        0.0,         0.0,          0.5,           0.5,           0.53582823,    -0.84432703,
        0.7679141,   0.9221635,    0.5877866,     -0.809016,     0.79389334,    0.904508,
        0.0,         0.0,          0.5,           0.5,           0.5877866,     -0.809016,
        0.79389334,  0.904508,     0.6374253,     -0.77051216,   0.81871265,    0.88525605,
        0.0,         0.0,          0.5,           0.5,           0.6374253,     -0.77051216,
        0.81871265,  0.88525605,   0.6845484,     -0.7289674,    0.8422742,     0.8644837,
        0.0,         0.0,          0.5,           0.5,           0.6845484,     -0.7289674,
        0.8422742,   0.8644837,    0.7289699,     -0.6845458,    0.8644849,     0.8422729,
        0.0,         0.0,          0.5,           0.5,           0.7289699,     -0.6845458,
        0.8644849,   0.8422729,    0.77051437,    -0.6374226,    0.8852572,     0.8187113,
        0.0,         0.0,          0.5,           0.5,           0.77051437,    -0.6374226,
        0.8852572,   0.8187113,    0.8090181,     -0.58778375,   0.90450907,    0.7938919,
        0.0,         0.0,          0.5,           0.5,           0.8090181,     -0.58778375,
        0.90450907,  0.7938919,    0.84432894,    -0.53582525,   0.92216444,    0.7679126,
        0.0,         0.0,          0.5,           0.5,           0.84432894,    -0.53582525,
        0.92216444,  0.7679126,    0.8763076,     -0.48175204,   0.9381538,     0.740876,
        0.0,         0.0,          0.5,           0.5,           0.8763076,     -0.48175204,
        0.9381538,   0.740876,     0.90482783,    -0.42577758,   0.9524139,     0.7128888,
        0.0,         0.0,          0.5,           0.5,           0.90482783,    -0.42577758,
        0.9524139,   0.7128888,    0.9297772,     -0.36812276,   0.9648886,     0.6840614,
        0.0,         0.0,          0.5,           0.5,           0.9297772,     -0.36812276,
        0.9648886,   0.6840614,    0.95105714,    -0.30901513,   0.9755286,     0.6545076,
        0.0,         0.0,          0.5,           0.5,           0.95105714,    -0.30901513,
        0.9755286,   0.6545076,    0.96858364,    -0.24868797,   0.9842918,     0.624344,
        0.0,         0.0,          0.5,           0.5,           0.96858364,    -0.24868797,
        0.9842918,   0.624344,     0.98228765,    -0.18737935,   0.9911438,     0.5936897,
        0.0,         0.0,          0.5,           0.5,           0.98228765,    -0.18737935,
        0.9911438,   0.5936897,    0.99211496,    -0.12533122,   0.9960575,     0.5626656,
        0.0,         0.0,          0.5,           0.5,           0.99211496,    -0.12533122,
        0.9960575,   0.5626656,    0.99802685,    -0.062788464,  0.9990134,     0.53139424,
        0.0,         0.0,          0.5,           0.5,           0.99802685,    -0.062788464,
        0.9990134,   0.53139424,   1.0,           0.0,           1.0,           0.5,
    };

    glBufferData(GL_ARRAY_BUFFER, sizeof(bufData), reinterpret_cast<const GLvoid *>(bufData),
                 GL_STATIC_DRAW);
    ASSERT_GL_NO_ERROR();

    glEnableVertexAttribArray(0);

    // NOTE: Deliberately leaving VertexAttribArray index 2 disabled
    // to reproduce behavior of application.
    glEnableVertexAttribArray(4);

    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 16, 0);
    glVertexAttribPointer(4, 2, GL_FLOAT, GL_TRUE, 16, reinterpret_cast<void *>(8));

    //    setupQuadVertexBuffer(0.5f, 1.0f);

    glDrawArrays(GL_TRIANGLES, 0, 300);
    ASSERT_GL_NO_ERROR();
    // EXPECT_PIXEL_COLOR_EQ(0, 0, GLColor::green);
}

// Test that even inactive attributes are taken into account when checking for aliasing in case the
// shader version is >= 3.00. GLSL ES 3.00.6 section 12.46.
TEST_P(VertexAttributeTestES3, InactiveAttributeAliasing)
{
    const std::string &vertexShader =
        R"(#version 300 es
        precision mediump float;
        in vec4 input_active;
        in vec4 input_unused;
        void main()
        {
            gl_Position = input_active;
        })";

    const std::string &fragmentShader =
        R"(#version 300 es
        precision mediump float;
        out vec4 color;
        void main()
        {
            color = vec4(0.0);
        })";

    ANGLE_GL_PROGRAM(program, vertexShader, fragmentShader);
    glBindAttribLocation(program, 0, "input_active");
    glBindAttribLocation(program, 0, "input_unused");
    glLinkProgram(program);
    EXPECT_GL_NO_ERROR();
    GLint linkStatus = 0;
    glGetProgramiv(program, GL_LINK_STATUS, &linkStatus);
    EXPECT_GL_FALSE(linkStatus);
}

// Use this to select which configurations (e.g. which renderer, which GLES major version) these
// tests should be run against.
// D3D11 Feature Level 9_3 uses different D3D formats for vertex attribs compared to Feature Levels
// 10_0+, so we should test them separately.
ANGLE_INSTANTIATE_TEST(VertexAttributeTest,
                       ES2_D3D9(),
                       ES2_D3D11(),
                       ES2_D3D11_FL9_3(),
                       ES2_OPENGL(),
                       ES3_OPENGL(),
                       ES2_OPENGLES(),
                       ES3_OPENGLES());

ANGLE_INSTANTIATE_TEST(VertexAttributeTestMaps,
                       ES2_OPENGL(),
                       ES3_OPENGL(),
                       ES2_OPENGLES(),
                       ES3_OPENGLES(),
                       ES2_VULKAN());

ANGLE_INSTANTIATE_TEST(VertexAttributeTestES3, ES3_D3D11(), ES3_OPENGL(), ES3_OPENGLES());

ANGLE_INSTANTIATE_TEST(VertexAttributeTestES31, ES31_D3D11(), ES31_OPENGL(), ES31_OPENGLES());

ANGLE_INSTANTIATE_TEST(VertexAttributeTestES31_OpenGL, ES31_OPENGL(), ES31_OPENGLES());

ANGLE_INSTANTIATE_TEST(VertexAttributeCachingTest,
                       ES2_D3D9(),
                       ES2_D3D11(),
                       ES3_D3D11(),
                       ES3_OPENGL());

}  // anonymous namespace
