//
// Copyright (c) 2016 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//
// gl_raii:
//   Helper methods for containing GL objects like buffers and textures.

#include <functional>

#include "angle_gl.h"

namespace angle
{

using GLGen    = void(GL_APICALL *)(GLsizei, GLuint *);
using GLDelete = void(GL_APICALL *)(GLsizei, const GLuint *);

template <GLGen GenF, GLDelete DeleteF>
class GLWrapper
{
  public:
    GLWrapper() {}
    ~GLWrapper() { DeleteF(1, &mHandle); }

    GLuint get()
    {
        if (!mHandle)
        {
            GenF(1, &mHandle);
        }
        return mHandle;
    }

  private:
    GLuint mHandle = 0;
};

class GLBuffer : public GLWrapper<reinterpret_cast<GLGen>(glGenBuffers),
                                  reinterpret_cast<GLDelete>(glDeleteBuffers)>
{
};

class GLTexture : public GLWrapper<reinterpret_cast<GLGen>(glGenTextures),
                                   reinterpret_cast<GLDelete>(glDeleteTextures)>
{
};

class GLFramebuffer : public GLWrapper<reinterpret_cast<GLGen>(glGenFramebuffers),
                                       reinterpret_cast<GLDelete>(glDeleteFramebuffers)>
{
};

class GLRenderbuffer : public GLWrapper<reinterpret_cast<GLGen>(glGenRenderbuffers),
                                        reinterpret_cast<GLDelete>(glDeleteRenderbuffers)>
{
};

}  // namespace angle
