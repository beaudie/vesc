//
// Copyright (c) 2016 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//

// Stream.h: Defines the egl::Stream class, representing the stream
// where frames are streamed in. Implements EGLStreanKHR.

#ifndef LIBANGLE_STREAM_H_
#define LIBANGLE_STREAM_H_

#include <EGL/egl.h>
#include <EGL/eglext.h>

#include <set>
#include <vector>

#include "libANGLE/Error.h"
#include "libANGLE/Caps.h"
#include "libANGLE/Config.h"
#include "libANGLE/AttributeMap.h"
#include "libANGLE/renderer/Renderer.h"

namespace gl
{
class Context;
}

namespace egl
{
class Device;
class Image;
class Surface;
class Display;

class Stream final : angle::NonCopyable
{
  public:
    Stream(const AttributeMap &attribs);
    ~Stream();

    void streamAttribute(EGLint attribute, EGLint value);

    EGLint queryStream(EGLint attribute);
    EGLuint64KHR queryStreamu64(EGLint attribute);

    EGLenum getState() const;

  private:
    // EGL defined attributes
    EGLint mState;
    EGLuint64KHR mProducerFrame;
    EGLuint64KHR mConsumerFrame;
    EGLint mConsumerLatency;
};
} // namespace egl

#endif  // LIBANGLE_STREAM_H_
