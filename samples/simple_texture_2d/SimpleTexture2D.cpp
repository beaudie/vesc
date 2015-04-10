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
        : SampleApplication("SimpleTexture2D", 1280, 720)
    {
    }

    virtual bool initialize()
    {
        const std::string vs = SHADER_SOURCE
        (
            attribute vec4 a_position;
            attribute vec2 a_texCoord;
            varying vec2 v_texCoord;
            void main()
            {
                gl_Position = a_position;
                v_texCoord = a_texCoord;
            }
        );

        const std::string fs = SHADER_SOURCE
        (
            precision mediump float;
            varying vec2 v_texCoord;
            uniform sampler2D s_texture;
            void main()
            {
                gl_FragColor = texture2D(s_texture, v_texCoord);
            }
        );

        mProgram = CompileProgram(vs, fs);
        if (!mProgram)
        {
            return false;
        }

        // Get the attribute locations
        mPositionLoc = glGetAttribLocation(mProgram, "a_position");
        mTexCoordLoc = glGetAttribLocation(mProgram, "a_texCoord");

        // Get the sampler location
        mSamplerLoc = glGetUniformLocation(mProgram, "s_texture");

        // Load the texture
        mTextureSource = CreateSimpleTexture2D();

        EGLImageKHR image = eglCreateImageKHR(getDisplay(), getContext(),
                                              EGL_GL_TEXTURE_2D_KHR, reinterpret_cast<EGLClientBuffer>(mTextureSource),
                                              nullptr);

        glGenTextures(1, &mTextureTarget);
        glBindTexture(GL_TEXTURE_2D, mTextureTarget);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

        glEGLImageTargetTexture2DOES(GL_TEXTURE_2D, image);

        GLubyte purple[4] = { 255, 0, 255 };
        glTexSubImage2D(GL_TEXTURE_2D, 0, 1, 1, 1, 1, GL_RGB, GL_UNSIGNED_BYTE, purple);
        glClearColor(0.0f, 0.0f, 0.0f, 0.0f);

        return true;
    }

    virtual void destroy()
    {
        glDeleteProgram(mProgram);
        glDeleteTextures(1, &mTextureSource);
        glDeleteTextures(1, &mTextureTarget);
    }

    virtual void draw()
    {

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
        GLushort indices[] = { 0, 1, 2, 0, 2, 3 };

        // Set the viewport
        glViewport(0, 0, getWindow()->getWidth(), getWindow()->getHeight());

        // Clear the color buffer
        glClear(GL_COLOR_BUFFER_BIT);

        // Use the program object
        glUseProgram(mProgram);

        // Load the vertex position
        glVertexAttribPointer(mPositionLoc, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(GLfloat), vertices);
        // Load the texture coordinate
        glVertexAttribPointer(mTexCoordLoc, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(GLfloat), vertices + 3);

        glEnableVertexAttribArray(mPositionLoc);
        glEnableVertexAttribArray(mTexCoordLoc);

        // Set the texture sampler to texture unit to 0
        glUniform1i(mSamplerLoc, 0);
        // Bind the texture
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, mTextureSource);

        glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_SHORT, indices);

        // Bind the texture
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, mTextureTarget);

        glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_SHORT, indices);
    }

  private:
    // Handle to a program object
    GLuint mProgram;

    // Attribute locations
    GLint mPositionLoc;
    GLint mTexCoordLoc;

    // Sampler location
    GLint mSamplerLoc;

    // Texture handle
    GLuint mTextureSource;
    GLuint mTextureTarget;
};

int main(int argc, char **argv)
{
    SimpleTexture2DSample app;
    return app.run();
}
