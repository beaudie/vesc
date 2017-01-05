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
#include "math.h"
#include <iostream>

class HelloTriangleSample : public SampleApplication
{
  public:
    HelloTriangleSample()
        : SampleApplication("HelloTriangle", 1280, 720)
    {
    }

    virtual bool initialize()
    {
#if 1
        const std::string vs = SHADER_SOURCE
        (
            attribute vec3 vPosition;
            void main()
            {
                gl_Position = vec4(vPosition, 1.);
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
#else
        const std::string vs = SHADER_SOURCE
        (
            attribute vec3 vPosition;
            attribute vec3 vColor;
            varying vec3 fColor;
            void main()
            {
                fColor = vColor;
                gl_Position = vec4(vPosition, 1.);
            }
        );

        const std::string fs = SHADER_SOURCE
        (
            precision mediump float;
            varying vec3 fColor;
            void main()
            {
                gl_FragColor = vec4(fColor, 1.);
            }
        );
#endif
        mProgram = CompileProgram(vs, fs);

        if (!mProgram)
        {
            return false;
        }

        glClearColor(0.0f, 0.0f, 0.0f, 0.0f);

        return true;
    }

    virtual void destroy()
    {
        glDeleteProgram(mProgram);
    }

#define P(p) ((GLfloat)sin(mFrame*.008 + p*M_PI/180.))
    virtual void draw()
    {
        GLfloat vertices[] =
        {
#if 0
             0.0f,  0.5f, 0.0f,  1.f, 0.f, 0.f,
            -0.5f, -0.5f, 0.0f,  0.f, 1.f, 0.f,
             0.5f, -0.5f, 0.0f,  0.f, 0.f, 1.f,
#else
             P(  0) ,P( 90), 0.f,  P(45), 0.f, 0.f,
             P(120), P(210), 0.f,  0.f, P(165), 0.f,
             P(240), P(330), 0.f,  0.f, 0.f, P(285),
#endif
        };

        // Set the viewport
        glViewport(0, 0, getWindow()->getWidth(), getWindow()->getHeight());

        // Clear the color buffer
        glClear(GL_COLOR_BUFFER_BIT);

        // Use the program object
        glUseProgram(mProgram);

        // Load the vertex data
        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 24, vertices);
        glEnableVertexAttribArray(0);

        glDrawArrays(GL_TRIANGLES, 0, 3);
        mFrame += 1.;
    }

  private:
    GLuint mProgram;
    double mFrame;
};

int main(int argc, char **argv)
{
    HelloTriangleSample app;
    return app.run();
}
