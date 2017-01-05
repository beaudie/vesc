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
#include "unistd.h"
#include <iostream>

class HelloTriangleSample : public SampleApplication
{
  public:
    HelloTriangleSample(EGLint displayType)
        : SampleApplication("HelloTriangle", 1280, 720, 2, 0, displayType)
    {
    }

    virtual bool initialize()
    {

//#define ONE_ATTRIB
//#define VBO
//#define INDEXED
//#define IBO
#define FIRST 2
#define STRIDE 6

#ifdef ONE_ATTRIB
        const std::string vs =
            R"(attribute vec3 vPosition;
            void main()
            {
                gl_Position = vec4(vPosition, 1.0);
            })";
        const std::string fs =
            R"(precision mediump float;
            void main()
            {
                gl_FragColor = vec4(1.0, 0.0, 0.0, 1.0);
            })";

#else
        const std::string vs =
            R"(attribute vec3 vPosition;
            attribute vec3 vColor;
            varying vec3 fColor;
            void main()
            {
                fColor = vColor;
                gl_Position = vec4(vPosition, 1.0);
            })";
        const std::string fs =
            R"(precision mediump float;
            varying vec3 fColor;
            void main()
            {
                gl_FragColor = vec4(fColor, 1.0);
            })";
#endif

#if 0
        const std::string vs =
            R"(attribute vec4 vPosition;
            attribute vec3 vColor;
            varying vec3 fColor;
            void main()
            {
                fColor = vColor;
                gl_Position = vPosition;
            })";

        const std::string fs =
            R"(precision mediump float;
            varying vec3 fColor;
            void main()
            {
                gl_FragColor = vec4(fColor, 1.);
            })";
#endif
        mProgram = CompileProgram(vs, fs);

        if (!mProgram)
        {
            return false;
        }

        glClearColor(0.0f, 0.0f, 0.0f, 0.0f);

        glGenBuffers(2, buf);
#ifdef VBO
        glBindBuffer(GL_ARRAY_BUFFER, buf[0]);
#endif
#ifdef IBO
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, buf[1]);
#endif

        return true;
    }

    virtual void destroy()
    {
        glDeleteProgram(mProgram);
    }

#define P(p) ((GLfloat) sin(mFrame*.01  + p*M_PI/180.))
#define C(p) ((GLfloat) fabs(P(p)))
    virtual void draw()
    {
        GLfloat vertices[] =
        {
#if 0
             0.0f,  0.5f, 0.0f,  1.f, 0.f, 0.f,
            -0.5f, -0.5f, 0.0f,  0.f, 1.f, 0.f,
             0.5f, -0.5f, 0.0f,  0.f, 0.f, 1.f,
#else

#if STRIDE==3

#if FIRST>=3
#error you suck
#endif
#if FIRST>=2
             99.,99.,99.,
#endif
#if FIRST>=1
             99.,99.,99.,
#endif

             P(  0) ,P( 90), 0.f,
             P(120), P(210), 0.f,
             P(240), P(330), 0.f,
             99.,99.,99.,
             99.,99.,99.,
#elif STRIDE==6

#if FIRST>=2
             99.,99.,99., 99.,99.,99.,
#endif
#if FIRST>=1
             99.,99.,99., 99.,99.,99.,
#endif

             P(  0) ,P( 90), 0.f,  C(0), C(60), C(120),
             P(120), P(210), 0.f,  C(30), C(90), C(150),
             P(240), P(330), 0.f,  C(60), C(120), C(0),

             P( 60) ,P(150), 0.f,  .5f, .0f, .0f,
             P(180), P(270), 0.f,  .0f, .5f, .0f,
             P(300), P( 30), 0.f,  .0f, .0f, .5f,
#else
#error you really suck
#endif

#endif
        };

#ifdef VBO
        glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);
#endif
#ifdef INDEXED
        GLushort indices[] = {
#if FIRST==0
                              1,2,0,
#elif FIRST==1
                              2,3,1,
#else
                              3,4,2,
#endif
                             };
#endif
#ifdef IBO
        glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW);
#endif

        // Set the viewport
        glViewport(0, 0, getWindow()->getWidth(), getWindow()->getHeight());

        // Clear the color buffer
        glClear(GL_COLOR_BUFFER_BIT);

        // Use the program object
        glUseProgram(mProgram);

        // Load the vertex data
        if(mFrame==0)
        {
            printf("glVertexAttribPointer %p\n", vertices);
            glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(GLfloat)*STRIDE,
#ifdef VBO
            0
#else
            vertices
#endif
            );
            glEnableVertexAttribArray(0);
#ifndef ONE_ATTRIB
            glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(GLfloat)*STRIDE,
#ifdef VBO
            (void*) (sizeof(GLfloat)*3)
#else
            vertices+3
#endif
            );
            glEnableVertexAttribArray(1);
#endif
        }

#ifdef INDEXED
        glDrawElements(GL_TRIANGLES, 3, GL_UNSIGNED_SHORT,
#ifdef IBO
        0
#else
        indices
#endif
        );
#else
        glDrawArrays(GL_TRIANGLES, FIRST, 3);
#endif
        mFrame += 1;
        usleep(999999/60);
    }

  private:
    GLuint mProgram;
    GLuint buf[2];
    unsigned mFrame;
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
