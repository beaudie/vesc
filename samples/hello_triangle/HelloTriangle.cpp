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

#include "texture_utils.h"
#include "util/shader_utils.h"

class HelloTriangleSample : public SampleApplication
{
  public:
    HelloTriangleSample(int argc, char **argv)
        : SampleApplication("HelloTriangle", argc, argv, ClientType::ES3_0)
    {}

    bool initialize() override
    {
        constexpr char kVS[] = R"(
attribute vec4 a_position;
attribute vec2 a_texCoord;
varying vec2 v_texCoord;
varying float arrayIndex;
void main()
{
    gl_Position = a_position;
    v_texCoord = a_texCoord;
    arrayIndex = clamp(gl_Position.x, 0.0, 1000.0);
})";

        constexpr char kFS[] = R"(
precision mediump float;

uniform vec4 u_color;
uniform vec4 u_bigArray[1000];

uniform sampler2D s_texture;

varying vec2 v_texCoord;
varying float arrayIndex;

void main()
{
    gl_FragColor = u_color * texture2D(s_texture, v_texCoord) + u_bigArray[int(arrayIndex)];
})";

        mProgram = CompileProgram(kVS, kFS);
        if (!mProgram)
        {
            return false;
        }

        mColorUniform  = glGetUniformLocation(mProgram, "u_color");
        mSampleUniform = glGetUniformLocation(mProgram, "s_texture");

        GLint positionLoc = glGetAttribLocation(mProgram, "a_position");
        GLint texCoordLoc = glGetAttribLocation(mProgram, "a_texCoord");

        constexpr int vaoCount = 1600;
        mVertexArrays.resize(vaoCount);
        glGenVertexArrays(vaoCount, mVertexArrays.data());
        for (int i = 0; i < vaoCount; i++)
        {
            glBindVertexArray(mVertexArrays[i]);

            {
                GLuint vertexBuffer = 0;
                glGenBuffers(1, &vertexBuffer);

                float sz     = 0.05f;
                int colCount = static_cast<int>(2.0f / sz);

                int row = i % colCount;
                int col = i / colCount;

                float x0 = -1.0f + (col * sz);
                float y0 = -1.0f + (row * sz);
                float x1 = -1.0f + ((col + 1) * sz);
                float y1 = -1.0f + ((row + 1) * sz);

                GLfloat vertices[] = {
                    x0, y1, 0.0f,  // Position 0
                    x0, y0, 0.0f,  // Position 1
                    x1, y0, 0.0f,  // Position 2
                    x1, y1, 0.0f,  // Position 3
                };

                glBindBuffer(GL_ARRAY_BUFFER, vertexBuffer);
                glBufferData(GL_ARRAY_BUFFER, sizeof(float) * 12, vertices, GL_STATIC_DRAW);
                glVertexAttribPointer(positionLoc, 3, GL_FLOAT, GL_FALSE, 0, nullptr);
                glEnableVertexAttribArray(positionLoc);
            }

            {
                GLuint texcoordBuffer = 0;
                glGenBuffers(1, &texcoordBuffer);

                constexpr GLfloat texcoords[] = {
                    0.0f, 0.0f,  // TexCoord 0
                    0.0f, 1.0f,  // TexCoord 1
                    1.0f, 1.0f,  // TexCoord 2
                    1.0f, 0.0f   // TexCoord 3
                };

                glBindBuffer(GL_ARRAY_BUFFER, texcoordBuffer);
                glBufferData(GL_ARRAY_BUFFER, sizeof(float) * 8, texcoords, GL_STATIC_DRAW);
                glVertexAttribPointer(texCoordLoc, 2, GL_FLOAT, GL_FALSE, 0, nullptr);
                glEnableVertexAttribArray(texCoordLoc);
            }

            {
                GLuint indexBuffer = 0;
                glGenBuffers(1, &indexBuffer);

                GLushort indices[] = {0, 1, 2, 0, 2, 3};

                glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, indexBuffer);
                glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(GLushort) * 6, indices,
                             GL_STATIC_DRAW);
            }
        }

        constexpr int textureCount = 10;
        for (int i = 0; i < textureCount; i++)
        {
            mTextures.push_back(CreateSimpleTexture2D());
        }

        glClearColor(0.0f, 0.0f, 0.0f, 0.0f);

        if (glGetError() != GL_NO_ERROR)
        {
            return false;
        }

        std::cout << glGetString(GL_RENDERER) << std::endl;

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

        for (size_t i = 0; i < mVertexArrays.size(); i++)
        {
            glBindVertexArray(mVertexArrays[i]);

            if (i % 2 == 0)
            {
                glBindTexture(GL_TEXTURE_2D, mTextures[i % mTextures.size()]);
            }

            glUniform4f(mColorUniform, float(i) / mVertexArrays.size(), 1.0f, 1.0f, 1.0f);

            glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_SHORT, nullptr);
        }
    }

  private:
    GLuint mProgram;
    GLuint mColorUniform;
    GLuint mSampleUniform;

    std::vector<GLuint> mTextures;

    std::vector<GLuint> mVertexArrays;
};

int main(int argc, char **argv)
{
    HelloTriangleSample app(argc, argv);
    return app.run();
}
