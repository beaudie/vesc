//
// Copyright (c) 2014 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//

//            Based on Simple_Texture2D.c from
// Book:      OpenGL(R) ES 2.0 Programming Guide
// Authors:   Aaftab Munshi, Dan Ginsburg, Dave Shreiner
// ISBN-10:   0321502795
// ISBN-13:   9780321502797
// Publisher: Addison-Wesley Professional
// URLs:      http://safari.informit.com/9780321563835
//            http://www.opengles-book.com

#include "SampleApplication.h"
#include "shader_utils.h"
#include "texture_utils.h"

class SimpleTexture2DSample : public SampleApplication
{
  public:
    SimpleTexture2DSample()
        : SampleApplication("SimpleTexture2D",
                            1280,
                            720,
                            2,
                            0,
                            EGL_PLATFORM_ANGLE_TYPE_VULKAN_ANGLE)
    {
    }

    virtual bool initialize()
    {
        const std::string vs =
            R"(attribute vec4 a_position;
            attribute vec2 a_texCoord;
            varying vec2 v_texCoord;
            void main()
            {
                gl_Position = a_position;
                v_texCoord = a_texCoord;
            })";

        const std::string fs =
            R"(precision mediump float;
            varying vec2 v_texCoord;
            uniform sampler2D s_texture;
            void main()
            {
                gl_FragColor = texture2D(s_texture, v_texCoord);
            })";

        mProgram = CompileProgram(vs, fs);
        if (!mProgram)
        {
            return false;
        }

        // Get the attribute locations
        GLint positionLoc = glGetAttribLocation(mProgram, "a_position");
        GLint texCoordLoc = glGetAttribLocation(mProgram, "a_texCoord");

        // Get the sampler location
        mSamplerLoc = glGetUniformLocation(mProgram, "s_texture");

        // Load the texture
        mTexture = CreateSimpleTexture2D();

        glClearColor(0.0f, 0.0f, 0.0f, 0.0f);

        // Use the program object
        glUseProgram(mProgram);

        glGenBuffers(1, &mVertexBuffer);
        glBindBuffer(GL_ARRAY_BUFFER, mVertexBuffer);

        GLfloat vertices[] =
        {
            -0.5f,  0.5f, 0.0f,  // Position 0
             0.0f,  0.0f,        // TexCoord 0
            -0.5f, -0.5f, 0.0f,  // Position 1
             0.0f,  1.0f,        // TexCoord 1
             0.5f, -0.5f, 0.0f,  // Position 2
             1.0f,  1.0f,        // TexCoord 2
             0.5f,  0.5f, 0.0f,  // Position 3
             1.0f,  0.0f         // TexCoord 3
        };

        glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

        // Load the vertex position
        glVertexAttribPointer(positionLoc, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(GLfloat), nullptr);

        // Load the texture coordinate
        glVertexAttribPointer(texCoordLoc, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(GLfloat),
                              reinterpret_cast<void *>(3 * sizeof(float)));

        glEnableVertexAttribArray(positionLoc);
        glEnableVertexAttribArray(texCoordLoc);

        GLushort indices[] = { 0, 1, 2, 0, 2, 3 };

        glGenBuffers(1, &mIndexBuffer);
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, mIndexBuffer);
        glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW);

        return true;
    }

    virtual void destroy()
    {
        glDeleteBuffers(1, &mIndexBuffer);
        glDeleteBuffers(1, &mVertexBuffer);
        glDeleteProgram(mProgram);
        glDeleteTextures(1, &mTexture);
    }

    virtual void draw()
    {
        // Set the viewport
        glViewport(0, 0, getWindow()->getWidth(), getWindow()->getHeight());

        // Clear the color buffer
        glClear(GL_COLOR_BUFFER_BIT);

        // Bind the texture
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, mTexture);

        // Set the texture sampler to texture unit to 0
        glUniform1i(mSamplerLoc, 0);

        glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_SHORT, nullptr);
    }

  private:
    // Handle to a program object
    GLuint mProgram = 0;

    // Sampler location
    GLint mSamplerLoc = -1;

    // Texture handle
    GLuint mTexture = 0;

    GLuint mVertexBuffer = 0;
    GLuint mIndexBuffer  = 0;
};

int main(int argc, char **argv)
{
    SimpleTexture2DSample app;
    return app.run();
}
