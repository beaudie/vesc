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

#include "texture_utils.h"
#include "util/shader_utils.h"

class SimpleTexture3DSample : public SampleApplication
{
  public:
    SimpleTexture3DSample(int argc, char **argv) : SampleApplication("SimpleTexture3D", argc, argv)
    {}

    bool initialize() override
    {
        constexpr char kVS[] = R"(#version 300 es
in vec4 a_position;
in vec3 a_texCoord;
out vec3 v_texCoord;
void main()
{
    gl_Position = a_position;
    v_texCoord = a_texCoord;
})";

        constexpr char kFS[] = R"(#version 300 es
precision mediump float;
in vec3 v_texCoord;
out vec4 fragColor;
uniform mediump sampler2DArray s_texture;
void main()
{
    fragColor = texture(s_texture, vec3(v_texCoord.xy, 1.0));
})";

        mProgram = CompileProgram(kVS, kFS);
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
        mTexture = CreateSimpleTexture2DArray();

        glClearColor(0.0f, 0.0f, 0.0f, 0.0f);

        return true;
    }

    void destroy() override
    {
        glDeleteProgram(mProgram);
        glDeleteTextures(1, &mTexture);
    }

    void draw() override
    {
        const size_t vertex_stride = 6;
        GLfloat vertices[]         = {
            -0.5f, 0.5f,  0.0f,  // Position 0
            0.0f,  0.0f,  0.0f,  // TexCoord 0
            -0.5f, -0.5f, 0.0f,  // Position 1
            0.0f,  1.0f,  0.3f,  // TexCoord 1
            0.5f,  -0.5f, 0.0f,  // Position 2
            1.0f,  1.0f,  0.6f,  // TexCoord 2
            0.5f,  0.5f,  0.0f,  // Position 3
            0.0f,  1.0f,  1.0f   // TexCoord 3
        };
        GLushort indices[] = {0, 1, 2, 0, 2, 3};

        // Set the viewport
        glViewport(0, 0, getWindow()->getWidth(), getWindow()->getHeight());

        // Clear the color buffer
        glClear(GL_COLOR_BUFFER_BIT);

        // Use the program object
        glUseProgram(mProgram);

        // Load the vertex position
        glVertexAttribPointer(mPositionLoc, 3, GL_FLOAT, GL_FALSE, vertex_stride * sizeof(GLfloat),
                              vertices);
        // Load the texture coordinate
        glVertexAttribPointer(mTexCoordLoc, 3, GL_FLOAT, GL_FALSE, vertex_stride * sizeof(GLfloat),
                              vertices + 3);

        glEnableVertexAttribArray(mPositionLoc);
        glEnableVertexAttribArray(mTexCoordLoc);

        // Bind the texture
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D_ARRAY, mTexture);

        // Set the texture sampler to texture unit to 0
        glUniform1i(mSamplerLoc, 0);

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
    GLuint mTexture;
};

int main(int argc, char **argv)
{
    SimpleTexture3DSample app(argc, argv);
    return app.run();
}
