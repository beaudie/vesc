//
// Copyright (c) 2018 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//

#include "SampleApplication.h"
#include "shader_utils.h"
#include "texture_utils.h"

#include <GLES/gl.h>
#include <GLES/glext.h>

class SimplePointRasterizationSample : public SampleApplication
{
  public:
    SimplePointRasterizationSample(EGLint displayType)
        : SampleApplication("SimplePointRasterizationSample", 1280, 720,
                            1 /* gles1 */, 0, displayType)
    {
    }

    virtual bool initialize()
    {
        glClearColor(0.0f, 0.0f, 0.0f, 0.0f);
        return true;
    }

    virtual void destroy()
    {
    }

    virtual void draw()
    {
        GLfloat vertices[] =
        {
            -0.5f,  0.5f, 0.0f,  // Position 0
            -0.5f, -0.5f, 0.0f,  // Position 1
             0.5f, -0.5f, 0.0f,  // Position 2
             0.5f,  0.5f, 0.0f,  // Position 3
        };

        glViewport(0, 0, getWindow()->getWidth(), getWindow()->getHeight());
        glClear(GL_COLOR_BUFFER_BIT);

        glEnableClientState(GL_VERTEX_ARRAY);

        glMatrixMode(GL_MODELVIEW);

        GLfloat att[3];
        att[0] = 1.0f;
        att[1] = 1.0f;
        att[2] = 1.0f;

        glPointParameterfv(GL_POINT_DISTANCE_ATTENUATION, att);
        glPointParameterf(GL_POINT_SIZE_MIN, 0.01f);

        int numPatterns = 36;
        float currX = 0.0f;
        float currY = 0.0f;
        float currZ = 0.0f;
        // float twopi = 2.0f * 3.1415926535897932384626f;

        float q = ((float)1.0f) / ((float)numPatterns);

        for (int i = 0; i < numPatterns; i++) {
            glLoadIdentity();
            float s = ((float)i) / ((float)numPatterns);
            float t = 180.0f * s;
            // float r = twopi * s;

            glRotatef(t, 0.0f, 0.0f, 1.0f);
            glTranslatef(currX, currY, currZ);

            {
                glVertexPointer(3, GL_FLOAT, 3 * sizeof(GLfloat), vertices);
                glColor4f(0.2f, 0.4f, 0.6f, 1.0f);
                glPointSize(8.0f);
                glDrawArrays(GL_POINTS, 0, 1);

                glVertexPointer(3, GL_FLOAT, 3 * sizeof(GLfloat), vertices + 3);
                glColor4f(0.8f, 0.4f, 0.3f, 1.0f);
                glPointSize(16.0f);
                glDrawArrays(GL_POINTS, 0, 1);

                glVertexPointer(3, GL_FLOAT, 3 * sizeof(GLfloat), vertices + 6);
                glColor4f(0.1f, 0.4f, 0.7f, 1.0f);
                glPointSize(32.0f);
                glDrawArrays(GL_POINTS, 0, 1);

                glVertexPointer(3, GL_FLOAT, 3 * sizeof(GLfloat), vertices + 9);
                glColor4f(0.1f, 0.9f, 0.3f, 1.0f);
                glPointSize(64.0f);
                glDrawArrays(GL_POINTS, 0, 1);
            }

            currZ -= q;
        }

        glDisableClientState(GL_VERTEX_ARRAY);
    }

  private:
};

int main(int argc, char **argv)
{
    EGLint displayType = EGL_PLATFORM_ANGLE_TYPE_DEFAULT_ANGLE;

    if (argc > 1)
    {
        displayType = GetDisplayTypeFromArg(argv[1]);
    }

    SimplePointRasterizationSample app(displayType);
    return app.run();
}
