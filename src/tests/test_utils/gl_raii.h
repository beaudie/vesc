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

// This is a bit of hack to work around a bug in MSVS intellisense, and make it very easy to
// use the correct function pointer type without worrying about the various definitions of
// GL_APICALL.
using GLGen    = decltype(glGenBuffers);
using GLDelete = decltype(glDeleteBuffers);

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

class GLBuffer : public GLWrapper<glGenBuffers, glDeleteBuffers>
{
};

class GLTexture : public GLWrapper<glGenTextures, glDeleteTextures>
{
};

class GLFramebuffer : public GLWrapper<glGenFramebuffers, glDeleteFramebuffers>
{
};

class GLRenderbuffer : public GLWrapper<glGenRenderbuffers, glDeleteRenderbuffers>
{
};

}  // namespace angle
