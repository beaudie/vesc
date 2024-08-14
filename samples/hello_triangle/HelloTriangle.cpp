//
// Copyright 2014 The ANGLE Project Authors. All rights reserved.
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

#include "util/shader_utils.h"

class HelloTriangleSample : public SampleApplication
{
  public:
    HelloTriangleSample(int argc, char **argv)
        : SampleApplication("HelloTriangle", argc, argv, ClientType::ES3_0)
    {}

    bool initialize() override
    {
        constexpr char kVS[] = R"(#version 300 es
precision mediump float;

out vec4 vColor;

void main()
{
    if (gl_VertexID == 0)
    {
        gl_Position = vec4(0.0, 0.5, 0.0, 1.0);
        vColor = vec4(1.0, 0.0, 0.0, 1.0);
    }
    else if (gl_VertexID == 1)
    {
        gl_Position = vec4(-0.5, -0.5, 0.0, 1.0);
        vColor = vec4(0.0, 1.0, 0.0, 1.0);
    }
    else if (gl_VertexID == 2)
    {
        gl_Position = vec4(0.5, -0.5, 0.0, 1.0);
        vColor = vec4(0.0, 0.0, 1.0, 1.0);
    }
})";

        constexpr char kFS[] = R"(#version 300 es
precision mediump float;

in vec4 vColor;

out vec4 oColor;

void main()
{
    oColor = vColor;
})";

        mProgram = CompileProgram(kVS, kFS);
        if (!mProgram)
        {
            return false;
        }

        glClearColor(0.0f, 0.0f, 0.0f, 0.0f);

        return true;
    }

    void destroy() override { glDeleteProgram(mProgram); }

    void draw() override
    {
        // Set the viewport
        glViewport(0, 0, getWindow()->getWidth(), getWindow()->getHeight());

        // Clear the color buffer
        glClear(GL_COLOR_BUFFER_BIT);

        // Use the program object
        glUseProgram(mProgram);

        glDrawArrays(GL_TRIANGLES, 0, 3);
    }

  private:
    GLuint mProgram;
};

int main(int argc, char **argv)
{
    HelloTriangleSample app(argc, argv);
    return app.run();
}
