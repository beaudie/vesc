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

class HelloTriangleSample : public SampleApplication
{
  public:
    HelloTriangleSample()
        : SampleApplication("HelloTriangle", 1280, 720)
    {
    }

    virtual bool initialize()
    {
        std::string vs = SHADER_SOURCE(
            layout(location = 0) in vec4 vPosition; layout(location = 1) in vec4 vColor;
            layout(location = 2) in vec4 vOffset; out vec4 oColor; void main() {
                gl_Position = vPosition + vOffset;
                oColor      = vColor;
            });

        std::string fs =
            SHADER_SOURCE(precision mediump float; in vec4 oColor;
                          layout(location = 0) out vec4 fColor; void main() { fColor = oColor; });

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
        GLfloat vertices[] = {
            0.0f, 0.5f, 0.0f, -0.5f, -0.5f, 0.0f, 0.5f, -0.5f,
            0.0f, 1.0f, 0.5f, 0.0f,  0.5f,  1.0f, 0.0f,
        };

        GLfloat colors[] = {
            1.0f, 0.0f, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f, 0.0f,
            1.0f, 1.0f, 1.0f, 0.0f, 0.0f, 1.0f, 1.0f,
        };

        GLfloat attribs[] = {
            0.0f, 0.5f, 0.0f, 1.0f,  0.0f, 0.0f, -0.5f, -0.5f, 0.0f, 0.0f,
            1.0f, 0.0f, 0.5f, -0.5f, 0.0f, 0.0f, 0.0f,  1.0f,  1.0f, 0.5f,
            0.0f, 1.0f, 1.0f, 0.0f,  0.5f, 1.0f, 0.0f,  0.0f,  1.0f, 1.0f,
        };

        GLfloat offsets[] = {
            0.0f, 0.0f, 0.0f, 0.1f, 0.1f, 0.0f, 0.2f, 0.2f,
            0.0f, 0.3f, 0.3f, 0.0f, 0.4f, 0.4f, 0.0f,
        };

        GLubyte indices[] = {0, 1, 2, 0xffu, 3, 0, 4};

        GLuint vao, vbo;
        // glGenVertexArrays(1, &vao);
        // glBindVertexArray(vao);

        // Set the viewport
        glViewport(0, 0, getWindow()->getWidth(), getWindow()->getHeight());

        // Clear the color buffer
        glClear(GL_COLOR_BUFFER_BIT);

        // Use the program object
        glUseProgram(mProgram);

        // glGenBuffers(1, &vbo);
        // glBindBuffer(GL_ARRAY_BUFFER, vbo);
        // glBufferData(GL_ARRAY_BUFFER, 30 * sizeof(GLfloat), attribs, GL_STATIC_DRAW);

        // Load the vertex data
        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, vertices);
        // glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 24, attribs);
        // glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 24, 0);
        glEnableVertexAttribArray(0);

        // glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 24, (const void *)12);
        // glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 0, 0);
        glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 0, colors);
        // glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 24, &attribs[3]);
        glEnableVertexAttribArray(1);

        glGenBuffers(1, &vbo);
        glBindBuffer(GL_ARRAY_BUFFER, vbo);
        glBufferData(GL_ARRAY_BUFFER, 15 * sizeof(GLfloat), offsets, GL_STATIC_DRAW);
        glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, 0, 0);

        // glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, 0, offsets);

        // glVertexAttribDivisor(2, 1);
        glVertexAttribDivisor(2, 2);
        glEnableVertexAttribArray(2);

        // GLuint elementbuffer;
        // glGenBuffers(1, &elementbuffer);
        // glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, elementbuffer);
        // glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), &indices[0], GL_STATIC_DRAW);

        glEnable(GL_PRIMITIVE_RESTART_FIXED_INDEX);

        // glDrawArrays(GL_TRIANGLES, 0, 3);
        // glDrawElements(GL_TRIANGLE_STRIP, 3, GL_UNSIGNED_BYTE, indices);
        // glDrawElementsInstanced(GL_TRIANGLE_STRIP, 3, GL_UNSIGNED_BYTE, indices, 5);
        // glDrawElementsInstanced(GL_TRIANGLE_STRIP, 3, GL_UNSIGNED_BYTE, nullptr, 5);
        glDrawArraysInstanced(GL_TRIANGLE_STRIP, 0, 3, 5);

        glDisable(GL_PRIMITIVE_RESTART_FIXED_INDEX);

        glUseProgram(0);
        glDisableVertexAttribArray(0);
        glDisableVertexAttribArray(1);
        glDisableVertexAttribArray(2);
        glDeleteBuffers(1, &vbo);
        // glDeleteBuffers(1, &elementbuffer);
        // glDeleteVertexArrays(1, &vao);
    }

  private:
    GLuint mProgram;
};

int main(int argc, char **argv)
{
    HelloTriangleSample app;
    return app.run();
}
