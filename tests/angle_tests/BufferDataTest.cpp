#include "ANGLETest.h"

class BufferDataTest : public ANGLETest
{
protected:
    BufferDataTest()
    {
        setWindowWidth(1);
        setWindowHeight(1);
    }

    virtual void SetUp()
    {
        ANGLETest::SetUp();

        glGenBuffers(1, &mBuffer);
        ASSERT_NE(mBuffer, 0U);
    }

    virtual void TearDown()
    {
        glDeleteBuffers(1, &mBuffer);

        ANGLETest::TearDown();
    }

    GLuint mBuffer;
};

TEST_F(BufferDataTest, null_data)
{
    glBindBuffer(GL_ARRAY_BUFFER, mBuffer);
    EXPECT_GL_NO_ERROR();

    const int numIterations = 128;
    for (int i = 0; i < numIterations; ++i)
    {
        GLsizei bufferSize = sizeof(GLfloat) * (i + 1);
        glBufferData(GL_ARRAY_BUFFER, bufferSize, NULL, GL_STATIC_DRAW);
        EXPECT_GL_NO_ERROR();

        for (int j = 0; j < bufferSize; j++)
        {
            for (int k = 0; k < bufferSize - j; k++)
            {
                glBufferSubData(GL_ARRAY_BUFFER, k, j, NULL);
                EXPECT_GL_NO_ERROR();
            }
        }
    }
}

TEST_F(BufferDataTest, huge_setdata_should_not_crash)
{
    glBindBuffer(GL_ARRAY_BUFFER, mBuffer);
    EXPECT_GL_NO_ERROR();

    GLsizei hugeSize = (1 << 30);
    char *data = new (std::nothrow) char[hugeSize];
    EXPECT_NE((char * const)NULL, data);

    if (data == NULL)
    {
        return;
    }

    memset(data, 0, hugeSize);
    glBufferData(GL_ARRAY_BUFFER, hugeSize, data, GL_STATIC_DRAW);
    EXPECT_GL_ERROR(GL_OUT_OF_MEMORY);
    delete[] data;
}

