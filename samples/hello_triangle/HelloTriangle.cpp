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
#include "system_utils.h"

class HelloTriangleSample : public SampleApplication
{
  public:
    HelloTriangleSample()
        : SampleApplication("HelloTriangle", 1280, 720, 3, 0)
    {
    }

/*
function runTest() {
    var wtu = WebGLTestUtils;
    var gl = wtu.create3DContext('testbed', { preserveDrawingBuffer : true }, 2);
    if (!gl) {
        testFailed('could not create context');
        return;
    }
    var program = wtu.setupProgram(gl, ['vshader', 'fshader'])

    gl.enableVertexAttribArray(0);
    var pos = gl.createBuffer();
    gl.bindBuffer(gl.ARRAY_BUFFER, pos);
    gl.bufferData(gl.ARRAY_BUFFER, new Int32Array([-10, -10, 10, -10, -10, 10, 10, 10]), gl.STATIC_DRAW);

    gl.vertexAttribIPointer(0, 2, gl.INT, 4 * 2, 0);

    debug('Test vertexAttribI4[ui][v] by setting different combinations that add up to 15 and use that when rendering.');
    var vals = [[2, -3, 6, 10], [1, 3, 1, 10], [-10, 3, 2, 20], [5, 6, 2, 2]];
    var tests = ['vertexAttribI4i', 'vertexAttribI4ui', 'vertexAttribI4iv', 'vertexAttribI4uiv'];

    for (var ii = 0; ii < 4; ++ii) {
        if (ii != 1) {continue;}
        gl.clear(gl.COLOR_BUFFER_BIT | gl.DEPTH_BUFFER_BIT);
        if (ii < 2) {
            gl[tests[ii]](1, vals[ii][0], vals[ii][1], vals[ii][2], vals[ii][3]);
        } else {
            gl[tests[ii]](1, vals[ii]);
        }
        gl.drawArrays(gl.TRIANGLE_STRIP, 0, 4);

        if (checkRedPortion(gl, 50, 50 * 0.7, 50 * 0.8)) {
            testPassed('Attribute of ' + tests[ii] + ' was set correctly');
        } else {
            testFailed('Attribute of ' + tests[ii] + ' was not set correctly');
        }
    }
*/
    virtual bool initialize()
    {
        const std::string vs = "#version 300 es\n" SHADER_SOURCE
        (
            layout(location=0) in ivec2 p;
            layout(location=1) in ivec4 a;
            void main()
            {
                int sum = a.x + a.y + a.z + a.w;
                gl_Position = vec4(p.x + sum, p.y, 0.0, 20.0);
            }
        );

        const std::string fs = "#version 300 es\n" SHADER_SOURCE
        (
            precision mediump float;
            layout(location=0) out vec4 oColor;
            void main()
            {
                oColor = vec4(1.0, 0.0, 0.0, 1.0);
            }
        );

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

    virtual void draw()
    {
        GLint vertices[] =
        {
            -10, -10, 10, -10, -10, 10, 10, 10
        };

        // Set the viewport
        glViewport(0, 0, getWindow()->getWidth(), getWindow()->getHeight());

        // Clear the color buffer
        glClear(GL_COLOR_BUFFER_BIT);

        // Use the program object
        glUseProgram(mProgram);

        // Load the vertex data
        glVertexAttribIPointer(0, 2, GL_INT, 0, vertices);
        glEnableVertexAttribArray(0);

        glVertexAttribIPointer(1, 4, GL_INT, 0, vertices);
        glEnableVertexAttribArray(1);

        glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);

        glClear(GL_COLOR_BUFFER_BIT);
        glDisableVertexAttribArray(1);
        glVertexAttribI4i(1, 1, 3, 1, 11);

        glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);

        angle::Sleep(1000);
    }

  private:
    GLuint mProgram;
};

int main(int argc, char **argv)
{
    HelloTriangleSample app;
    return app.run();
}
