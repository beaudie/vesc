#include "test_utils/ANGLETest.h"
#include "test_utils/gl_raii.h"

using namespace angle;

// clang-format off
const GLfloat coord[] = {
    0,0,     
    -1,+1,  
    +1,+1, 
    0,0,       
    +1,+1,    
    +1,-1,   
    0,0,       
    +1,-1,    
    -1,-1,   
    0,0,       
    -1,-1,    
    -1,+1,   
};
const GLfloat color[] = {
    1,1,0,
    1,1,0,
    1,1,0,
    0,1,0,
    0,1,0,
    0,1,0,
    0,1,1,
    0,1,1,
    0,1,1,
    1,0,1,
    1,0,1,
    1,0,1,
};
// clang-format on

class BufferSubDataTest : public ANGLETest
{
  protected:
    BufferSubDataTest() : mProgram(0)
    {
        setWindowWidth(128);
        setWindowHeight(128);
        setConfigRedBits(8);
        setConfigGreenBits(8);
        setConfigBlueBits(8);
        setConfigAlphaBits(8);
    }

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
    }

    void TearDown() override
    {
        glDeleteProgram(mProgram);
        ANGLETest::TearDown();
    }

    GLuint mProgram;
};

TEST_P(BufferSubDataTest, WTF)
{
    glViewport(0, 0, getWindowWidth(), getWindowHeight());
    glUseProgram(mProgram);

    GLBuffer B0, B1;

    glBindBuffer(GL_ARRAY_BUFFER, B0);
    glBufferData(GL_ARRAY_BUFFER, sizeof(coord), coord, GL_STATIC_DRAW);

    glBindBuffer(GL_ARRAY_BUFFER, B1);
    glBufferData(GL_ARRAY_BUFFER, sizeof(color), color, GL_STATIC_DRAW);

    glBindBuffer(GL_ARRAY_BUFFER, B0);
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 8, 0);
    glEnableVertexAttribArray(0);

    glBindBuffer(GL_ARRAY_BUFFER, B1);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 12, 0);
    glEnableVertexAttribArray(1);

    const int w = getWindowWidth() / 4;
    const int h = getWindowHeight() / 4;

    for (unsigned i = 0; i < 2; ++i)
    {
        printf("LOOP %u\n", i);

        // test fails on linux/intel gpu, AFTER the first time around the loop (first time works)
        // comment out the glBufferSubData lines and the test works
        glBindBuffer(GL_ARRAY_BUFFER, B0);
        for (unsigned i = 0; i < ArraySize(coord); ++i)
            glBufferSubData(GL_ARRAY_BUFFER, i * sizeof(GLfloat), sizeof(GLfloat), coord + i);

        glBindBuffer(GL_ARRAY_BUFFER, B1);
        for (unsigned i = 0; i < ArraySize(color); ++i)
            glBufferSubData(GL_ARRAY_BUFFER, i * sizeof(GLfloat), sizeof(GLfloat), color + i);

        glClear(GL_COLOR_BUFFER_BIT);
        glDrawArrays(GL_TRIANGLES, 0, 12);

        EXPECT_PIXEL_EQ(w * 2, h * 3, 255, 255, 0, 0);
        EXPECT_PIXEL_EQ(w * 3, h * 2, 0, 255, 0, 0);
        EXPECT_PIXEL_EQ(w * 2, h * 1, 0, 255, 255, 0);
        EXPECT_PIXEL_EQ(w * 1, h * 2, 255, 0, 255, 0);
    }
}

ANGLE_INSTANTIATE_TEST(BufferSubDataTest, ES2_VULKAN());
