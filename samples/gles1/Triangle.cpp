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

class HelloTriangleSample : public SampleApplication
{
  public:
    HelloTriangleSample(EGLint displayType)
        : SampleApplication("HelloTriangle", 1280, 720, 1, 0, displayType)
    {
    }

    virtual bool initialize()
    {
        glClearColor(1.0f, 1.0f, 1.0f, 0.0f);

        return true;
    }

    virtual void destroy()
    {
        glDeleteProgram(mProgram);
    }

    virtual void draw()
    {
        fprintf(stderr, "%s: call\n", __func__);

        GLfloat vertices[] =
        {
             0.0f,  0.5f, 0.0f,
            -0.5f, -0.5f, 0.0f,
             0.5f, -0.5f, 0.0f,
        };

        // GLfloat colors[] =
        // {
        //      1.0f,  0.0f, 0.0f,
        //      0.0f,  1.0f, 0.0f,
        //      0.0f,  0.0f, 1.0f,
        // };

        // Set the viewport
        glViewport(0, 0, getWindow()->getWidth(), getWindow()->getHeight());

        // Clear the color buffer
        glClear(GL_COLOR_BUFFER_BIT);

        glVertexPointer(3, GL_FLOAT, 3 * sizeof(GLfloat), vertices);
        // glColorPointer(3, GL_FLOAT, 3 * sizeof(GLfloat), colors);
        fprintf(stderr, "%s: drawArr\n", __func__);
        glDrawArrays(GL_TRIANGLES, 0, 3);
    }

  private:
    GLuint mProgram;
};

int main(int argc, char **argv)
{
    EGLint displayType = EGL_PLATFORM_ANGLE_TYPE_DEFAULT_ANGLE;

    if (argc > 1)
    {
        displayType = GetDisplayTypeFromArg(argv[1]);
    }

    HelloTriangleSample app(displayType);
    return app.run();
}
