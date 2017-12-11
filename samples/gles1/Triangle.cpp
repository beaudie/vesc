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

#include "SampleApplication.h"
#include "shader_utils.h"

#include <GLES/gl.h>

#include "math.h"
#include <sys/time.h>

class GLES1HelloTriangleSample : public SampleApplication
{
  public:
    GLES1HelloTriangleSample(EGLint displayType)
        : SampleApplication("GLES1HelloTriangle", 1280, 720, 1, 0, displayType)
    {
    }

    virtual bool initialize()
    {
        glClearColor(1.0f, 1.0f, 1.0f, 0.0f);

        return true;
    }

    virtual void destroy()
    {

    }

    virtual void draw()
    {
        GLfloat vertices[] =
        {
             0.0f,  0.5f, 0.0f,
            -0.5f, -0.5f, 0.0f,
             0.5f, -0.5f, 0.0f,
        };

        GLfloat colors[] =
        {
             1.0f,  0.0f, 0.0f,
             0.0f,  1.0f, 0.0f,
             0.0f,  0.0f, 1.0f,
        };

        // Set the viewport
        int winWidth = getWindow()->getWidth();
        int winHeight = getWindow()->getHeight();

        int viewportDim = std::min(winWidth, winHeight);
        int xshift = 0;
        int yshift = 0;

        xshift = (winWidth > viewportDim) ? ((winWidth - viewportDim) / 2) : 0;
        yshift = (winHeight > viewportDim) ? ((winHeight - viewportDim) / 2) : 0;

        glViewport(xshift, yshift, viewportDim, viewportDim);

        // Clear the color buffer
        glClear(GL_COLOR_BUFFER_BIT);

        glEnableClientState(GL_VERTEX_ARRAY);

        if (mTime < 200.0f) {
            glShadeModel(GL_SMOOTH);
            glEnableClientState(GL_COLOR_ARRAY);
            glColorPointer(3, GL_FLOAT, 3 * sizeof(GLfloat), colors);
        } else if (mTime >= 200.0f && mTime < 400.0f) {
            glShadeModel(GL_FLAT);
        } else {
            glShadeModel(GL_SMOOTH);
            glDisableClientState(GL_COLOR_ARRAY);
            glColor4f(0.1f, mTime / mLoopTime, 0.4f, 1.0f);
        }

        if (mTime >= mLoopTime) {
            mTime = 0;
        }

        glVertexPointer(3, GL_FLOAT, 3 * sizeof(GLfloat), vertices);

        glMatrixMode(GL_MODELVIEW);

        glLoadIdentity();
        glTranslatef(sinf(2 * 3.14159265358979 * mTime / mLoopTime), 0.0f, 0.0f);
        glRotatef(360.0f * (mTime / mLoopTime), 0.0f, 0.0f, 1.0f);
        glDrawArrays(GL_TRIANGLES, 0, 3);

        struct timeval tv;
        gettimeofday(&tv, NULL);
        uint64_t currTimeUs =
            tv.tv_sec * 1000 * 1000 + tv.tv_usec;

        if (currTimeUs - mTimeUs > 16667) {
            mTime++;
            mTimeUs = currTimeUs;
        }
    }

  private:
    uint64_t mTimeUs = 0;
    float mTime = 0.0f;
    float mLoopTime = 500.0f;
};

int main(int argc, char **argv)
{
    EGLint displayType = EGL_PLATFORM_ANGLE_TYPE_DEFAULT_ANGLE;

    if (argc > 1)
    {
        displayType = GetDisplayTypeFromArg(argv[1]);
    }

    GLES1HelloTriangleSample app(displayType);
    return app.run();
}
