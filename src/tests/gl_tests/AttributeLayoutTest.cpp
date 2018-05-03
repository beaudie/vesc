//
// Copyright 2018 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//
// AttributeLayoutTest:
//   Test various layouts of vertex attribute data:
//   - in memory, in buffer object, or combination of both
//   - sequential or interleaved
//   - various combinations of data types

#include <vector>

#include "test_utils/ANGLETest.h"
#include "test_utils/gl_raii.h"

using namespace angle;

namespace
{

// Test will draw these four triangles.
// clang-format off
const double triangleData[] = {
    // xy       rgb
    0,0,        1,1,0,
    -1,+1,      1,1,0,
    +1,+1,      1,1,0,

    0,0,        0,1,0,
    +1,+1,      0,1,0,
    +1,-1,      0,1,0,

    0,0,        0,1,1,
    +1,-1,      0,1,1,
    -1,-1,      0,1,1,

    0,0,        1,0,1,
    -1,-1,      1,0,1,
    -1,+1,      1,0,1,
};
// clang-format on

constexpr size_t kNumVertices = ArraySize(triangleData) / 5;

// Vertex data source description.
class VertexData
{
  public:
    VertexData(int dimension, const double *data, unsigned offset, unsigned stride)
        : mDimension(dimension), mData(data), mOffset(offset), mStride(stride)
    {
    }
    int getDimension() const { return mDimension; }
    double getValue(unsigned vertexNumber, int component) const
    {
        return mData[mOffset + mStride * vertexNumber + component];
    }

  private:
    int mDimension;
    const double *mData;
    // offset and stride in doubles
    unsigned mOffset;
    unsigned mStride;
};

typedef void (*ConvertFunc)(double value, void *vdest);

// Format information and a function to convert from double.
template <class CType, GLenum GLType, bool Normalized>
struct Format
{
    static constexpr GLenum kGLType    = GLType;
    static constexpr bool kNormalized  = Normalized;
    static constexpr size_t kCTypeSize = sizeof(CType);
    static constexpr double kMaxInt    = (1ull << (8 * kCTypeSize)) - 1;

    static_assert(!(kNormalized && kGLType == GL_FLOAT), "Normalized float does not make sense.");

    static void convert(double value, void *vdest)
    {
        CType *dest = reinterpret_cast<CType *>(vdest);
        if (kGLType == GL_FIXED)
            *dest = value * (1 << 16);
        else if (kNormalized)
            *dest = std::is_signed<CType>::value ? ((value * kMaxInt) - 1) / 2 : value * kMaxInt;
        else
            *dest = value;
    }
};

Format<GLfloat, GL_FLOAT, false> Float;
Format<GLint, GL_FIXED, false> Fixed;

Format<GLint, GL_INT, false> SI;
Format<GLint, GL_INT, true> NSI;
Format<GLuint, GL_UNSIGNED_INT, false> UI;
Format<GLuint, GL_UNSIGNED_INT, true> NUI;

Format<GLshort, GL_SHORT, false> SS;
Format<GLshort, GL_SHORT, true> NSS;
Format<GLushort, GL_UNSIGNED_SHORT, false> US;
Format<GLushort, GL_UNSIGNED_SHORT, true> NUS;

Format<GLbyte, GL_BYTE, false> SB;
Format<GLbyte, GL_BYTE, true> NSB;
Format<GLubyte, GL_UNSIGNED_BYTE, false> UB;
Format<GLubyte, GL_UNSIGNED_BYTE, true> NUB;

// A container for one or more vertex attributes.
class Container
{
  public:
    static constexpr size_t kSize = 1024;

    void open(void) { memset(mMemory, 0xff, kSize); }
    void *getDestination(size_t offset) { return mMemory + offset; }
    virtual void close(void) {}
    virtual ~Container() {}
    virtual const char *getAddress() = 0;
    virtual GLuint getBuffer()       = 0;

  protected:
    char mMemory[kSize];
};

// Vertex attribute data in client memory.
class Memory : public Container
{
  public:
    const char *getAddress() override { return mMemory; }
    GLuint getBuffer() override { return 0; }
};

// Vertex attribute data in buffer object.
class Buffer : public Container
{
  public:
    void close(void) override
    {
        glBindBuffer(GL_ARRAY_BUFFER, mBuffer);
        glBufferData(GL_ARRAY_BUFFER, sizeof(mMemory), mMemory, GL_STATIC_DRAW);
    }

    const char *getAddress() override { return nullptr; }
    GLuint getBuffer() override { return mBuffer; }

  protected:
    GLBuffer mBuffer;
};

// Encapsulates the data for one vertex attribute: container, layout, format.
class Attrib
{
  public:
    template <class Format>
    Attrib(std::shared_ptr<Container> container,
           unsigned offset,
           unsigned stride,
           const VertexData &data,
           Format)
        : mContainer(container),
          mOffset(offset),
          mStride(stride),
          mData(data),
          mConvert(Format::convert),
          mGLType(Format::kGLType),
          mNormalized(Format::kNormalized),
          mCTypeSize(Format::kCTypeSize)
    {
        ASSERT(mStride >= mCTypeSize * mData.getDimension());

        // Compiler complains about unused variable without these.
        (void)Float;
        (void)Fixed;
        (void)SI;
        (void)NSI;
        (void)UI;
        (void)NUI;
        (void)SS;
        (void)NSS;
        (void)US;
        (void)NUS;
        (void)SB;
        (void)NSB;
        (void)UB;
        (void)NUB;
    }

    void openContainer(void) const { mContainer->open(); }

    void fillContainer(void) const
    {
        for (unsigned i = 0; i < kNumVertices; ++i)
        {
            for (int j = 0; j < mData.getDimension(); ++j)
            {
                size_t destOffset = mOffset + mStride * i + mCTypeSize * j;
                ASSERT(destOffset + mCTypeSize <= Container::kSize);
                mConvert(mData.getValue(i, j), mContainer->getDestination(destOffset));
            }
        }
    }

    void closeContainer(void) const { mContainer->close(); }

    void enable(unsigned index) const
    {
        glBindBuffer(GL_ARRAY_BUFFER, mContainer->getBuffer());
        glVertexAttribPointer(index, mData.getDimension(), mGLType, mNormalized, mStride,
                              mContainer->getAddress() + mOffset);
        EXPECT_GL_NO_ERROR();
        glEnableVertexAttribArray(index);
    }

    bool inClientMemory(void) const { return mContainer->getAddress() != nullptr; }

  protected:
    std::shared_ptr<Container> mContainer;
    unsigned mOffset;
    unsigned mStride;
    const VertexData &mData;
    ConvertFunc mConvert;
    GLenum mGLType;
    GLboolean mNormalized;
    size_t mCTypeSize;
};

typedef std::vector<Attrib> TestCase;

void PrepareTestCase(const TestCase &tc)
{
    for (const Attrib &a : tc)
    {
        a.openContainer();
    }
    for (const Attrib &a : tc)
    {
        a.fillContainer();
    }
    for (const Attrib &a : tc)
    {
        a.closeContainer();
    }
    unsigned i = 0;
    for (const Attrib &a : tc)
    {
        a.enable(i++);
    }
}

class AttributeLayoutTest : public ANGLETest
{
  protected:
    AttributeLayoutTest()
        : mProgram(0), mCoord(2, triangleData, 0, 5), mColor(3, triangleData, 2, 5)
    {
        setWindowWidth(128);
        setWindowHeight(128);
        setConfigRedBits(8);
        setConfigGreenBits(8);
        setConfigBlueBits(8);
        setConfigAlphaBits(8);
    }

    void GetTestCases(void);

    void SetUp() override
    {
        ANGLETest::SetUp();

        glClearColor(.2f, .2f, .2f, .0f);
        glClear(GL_COLOR_BUFFER_BIT);

        glDisable(GL_DEPTH_TEST);

        const std::string vertexSource =
            "attribute mediump vec2 coord;\n"
            "attribute mediump vec3 color;\n"
            "varying mediump vec3 vcolor;\n"
            "void main(void)\n"
            "{\n"
            "    gl_Position = vec4(coord, 0, 1);\n"
            "    vcolor = color;\n"
            "}\n";

        const std::string fragmentSource =
            "varying mediump vec3 vcolor;\n"
            "void main(void)\n"
            "{\n"
            "    gl_FragColor = vec4(vcolor, 0);\n"
            "}\n";

        mProgram = CompileProgram(vertexSource, fragmentSource);
        ASSERT_NE(0u, mProgram);
        glUseProgram(mProgram);

        glGenBuffers(1, &mIndexBuffer);

        GetTestCases();
    }

    void TearDown() override
    {
        mTestCases.clear();
        glDeleteProgram(mProgram);
        glDeleteBuffers(1, &mIndexBuffer);
        ANGLETest::TearDown();
    }

    virtual bool Skip(const TestCase &) { return false; }
    virtual void Draw(int firstVertex, unsigned vertexCount, const GLushort *indices) = 0;

    void Run(bool drawFirstTriangle)
    {
        glViewport(0, 0, getWindowWidth(), getWindowHeight());
        glUseProgram(mProgram);

        for (unsigned i = 0; i < mTestCases.size(); ++i)
        {
            if (Skip(mTestCases[i]))
                continue;

            PrepareTestCase(mTestCases[i]);

            glClear(GL_COLOR_BUFFER_BIT);

            std::string testCase;
            if (drawFirstTriangle)
            {
                Draw(0, kNumVertices, mIndices);
                testCase = "draw";
            }
            else
            {
                Draw(3, kNumVertices - 3, mIndices + 3);
                testCase = "skip";
            }

            testCase += " first triangle case ";
            int w = getWindowWidth() / 4;
            int h = getWindowHeight() / 4;
            if (drawFirstTriangle)
            {
                EXPECT_PIXEL_EQ(w * 2, h * 3, 255, 255, 0, 0) << testCase << i;
            }
            else
            {
                EXPECT_PIXEL_EQ(w * 2, h * 3, 51, 51, 51, 0) << testCase << i;
            }
            EXPECT_PIXEL_EQ(w * 3, h * 2, 0, 255, 0, 0) << testCase << i;
            EXPECT_PIXEL_EQ(w * 2, h * 1, 0, 255, 255, 0) << testCase << i;
            EXPECT_PIXEL_EQ(w * 1, h * 2, 255, 0, 255, 0) << testCase << i;
        }
    }

    static const GLushort mIndices[kNumVertices];

    GLuint mProgram;
    GLuint mIndexBuffer;

    std::vector<TestCase> mTestCases;

    VertexData mCoord;
    VertexData mColor;
};
const GLushort AttributeLayoutTest::mIndices[kNumVertices] = {0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11};

void AttributeLayoutTest::GetTestCases(void)
{
    std::shared_ptr<Container> M0 = std::make_shared<Memory>();
    std::shared_ptr<Container> M1 = std::make_shared<Memory>();
    std::shared_ptr<Container> B0 = std::make_shared<Buffer>();
    std::shared_ptr<Container> B1 = std::make_shared<Buffer>();

    // 0. two buffers
    mTestCases.push_back({{B0, 0, 8, mCoord, Float}, {B1, 0, 12, mColor, Float}});

    // 1. two memory
    mTestCases.push_back({{M0, 0, 8, mCoord, Float}, {M1, 0, 12, mColor, Float}});

    // 2. one memory, sequential
    mTestCases.push_back({{M0, 0, 8, mCoord, Float}, {M0, 96, 12, mColor, Float}});

    // 3. one memory, interleaved
    mTestCases.push_back({{M0, 0, 20, mCoord, Float}, {M0, 8, 20, mColor, Float}});

    // 4. buffer and memory
    mTestCases.push_back({{B0, 0, 8, mCoord, Float}, {M0, 0, 12, mColor, Float}});

    // 5. stride != size
    mTestCases.push_back({{B0, 0, 16, mCoord, Float}, {B1, 0, 12, mColor, Float}});

    if (IsVulkan())
    {
        std::cout << "cases skipped on Vulkan: integer data, non-zero buffer offsets" << std::endl;
        return;
    }

    // 6. one buffer, sequential
    mTestCases.push_back({{B0, 0, 8, mCoord, Float}, {B0, 96, 12, mColor, Float}});

    // 7. one buffer, interleaved
    mTestCases.push_back({{B0, 0, 20, mCoord, Float}, {B0, 8, 20, mColor, Float}});

    // 8. memory and buffer, float and integer
    mTestCases.push_back({{M0, 0, 8, mCoord, Float}, {B0, 0, 12, mColor, SB}});

    // 9. buffer and memory, unusual offset and stride
    mTestCases.push_back({{B0, 11, 13, mCoord, Float}, {M0, 23, 17, mColor, Float}});
}

class AttributeLayoutNonIndexed : public AttributeLayoutTest
{
    void Draw(int firstVertex, unsigned vertexCount, const GLushort *indices) override
    {
        glDrawArrays(GL_TRIANGLES, firstVertex, vertexCount);
    }
};

class AttributeLayoutMemoryIndexed : public AttributeLayoutTest
{
    void Draw(int firstVertex, unsigned vertexCount, const GLushort *indices) override
    {
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
        glDrawElements(GL_TRIANGLES, vertexCount, GL_UNSIGNED_SHORT, indices);
    }
};

class AttributeLayoutBufferIndexed : public AttributeLayoutTest
{
    void Draw(int firstVertex, unsigned vertexCount, const GLushort *indices) override
    {
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, mIndexBuffer);
        glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(*mIndices) * vertexCount, indices,
                     GL_STATIC_DRAW);
        glDrawElements(GL_TRIANGLES, vertexCount, GL_UNSIGNED_SHORT, nullptr);
    }
};

TEST_P(AttributeLayoutNonIndexed, Test)
{
    Run(true);

    if (IsWindows() && IsAMD() && IsOpenGL())
    {
        std::cout << "test skipped on Windows ATI OpenGL: non-indexed non-zero vertex start"
                  << std::endl;
        return;
    }

    Run(false);
}

TEST_P(AttributeLayoutMemoryIndexed, Test)
{
    Run(true);

    if (IsWindows() && IsAMD() && (IsOpenGL() || GetParam() == ES2_D3D11_FL9_3()))
    {
        std::cout << "test skipped on Windows ATI OpenGL and D3D11_9_3: indexed non-zero vertex start"
                  << std::endl;
        return;
    }

    Run(false);
}

TEST_P(AttributeLayoutBufferIndexed, Test)
{
    Run(true);

    if (IsWindows() && IsAMD() && (IsOpenGL() || GetParam() == ES2_D3D11_FL9_3()))
    {
        std::cout << "test skipped on Windows ATI OpenGL and D3D11_9_3: indexed non-zero vertex start"
                  << std::endl;
        return;
    }

    Run(false);
}

#define PARAMS                                                                            \
    ES2_VULKAN(), ES2_OPENGL(), ES2_D3D9(), ES2_D3D11(), ES2_D3D11_FL9_3(), ES3_OPENGL(), \
        ES2_OPENGLES(), ES3_OPENGLES()

ANGLE_INSTANTIATE_TEST(AttributeLayoutNonIndexed, PARAMS);
ANGLE_INSTANTIATE_TEST(AttributeLayoutMemoryIndexed, PARAMS);
ANGLE_INSTANTIATE_TEST(AttributeLayoutBufferIndexed, PARAMS);

}  // anonymous namespace
