//
// Copyright (c) 2016 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//

// StreamProducerImpl.h: Defines the abstract rx::StreamProducerImpl class.

#ifndef LIBANGLE_RENDERER_STREAMPRODUCERIMPL_H_
#define LIBANGLE_RENDERER_STREAMPRODUCERIMPL_H_

#include "common/angleutils.h"
#include "libANGLE/Stream.h"

namespace rx
{

class StreamProducerImpl : angle::NonCopyable
{
  public:
    explicit StreamProducerImpl() {}
    virtual ~StreamProducerImpl() {}

    // Constructs a frame from an arbitrary external pointer that points to producer specific frame data. Validation on the frame will be performed if needed and an error will be returned if the frame cannot be accepted. If validation succeeds, the current internal frame will be replaced with the new frame. 
    virtual egl::Error produceFrameFromPointer(void *pointer, const egl::AttributeMap &attributes) = 0;

    // Returns an OpenGL texture interpretation of some frame attributes for the purpose of constructing an OpenGL texture from a frame. Depending on the producer and consumer, some frames may have multiple "planes" with different OpenGL texture representations.
    virtual egl::Stream::GLTextureDescription getGLFrameDescription(int planeIndex) = 0;
};
}  // namespace rx

#endif  // LIBANGLE_RENDERER_STREAMPRODUCERIMPL_H_
