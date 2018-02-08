//
// Copyright (c) 2018 The ANGLE Project Authors. All rights reserved.
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
#include "system_utils.h"

#include <GLES/gl.h>
#include <GLES/glext.h>

class DrawTexSample : public SampleApplication
{
  public:
    DrawTexSample(EGLint displayType)
        : SampleApplication("DrawTex", 1280, 720, 1 /* gles1 */, 0, displayType)
    {
    }

    virtual bool initialize()
    {
        glClearColor(0.0f, 0.0f, 0.0f, 0.0f);
        glEnable(GL_BLEND);
        glBlendFunc(GL_SRC_COLOR, GL_ONE_MINUS_SRC_COLOR);

        // Load the texture
        mBackground = CreateSimpleTexture2D();
        mScroller = CreateSimpleTexture2D();

        GLint cropRectBackground[4] = {
            0, 0, 2, 2,
        };

        glActiveTexture(GL_TEXTURE0);
        glEnable(GL_TEXTURE_2D);
        glBindTexture(GL_TEXTURE_2D, mBackground);

        glTexParameteriv(GL_TEXTURE_2D, GL_TEXTURE_CROP_RECT_OES, cropRectBackground);

        GLint cropRectScroller[4] = {
            0, 0, 2, 2,
        };

        glActiveTexture(GL_TEXTURE1);
        glEnable(GL_TEXTURE_2D);
        glBindTexture(GL_TEXTURE_2D, mScroller);
        glTexParameteriv(GL_TEXTURE_2D, GL_TEXTURE_CROP_RECT_OES, cropRectScroller);

        mTime = 0;

        return true;
    }

    virtual void destroy() { glDeleteTextures(1, &mBackground); }

    virtual void draw()
    {
        GLint w = getWindow()->getWidth();
        GLint h = getWindow()->getHeight();

        glViewport(0, 0, w, h);

        // Clear the color buffer
        glClear(GL_COLOR_BUFFER_BIT);

        GLint cropRectBackground[4] = {
            0, 0, mTime % w, mTime % h,
        };

        GLfloat cropRectScroller[4] = {
            mTime % w, 0, w / 32, h / 32,
        };

        glActiveTexture(GL_TEXTURE0);
        glTexParameteriv(GL_TEXTURE_2D, GL_TEXTURE_CROP_RECT_OES, cropRectBackground);

        glActiveTexture(GL_TEXTURE1);
        glTexParameterfv(GL_TEXTURE_2D, GL_TEXTURE_CROP_RECT_OES, cropRectScroller);

        glDrawTexfOES(-(float)(mTime % w), -(float)(mTime % h), 0.0f, w * 2, h * 2);

        angle::Sleep(4);

        mTime++;
    }

  private:
    GLuint mBackground;
    GLuint mScroller;

    int mTime;
};

int main(int argc, char **argv)
{
    EGLint displayType = EGL_PLATFORM_ANGLE_TYPE_DEFAULT_ANGLE;

    if (argc > 1)
    {
        displayType = GetDisplayTypeFromArg(argv[1]);
    }

    DrawTexSample app(displayType);
    return app.run();
}
