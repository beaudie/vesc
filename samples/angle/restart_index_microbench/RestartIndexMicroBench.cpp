//
// Copyright (c) 2014 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//

//            Based on Hello_Triangle.c from
// Book:      OpenGL(R) ES 2.0 Programming Guide
// Authors:   Aaftab Munshi, Dan Ginsburg, Dave Shreiner
// ISBN-10:   0321502795
// ISBN-13:   9780321502797
// Publisher: Addison-Wesley Professional
// URLs:      http://safari.informit.com/9780321563835
//            http://www.opengles-book.com

#include <iostream>
#include <vector>

#include "SampleApplication.h"
#include "shader_utils.h"

// This small sample compares the per-frame render time for a series of 
// squares drawn with TRIANGLE_FANS versus squares drawn with TRIANGLES.
// To exacerbate differences between the two, we use a large collection
// of short buffers with pre-translated vertex data.

class RestartIndexBenchSample : public SampleApplication
{
  public:
    RestartIndexBenchSample::RestartIndexBenchSample()
        : SampleApplication("Microbench", 1280, 1280),
          mFrameCount(0)
    {
    }

    void createVertexBuffers()
    {
        glGenBuffers(1, &mVertexVBO);
        glGenBuffers(1, &mUintIndexVBO);
        glGenBuffers(1, &mShortIndexVBO);

        GLfloat *vertices = new GLfloat[65536 * 3];

        GLfloat xMin = -1.0f; // using an untransformed modelview
        GLfloat yMin = -1.0f;
        GLfloat xMax = 1.0f;
        //GLfloat yMax = 1.0f;

        GLfloat curX = xMin;
        GLfloat curY = yMin;
        GLfloat step = 2 / 256;
        for (int i = 0; i < 65536; ++i)
        {
            vertices[i * 3] = curX;
            vertices[i * 3 + 1] = curY;
            vertices[i * 3 + 2] = 0.0f;

            curX += step;
            if (curX > xMax)
            {
                curX = xMin;
                curY += step;
            }
        }

        std::vector<GLuint> uintIndices;
        std::vector<GLushort> shortIndices;

        uintIndices.reserve(512 * 256);
        shortIndices.reserve(512 * 256);
        for (int i = 0; i < 255; ++i)
        {
            uintIndices.push_back((GLuint)((i + 1) * 256)); // repeat the first vertex for degenerate tri
            shortIndices.push_back((GLushort)((i + 1) * 256));
            // 256 rows of vertices make 255 rows of triangles
            for (int j = 0; j < 256; ++j)
            {
                // 256 columns of vertices
                uintIndices.push_back((GLuint)((i + 1) * 256 + j));
                shortIndices.push_back((GLushort)((i + 1) * 256 + j));
                uintIndices.push_back((GLuint)(i * 256 + j));
                shortIndices.push_back((GLushort)(i * 256 + j));
            }
            uintIndices.push_back((GLuint)((i + 1) * 256 - 1)); // repeat the last vertex for degenerate tri
            shortIndices.push_back((GLushort)((i + 1) * 256 - 1));
        }

        glBindBuffer(GL_ARRAY_BUFFER, mVertexVBO);
        glBufferData(GL_ARRAY_BUFFER, 65536 * 3 * sizeof(GLfloat), vertices, GL_STATIC_DRAW);

        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, mUintIndexVBO);
        glBufferData(GL_ELEMENT_ARRAY_BUFFER, uintIndices.size(), uintIndices.data(), GL_STATIC_DRAW);

        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, mShortIndexVBO);
        glBufferData(GL_ELEMENT_ARRAY_BUFFER, shortIndices.size(), shortIndices.data(), GL_STATIC_DRAW);

        mNumIndices = uintIndices.size();
    }

    virtual bool initialize()
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

        glClearColor(0.0f, 0.0f, 0.0f, 0.0f);

        createVertexBuffers();

        //mFanTimer = CreateTimer();
        //mTriTimer = CreateTimer();
        //mFanTotalTime = 0;
        //mTriTotalTime = 0;

        return true;
    }

    virtual void destroy()
    {
        //std::cout << "Total draw time using TRIANGLE_FAN: " << mFanTotalTime << "ms (" << (float)mFanTotalTime / (float)mFrameCount << " average per frame)" << std::endl;
        //std::cout << "Total draw time using TRIANGLES: " << mTriTotalTime << "ms (" << (float)mTriTotalTime / (float)mFrameCount << " average per frame)" << std::endl;
        glDeleteProgram(mProgram);
    }

    virtual void draw()
    {
        // Set the viewport
        glViewport(0, 0, getWindow()->getWidth(), getWindow()->getHeight());

        // Clear the color buffer
        glClear(GL_COLOR_BUFFER_BIT);

        // Use the program object
        glUseProgram(mProgram);

        glEnableVertexAttribArray(0);
        glBindBuffer(GL_ARRAY_BUFFER, mVertexVBO);
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, mUintIndexVBO);

        glDrawElements(GL_TRIANGLE_STRIP, mNumIndices, GL_UNSIGNED_INT, NULL);

        mFrameCount++;
    }

  private:
    GLuint mProgram;
    GLuint mVertexVBO;
    GLuint mUintIndexVBO;
    GLuint mShortIndexVBO;
    GLuint mNumIndices;

    Timer *mShortTimer;
    Timer *mUintTimer;
    unsigned int mFrameCount;
    unsigned int mShortTotalTime;
    unsigned int mUintTotalTime;
};

int main(int argc, char **argv)
{
    RestartIndexBenchSample app;
    return app.run();
}
