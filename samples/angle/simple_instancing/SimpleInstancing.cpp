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
#include "Vector.h"
#include "shader_utils.h"
#include "texture_utils.h"

#include <iostream>
#include <vector>

class SimpleInstancingSample : public SampleApplication
{
  public:
    SimpleInstancingSample::SimpleInstancingSample()
        : SampleApplication("SimpleInstancing", 1280, 720)
    {
    }

    virtual bool initialize()
    {
        // init instancing functions
        char *extensionString = (char*)glGetString(GL_EXTENSIONS);
        if (strstr(extensionString, "GL_ANGLE_instanced_arrays"))
        {
            mVertexAttribDivisorANGLE = (PFNGLVERTEXATTRIBDIVISORANGLEPROC)eglGetProcAddress("glVertexAttribDivisorANGLE");
            mDrawArraysInstancedANGLE = (PFNGLDRAWARRAYSINSTANCEDANGLEPROC)eglGetProcAddress("glDrawArraysInstancedANGLE");
            mDrawElementsInstancedANGLE = (PFNGLDRAWELEMENTSINSTANCEDANGLEPROC)eglGetProcAddress("glDrawElementsInstancedANGLE");
        }

        if (!mVertexAttribDivisorANGLE || !mDrawArraysInstancedANGLE || !mDrawElementsInstancedANGLE)
        {
            std::cerr << "Unable to load GL_ANGLE_instanced_arrays entry points.";
            return false;
        }

        const std::string vs = SHADER_SOURCE
        (
            attribute vec3 a_position;
            attribute vec2 a_texCoord;
            attribute vec3 a_instancePos;
            varying vec2 v_texCoord;
            void main()
            {
                gl_Position = vec4(a_position.xyz + a_instancePos.xyz, 1.0);
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
        mInstancePosLoc = glGetAttribLocation(mProgram, "a_instancePos");

        // Get the sampler location
        mSamplerLoc = glGetUniformLocation(mProgram, "s_texture");

        // Load the texture
        mTextureID = CreateSimpleTexture2D();

        // Initialize the vertex and index vectors
        const GLfloat quadRadius = 0.01f;

        std::vector<Vector4> vertices;
        vertices.push_back(Vector4(-quadRadius,  quadRadius, 0.0f, 1.0f));
        vertices.push_back(Vector4(-quadRadius, -quadRadius, 0.0f, 1.0f));
        vertices.push_back(Vector4( quadRadius, -quadRadius, 0.0f, 1.0f));
        vertices.push_back(Vector4( quadRadius,  quadRadius, 0.0f, 1.0f));

        std::vector<Vector4> texcoords;
        texcoords.push_back(Vector4(0.0f, 0.0f, 0.0f, 1.0f));
        texcoords.push_back(Vector4(0.0f, 1.0f, 0.0f, 1.0f));
        texcoords.push_back(Vector4(1.0f, 1.0f, 0.0f, 1.0f));
        texcoords.push_back(Vector4(1.0f, 0.0f, 0.0f, 1.0f));

        std::vector<GLushort> indices;
        indices.push_back(0);
        indices.push_back(1);
        indices.push_back(2);
        indices.push_back(0);
        indices.push_back(2);
        indices.push_back(3);
        mPrimitiveCount = indices.size();

        // Tile thousands of quad instances
        std::vector<Vector4> instances;
        for (float y = -1.0f + quadRadius; y < 1.0f - quadRadius; y += quadRadius * 3)
        {
            for (float x = -1.0f + quadRadius; x < 1.0f - quadRadius; x += quadRadius * 3)
            {
                instances.push_back(Vector4(x, y, 0.0f, 1.0f));
            }
        }
        mInstanceCount = instances.size();

        // Load the vertex position
        glGenBuffers(1, &mVertices);
        glBindBuffer(GL_ARRAY_BUFFER, mVertices);
        glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(Vector4), vertices.data(), GL_STATIC_DRAW);
        glVertexAttribPointer(mPositionLoc, 4, GL_FLOAT, GL_FALSE, 0, NULL);
        glEnableVertexAttribArray(mPositionLoc);

        // Load the texcoords
        glGenBuffers(1, &mTexcoords);
        glBindBuffer(GL_ARRAY_BUFFER, mTexcoords);
        glBufferData(GL_ARRAY_BUFFER, texcoords.size() * sizeof(Vector4), texcoords.data(), GL_STATIC_DRAW);
        glVertexAttribPointer(mTexCoordLoc, 4, GL_FLOAT, GL_FALSE, 0, NULL);
        glEnableVertexAttribArray(mTexCoordLoc);

        // Load the vertex position
        glGenBuffers(1, &mInstances);
        glBindBuffer(GL_ARRAY_BUFFER, mInstances);
        glBufferData(GL_ARRAY_BUFFER, instances.size() * sizeof(Vector4), instances.data(), GL_STATIC_DRAW);
        glVertexAttribPointer(mInstancePosLoc, 4, GL_FLOAT, GL_FALSE, 0, NULL);
        glEnableVertexAttribArray(mInstancePosLoc);

        // Enable instancing
        mVertexAttribDivisorANGLE(mInstancePosLoc, 1);

        // Load the indices
        glGenBuffers(1, &mIndices);
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, mIndices);
        glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices.size() * sizeof(GLushort), indices.data(), GL_STATIC_DRAW);

        // Bind the texture
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, mTextureID);

        // Set the sampler texture unit to 0
        glUniform1i(mSamplerLoc, 0);

        // Use the program object
        glUseProgram(mProgram);

        glClearColor(0.0f, 0.0f, 0.0f, 0.0f);

        return true;
    }

    virtual void destroy()
    {
        glDeleteProgram(mProgram);
        glDeleteTextures(1, &mTextureID);
    }

    virtual void draw()
    {
        // Set the viewport
        glViewport(0, 0, getWindow()->getWidth(), getWindow()->getHeight());

        // Clear the color buffer
        glClear(GL_COLOR_BUFFER_BIT);

        // Do the instanced draw
        mDrawElementsInstancedANGLE(GL_TRIANGLES, mPrimitiveCount, GL_UNSIGNED_SHORT, 0, mInstanceCount);
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
    GLuint mTextureID;

    // Instance VBO
    GLint mInstancePosLoc;

    // Loaded entry points
    PFNGLVERTEXATTRIBDIVISORANGLEPROC mVertexAttribDivisorANGLE;
    PFNGLDRAWARRAYSINSTANCEDANGLEPROC mDrawArraysInstancedANGLE;
    PFNGLDRAWELEMENTSINSTANCEDANGLEPROC mDrawElementsInstancedANGLE;

    // Vertex data
    GLuint mVertices;
    GLuint mTexcoords;
    GLuint mInstances;
    GLuint mIndices;

    GLuint mPrimitiveCount;
    GLuint mInstanceCount;
};

int main(int argc, char **argv)
{
    SimpleInstancingSample app;
    return app.run();
}
