#include "ANGLETest.h"
#include <array>

class TransformFeedbackTest : public ANGLETest
{
protected:
    TransformFeedbackTest()
    {
        setWindowWidth(128);
        setWindowHeight(128);
        setConfigRedBits(8);
        setConfigGreenBits(8);
        setConfigBlueBits(8);
        setConfigAlphaBits(8);
        setConfigDepthBits(24);
        setClientVersion(3);
    }

    virtual void SetUp()
    {
        ANGLETest::SetUp();

        if (getClientVersion() < 3)
        {
            FAIL() << "client version must be at least 3 to test transform feedback.";
        }

        std::vector<std::string> vertexShaderSource(2);
        vertexShaderSource[0] = "#version 300 es\n";
        vertexShaderSource[1] = SHADER_SOURCE
        (
            precision highp float;

            in vec4 position;

            out float float_varying;
            out vec2 vec2_varying;
            out vec3 vec3_varying;
            out vec4 vec4_varying;
            out mat2 mat2_varying;
            out mat3 mat3_varying;
            out mat4 mat4_varying;
            out mat2x3 mat2x3_varying;
            out mat2x4 mat2x4_varying;
            out mat3x2 mat3x2_varying;
            out mat3x4 mat3x4_varying;
            out mat4x2 mat4x2_varying;
            out mat4x3 mat4x3_varying;

            flat out int int_varying;
            flat out ivec2 ivec2_varying;
            flat out ivec3 ivec3_varying;
            flat out ivec4 ivec4_varying;

            flat out uint uint_varying;
            flat out uvec2 uvec2_varying;
            flat out uvec3 uvec3_varying;
            flat out uvec4 uvec4_varying;

            void main()
            {
                gl_PointSize = 1.5;
                gl_Position = position;
            }
        );

        std::vector<std::string> fragmentShaderSource(2);
        fragmentShaderSource[0] = "#version 300 es\n";
        fragmentShaderSource[1] = SHADER_SOURCE
        (
            precision highp float;

            in vec2 vec2_varying;
            in mat2 mat2_varying;
            in mat3x4 mat3x4_varying;
            flat in uvec3 uvec3_varying;

            out vec4 fragColor;

            void main()
            {
                fragColor = vec4(vec2_varying.x, mat3x4_varying[0][0], 0.0, 1.0);
                fragColor = vec4(1.0, 0.0, 0.0, 1.0);
            }
        );

        mProgram = glCreateProgram();

        GLuint vs = compileShader(GL_VERTEX_SHADER, vertexShaderSource);
        GLuint fs = compileShader(GL_FRAGMENT_SHADER, fragmentShaderSource);

        if (vs == 0 || fs == 0)
        {
            glDeleteShader(fs);
            glDeleteShader(vs);
            glDeleteProgram(mProgram);
            FAIL();
        }

        glAttachShader(mProgram, vs);
        glDeleteShader(vs);

        glAttachShader(mProgram, fs);
        glDeleteShader(fs);

        const char *varyings[] =
        {
            "gl_Position",
            "float_varying",
            "gl_PointSize",
        };

        glTransformFeedbackVaryings(mProgram, ArraySize(varyings), varyings, GL_SEPARATE_ATTRIBS);
        glLinkProgram(mProgram);

        GLint linkStatus;
        glGetProgramiv(mProgram, GL_LINK_STATUS, &linkStatus);

        if (linkStatus == 0)
        {
            FAIL();
        }
    }

    virtual void TearDown()
    {
        glDeleteProgram(mProgram);

        ANGLETest::TearDown();
    }

    GLuint mProgram;
};

TEST_F(TransformFeedbackTest, primitive_count)
{
    glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT);

    GLuint query = 0;
    glGenQueries(1, &query);

    GLuint tfBuffers[4];
    glGenBuffers(ArraySize(tfBuffers), tfBuffers);

    const GLsizei bufferSize = 1 << 16;

    for (size_t i = 0; i < ArraySize(tfBuffers); i++)
    {
        glBindBuffer(GL_ARRAY_BUFFER, tfBuffers[i]);
        glBufferData(GL_ARRAY_BUFFER, bufferSize, NULL, GL_DYNAMIC_DRAW);
        glBindBuffer(GL_ARRAY_BUFFER, 0);

        glBindBufferBase(GL_TRANSFORM_FEEDBACK_BUFFER, i, tfBuffers[i]);
    }

    glBeginTransformFeedback(GL_TRIANGLES);

    glBeginQuery(GL_TRANSFORM_FEEDBACK_PRIMITIVES_WRITTEN, query);

    const size_t numDrawCalls = 50;
    for (size_t i = 0; i < numDrawCalls; i++)
    {
        float depth = 1.0f - (float(i) / (numDrawCalls - 1));
        drawQuad(mProgram, "position", depth);
    }

    glEndQuery(GL_TRANSFORM_FEEDBACK_PRIMITIVES_WRITTEN);

    glEndTransformFeedback();

    GLuint ready = GL_FALSE;
    while (ready == GL_FALSE)
    {
        Sleep(0);
        glGetQueryObjectuivEXT(query, GL_QUERY_RESULT_AVAILABLE_EXT, &ready);
    }

    GLuint result = GL_TRUE;
    glGetQueryObjectuivEXT(query, GL_QUERY_RESULT_EXT, &result);

    EXPECT_EQ(result, numDrawCalls * 2);

    glDeleteBuffers(ArraySize(tfBuffers), tfBuffers);
    glDeleteQueries(1, &query);
}

TEST_F(TransformFeedbackTest, limits)
{
    GLint maxSeperatedAttribs;
    glGetIntegerv(GL_MAX_TRANSFORM_FEEDBACK_SEPARATE_ATTRIBS, &maxSeperatedAttribs);

    GLint maxInterleavedComponents;
    glGetIntegerv(GL_MAX_TRANSFORM_FEEDBACK_INTERLEAVED_COMPONENTS, &maxInterleavedComponents);

    GLint maxSeperateComponents;
    glGetIntegerv(GL_MAX_TRANSFORM_FEEDBACK_SEPARATE_COMPONENTS, &maxSeperateComponents);

    ASSERT_GL_NO_ERROR();

    std::vector<std::string> vertexShaderSource(4);
    vertexShaderSource[0] = "#version 300 es\n";
    vertexShaderSource[1] = SHADER_SOURCE
    (
        precision highp float;

        in vec4 position;
    );

    std::vector<std::string> varyingNames(maxSeperatedAttribs);
    size_t varyingArraySize = maxSeperateComponents / 4;

    std::stringstream ss;
    for (GLint i = 0; i < maxSeperatedAttribs; i++)
    {
        std::stringstream nameStream;
        nameStream << "vec4_varying_" << i;
        varyingNames[i] = nameStream.str();

        ss << "out vec4 " << varyingNames[i] << "[" << varyingArraySize << "];\n";
    }
    vertexShaderSource[2] = ss.str();

    vertexShaderSource[3] = SHADER_SOURCE
    (
        void main()
        {
            gl_Position = position;
        }
    );

    std::vector<std::string> fragmentShaderSource(2);
    fragmentShaderSource[0] = "#version 300 es\n";
    fragmentShaderSource[1] = SHADER_SOURCE
    (
        precision highp float;

        out vec4 fragColor;

        void main()
        {
            fragColor = vec4(1.0, 0.0, 0.0, 1.0);
        }
    );

    GLuint program = glCreateProgram();

    GLuint vs = compileShader(GL_VERTEX_SHADER, vertexShaderSource);
    GLuint fs = compileShader(GL_FRAGMENT_SHADER, fragmentShaderSource);

    if (vs == 0 || fs == 0)
    {
        glDeleteShader(fs);
        glDeleteShader(vs);
        glDeleteProgram(program);
        FAIL();
    }

    glAttachShader(program, vs);
    glDeleteShader(vs);

    glAttachShader(program, fs);
    glDeleteShader(fs);

    std::vector<const char*> varyingCStrings(maxSeperatedAttribs);
    for (GLint i = 0; i < maxSeperatedAttribs; i++)
    {
        varyingCStrings[i] = varyingNames[i].c_str();
    }

    glTransformFeedbackVaryings(program, varyingCStrings.size(), varyingCStrings.data(), GL_INTERLEAVED_ATTRIBS);
    glLinkProgram(program);

    GLint linkStatus;
    glGetProgramiv(program, GL_LINK_STATUS, &linkStatus);

    if (linkStatus == 0)
    {
        GLint infoLogLength;
        glGetProgramiv(program, GL_INFO_LOG_LENGTH, &infoLogLength);

        std::vector<GLchar> infoLog(infoLogLength);
        glGetProgramInfoLog(program, infoLog.size(), NULL, infoLog.data());

        std::cerr << "program link failed: " << infoLog.data();

        glDeleteProgram(program);
    }
    ASSERT_NE(linkStatus, 0);

    glBeginTransformFeedback(GL_TRIANGLES);
    drawQuad(program, "position", 0.5f);
    glEndTransformFeedback();

    EXPECT_GL_NO_ERROR();
}


struct varying
{
    std::string name;
    GLenum type;
    GLsizei size;
};

static varying readVarying(GLuint program, GLuint index)
{
    GLint maxLen;
    glGetProgramiv(program, GL_TRANSFORM_FEEDBACK_VARYING_MAX_LENGTH, &maxLen);

    varying var;

    std::vector<GLchar> nameBuf(maxLen);
    GLsizei length;
    glGetTransformFeedbackVarying(program, index, nameBuf.size(), &length, &var.size, &var.type, nameBuf.data());
    var.name = nameBuf.data();

    return var;
}

TEST_F(TransformFeedbackTest, program_binary)
{
    GLint originalVaryingCount = 0;
    glGetProgramiv(mProgram, GL_TRANSFORM_FEEDBACK_VARYINGS, &originalVaryingCount);

    EXPECT_GL_NO_ERROR();

    std::vector<varying> varyings;
    for (GLint i = 0; i < originalVaryingCount; i++)
    {
        varyings.push_back(readVarying(mProgram, i));
    }

    EXPECT_GL_NO_ERROR();

    GLint programBinarySize = 0;
    glGetProgramiv(mProgram, GL_PROGRAM_BINARY_LENGTH, &programBinarySize);

    EXPECT_GL_NO_ERROR();

    std::vector<unsigned char> programBinary(programBinarySize);
    GLsizei length;
    GLenum binaryFormat;
    glGetProgramBinaryOES(mProgram, programBinary.size(), &length, &binaryFormat, programBinary.data());

    EXPECT_GL_NO_ERROR();

    GLuint newProgram = glCreateProgram();
    glProgramBinaryOES(newProgram, binaryFormat, programBinary.data(), length);

    EXPECT_GL_NO_ERROR();

    GLint newVaryingCount = 0;
    glGetProgramiv(newProgram, GL_TRANSFORM_FEEDBACK_VARYINGS, &newVaryingCount);

    EXPECT_GL_NO_ERROR();
    EXPECT_EQ(newVaryingCount, originalVaryingCount);

    for (GLint i = 0; i < originalVaryingCount; i++)
    {
        const varying originalVarying = varyings[i];
        varying newVarying = readVarying(newProgram, i);

        EXPECT_EQ(originalVarying.name, newVarying.name);
        EXPECT_EQ(originalVarying.type, newVarying.type);
        EXPECT_EQ(originalVarying.size, newVarying.size);
    }

    EXPECT_GL_NO_ERROR();

    glDeleteProgram(newProgram);
}
