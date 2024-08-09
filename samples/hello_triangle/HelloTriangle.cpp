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
    HelloTriangleSample(int argc, char **argv) : SampleApplication("HelloTriangle", argc, argv) {}

    bool initialize() override
    {
        constexpr char kVS[] = R"(attribute vec2 aPosition;
attribute vec4 aColor;

varying vec4 vColor;

uniform mat3 translationMatrix;
uniform mat3 projectionMatrix;

void main()
{
   gl_PointSize = 10.0;
   gl_Position = vec4((projectionMatrix * translationMatrix * vec3(aPosition, 1.0)).xy, 0.0, 1.0);
    vColor = aColor;
})";

        constexpr char kFS[] = R"(precision mediump float;
varying vec4 vColor;
void main()
{
    gl_FragColor = vColor;
})";

        mProgram = CompileProgram(kVS, kFS);
        if (!mProgram)
        {
            return false;
        }

        glUseProgram(mProgram);

        glClearColor(0.0f, 1.0f, 0.0f, 0.0f);

        glEnable(GL_BLEND);
        glBlendFunc(GL_ONE, GL_ONE_MINUS_SRC_ALPHA);

        GLuint renderbuffer;
        glGenRenderbuffers(1, &renderbuffer);
        glBindRenderbuffer(GL_RENDERBUFFER, renderbuffer);
        glRenderbufferStorageMultisampleANGLE(GL_RENDERBUFFER, 4, GL_RGBA8, getWindow()->getWidth(),
                                              getWindow()->getHeight());

        glGenFramebuffers(1, &mFBO);
        glBindFramebuffer(GL_FRAMEBUFFER, mFBO);
        glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_RENDERBUFFER,
                                  renderbuffer);

        GLfloat vertices[] = {
            90.0000,  90.0000,  1.0000, 0.0000, 1.0000, 1.0000,  // 0
            310.0000, 90.0000,  0.0000, 1.0000, 1.0000, 1.0000,  // 1
            90.0000,  310.0000, 1.0000, 1.0000, 0.0000, 1.0000,  // 2
            310.0000, 310.0000, 1.0000, 1.0000, 1.0000, 1.0000,  // 3

            100.0000, 100.0000, 0.1333, 0.1333, 0.1333, 1.0000,  // 4
            300.0000, 100.0000, 0.1333, 0.1333, 0.1333, 1.0000,  // 5
            100.0000, 300.0000, 0.1333, 0.1333, 0.1333, 1.0000,  // 6
            300.0000, 300.0000, 0.1333, 0.1333, 0.1333, 1.0000,  // 7
        };

        GLuint vertexBuffer;
        glGenBuffers(1, &vertexBuffer);
        glBindBuffer(GL_ARRAY_BUFFER, vertexBuffer);
        glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

        glVertexAttribPointer(glGetAttribLocation(mProgram, "aPosition"), 2, GL_FLOAT, GL_FALSE,
                              6 * sizeof(float), vertices);
        glEnableVertexAttribArray(glGetAttribLocation(mProgram, "aPosition"));

        glVertexAttribPointer(glGetAttribLocation(mProgram, "aColor"), 4, GL_FLOAT, GL_FALSE,
                              6 * sizeof(float), vertices + 2);
        glEnableVertexAttribArray(glGetAttribLocation(mProgram, "aColor"));

        const GLushort indices[] = {0, 0, 1, 2, 3, 3, 4, 4, 5, 6, 7, 7};

        GLuint indexBuffer;
        glGenBuffers(1, &indexBuffer);
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, indexBuffer);
        glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW);

        const GLint projectionMatrixLocation  = glGetUniformLocation(mProgram, "projectionMatrix");
        const GLint translationMatrixLocation = glGetUniformLocation(mProgram, "translationMatrix");

        GLfloat projectionMatrix[] = {0.005, 0, 0, 0, -0.005, 0, -1, 1, 1};

        GLfloat translationMatrix[] = {1, 0, 0, 0, 1, 0, 0, 0, 1};

        glUniformMatrix3fv(projectionMatrixLocation, 1, false, projectionMatrix);
        glUniformMatrix3fv(translationMatrixLocation, 1, false, translationMatrix);

        // Load the vertex data
        glVertexAttribPointer(glGetAttribLocation(mProgram, "aPosition"), 2, GL_FLOAT, GL_FALSE,
                              6 * sizeof(float), 0);
        glEnableVertexAttribArray(glGetAttribLocation(mProgram, "aPosition"));

        glVertexAttribPointer(glGetAttribLocation(mProgram, "aColor"), 4, GL_FLOAT, GL_FALSE,
                              6 * sizeof(float), (void *)(sizeof(float) * 2));
        glEnableVertexAttribArray(glGetAttribLocation(mProgram, "aColor"));

        return true;
    }

    void destroy() override { glDeleteProgram(mProgram); }

    void draw() override
    {
        // Set the viewport
        glViewport(0, 0, getWindow()->getWidth(), getWindow()->getHeight());
        glBindFramebuffer(GL_FRAMEBUFFER, mFBO);

        // Clear the color buffer
        glClear(GL_COLOR_BUFFER_BIT);

        glDrawElements(GL_TRIANGLE_STRIP, 12, GL_UNSIGNED_SHORT, 0);

        glBindFramebuffer(GL_READ_FRAMEBUFFER, mFBO);
        glBindFramebuffer(GL_DRAW_FRAMEBUFFER, 0);

        glBlitFramebufferANGLE(0, 0, getWindow()->getWidth(), getWindow()->getHeight(), 0, 0,
                               getWindow()->getWidth(), getWindow()->getHeight(),
                               GL_COLOR_BUFFER_BIT, GL_NEAREST);
    }

  private:
    GLuint mProgram;
    GLuint mFBO;
};

int main(int argc, char **argv)
{
    HelloTriangleSample app(argc, argv);
    return app.run();
}
