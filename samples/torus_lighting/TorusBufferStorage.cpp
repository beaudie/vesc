//
// Copyright 2021 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//

// Based on CubeMapActivity.java from The Android Open Source Project ApiDemos
// https://android.googlesource.com/platform/development/+/refs/heads/master/samples/ApiDemos/src/com/example/android/apis/graphics/CubeMapActivity.java

// Hue to RGB conversion in GLSL based on
// https://github.com/tobspr/GLSL-Color-Spaces

#include "SampleApplication.h"

#include "torus.h"
#include "util/Matrix.h"
#include "util/shader_utils.h"

#include <chrono>

const float kDegreesPerSecond = 90.0f;
const GLushort kHuesSize      = (kSize + 1) * (kSize + 1);

class BufferStorageSample : public SampleApplication
{
  public:
    BufferStorageSample(int argc, char **argv)
        : SampleApplication("GLES3.1 Buffer Storage", argc, argv, 3, 1)
    {}

    bool initialize() override
    {
        constexpr char kVS[] = R"(#version 300 es
uniform mat4 mv;
uniform mat4 mvp;

in vec4 position;
in vec3 normal;
in float hue;

out vec3 normal_view;
out vec4 color;

vec4 hue_to_rgba(float hue)
{
    hue = mod(hue, 1.0);
    float r = abs(hue * 6.0 - 3.0) - 1.0;
    float g = 2.0 - abs(hue * 6.0 - 2.0);
    float b = 2.0 - abs(hue * 6.0 - 4.0);
    return vec4(r, g, b, 1.0);
}

void main()
{
    normal_view = vec3(mv * vec4(normal, 0.0));
    color = hue_to_rgba(hue);
    gl_Position = mvp * position;
})";

        constexpr char kFS[] = R"(#version 300 es
precision mediump float;

in vec3 normal_view;
in vec4 color;

out vec4 frag_color;

void main()
{
    frag_color = color * dot(vec3(0.0, 0.0, 1.0), normalize(normal_view));
})";

        mProgram = CompileProgram(kVS, kFS);
        if (!mProgram)
        {
            return false;
        }

        mPositionLoc = glGetAttribLocation(mProgram, "position");
        mNormalLoc   = glGetAttribLocation(mProgram, "normal");
        mHueLoc      = glGetAttribLocation(mProgram, "hue");

        mMVPMatrixLoc = glGetUniformLocation(mProgram, "mvp");
        mMVMatrixLoc  = glGetUniformLocation(mProgram, "mv");

        glClearColor(0.0f, 0.0f, 0.0f, 0.0f);
        glEnable(GL_DEPTH_TEST);

        std::string extensionList(reinterpret_cast<const char *>(glGetString(GL_EXTENSIONS)));
        if (extensionList.find("GL_EXT_buffer_storage") == std::string::npos)
        {
            std::cout << "GL_EXT_buffer_storage not available." << std::endl;
            return false;
        }

        generateTorus();

        mLastFrameTime = std::chrono::system_clock::now();
        mStartTime     = std::chrono::system_clock::now();

        return true;
    }

    void destroy() override
    {
        glBindBuffer(GL_ARRAY_BUFFER, mHueBuffer);
        glUnmapBuffer(GL_ARRAY_BUFFER);
        glDeleteProgram(mProgram);
        glDeleteBuffers(1, &mHueBuffer);
        glDeleteBuffers(1, &mVertexBuffer);
        glDeleteBuffers(1, &mIndexBuffer);
    }

    void draw() override
    {
        glViewport(0, 0, getWindow()->getWidth(), getWindow()->getHeight());
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        glUseProgram(mProgram);

        float ratio = (float)getWindow()->getWidth() / (float)getWindow()->getHeight();
        Matrix4 perspectiveMatrix = Matrix4::frustum(-ratio, ratio, -1, 1, 1.0f, 20.0f);

        Matrix4 modelMatrix = Matrix4::translate(angle::Vector3(0, 0, -5)) *
                              Matrix4::rotate(mAngle, angle::Vector3(0.0f, 1.0f, 0.0f)) *
                              Matrix4::rotate(mAngle * 0.25f, angle::Vector3(1.0f, 0.0f, 0.0f));

        Matrix4 mvpMatrix = perspectiveMatrix * modelMatrix;

        glUniformMatrix4fv(mMVMatrixLoc, 1, GL_FALSE, modelMatrix.data);
        glUniformMatrix4fv(mMVPMatrixLoc, 1, GL_FALSE, mvpMatrix.data);

        glEnableVertexAttribArray(mPositionLoc);
        glBindBuffer(GL_ARRAY_BUFFER, mVertexBuffer);
        glVertexAttribPointer(mPositionLoc, 3, GL_FLOAT, false, 6 * sizeof(GLfloat), nullptr);

        glVertexAttribPointer(mNormalLoc, 3, GL_FLOAT, false, 6 * sizeof(GLfloat),
                              reinterpret_cast<const void *>(3 * sizeof(GLfloat)));
        glEnableVertexAttribArray(mNormalLoc);

        glBindBuffer(GL_ARRAY_BUFFER, mHueBuffer);
        glVertexAttribPointer(mHueLoc, 1, GL_FLOAT, false, sizeof(GLfloat), nullptr);
        glEnableVertexAttribArray(mHueLoc);

        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, mIndexBuffer);
        glDrawElements(GL_TRIANGLES, mIndexCount, GL_UNSIGNED_SHORT, 0);
        glBindBuffer(GL_ARRAY_BUFFER, 0);
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);

        std::chrono::time_point<std::chrono::system_clock> now = std::chrono::system_clock::now();

        std::chrono::duration<float> elapsed = now - mLastFrameTime;

        mAngle += kDegreesPerSecond * elapsed.count();

        mLastFrameTime = now;

        std::chrono::duration<float> elapsedTotal = now - mStartTime;
        updateHues(elapsedTotal.count());

        GLsync sync = glFenceSync(GL_SYNC_GPU_COMMANDS_COMPLETE, 0);
        glClientWaitSync(sync, 0, 0);
        glDeleteSync(sync);

        assert(static_cast<GLenum>(GL_NO_ERROR) == glGetError());
    }

    void updateHues(GLfloat time)
    {
        for (uint32_t i = 0; i < kHuesSize; i++)
        {
            mHueMapPtr[i] = static_cast<GLfloat>(i) / static_cast<GLfloat>(kHuesSize) + time;
        }
    }

    void generateTorus()
    {
        GenerateTorus(&mVertexBuffer, &mIndexBuffer, &mIndexCount);

        std::vector<GLfloat> hues(kHuesSize, 0.0f);

        glGenBuffers(1, &mHueBuffer);
        glBindBuffer(GL_ARRAY_BUFFER, mHueBuffer);
        glBufferStorageEXT(GL_ARRAY_BUFFER, kHuesSize * sizeof(GLfloat), hues.data(),
                           GL_MAP_WRITE_BIT | GL_MAP_PERSISTENT_BIT_EXT | GL_MAP_COHERENT_BIT_EXT);

        mHueMapPtr = static_cast<float *>(
            glMapBufferRange(GL_ARRAY_BUFFER, 0, kHuesSize * sizeof(GLfloat),
                             GL_MAP_WRITE_BIT | GL_MAP_UNSYNCHRONIZED_BIT |
                                 GL_MAP_PERSISTENT_BIT_EXT | GL_MAP_COHERENT_BIT_EXT));

        assert(mHueMapPtr != nullptr);
        assert(static_cast<GLenum>(GL_NO_ERROR) == glGetError());
    }

  private:
    GLuint mProgram;

    GLint mPositionLoc;
    GLint mNormalLoc;
    GLint mHueLoc;

    GLuint mMVPMatrixLoc;
    GLuint mMVMatrixLoc;

    GLuint mVertexBuffer;
    GLuint mHueBuffer;
    GLuint mIndexBuffer;
    GLsizei mIndexCount;

    float *mHueMapPtr;

    float mAngle = 0;

    std::chrono::time_point<std::chrono::system_clock> mLastFrameTime, mStartTime;
};

int main(int argc, char **argv)
{
    BufferStorageSample app(argc, argv);
    return app.run();
}
