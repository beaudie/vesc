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

#include "SampleApplication.h"
#include "shader_utils.h"

class MicrobenchSample : public SampleApplication
{
  public:
    MicrobenchSample::MicrobenchSample()
        : SampleApplication("Microbench", 1280, 720),
          mFrameCount(0)
    {
    }

    void createVertexBuffers()
    {
        const unsigned int slices = 360;
        const unsigned int numFanVertices = slices + 2;
        const unsigned int fanFloats = numFanVertices * 3;
        // Create 360-triangle polygon with TRIANGLE_FAN vertices
        GLfloat fanVertices[fanFloats];
        fanVertices[0] = -1.0f;
        fanVertices[1] = -1.0f;
        fanVertices[2] = 0.0f; // center vertex

        // We'll start at the top of the shape, and increment x & y
        // coords, flipping direction when appropriate
        GLfloat rad = 0.25f;
        GLfloat xDir = 1.0f;
        GLfloat yDir = -1.0f;
        GLfloat xCur = -1.0f;
        GLfloat yCur = -1.0f + rad;
        GLfloat xOffset = 0.5f;
        GLfloat yOffset = 0.0f;
        GLfloat offsetIncr = 0.5f;

        for (int i = 1; i < numFanVertices; ++i)
        {
            fanVertices[i * 3] = xCur;
            fanVertices[i * 3 + 1] = yCur;
            fanVertices[i * 3 + 2] = 0.0f;

            if (i % (slices / 2) == 0)
                yDir *= -1.0f;
            else if (i % (slices / 4) == 0)
                xDir *= -1.0f;

            xCur += 2.0f * rad / (slices / 2.0f) * xDir;
            yCur += 2.0f * rad / (slices / 2.0f) * yDir;
        }

        glGenBuffers(26, mFanBufId);
        glBindBuffer(GL_ARRAY_BUFFER, mFanBufId[0]);
        glBufferData(GL_ARRAY_BUFFER, fanFloats * sizeof(GLfloat), fanVertices, GL_STATIC_DRAW);

        GLfloat fanVertices2[fanFloats];

        for (int bufItr = 1; bufItr < 26; ++bufItr)
        {
            for (int i = 0; i < numFanVertices; ++i)
            {
                fanVertices2[i * 3] = fanVertices[i * 3] + xOffset;
                fanVertices2[i * 3 + 1] = fanVertices[i * 3 + 1] + yOffset;
                fanVertices2[i * 3 + 2] = 0.0f;
            }

            glBindBuffer(GL_ARRAY_BUFFER, mFanBufId[bufItr]);
            glBufferData(GL_ARRAY_BUFFER, fanFloats * sizeof(GLfloat), fanVertices2, GL_STATIC_DRAW);
            xOffset += offsetIncr;
            if (bufItr % 5 == 0)
            {
                yOffset += offsetIncr;
                xOffset = 0.0f;
            }
        }

        const unsigned int numTriVertices = slices * 3;
        const unsigned int triFloats = numTriVertices * 3;
        GLfloat triVertices[triFloats];
        GLfloat *triPointer = triVertices;

        for (int i = 0; i < slices; ++i)
        {
            memcpy(triPointer, fanVertices, 3 * sizeof(GLfloat)); // copy center point as first vertex for this slice
            triPointer += 3;
            for (int j = 1; j < 3; ++j)
            {
                GLfloat *vertex = &(fanVertices[(i + j) * 3]); // copy two outer vertices for this point
                memcpy(triPointer, vertex, 3 * sizeof(GLfloat));
                triPointer += 3;
            }
        }

        GLfloat triVertices2[triFloats];
        glGenBuffers(26, mTriBufId);
        glBindBuffer(GL_ARRAY_BUFFER, mTriBufId[0]);
        glBufferData(GL_ARRAY_BUFFER, triFloats * sizeof(GLfloat), triVertices, GL_STATIC_DRAW);

        xOffset = 0.5f;
        yOffset = 0.0f;

        for (int bufItr = 1; bufItr < 26; ++bufItr)
        {
            triPointer = triVertices;
            for (int i = 0; i < numTriVertices; ++i)
            {
                triVertices2[i * 3] = triPointer[0] + xOffset;
                triVertices2[i * 3 + 1] = triPointer[1] + yOffset;
                triVertices2[i * 3 + 2] = 0.0f;
                triPointer += 3;
            }

            glBindBuffer(GL_ARRAY_BUFFER, mTriBufId[bufItr]);
            glBufferData(GL_ARRAY_BUFFER, triFloats * sizeof(GLfloat), triVertices2, GL_STATIC_DRAW);
            xOffset += offsetIncr;
            if (bufItr % 5 == 0)
            {
                yOffset += offsetIncr;
                xOffset = 0.0f;
            }
        }
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

        mFanTimer = CreateTimer();
        mTriTimer = CreateTimer();
        mFanTotalTime = 0;
        mTriTotalTime = 0;

        return true;
    }

    virtual void destroy()
    {
        std::cout << "Total draw time using TRIANGLE_FAN: " << mFanTotalTime << "ms (" << (float)mFanTotalTime / (float)mFrameCount << " average per frame)" << std::endl;
        std::cout << "Total draw time using TRIANGLES: " << mTriTotalTime << "ms (" << (float)mTriTotalTime / (float)mFrameCount << " average per frame)" << std::endl;
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

        // Bind the vertex data
        glEnableVertexAttribArray(0);

        // Draw using triangle fans, stored in VBO
        mFanTimer->start();
        for (int i = 0; i < 26; ++i)
        {
            glBindBuffer(GL_ARRAY_BUFFER, mFanBufId[i]);
            glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, NULL);
            glDrawArrays(GL_TRIANGLE_FAN, 0, 361);
        }
        mFanTimer->stop();

        mFanTotalTime += mFanTimer->getElapsedTime() * 1000; // convert from usec to msec when accumulating

        // Clear to eliminate driver-side gains from occlusion
        glClear(GL_COLOR_BUFFER_BIT);
        
        // Draw using triangles, stored in VBO
        mTriTimer->start();
        for (int i = 1; i < 26; ++i)
        {
            glBindBuffer(GL_ARRAY_BUFFER, mTriBufId[i]);
            glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, NULL);
            glDrawArrays(GL_TRIANGLES, 0, 360 * 3);
        }
        mTriTimer->stop();

        mTriTotalTime += mTriTimer->getElapsedTime() * 1000; // convert from usec to msec when accumulating

        mFrameCount++;
    }

  private:
    GLuint mProgram;
    GLuint mFanBufId[26];
    GLuint mTriBufId[26];

    Timer *mFanTimer;
    Timer *mTriTimer;
    unsigned int mFrameCount;
    unsigned int mTriTotalTime;
    unsigned int mFanTotalTime;
};

int main(int argc, char **argv)
{
    MicrobenchSample app;
    return app.run();
}
