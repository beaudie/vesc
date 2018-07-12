//
// Copyright 2018 The ANGLE Project Authors. All rights reserved.
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

class GLES1SimpleDrawTextureSample : public SampleApplication
{
  public:
    GLES1SimpleDrawTextureSample(int argc, char **argv)
        : SampleApplication("GLES1SimpleDrawTexture", argc, argv, 1, 0)
    {
    }

    bool initialize() override
    {
        // Load the texture
        mTexture = CreateSimpleTexture2D();

        glClearColor(0.0f, 0.0f, 0.0f, 0.0f);
        glActiveTexture(GL_TEXTURE0);
        glEnable(GL_TEXTURE_2D);

        glActiveTexture(GL_TEXTURE0);
        glEnable(GL_TEXTURE_2D);
        glBindTexture(GL_TEXTURE_2D, mTexture);
        GLint crop[4] = {0, 0, 2, 2};
        glTexParameteriv(GL_TEXTURE_2D, GL_TEXTURE_CROP_RECT_OES, crop);

        return true;
    }

    void destroy() override { glDeleteTextures(1, &mTexture); }

    void draw() override
    {
        glViewport(0, 0, getWindow()->getWidth(), getWindow()->getHeight());
        glClear(GL_COLOR_BUFFER_BIT);

        glDrawTexiOES(0, 0, 0, getWindow()->getWidth(), getWindow()->getHeight());
    }

  private:
    // Texture handle
    GLuint mTexture = 0;
};

int main(int argc, char **argv)
{
    GLES1SimpleDrawTextureSample app(argc, argv);
    return app.run();
}
