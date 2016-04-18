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

#include "common/angleutils.h"
#include "libANGLE/AttributeMap.h"

namespace rx
{
class StreamImpl;
}

namespace gl
{
class Context;
class Texture;
}

namespace egl
{
class Error;

class Stream final : angle::NonCopyable
{
  public:
    Stream(rx::StreamImpl *impl, const AttributeMap &attribs);
    ~Stream();

    enum ConsumerType
    {
        NoConsumer,
        GLTextureRGB,
        GLTextureYUV,
    };

    enum ProducerType
    {
        NoProducer,
        D3D11TextureNV12,
    };

    EGLenum getState() const;

    void setConsumerLatency(EGLint latency);
    EGLint getConsumerLatency() const;

    EGLuint64KHR getProducerFrame() const;
    EGLuint64KHR getConsumerFrame() const;

    void setConsumerAcquireTimeout(EGLint timeout);
    EGLint getConsumerAcquireTimeout() const;

    ConsumerType getConsumerType() const;
    ProducerType getProducerType() const;

    EGLint getPlaneCount() const;

    // Consumer creation methods
    Error createConsumerGLTextureExternal(const AttributeMap &attributes, gl::Context *context);

    // Producer creation methods
    Error createProducerD3D11TextureNV12(const AttributeMap &attributes);

    // Consumer methods
    Error consumerAcquire();
    Error consumerRelease();

    // Producer methods
    Error postD3D11NV12Texture(void *texture, const AttributeMap &attributes);

  private:
    // Implementation
    rx::StreamImpl *mImplementation;

    // Associated GL context
    gl::Context *mContext;

    // EGL defined attributes
    EGLint mState;
    EGLuint64KHR mProducerFrame;
    EGLuint64KHR mConsumerFrame;
    EGLint mConsumerLatency;

    // EGL gltexture consumer attributes
    EGLint mConsumerAcquireTimeout;

    // EGL gltexture yuv consumer attributes
    EGLint mPlaneCount;
    EGLint mPlaneTextureUnits[3];

    // Pointers to the textures bound to the stream
    gl::Texture *mTextures[3];

    // Consumer and producer types
    ConsumerType mConsumerType;
    ProducerType mProducerType;

    // Pointer to the current ID3D11Texture in the stream
    void *mCurrentD3D11NV12Texture;

    // Subresource ID to reference in current ID3D11Texture
    EGLint mCurrentSubresourceID;

    // ANGLE-only method, used internally
    friend class gl::Texture;
    void releaseTextures();
};
}  // namespace egl

#endif  // LIBANGLE_STREAM_H_
