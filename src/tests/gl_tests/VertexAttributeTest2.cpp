//
// Copyright 2018 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//

#include <array>
#include <vector>

#include "test_utils/ANGLETest.h"

using namespace angle;

namespace
{

// Test will draw these four triangles.
// clang-format off
const GLfloat triangleData[] = {
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

constexpr size_t numVertex = sizeof(triangleData) / sizeof(triangleData[0]) / 5;

typedef std::array<GLfloat, 2> Vec2;
typedef std::array<Vec2, numVertex> AttribData;

// A container for one or more vertex attributes.
class Container
{
  public:
    // Size of container, in Vec2's.  At most we need numVertex * 4
    // but we make it larger so we can test with loosely packed data.
    static constexpr size_t kNumAttrib = numVertex * 10;

    virtual void init(void)                                               = 0;
    virtual void enable(unsigned index, unsigned offset, unsigned stride) = 0;

    void fill(unsigned offset, unsigned stride, const AttribData &data)
    {
        for (unsigned i = 0; i < data.size(); ++i)
        {
            // Check for two possible errors in the test case:
            // - data does not fit in container
            ASSERT(offset < kNumAttrib);
            // - data overlap
            ASSERT(mUsed[offset] == false);

            putItem(offset, data[i]);
            mUsed[offset] = true;
            offset += stride;
        }
    }

  protected:
    virtual void putItem(unsigned index, const Vec2 &item) = 0;

    std::array<bool, kNumAttrib> mUsed;
};

// Vertex attribute data in client memory.
class Memory : public Container
{
  public:
    void init(void) override
    {
        mMemory.fill({0, 0});
        mUsed.fill(false);
    }

    void enable(unsigned index, unsigned offset, unsigned stride) override
    {
        glBindBuffer(GL_ARRAY_BUFFER, 0);
        glVertexAttribPointer(index, 2, GL_FLOAT, GL_FALSE, stride * sizeof(Vec2),
                              mMemory.data() + offset);
    }

  protected:
    void putItem(unsigned index, const Vec2 &item) override { mMemory[index] = item; }

    std::array<Vec2, kNumAttrib> mMemory;
};

// Vertex attribute data in buffer object.
class Buffer : public Container
{
  public:
    Buffer() : mBuffer(0) {}

    void init(void) override
    {
        if (!mBuffer)
        {
            glGenBuffers(1, &mBuffer);
            mZeroes.fill({0, 0});
        }
        glBindBuffer(GL_ARRAY_BUFFER, mBuffer);
        glBufferData(GL_ARRAY_BUFFER, sizeof(mZeroes), mZeroes.data(), GL_STATIC_DRAW);
        mUsed.fill(false);
    }

    void enable(unsigned index, unsigned offset, unsigned stride) override
    {
        glBindBuffer(GL_ARRAY_BUFFER, mBuffer);
        glVertexAttribPointer(index, 2, GL_FLOAT, GL_FALSE, stride * sizeof(Vec2),
                              (Vec2 *)(0) + offset);
    }

  protected:
    void putItem(unsigned index, const Vec2 &item) override
    {
        glBindBuffer(GL_ARRAY_BUFFER, mBuffer);
        glBufferSubData(GL_ARRAY_BUFFER, index * sizeof(Vec2), sizeof(Vec2), &item);
    }

    GLenum mBuffer;
    static std::array<Vec2, kNumAttrib> mZeroes;
};
std::array<Vec2, Buffer::kNumAttrib> Buffer::mZeroes;

// Encapsulates the data for one vertex attribute, where it lives, and how it is layed out.
class Attrib
{
  public:
    // Offset and stride are in terms of Vec2's, not bytes.
    Attrib(Container &container, unsigned offset, unsigned stride, const AttribData &data)
        : mContainer(container), mOffset(offset), mStride(stride), mData(data)
    {
    }

    void initContainer(void) const { mContainer.init(); }

    void fillContainer(void) const { mContainer.fill(mOffset, mStride, mData); }

    void enable(unsigned index) const
    {
        mContainer.enable(index, mOffset, mStride);
        glEnableVertexAttribArray(index);
    }

  protected:
    Container &mContainer;
    unsigned mOffset;
    unsigned mStride;
    const AttribData &mData;
};

AttribData a0, a1, a2, a3;
Memory M0, M1, M2, M3;
Buffer B0;

/* per case:
0 vs non-0 starting vertex - just some offset when we fill out the attrib data
non-indexed, indices in mem, indices in buf
*/
typedef std::vector<Attrib> TestCase;
// clang-format off
const TestCase testCases[] = {
    //{ Attrib(M0, 0, 1, a0), Attrib(B0, 0, 1, a1), Attrib(M2, 0, 1, a2), Attrib(M3, 0, 1, a3), },
    { Attrib(M0, 0, 4, a0), Attrib(M0, 1, 4, a1), Attrib(M0, 2, 4, a2), Attrib(M0, 3, 4, a3), },
    { Attrib(B0, 0, 4, a0), Attrib(B0, 1, 4, a1), Attrib(B0, 2, 4, a2), Attrib(B0, 3, 4, a3), },
    //{ Attrib(M0, 0, 1, a0), Attrib(M0, 12, 1, a1), Attrib(M0, 24, 1, a2), Attrib(M0, 36, 1, a3), },
    //{ Attrib(B0, 0, 1, a0), Attrib(B0, 12, 1, a1), Attrib(B0, 24, 1, a2), Attrib(B0, 36, 1, a3), },
};
// clang-format on

// Transform 5 floats (x,y,r,g,b) into 4 vec2 attribs.  The shader reverses the process to
// recover (x,y,r,g,b).
// After transformation there are no zeroes in the data, to contrast with the zero-initialized
// storage, hopefully making bugs more apparent.
void PrepareAttribData(void)
{
    a0.fill({0, 0});
    a1.fill({0, 0});
    a2.fill({0, 0});
    a3.fill({0, 0});

    for (unsigned i = 0; i < numVertex; ++i)
    {
        GLfloat x = triangleData[i * 5 + 0];
        GLfloat y = triangleData[i * 5 + 1];
        GLfloat r = triangleData[i * 5 + 2];
        GLfloat g = triangleData[i * 5 + 3];
        GLfloat b = triangleData[i * 5 + 4];

        a0[i][0] = x + 2;
        a0[i][1] = y + 2;
        a1[i][0] = r + 2;
        a2[i][0] = g + 2;
        a3[i][0] = b + 2;
    }
}

void PrepareTestCase(const TestCase &tc)
{
    for (const Attrib &a : tc)
    {
        a.initContainer();
    }
    for (const Attrib &a : tc)
    {
        a.fillContainer();
    }
    unsigned i = 0;
    for (const Attrib &a : tc)
    {
        a.enable(i++);
    }
}

class VertexAttributeTest2 : public ANGLETest
{
  protected:
    VertexAttributeTest2() : mProgram(0)
    {
        setWindowWidth(128);
        setWindowHeight(128);
        setConfigRedBits(8);
        setConfigGreenBits(8);
        setConfigBlueBits(8);
        setConfigAlphaBits(8);
        setConfigDepthBits(24);
    }

    void SetUp() override
    {
        ANGLETest::SetUp();

        glClearColor(0, 0, 0, 0);
        glClearDepthf(0.0);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        glDisable(GL_DEPTH_TEST);

        const std::string vertexSource =
            "attribute mediump vec2 a0;\n"
            "attribute mediump vec2 a1;\n"
            "attribute mediump vec2 a2;\n"
            "attribute mediump vec2 a3;\n"
            "varying mediump vec3 vcolor;\n"
            "void main(void)\n"
            "{\n"
            "    gl_Position = vec4(a0[0]-2., a0[1]-2., 0, 0);\n"
            "    vcolor = vec3(a1[0]-2., a2[0]-2., a3[0]-2.);\n"
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

        PrepareAttribData();
    }

    void TearDown() override
    {
        glDeleteProgram(mProgram);
        ANGLETest::TearDown();
    }

    enum DrawMethod
    {
        kNonIndexed,
        kMemoryIndexed,
        kBufferIndexed
    };

    void Draw(DrawMethod method)
    {
        if (method == kNonIndexed)
        {
            glDrawArrays(GL_TRIANGLES, 0, numVertex);
        }
        else if (method == kMemoryIndexed)
        {
            glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
            glDrawElements(GL_TRIANGLES, numVertex, GL_UNSIGNED_SHORT, mIndices);
        }
        else
        {
            glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, mIndexBuffer);
            glDrawElements(GL_TRIANGLES, numVertex, GL_UNSIGNED_SHORT, nullptr);
        }
    }

    void Run(DrawMethod method)
    {
        glViewport(0, 0, getWindowWidth(), getWindowHeight());
        glUseProgram(mProgram);

        for (unsigned i = 0; i < sizeof(testCases) / sizeof(testCases[0]); ++i)
        {
            PrepareTestCase(testCases[i]);

            glClear(GL_COLOR_BUFFER_BIT);
            Draw(method);

            // swapBuffers(); sleep(2);

            int w = getWindowWidth() / 4;
            int h = getWindowHeight() / 4;
            EXPECT_PIXEL_EQ(w * 1, h * 2, 255, 0, 255, 0) << " test case " << i;
            EXPECT_PIXEL_EQ(w * 3, h * 2, 0, 255, 0, 0) << " test case " << i;
            EXPECT_PIXEL_EQ(w * 2, h * 1, 0, 255, 255, 0) << " test case " << i;
            EXPECT_PIXEL_EQ(w * 2, h * 3, 255, 255, 0, 0) << " test case " << i;
        }
    }

    GLuint mProgram;
    GLuint mIndexBuffer;
    static const GLushort mIndices[numVertex];
};
const GLushort VertexAttributeTest2::mIndices[numVertex] = {0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11};

TEST_P(VertexAttributeTest2, MemoryIndexed)
{
    Run(kMemoryIndexed);
}

TEST_P(VertexAttributeTest2, BufferIndexed)
{
    // Run(kBufferIndexed);
}

TEST_P(VertexAttributeTest2, NonIndexed)
{
    Run(kNonIndexed);
}

ANGLE_INSTANTIATE_TEST(VertexAttributeTest2,
                       // ES2_D3D9(),
                       // ES2_D3D11(),
                       // ES2_D3D11_FL9_3(),
                       ES2_OPENGL(),
                       // ES2_VULKAN(),
                       // ES3_OPENGL(),
                       // ES2_OPENGLES(),
                       // ES3_OPENGLES()
                       );

}  // anonymous namespace
