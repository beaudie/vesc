//
// Copyright 2018 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//

// EglSyncImpl.h: Defines the rx::EglSyncImpl class.

#ifndef LIBANGLE_RENDERER_EGLSYNCIMPL_H_
#define LIBANGLE_RENDERER_EGLSYNCIMPL_H_

#include "libANGLE/Error.h"

#include "common/angleutils.h"

#include "angle_gl.h"

namespace egl
{
class Display;
}  // namespace egl

namespace rx
{
class EglSyncImpl : angle::NonCopyable
{
  public:
    EglSyncImpl(){};
    virtual ~EglSyncImpl(){};

    virtual egl::Error set(const egl::Display *display, EGLenum type)               = 0;
    virtual egl::Error clientWait(const egl::Display *display,
                                  EGLint flags,
                                  EGLTime timeout,
                                  EGLint *outResult)                                = 0;
    virtual egl::Error serverWait(const egl::Display *display, EGLint flags)        = 0;
    virtual egl::Error getStatus(const egl::Display *display, EGLAttrib *outStatus) = 0;

    virtual void destroySync(const egl::Display *display) = 0;
};
}  // namespace rx

#endif  // LIBANGLE_RENDERER_EGLSYNCIMPL_H_
