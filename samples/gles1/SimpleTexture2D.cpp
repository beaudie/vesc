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

class SimpleTexture2DSample : public SampleApplication
{
  public:
    SimpleTexture2DSample(EGLint displayType)
        : SampleApplication("SimpleTexture2D", 1280, 720,
                            1 /* gles1 */, 0, displayType)
    {
    }

    virtual bool initialize()
    {
        // Load the texture
        mTexture = CreateSimpleTexture2D();

        glClearColor(0.0f, 0.0f, 0.0f, 0.0f);

        return true;
    }

    virtual void destroy()
    {
        glDeleteTextures(1, &mTexture);
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

        glEnable(GL_POINT_SPRITE_OES);
        glEnable(GL_TEXTURE_2D);
        glEnableClientState(GL_VERTEX_ARRAY);
        glEnableClientState(GL_TEXTURE_COORD_ARRAY);

        // Load the vertex position
        glVertexPointer(3, GL_FLOAT, 5 * sizeof(GLfloat), vertices);
        // Load the texture coordinate
        glTexCoordPointer(2, GL_FLOAT, 5 * sizeof(GLfloat), vertices + 3);

        // Bind the texture
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, mTexture);

        glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_SHORT, indices);

        glDisableClientState(GL_VERTEX_ARRAY);
        glDisableClientState(GL_TEXTURE_COORD_ARRAY);
    }

  private:
    // Texture handle
    GLuint mTexture;
};

int main(int argc, char **argv)
{
    EGLint displayType = EGL_PLATFORM_ANGLE_TYPE_DEFAULT_ANGLE;

    if (argc > 1)
    {
        displayType = GetDisplayTypeFromArg(argv[1]);
    }

    SimpleTexture2DSample app(displayType);
    return app.run();
}
