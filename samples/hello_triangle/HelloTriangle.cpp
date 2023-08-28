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

#include <algorithm>

static constexpr int fbCount   = 2;
static constexpr int fbSize[2] = {10000, 1000};

class HelloTriangleSample : public SampleApplication
{
  public:
    HelloTriangleSample(int argc, char **argv)
        : SampleApplication("HelloTriangle", argc, argv, ClientType::ES3_0)
    {}

    bool initialize() override
    {
        GLuint renderbuffers[fbCount]   = {0};
        GLuint dsRenderbuffers[fbCount] = {0};
        glGenRenderbuffers(fbCount, renderbuffers);
        glGenRenderbuffers(fbCount, dsRenderbuffers);
        glGenFramebuffers(fbCount, mFramebuffers);

        GLuint resolveRenderbuffers[fbCount] = {0};
        glGenRenderbuffers(fbCount, resolveRenderbuffers);
        glGenFramebuffers(fbCount, mResolveFramebuffers);

        for (int i = 0; i < fbCount; i++)
        {
            glBindRenderbuffer(GL_RENDERBUFFER, renderbuffers[i]);
            glRenderbufferStorageMultisample(GL_RENDERBUFFER, 4, GL_BGRA8_EXT, fbSize[0],
                                             fbSize[1]);

            glBindRenderbuffer(GL_RENDERBUFFER, dsRenderbuffers[i]);
            glRenderbufferStorageMultisample(GL_RENDERBUFFER, 4, GL_DEPTH24_STENCIL8, fbSize[0],
                                             fbSize[1]);

            glBindFramebuffer(GL_FRAMEBUFFER, mFramebuffers[i]);
            glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_RENDERBUFFER,
                                      renderbuffers[i]);
            glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER,
                                      dsRenderbuffers[i]);
            glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_STENCIL_ATTACHMENT, GL_RENDERBUFFER,
                                      dsRenderbuffers[i]);

            glBindRenderbuffer(GL_RENDERBUFFER, resolveRenderbuffers[i]);
            glRenderbufferStorage(GL_RENDERBUFFER, GL_BGRA8_EXT, fbSize[0], fbSize[1]);

            glBindFramebuffer(GL_FRAMEBUFFER, mResolveFramebuffers[i]);
            glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_RENDERBUFFER,
                                      resolveRenderbuffers[i]);
        }

        return true;
    }

    void destroy() override {}

    void draw() override
    {
        glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
        glViewport(0, 0, getWindow()->getWidth(), getWindow()->getHeight());
        glClear(GL_COLOR_BUFFER_BIT);

        for (int i = 0; i < fbCount; i++)
        {
            glClearColor(1.0f, float(i) / std::max(fbCount - 1, 1), 0.0f, 1.0f);

            glBindFramebuffer(GL_FRAMEBUFFER, mFramebuffers[i]);
            glViewport(0, 0, fbSize[0], fbSize[1]);
            glClear(GL_COLOR_BUFFER_BIT);
        }

        for (int i = 0; i < fbCount; i++)
        {
            glBindFramebuffer(GL_DRAW_FRAMEBUFFER, mResolveFramebuffers[i]);
            glBindFramebuffer(GL_READ_FRAMEBUFFER, mFramebuffers[i]);
            glBlitFramebuffer(0, 0, fbSize[0], fbSize[1], 0, 0, fbSize[0], fbSize[1],
                              GL_COLOR_BUFFER_BIT, GL_NEAREST);

            glBindFramebuffer(GL_DRAW_FRAMEBUFFER, 0);
            glBindFramebuffer(GL_READ_FRAMEBUFFER, mResolveFramebuffers[i]);
            glBlitFramebuffer(0, 0, fbSize[0], fbSize[1], i * 100, 0, (i + 1) * 100, 100,
                              GL_COLOR_BUFFER_BIT, GL_NEAREST);
        }
    }

  private:
    GLuint mFramebuffers[fbCount];
    GLuint mResolveFramebuffers[fbCount];
};

int main(int argc, char **argv)
{
    HelloTriangleSample app(argc, argv);
    return app.run();
}
