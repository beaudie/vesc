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

#include <GLES/gl.h>
#include <GLES/glext.h>

#include <array>
#include <vector>

class SimpleTextureEnvironmentSample : public SampleApplication
{
  public:
    SimpleTextureEnvironmentSample()
        : SampleApplication("SimpleTextureEnvironment", 1280, 720,
                            1 /* gles1 */)
    {
    }

    virtual bool initialize()
    {
        // Load the textures
        mTextures.push_back(CreateSimpleTexture2D());
        mTextures.push_back(CreateSimpleTexture2DAlphaHalf());

        // Populate texture environment parameters
        mColors.push_back({ 1.0f, 1.0f, 1.0f, 1.0f });
        mColors.push_back({ 0.0f, 0.0f, 0.0f, 1.0f });
        mColors.push_back({ 1.0f, 1.0f, 1.0f, 0.5f });
        mColors.push_back({ 0.0f, 0.0f, 0.0f, 0.5f });
        mColors.push_back({ 1.0f, 0.0f, 0.0f, 1.0f });
        mColors.push_back({ 0.0f, 1.0f, 0.0f, 1.0f });
        mColors.push_back({ 0.0f, 0.0f, 1.0f, 1.0f });
        mColors.push_back({ 1.0f, 0.0f, 0.0f, 0.5f });
        mColors.push_back({ 0.0f, 1.0f, 0.0f, 0.5f });
        mColors.push_back({ 0.0f, 0.0f, 1.0f, 0.5f });

        mModes.push_back(GL_REPLACE);
        mModes.push_back(GL_MODULATE);
        mModes.push_back(GL_DECAL);
        mModes.push_back(GL_BLEND);
        mModes.push_back(GL_ADD);

        glClearColor(0.0f, 0.0f, 0.0f, 0.0f);

        return true;
    }

    virtual void destroy()
    {
        glDeleteTextures(mTextures.size(), mTextures.data());
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

        glEnable(GL_TEXTURE_2D);
        glEnableClientState(GL_VERTEX_ARRAY);
        glEnableClientState(GL_TEXTURE_COORD_ARRAY);

        // Load the vertex position
        glVertexPointer(3, GL_FLOAT, 5 * sizeof(GLfloat), vertices);
        // Load the texture coordinate
        glTexCoordPointer(2, GL_FLOAT, 5 * sizeof(GLfloat), vertices + 3);

        glActiveTexture(GL_TEXTURE0);

        glMatrixMode(GL_MODELVIEW);

        float currX = -0.5f;
        float currY = -0.5f;

        for (int i = 0; i < 10; i++) {
            currX = -0.5f;
            for (int j = 0; j < 10; j++) {
                const auto& currColor = mColors[i % mColors.size()];
                const auto currMode = mModes[j % mModes.size()];

                glBindTexture(GL_TEXTURE_2D, mTextures[(j / (10 / mTextures.size())) % mTextures.size()]);
                glColor4f(currColor[0],
                          currColor[1],
                          currColor[2],
                          currColor[3]);

                glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, currMode);

                glLoadIdentity();
                glTranslatef(currX, currY, 0.0f);
                glPushMatrix();
                glScalef(0.07, 0.07, 0.07);
                glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_SHORT, indices);
                glPopMatrix();

                currX += 0.1f;
            }
            currY += 0.1f;
        }

        glDisableClientState(GL_VERTEX_ARRAY);
        glDisableClientState(GL_TEXTURE_COORD_ARRAY);
    }

  private:
    // Texture handles
    std::vector<GLuint> mTextures;

    // Texture environment stuff
    std::vector<std::array<GLfloat, 4> > mColors;
    std::vector<GLenum> mModes;
};

int main(int argc, char **argv)
{
    SimpleTextureEnvironmentSample app;
    return app.run();
}
