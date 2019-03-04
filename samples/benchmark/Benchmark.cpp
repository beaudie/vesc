//
// Copyright (c) 2014 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//

#include <assert.h>
#include "SampleApplication.h"

#include "common/vector_utils.h"
#define _USE_MATH_DEFINES
#include <math.h>
#undef _USE_MATH_DEFINES

#include "util/shader_utils.h"

#define UNUSED(x) (void)(x)

struct TorusGeometry
{
    float innerRadius;
    float diameter;
};

struct TorusSampling
{
    unsigned radialSegmentCount;
    unsigned surfaceSegmentCount;
};

enum ShaderProgram
{
    SolidColor,
    Texture,
    Normal,
    DirectionalLight,
    EnvironmentMap,
    ShaderProgramCount
};

static const char *vertexProgramSource(ShaderProgram program)
{
    switch (program)
    {
        case ShaderProgram::SolidColor:
            return "attribute vec4 vertexPosition;"
                   "\n"
                   "uniform mat4 cameraTransformation;"
                   "\n"
                   "uniform mat4 componentTransformation;"
                   "\n"
                   "varying vec2 fragmentTexturePosition;"
                   "\n"
                   "uniform mat4 modelTransformation;"
                   "\n"
                   "void main()"
                   "\n"
                   "{"
                   "\n"
                   "  gl_Position = "
                   "cameraTransformation*modelTransformation*componentTransformation*"
                   "vertexPosition;"
                   "\n"
                   "}"
                   "\n";
        case ShaderProgram::Texture:
            return "attribute vec4 vertexPosition;"
                   "\n"
                   "attribute vec2 vertexTexturePosition;"
                   "\n"
                   "uniform mat4 cameraTransformation;"
                   "\n"
                   "uniform mat4 componentTransformation;"
                   "\n"
                   "uniform mat4 modelTransformation;"
                   "\n"
                   "varying vec2 fragmentTexturePosition;"
                   "\n"
                   "void main()"
                   "\n"
                   "{"
                   "\n"
                   "  gl_Position = "
                   "cameraTransformation*modelTransformation*componentTransformation*"
                   "vertexPosition;"
                   "\n"
                   "  fragmentTexturePosition = vertexTexturePosition;"
                   "\n"
                   "}"
                   "\n";
        case ShaderProgram::Normal:
            return "attribute vec4 vertexPosition;"
                   "\n"
                   "attribute vec3 vertexNormal;"
                   "\n"
                   "uniform mat4 cameraTransformation;"
                   "\n"
                   "uniform mat4 componentTransformation;"
                   "\n"
                   "varying vec2 fragmentTexturePosition;"
                   "\n"
                   "uniform mat4 modelTransformation;"
                   "\n"
                   "varying vec3 fragmentNormal;"
                   "\n"
                   "void main()"
                   "\n"
                   "{"
                   "\n"
                   "  gl_Position = "
                   "cameraTransformation*modelTransformation*componentTransformation*"
                   "vertexPosition;"
                   "\n"
                   "  fragmentNormal = vertexNormal;"
                   "\n"
                   "}"
                   "\n";
        case ShaderProgram::DirectionalLight:
            return "attribute vec4 vertexPosition;"
                   "\n"
                   "attribute vec3 vertexNormal;"
                   "\n"
                   "uniform mat4 cameraTransformation;"
                   "\n"
                   "varying vec2 fragmentTexturePosition;"
                   "\n"
                   "uniform mat4 componentTransformation;"
                   "\n"
                   "uniform mat4 modelTransformation;"
                   "\n"
                   "varying vec3 fragmentNormal;"
                   "\n"
                   "void main()"
                   "\n"
                   "{"
                   "\n"
                   "  gl_Position = "
                   "cameraTransformation*modelTransformation*componentTransformation*"
                   "vertexPosition;"
                   "\n"
                   "  vec3 vertexNormalCamera = "
                   "mat3(modelTransformation)*mat3(componentTransformation)*vertexNormal;"
                   "\n"
                   "  fragmentNormal = vertexNormalCamera;"
                   "\n"
                   "}"
                   "\n";
        case ShaderProgram::EnvironmentMap:
            return "attribute vec4 vertexPosition;"
                   "\n"
                   "attribute vec3 vertexNormal;"
                   "\n"
                   "uniform mat4 cameraTransformation;"
                   "\n"
                   "uniform mat4 componentTransformation;"
                   "\n"
                   "uniform mat4 modelTransformation;"
                   "\n"
                   "varying vec2 fragmentTexturePosition;"
                   "\n"
                   "varying vec3 fragmentNormalCamera;"
                   "\n"
                   "varying vec3 fragmentPositionCamera;"
                   "\n"
                   "void main()"
                   "\n"
                   "{"
                   "\n"
                   "  gl_Position = "
                   "cameraTransformation*modelTransformation*componentTransformation*"
                   "vertexPosition;"
                   "\n"
                   "  vec4 p = modelTransformation*componentTransformation*vertexPosition;"
                   "\n"
                   "  fragmentPositionCamera = p.xyz/p.w;"
                   "\n"
                   "  fragmentNormalCamera = "
                   "mat3(modelTransformation)*mat3(componentTransformation)*vertexNormal;"
                   "\n"
                   "}"
                   "\n";
        default:
            assert(false);
            return nullptr;
    }
}

static const char *fragmentProgramSource(ShaderProgram program)
{
    switch (program)
    {
        case ShaderProgram::SolidColor:
            return "precision mediump float;"
                   "\n"
                   "uniform vec4 color;"
                   "\n"
                   "uniform sampler2D sampler;"
                   "\n"
                   "varying vec2 fragmentTexturePosition;"
                   "\n"
                   "void main()"
                   "\n"
                   "{"
                   "\n"
                   "  gl_FragColor = "
                   "\n"
                   "    mix(texture2D(sampler, fragmentTexturePosition), vec4(color.r, color.g, "
                   "color.b, 1.0), 0.5);"
                   "\n"
                   "}"
                   "\n";
        case ShaderProgram::Texture:
            return "precision mediump float;"
                   "\n"
                   "uniform vec4 color;"
                   "\n"
                   "uniform sampler2D sampler;"
                   "\n"
                   "varying vec2 fragmentTexturePosition;"
                   "\n"
                   "void main()"
                   "\n"
                   "{"
                   "\n"
                   "  gl_FragColor = mix(color, texture2D(sampler, fragmentTexturePosition), 0.5);"
                   "\n"
                   "}"
                   "\n";
        case ShaderProgram::Normal:
            return "precision mediump float;"
                   "\n"
                   "varying vec3 fragmentNormal;"
                   "\n"
                   "uniform sampler2D sampler;"
                   "\n"
                   "varying vec2 fragmentTexturePosition;"
                   "\n"
                   "void main()"
                   "\n"
                   "{"
                   "\n"
                   "  vec3 color = (fragmentNormal + vec3(1,1,1))*0.5;"
                   "\n"
                   "  gl_FragColor = "
                   "\n"
                   "    mix(vec4(color, 1.0), texture2D(sampler, fragmentTexturePosition), 0.5);"
                   "\n"
                   "}"
                   "\n";
        case ShaderProgram::DirectionalLight:
            return "precision mediump float;"
                   "\n"
                   "varying vec3 fragmentNormal;"
                   "\n"
                   "uniform sampler2D sampler;"
                   "\n"
                   "varying vec2 fragmentTexturePosition;"
                   "\n"
                   "void main()"
                   "\n"
                   "{"
                   "\n"
                   "  vec3 lightNormal = vec3(-1, -1, 1)*(1.0/sqrt(3.0));"
                   "\n"
                   "  float shade = max(-dot(normalize(fragmentNormal), lightNormal), 0.0);"
                   "\n"
                   "  vec3 color = shade*vec3(0.0, 1.0, 0.5);"
                   "\n"
                   "  gl_FragColor = mix(texture2D(sampler, fragmentTexturePosition), vec4(color, "
                   "1.0), 0.5);"
                   "\n"
                   "}"
                   "\n";
        case ShaderProgram::EnvironmentMap:
            return "precision mediump float;"
                   "\n"
                   "varying vec3 fragmentNormalCamera;"
                   "\n"
                   "varying vec3 fragmentPositionCamera;"
                   "\n"
                   "uniform sampler2D sampler;"
                   "\n"
                   "varying vec2 fragmentTexturePosition;"
                   "\n"
                   "uniform vec4 color;"
                   "\n"
                   "#define R 1000.0"
                   "\n"
                   "#define PI 3.1415926538"
                   "\n"
                   "void main()"
                   "\n"
                   "{"
                   "\n"
                   "  vec3 n = normalize(fragmentNormalCamera);"
                   "\n"
                   "  vec3 p = fragmentPositionCamera;"
                   "\n"
                   "  float t = sqrt(pow(dot(p, n), 2.0) - dot(p, p) + pow(R, 2.0)) - dot(p, n);"
                   "\n"
                   "  vec3 q = p + t*n;"
                   "\n"
                   "  float theta = atan(length(q.xy), q.z);"
                   "\n"
                   "  float phi = atan(q.y, q.x);"
                   "\n"
                   "  float u = 0.5*(theta/PI + 1.0);"
                   "\n"
                   "  float v = 0.5*(phi/PI + 1.0);"
                   "\n"
                   "  float i = (floor(20.0*u))/20.0;"
                   "\n"
                   "  float j = (floor(20.0*v))/20.0;"
                   "\n"
                   "  float a = i;"
                   "\n"
                   "  vec3 c = vec3(a, a, a);"
                   "\n"
                   "  gl_FragColor = mix(texture2D(sampler, fragmentTexturePosition), mix(color, "
                   "vec4(c, 1.0), 0.5), 0.5);"
                   "\n"
                   "}"
                   "\n";
        default:
            assert(false);
            return nullptr;
    }
}

static unsigned torusVertexCount(const TorusSampling &sampling)
{
    const unsigned radialSliceCount  = sampling.radialSegmentCount;
    const unsigned surfaceSliceCount = sampling.surfaceSegmentCount;
    const unsigned vertexCount       = radialSliceCount * surfaceSliceCount;
    return vertexCount;
}

static unsigned torusIndexCount(const TorusSampling &sampling)
{
    const unsigned quadCount     = sampling.radialSegmentCount * sampling.surfaceSegmentCount;
    const unsigned triangleCount = 2 * quadCount;
    const unsigned indexCount    = 3 * triangleCount;
    return indexCount;
}

// * 'positions' is assumed to have capacity for 3*torusVertexCount(sampling) elements
static void torusVertexPositions(const TorusSampling &sampling,
                                 const TorusGeometry &geometry,
                                 float *positions)
{
    using namespace angle;
    const unsigned radialSliceCount  = sampling.radialSegmentCount;
    const unsigned surfaceSliceCount = sampling.surfaceSegmentCount;
    const float r                    = geometry.diameter / 2.0f;
    const float centerRadius         = geometry.innerRadius + r;
    const Vector3 axialDirection(0.0f, 0.0f, 1.0f);
    for (unsigned radialSliceIdx = 0; radialSliceIdx < radialSliceCount; radialSliceIdx++)
    {
        const float zAngle = (2.0f * M_PI / sampling.radialSegmentCount) * radialSliceIdx;
        const Vector3 radialDirection(cosf(zAngle), sinf(zAngle), 0.0f);
        for (unsigned surfaceSliceIdx = 0; surfaceSliceIdx < surfaceSliceCount; surfaceSliceIdx++)
        {
            const float surfaceAngle =
                (2.0f * M_PI / sampling.surfaceSegmentCount) * surfaceSliceIdx;
            const Vector3 position = (centerRadius + r * cosf(surfaceAngle)) * radialDirection +
                                     r * sinf(surfaceAngle) * axialDirection;
            positions[0] = position.x();
            positions[1] = position.y();
            positions[2] = position.z();
            positions += 3;
        }
    }
}

// * 'normals' is assumed to have capacity for 3*torusVertexCount(sampling) elements
static void torusVertexNormals(const TorusSampling &sampling,
                               const TorusGeometry &geometry,
                               float *normals)
{
    using namespace angle;
    const unsigned radialSliceCount  = sampling.radialSegmentCount;
    const unsigned surfaceSliceCount = sampling.surfaceSegmentCount;
    const Vector3 axialDirection(0.0f, 0.0f, 1.0f);
    for (unsigned radialSliceIdx = 0; radialSliceIdx < radialSliceCount; radialSliceIdx++)
    {
        const float zAngle = (2.0f * M_PI / sampling.radialSegmentCount) * radialSliceIdx;
        const Vector3 radialDirection(cosf(zAngle), sinf(zAngle), 0.0f);
        for (unsigned surfaceSliceIdx = 0; surfaceSliceIdx < surfaceSliceCount; surfaceSliceIdx++)
        {
            const float surfaceAngle =
                (2.0f * M_PI / sampling.surfaceSegmentCount) * surfaceSliceIdx;
            const Vector3 normal =
                cosf(surfaceAngle) * radialDirection + sinf(surfaceAngle) * axialDirection;
            normals[0] = normal.x();
            normals[1] = normal.y();
            normals[2] = normal.z();
            normals += 3;
        }
    }
}

// * 'positions' is assumed to have capacity for 2*torusVertexCount(sampling) elements
static void torusVertexTexturePositions(const TorusSampling &sampling, float *positions)
{
    using namespace angle;
    const unsigned radialSegmentCount  = sampling.radialSegmentCount;
    const unsigned radialSliceCount    = radialSegmentCount;
    const unsigned surfaceSegmentCount = sampling.surfaceSegmentCount;
    const unsigned surfaceSliceCount   = surfaceSegmentCount;
    for (unsigned radialSliceIdx = 0; radialSliceIdx < radialSliceCount; radialSliceIdx++)
    {
        const float u = (1.0f / float(radialSegmentCount)) * radialSliceIdx;
        for (unsigned surfaceSliceIdx = 0; surfaceSliceIdx < surfaceSliceCount; surfaceSliceIdx++)
        {
            const float v = (1.0f / float(surfaceSegmentCount)) * surfaceSliceIdx;
            positions[0]  = u;
            positions[1]  = v;
            positions += 2;
        }
    }
}

// * 'indices' is assumed to have capacity for torusIndexCount(sampling) elements
static void torusIndices(const TorusSampling &sampling, uint32_t *indices)
{
    const unsigned radialSegmentCount  = sampling.radialSegmentCount;
    const unsigned radialSliceCount    = radialSegmentCount;
    const unsigned surfaceSegmentCount = sampling.surfaceSegmentCount;
    const unsigned surfaceSliceCount   = surfaceSegmentCount;
    for (unsigned radialSegmentIdx = 0; radialSegmentIdx < radialSegmentCount; radialSegmentIdx++)
    {
        const int radialSliceIdxLo = radialSegmentIdx + 0;
        const int radialSliceIdxHi = (radialSliceIdxLo + 1) % radialSliceCount;
        for (unsigned surfaceSegmentIdx = 0; surfaceSegmentIdx < surfaceSegmentCount;
             surfaceSegmentIdx++)
        {
            const int surfaceSliceIdxLo = surfaceSegmentIdx + 0;
            const int surfaceSliceIdxHi = (surfaceSliceIdxLo + 1) % surfaceSliceCount;
            indices[0]                  = surfaceSliceCount * radialSliceIdxLo + surfaceSliceIdxLo;
            indices[1]                  = surfaceSliceCount * radialSliceIdxHi + surfaceSliceIdxLo;
            indices[2]                  = surfaceSliceCount * radialSliceIdxLo + surfaceSliceIdxHi;
            indices += 3;
            indices[0] = surfaceSliceCount * radialSliceIdxHi + surfaceSliceIdxLo;
            indices[1] = surfaceSliceCount * radialSliceIdxHi + surfaceSliceIdxHi;
            indices[2] = surfaceSliceCount * radialSliceIdxLo + surfaceSliceIdxHi;
            indices += 3;
        }
    }
}

static void assertNoGLError()
{
    GLenum errorCode = glGetError();
    assert(errorCode == GL_NO_ERROR);
    UNUSED(errorCode);
}

class BenchmarkSample : public SampleApplication
{
  public:
    static constexpr int kNumRows            = 4;
    static constexpr int kNumCols            = 4;
    static constexpr int kNumLayers          = 4;
    static constexpr int kRadialSliceCount   = 12;
    static constexpr int kShapeCount         = 1000;
    static constexpr int kShaderProgramCount = 3 * ShaderProgram::ShaderProgramCount;
    static constexpr int kTextureCount       = 5;

    BenchmarkSample(int argc, char **argv) : SampleApplication("Benchmark", argc, argv, 2, 0) {}

    bool initialize() override;
    void destroy() override;
    void draw() override;
    void step(float dt, double totalTime) override;

  private:
    struct
    {
        unsigned drawElements;
        unsigned bindBuffer;
        unsigned useProgram;
    } mCounters;
    void clearCounters();
    void printCounters();
    void drawCell(const int programIdx, int const textureIdx, const angle::Vector3 &cellCenter);
    void setupVertexAttributes(const int programIdx, const int shapeIdx);
    void setupProgram(const int programIdx, const int textureIdx, const angle::Vector3 &c);
    void runProgram(const angle::Vector3 &componentPosition, const int programIdx);
    void drawObject(const angle::Vector3 &layerPosition,
                    int radialSliceIdx,
                    const int programIdx,
                    const int textureIdx,
                    const int shapeIdx);

    GLsizei mIndexCount;
    GLuint mShaderPrograms[kShaderProgramCount];
    GLint mCameraTransformationMatrixUniformLocation[kShaderProgramCount];
    GLint mComponentTransformationMatrixUniformLocation[kShaderProgramCount];
    GLint mModelTransformationMatrixUniformLocation[kShaderProgramCount];
    GLint mPositionAttributeLocation[kShaderProgramCount];
    GLint mNormalAttributeLocation[kShaderProgramCount];
    GLint mColorUniformLocation[kShaderProgramCount];
    GLint mSamplerUniformLocation[kShaderProgramCount];
    GLint mTexturePositionAttributeLocation[kShaderProgramCount];
    uint64_t mPerformanceCounterFrequency;
    uint64_t mInitialPerformanceCount;
    uint64_t mFrameCount = 0;
    GLuint mIndexBuffer[kShapeCount];
    GLuint mVertexPositionBuffer[kShapeCount];
    GLuint mVertexTexturePositionBuffer[kShapeCount];
    GLuint mVertexNormalBuffer[kShapeCount];
    GLuint mTexture[kTextureCount];
    float mModelRotationZ;
};

void BenchmarkSample::clearCounters()
{
    mCounters = {};
}

void BenchmarkSample::printCounters()
{
    printf("%05d drawElements\n", mCounters.drawElements);
    printf("%05d bindBuffer\n", mCounters.bindBuffer);
    printf("%05d useProgram\n", mCounters.useProgram);
}

bool BenchmarkSample::initialize()
{

    TorusSampling torusSampling;
    torusSampling.radialSegmentCount  = 10;
    torusSampling.surfaceSegmentCount = 10;

    mIndexCount = torusIndexCount(torusSampling);

    {
        LARGE_INTEGER frequency;
        BOOL result  = QueryPerformanceFrequency(&frequency);
        bool failure = result == 0;
        if (failure)
        {
            ::exit(-1);
        }
        mPerformanceCounterFrequency = frequency.QuadPart;
    }

    for (int programIdx = 0; programIdx < kShaderProgramCount; programIdx++)
    {
        const ShaderProgram program =
            static_cast<ShaderProgram>(programIdx % ShaderProgram::ShaderProgramCount);
        mShaderPrograms[programIdx] =
            CompileProgram(vertexProgramSource(program), fragmentProgramSource(program));
        if (!mShaderPrograms[programIdx])
            ::exit(-1);
    }

    for (int programIdx = 0; programIdx < kShaderProgramCount; programIdx++)
    {
        GLuint program = mShaderPrograms[programIdx];
        {
            const GLchar *name = "cameraTransformation";
            mCameraTransformationMatrixUniformLocation[programIdx] =
                glGetUniformLocation(program, name);
            assertNoGLError();
            assert(mCameraTransformationMatrixUniformLocation[programIdx] != -1);
        }

        {
            const GLchar *name = "componentTransformation";
            mComponentTransformationMatrixUniformLocation[programIdx] =
                glGetUniformLocation(program, name);
            assertNoGLError();
            assert(mComponentTransformationMatrixUniformLocation[programIdx] != -1);
        }

        {
            const GLchar *name = "modelTransformation";
            mModelTransformationMatrixUniformLocation[programIdx] =
                glGetUniformLocation(program, name);
            assertNoGLError();
            assert(mModelTransformationMatrixUniformLocation[programIdx] != -1);
        }

        {
            const GLchar *name                     = "vertexPosition";
            mPositionAttributeLocation[programIdx] = glGetAttribLocation(program, name);
            assertNoGLError();
            assert(mPositionAttributeLocation[programIdx] != -1);
        }

        {
            GLuint program                       = mShaderPrograms[programIdx];
            const GLchar *name                   = "vertexNormal";
            mNormalAttributeLocation[programIdx] = glGetAttribLocation(program, name);
            assertNoGLError();
        }

        {
            mSamplerUniformLocation[programIdx] =
                glGetUniformLocation(mShaderPrograms[programIdx], "sampler");
            assertNoGLError();
        }

        {
            GLuint program                    = mShaderPrograms[programIdx];
            const GLchar *name                = "color";
            mColorUniformLocation[programIdx] = glGetUniformLocation(program, name);
            assertNoGLError();
        }

        {
            GLuint program                                = mShaderPrograms[programIdx];
            const GLchar *name                            = "vertexTexturePosition";
            mTexturePositionAttributeLocation[programIdx] = glGetAttribLocation(program, name);
            assertNoGLError();
        }
    }

    glClearColor(0.0f, 0.0f, 0.0f, 0.0f);

    {
        LARGE_INTEGER count;
        BOOL result = QueryPerformanceCounter(&count);
        if (result == 0)
        {
            ::exit(-1);
        }
        mInitialPerformanceCount = count.QuadPart;
    }

    {
        GLsizei count = kShapeCount;
        glGenBuffers(count, mIndexBuffer);
        assertNoGLError();
    }

    {
        GLsizei count = kShapeCount;
        glGenBuffers(count, mVertexPositionBuffer);
        assertNoGLError();
    }

    {
        GLsizei count = kShapeCount;
        glGenBuffers(count, mVertexTexturePositionBuffer);
        assertNoGLError();
    }

    {
        GLsizei count = kShapeCount;
        glGenBuffers(count, mVertexNormalBuffer);
        assertNoGLError();
    }

    for (int shapeIdx = 0; shapeIdx < kShapeCount; shapeIdx++)
    {
        TorusGeometry torusGeometry;
        float const s             = float(shapeIdx) / float(kShapeCount);
        torusGeometry.innerRadius = 0.2f + s * 0.5f;
        torusGeometry.diameter    = 0.5f + s * 0.4f;

        {
            GLenum target = GL_ELEMENT_ARRAY_BUFFER;
            GLuint buffer = mIndexBuffer[shapeIdx];
            glBindBuffer(target, buffer);
            mCounters.bindBuffer++;
            assertNoGLError();
        }

        {
            const GLsizeiptr bufferSize = sizeof(uint32_t) * torusIndexCount(torusSampling);
            uint32_t *const buffer      = (uint32_t *)malloc(bufferSize);
            torusIndices(torusSampling, buffer);
            GLenum usage = GL_STATIC_DRAW;
            glBufferData(GL_ELEMENT_ARRAY_BUFFER, bufferSize, buffer, usage);
            assertNoGLError();
        }

        {
            GLenum target = GL_ARRAY_BUFFER;
            GLuint buffer = mVertexPositionBuffer[shapeIdx];
            glBindBuffer(target, buffer);
            mCounters.bindBuffer++;
            assertNoGLError();
        }

        {
            const size_t bufferSize = sizeof(GLfloat) * 3 * torusVertexCount(torusSampling);
            float *const buffer     = (float *)malloc(bufferSize);
            torusVertexPositions(torusSampling, torusGeometry, buffer);
            GLenum usage = GL_STATIC_DRAW;
            glBufferData(GL_ARRAY_BUFFER, bufferSize, buffer, usage);
            assertNoGLError();
        }

        {
            GLenum target = GL_ARRAY_BUFFER;
            GLuint buffer = mVertexTexturePositionBuffer[shapeIdx];
            glBindBuffer(target, buffer);
            mCounters.bindBuffer++;
            assertNoGLError();
        }

        {
            const GLsizeiptr bufferSize = sizeof(GLfloat) * 2 * torusVertexCount(torusSampling);
            GLfloat *buffer             = (GLfloat *)malloc(bufferSize);
            torusVertexTexturePositions(torusSampling, buffer);
            GLenum usage = GL_STATIC_DRAW;
            glBufferData(GL_ARRAY_BUFFER, bufferSize, buffer, usage);
            assertNoGLError();
        }

        {
            GLenum target = GL_ARRAY_BUFFER;
            GLuint buffer = mVertexNormalBuffer[shapeIdx];
            glBindBuffer(target, buffer);
            mCounters.bindBuffer++;
            assertNoGLError();
        }

        {
            const GLsizeiptr bufferSize = sizeof(GLfloat) * 3 * torusVertexCount(torusSampling);
            GLfloat *buffer             = (GLfloat *)malloc(bufferSize);
            torusVertexNormals(torusSampling, torusGeometry, buffer);
            GLenum usage = GL_STATIC_DRAW;
            glBufferData(GL_ARRAY_BUFFER, bufferSize, buffer, usage);
            assertNoGLError();
        }
    }

    {
        GLsizei count = kTextureCount;
        glGenTextures(count, mTexture);
        assertNoGLError();
    }

    for (int textureIdx = 0; textureIdx < kTextureCount; textureIdx++)
    {
        {
            glActiveTexture(GL_TEXTURE0);
            assertNoGLError();
        }

        {
            GLenum target  = GL_TEXTURE_2D;
            GLuint texture = mTexture[textureIdx];
            glBindTexture(target, texture);
            assertNoGLError();
        }
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        assertNoGLError();
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        assertNoGLError();

        {
            GLenum target         = GL_TEXTURE_2D;
            GLint level           = 0;
            GLint internalFormat  = GL_RGBA;
            GLsizei width         = 1024;
            GLsizei height        = 1024;
            GLint border          = 0;
            GLenum format         = GL_RGBA;
            GLenum type           = GL_UNSIGNED_BYTE;
            uint8_t *const pixels = static_cast<uint8_t *>(malloc(4 * width * height));
            for (int colIdx = 0; colIdx < width; colIdx++)
            {
                for (int rowIdx = 0; rowIdx < height; rowIdx++)
                {
                    uint8_t *pixel = &pixels[4 * rowIdx * width + 4 * colIdx];
                    pixel[0]       = 255;                        // R
                    pixel[1]       = 255 * ((colIdx / 32) % 2);  // G
                    pixel[2]       = 255 * ((rowIdx / 32) % 2);  // B
                    pixel[3]       = 255;                        // A
                }
            }
            const GLvoid *data = pixels;
            glTexImage2D(target, level, internalFormat, width, height, border, format, type, data);
            free(pixels);
            assertNoGLError();
        }
    }

    return true;
}

void BenchmarkSample::destroy()
{
    for (int programIdx = 0; programIdx < kShaderProgramCount; programIdx++)
        glDeleteProgram(mShaderPrograms[programIdx]);
}

static void setCameraTransform(float dimensionsYXRatio, GLint location)
{
    const float zNear = 0.1f;
    const float zFar  = 1000.0f;
    // Angle of frustum shape about the Y angle.
    const float yAngle     = 60.0f * (M_PI / 180.0f);
    const float yAngleSine = sinf(yAngle);
    // Angle of frustum shape about the Y angle.
    const float xAngleSine = yAngleSine * dimensionsYXRatio;
    GLsizei count          = 1;
    // The data is provided in column major order
    GLboolean transposed    = GL_FALSE;
    const GLfloat value[16] = {
        1.0f / yAngleSine,
        0.0f,
        0.0f,
        0.0f,
        0.0f,
        1.0f / xAngleSine,
        0.0f,
        0.0f,
        0.0f,
        0.0f,
        (zNear + zFar) / (zFar - zNear),
        1.0f,
        0.0f,
        0.0f,
        2.0f * zFar * zNear / (zNear - zFar),
        0.0f,
    };
    glUniformMatrix4fv(location, count, transposed, value);
    assertNoGLError();
}

void BenchmarkSample::step(float dt, double totalTime)
{
    mModelRotationZ = 2.0f * M_PI * 0.015f * totalTime;
}

void BenchmarkSample::setupVertexAttributes(const int programIdx, const int shapeIdx)
{
    {
        GLenum target = GL_ARRAY_BUFFER;
        GLuint buffer = mVertexPositionBuffer[shapeIdx];
        glBindBuffer(target, buffer);
        mCounters.bindBuffer++;
        assertNoGLError();
    }

    {
        GLuint index          = mPositionAttributeLocation[programIdx];
        GLint size            = 3;
        GLenum type           = GL_FLOAT;
        GLboolean normalized  = GL_FALSE;
        GLsizei stride        = 0;
        const GLvoid *pointer = 0;
        glVertexAttribPointer(index, size, type, normalized, stride, pointer);
        assertNoGLError();
    }

    {
        GLenum target = GL_ARRAY_BUFFER;
        GLuint buffer = 0;
        glBindBuffer(target, buffer);
        mCounters.bindBuffer++;
        assertNoGLError();
    }

    {
        GLuint index = mPositionAttributeLocation[programIdx];
        glEnableVertexAttribArray(index);
        assertNoGLError();
    }

    if (mTexturePositionAttributeLocation[programIdx] != -1)
    {
        {
            GLenum target = GL_ARRAY_BUFFER;
            GLuint buffer = mVertexTexturePositionBuffer[shapeIdx];
            glBindBuffer(target, buffer);
            mCounters.bindBuffer++;
            assertNoGLError();
        }

        {
            GLuint index          = mTexturePositionAttributeLocation[programIdx];
            GLint size            = 2;
            GLenum type           = GL_FLOAT;
            GLboolean normalized  = GL_FALSE;
            GLsizei stride        = 0;
            const GLvoid *pointer = 0;
            glVertexAttribPointer(index, size, type, normalized, stride, pointer);
            assertNoGLError();
        }

        {
            GLenum target = GL_ARRAY_BUFFER;
            GLuint buffer = 0;
            glBindBuffer(target, buffer);
            mCounters.bindBuffer++;
            assertNoGLError();
        }

        {
            GLuint index = mTexturePositionAttributeLocation[programIdx];
            glEnableVertexAttribArray(index);
            assertNoGLError();
        }
    }

    if (mNormalAttributeLocation[programIdx] != -1)
    {
        {
            GLenum target = GL_ARRAY_BUFFER;
            GLuint buffer = mVertexNormalBuffer[shapeIdx];
            glBindBuffer(target, buffer);
            mCounters.bindBuffer++;
            assertNoGLError();
        }

        {
            GLuint index          = mNormalAttributeLocation[programIdx];
            GLint size            = 3;
            GLenum type           = GL_FLOAT;
            GLboolean normalized  = GL_TRUE;
            GLsizei stride        = 0;
            const GLvoid *pointer = 0;
            glVertexAttribPointer(index, size, type, normalized, stride, pointer);
            assertNoGLError();
        }

        {
            GLenum target = GL_ARRAY_BUFFER;
            GLuint buffer = 0;
            glBindBuffer(target, buffer);
            mCounters.bindBuffer++;
            assertNoGLError();
        }

        {
            GLuint index = mNormalAttributeLocation[programIdx];
            glEnableVertexAttribArray(index);
            assertNoGLError();
        }
    }
}

void BenchmarkSample::setupProgram(const int programIdx,
                                   const int textureIdx,
                                   const angle::Vector3 &c)
{
    if (mSamplerUniformLocation[programIdx] != -1)
    {
        {
            glActiveTexture(GL_TEXTURE0);
            assertNoGLError();
        }
        {
            GLenum target  = GL_TEXTURE_2D;
            GLuint texture = mTexture[textureIdx];
            glBindTexture(target, texture);
            assertNoGLError();
        }

        {
            GLint samplerTextureUnit = 0;
            glUniform1i(mSamplerUniformLocation[programIdx], samplerTextureUnit);
            assertNoGLError();
        }
    }

    if (mColorUniformLocation[programIdx] != -1)
    {
        const float color[3] = {c.x(), c.y(), c.z()};
        GLint location       = mColorUniformLocation[programIdx];
        GLsizei count        = 1;
        const GLfloat *value = color;
        glUniform4fv(location, count, value);
        assertNoGLError();
    }
}

void BenchmarkSample::runProgram(const angle::Vector3 &componentPosition, const int programIdx)
{
    const float x = componentPosition.x();
    const float y = componentPosition.y();
    const float z = componentPosition.z();

    {
        GLint location = mComponentTransformationMatrixUniformLocation[programIdx];
        GLsizei count  = 1;
        // The data is provided in column major order
        GLboolean transposed    = GL_FALSE;
        const GLfloat value[16] = {
            1.0f, 0.0f, 0.0f, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f, 0.0f, 0.0f, 1.0f, 0.0f, x, y, z, 1.0f,
        };
        glUniformMatrix4fv(location, count, transposed, value);
        assertNoGLError();
    }

    {
        GLenum mode           = GL_TRIANGLES;
        GLsizei count         = mIndexCount;
        GLenum type           = GL_UNSIGNED_INT;
        const GLvoid *indices = 0;
        glDrawElements(mode, count, type, indices);
        mCounters.drawElements++;
        assertNoGLError();
    }
}

void BenchmarkSample::drawObject(const angle::Vector3 &layerPosition,
                                 int radialSliceIdx,
                                 const int programIdx,
                                 const int textureIdx,
                                 const int shapeIdx)
{
    using namespace angle;

    const float r   = 5.0f;
    const float a   = (2.0f * M_PI / kRadialSliceCount) * radialSliceIdx;
    const float x   = layerPosition.x() + r * cosf(a);
    const float y   = layerPosition.y() + r * sinf(a);
    const float z   = layerPosition.z();
    const Vector3 c = Vector3((1.0f + cosf(a)) * 0.5f, (1.0f + sinf(a)) * 0.5f, 1.0f);

    setupProgram(programIdx, textureIdx, c);
    setupVertexAttributes(programIdx, shapeIdx);
    runProgram(Vector3(x, y, z), programIdx);
}

unsigned gShapeIdx = 0;

void BenchmarkSample::drawCell(const int programIdx,
                               const int textureIdx,
                               const angle::Vector3 &cellCenter)
{
    using namespace angle;
    const float layerSpacing = 1.2f;

    const int repetitionCount = 3;

    for (int repetitionIdx = 0; repetitionIdx < repetitionCount; repetitionIdx++)
    {
        const Vector3 layerPosition = cellCenter + Vector3(0.0f, 0.0f, layerSpacing * programIdx);

        for (int radialSliceIdx = 0; radialSliceIdx < kRadialSliceCount; radialSliceIdx++)
        {
            const int shapeIdx = gShapeIdx % kShapeCount;
            drawObject(layerPosition, radialSliceIdx, programIdx, textureIdx, shapeIdx);
            gShapeIdx++;
        }
    }
}

void BenchmarkSample::draw()
{
    gShapeIdx = 0;
    clearCounters();

    glEnable(GL_DEPTH_TEST);

    // Set the viewport
    glViewport(0, 0, getWindow()->getWidth(), getWindow()->getHeight());

    const float windowDimensionsYXRatio =
        static_cast<float>(getWindow()->getHeight()) / static_cast<float>(getWindow()->getWidth());

    glDepthMask(GL_TRUE);
    // Our camera matrix maps the near plane of the frustum to -1, and the far plane to +1, in NDC.
    // We want depth values of near plane to be 0 and far plane to be 1.
    glDepthRangef(0.0f, 1.0f);
    // Clear to depth of far plane
    glClearDepthf(1.0f);
    // Draw fragments with depth less than the currently stored depth.
    glDepthFunc(GL_LESS);

    // Clear the color buffer
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    const angle::Vector3 modelPosition(0.0f, 0.0f, 50.0f);

    for (int programIdx = 0; programIdx < kShaderProgramCount; programIdx++)
    {
        glUseProgram(mShaderPrograms[programIdx]);
        mCounters.useProgram++;
        {
            setCameraTransform(windowDimensionsYXRatio,
                               mCameraTransformationMatrixUniformLocation[programIdx]);
            {
                GLint location = mModelTransformationMatrixUniformLocation[programIdx];
                GLsizei count  = 1;
                // The data is provided in column major order
                GLboolean transposed    = GL_FALSE;
                const float x           = modelPosition.x();
                const float y           = modelPosition.y();
                const float z           = modelPosition.z();
                const float c           = cosf(mModelRotationZ);
                const float s           = sinf(mModelRotationZ);
                const GLfloat value[16] = {
                    c, 0.0f, -s, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f, s, 0.0f, c, 0.0f, x, y, z, 1.0f,
                };
                glUniformMatrix4fv(location, count, transposed, value);
                assertNoGLError();
            }
        }

        for (int rowIdx = 0; rowIdx < kNumRows; rowIdx++)
        {
            for (int colIdx = 0; colIdx < kNumCols; colIdx++)
            {
                for (int layerIdx = 0; layerIdx < kNumLayers; layerIdx++)
                {
                    angle::Vector3 center(20.0f * (rowIdx - float(kNumRows - 1) / 2.0f),
                                          20.0f * (colIdx - float(kNumCols - 1) / 2.0f),
                                          30.0f * layerIdx);
                    const int textureIdx = programIdx % kTextureCount;
                    drawCell(programIdx, textureIdx, center);
                }
            }
        }
    }

    mFrameCount++;
    if (mFrameCount == 1000)
    {
        {
            uint64_t finalPerformanceCount;
            {
                LARGE_INTEGER count;
                BOOL result = QueryPerformanceCounter(&count);
                if (result == 0)
                {
                    ::exit(-1);
                }
                finalPerformanceCount = count.QuadPart;
            }
            uint64_t performanceCounts = finalPerformanceCount - mInitialPerformanceCount;
            double averageFrameDurationMilliSeconds =
                1000.0 * double(performanceCounts) /
                (double(mPerformanceCounterFrequency) * double(mFrameCount));
            printf("average frame duration over %lld frames: %f ms\n", mFrameCount,
                   averageFrameDurationMilliSeconds);
        }

        ::exit(0);
    }
#if 0
    printCounters();
    ::exit(0);
#endif
}

int main(int argc, char **argv)
{
    BenchmarkSample app(argc, argv);
    return app.run();
}
