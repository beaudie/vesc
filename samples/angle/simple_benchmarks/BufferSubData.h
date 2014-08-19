//
// Copyright (c) 2014 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//

#include "SimpleBenchmark.h"
#include "shader_utils.h"

const int BUFFER_SIZE = 1024 * 1024;
const int UPDATE_SIZE = 300;

class BufferSubDataBenchmark : public SimpleBenchmark
{
public:
    BufferSubDataBenchmark::BufferSubDataBenchmark(EGLint requestedRenderer)
        : SimpleBenchmark("BufferSubData", 512, 512, 2, requestedRenderer)
    {
    }

    virtual bool initializeBenchmark()
    {
        const std::string vs = SHADER_SOURCE
            (
            attribute vec4 vPosition;
        void main()
        {
            gl_Position = vPosition;
        }
        );

        const std::string fs = SHADER_SOURCE
            (
            precision mediump float;
        void main()
        {
            gl_FragColor = vec4(1.0, 0.0, 0.0, 1.0);
        }
        );

        mProgram = CompileProgram(vs, fs);
        if (!mProgram)
        {
            return false;
        }

        // Use the program object
        glUseProgram(mProgram);

        glClearColor(0.0f, 0.0f, 0.0f, 0.0f);

        glGenBuffers(1, &mBuffer);
        glBindBuffer(GL_ARRAY_BUFFER, mBuffer);
        glBufferData(GL_ARRAY_BUFFER, BUFFER_SIZE, 0, GL_DYNAMIC_DRAW);

        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, 0);
        glEnableVertexAttribArray(0);

        mUpdateData = new char[UPDATE_SIZE];

        GLfloat vertices[] =
        {
            0.0f, 0.5f, 0.0f,
            -0.5f, -0.5f, 0.0f,
            0.5f, -0.5f, 0.0f,
        };

        mNumTris = UPDATE_SIZE / sizeof(vertices);
        int offset = 0;
        for (int i = 0; i < mNumTris; ++i)
        {
            memcpy(mUpdateData + offset, vertices, sizeof(vertices));
            offset += sizeof(vertices);
        }

        // Set the viewport
        glViewport(0, 0, getWindow()->getWidth(), getWindow()->getHeight());

        GLenum glErr = glGetError();
        if (glErr != GL_NO_ERROR)
        {
            return false;
        }

        return true;
    }

    virtual void destroyBenchmark()
    {
        glDeleteProgram(mProgram);
        glDeleteBuffers(1, &mBuffer);
        delete[] mUpdateData;
    }

    virtual void beginDrawBenchmark()
    {
        // Clear the color buffer
        glClear(GL_COLOR_BUFFER_BIT);
    }

    virtual void drawBenchmark()
    {
        glBufferSubData(GL_ARRAY_BUFFER, 0, UPDATE_SIZE, mUpdateData);
        glDrawArrays(GL_TRIANGLES, 0, 3 * mNumTris);
    }

private:
    GLuint mProgram;
    GLuint mBuffer;
    char* mUpdateData;
    int mNumTris;
};