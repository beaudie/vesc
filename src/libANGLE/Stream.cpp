//
// Copyright (c) 2016 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//

// Stream.cpp: Implements the egl::Stream class, representing the stream
// where frames are streamed in. Implements EGLStreanKHR.

#include "libANGLE/Stream.h"

#include <platform/Platform.h>
#include <EGL/eglext.h>

#include "common/debug.h"
#include "common/mathutil.h"
#include "common/platform.h"
#include "common/utilities.h"
#include "libANGLE/Context.h"
#include "libANGLE/Display.h"
#include "libANGLE/renderer/DisplayImpl.h"
#include "libANGLE/renderer/StreamProducerImpl.h"

namespace egl
{

Stream::Stream(Display *display, const AttributeMap &attribs)
    : mDisplay(display),
      mProducerImplementation(nullptr),
      mState(EGL_STREAM_STATE_CREATED_KHR),
      mProducerFrame(0),
      mConsumerFrame(0),
      mConsumerLatency(attribs.getAsInt(EGL_CONSUMER_LATENCY_USEC_KHR, 0)),
      mConsumerAcquireTimeout(attribs.getAsInt(EGL_CONSUMER_ACQUIRE_TIMEOUT_USEC_KHR, 0)),
      mConsumerType(NoConsumer),
      mProducerType(NoProducer)
{
    for (auto &plane : mPlanes)
    {
        plane.textureUnit = -1;
        plane.texture     = nullptr;
    }
}

Stream::~Stream()
{
    SafeDelete(mProducerImplementation);
    for (auto &plane : mPlanes)
    {
        if (plane.texture != nullptr)
        {
            plane.texture->releaseStream();
        }
    }
}

void Stream::setConsumerLatency(EGLint latency)
{
    mConsumerLatency = latency;
}

EGLint Stream::getConsumerLatency() const
{
    return mConsumerLatency;
}

EGLuint64KHR Stream::getProducerFrame() const
{
    return mProducerFrame;
}

EGLuint64KHR Stream::getConsumerFrame() const
{
    return mConsumerFrame;
}

EGLenum Stream::getState() const
{
    return mState;
}

void Stream::setConsumerAcquireTimeout(EGLint timeout)
{
    mConsumerAcquireTimeout = timeout;
}

EGLint Stream::getConsumerAcquireTimeout() const
{
    return mConsumerAcquireTimeout;
}

Stream::ProducerType Stream::getProducerType() const
{
    return mProducerType;
}

Stream::ConsumerType Stream::getConsumerType() const
{
    return mConsumerType;
}

EGLint Stream::getPlaneCount() const
{
    return mPlaneCount;
}

rx::StreamProducerImpl *Stream::getImplementation()
{
    return mProducerImplementation;
}

Error Stream::createConsumerGLTextureExternal(const AttributeMap &attributes, gl::Context *context)
{
    ASSERT(mState == EGL_STREAM_STATE_CREATED_KHR);
    ASSERT(mConsumerType == NoConsumer);
    ASSERT(mProducerType == NoProducer);
    ASSERT(context != nullptr);

    // Ensure the context supports external textures before querying it for them
    if (!context->getExtensions().eglStreamConsumerExternal)
    {
        return Error(EGL_BAD_ACCESS, "EGL stream consumer external GL extension not enabled");
    }

    EGLenum bufferType = attributes.getAsInt(EGL_COLOR_BUFFER_TYPE, EGL_RGB_BUFFER);
    if (bufferType == EGL_RGB_BUFFER)
    {
        // Try and get the texture
        mPlanes[0].texture = context->getState().getTargetTexture(GL_TEXTURE_EXTERNAL_OES);
        if (mPlanes[0].texture == nullptr || mPlanes[0].texture->getId() == 0)
        {
            mPlanes[0].texture = nullptr;
            return Error(EGL_BAD_ACCESS, "No external texture bound");
        }
        mPlanes[0].texture->bindStream(this);
        mConsumerType = GLTextureRGB;
        mPlaneCount   = 1;
    }
    else
    {
        mPlaneCount = attributes.getAsInt(EGL_YUV_NUMBER_OF_PLANES_EXT, 2);
        std::set<gl::Texture *> textureSet;
        for (int i = 0; i < mPlaneCount; i++)
        {
            // Fetch all the textures
            mPlanes[i].textureUnit = attributes.getAsInt(EGL_YUV_PLANE0_TEXTURE_UNIT_NV + i, -1);
            if (mPlanes[i].textureUnit != EGL_NONE)
            {
                mPlanes[i].texture = context->getState().getSamplerTexture(mPlanes[i].textureUnit,
                                                                           GL_TEXTURE_EXTERNAL_OES);
                if (mPlanes[i].texture == nullptr || mPlanes[i].texture == context->getTexture(0))
                {
                    for (int j = 0; j < 3; j++)
                    {
                        mPlanes[j].texture = nullptr;
                    }
                    return Error(
                        EGL_BAD_ACCESS,
                        "No external texture bound at one or more specified texture units");
                }
                if (textureSet.find(mPlanes[i].texture) != textureSet.end() ||
                    mPlanes[i].texture->getId() == 0)
                {
                    for (int j = 0; j < 3; j++)
                    {
                        mPlanes[j].texture = nullptr;
                    }
                    return Error(EGL_BAD_ACCESS,
                                 "Multiple planar surfaces bound to same texture object");
                }
                textureSet.insert(mPlanes[i].texture);
            }
        }

        // Bind them to the stream if there is no error
        for (int i = 0; i < mPlaneCount; i++)
        {
            if (mPlanes[i].textureUnit != EGL_NONE)
            {
                mPlanes[i].texture->bindStream(this);
            }
        }
        mConsumerType = GLTextureYUV;
    }

    mContext = context;
    mState   = EGL_STREAM_STATE_CONNECTING_KHR;

    return Error(EGL_SUCCESS);
}

Error Stream::createProducerD3D11TextureNV12(const AttributeMap &attributes)
{
    ASSERT(mState == EGL_STREAM_STATE_CONNECTING_KHR);
    ASSERT(mConsumerType == GLTextureYUV);
    ASSERT(mProducerType == NoConsumer);
    ASSERT(mPlaneCount == 2);

    mProducerImplementation = mDisplay->getImplementation()->createStreamProducer(
        D3D11TextureNV12, mConsumerType, attributes);
    if (mProducerImplementation == nullptr)
    {
        return Error(EGL_BAD_ALLOC, "Failed to create producer");
    }
    mProducerType = D3D11TextureNV12;
    mState        = EGL_STREAM_STATE_EMPTY_KHR;

    return Error(EGL_SUCCESS);
}

// Called when the consumer of this stream starts using the stream
Error Stream::consumerAcquire()
{
    ASSERT(mState == EGL_STREAM_STATE_NEW_FRAME_AVAILABLE_KHR ||
           mState == EGL_STREAM_STATE_OLD_FRAME_AVAILABLE_KHR);
    ASSERT(mConsumerType == GLTextureRGB || mConsumerType == GLTextureYUV);
    ASSERT(mProducerType == D3D11TextureNV12);

    mState = EGL_STREAM_STATE_OLD_FRAME_AVAILABLE_KHR;
    mConsumerFrame++;

    // Bind the D3D texture to the gl textures
    mPlanes[0].texture->acquireImageFromStream(mProducerImplementation->getGLFrameDescription(0));
    mPlanes[1].texture->acquireImageFromStream(mProducerImplementation->getGLFrameDescription(1));

    return Error(EGL_SUCCESS);
}

Error Stream::consumerRelease()
{
    ASSERT(mState == EGL_STREAM_STATE_NEW_FRAME_AVAILABLE_KHR ||
           mState == EGL_STREAM_STATE_OLD_FRAME_AVAILABLE_KHR);
    ASSERT(mConsumerType == GLTextureRGB || mConsumerType == GLTextureYUV);
    ASSERT(mProducerType == D3D11TextureNV12);

    // Release the images
    mPlanes[0].texture->releaseImageFromStream();
    mPlanes[1].texture->releaseImageFromStream();

    return Error(EGL_SUCCESS);
}

Error Stream::postD3D11NV12Texture(void *texture, const AttributeMap &attributes)
{
    ASSERT(mConsumerType == GLTextureRGB || mConsumerType == GLTextureYUV);
    ASSERT(mProducerType == D3D11TextureNV12);

    mProducerImplementation->produceFrameFromPointer(texture, attributes);
    mProducerFrame++;

    mState = EGL_STREAM_STATE_NEW_FRAME_AVAILABLE_KHR;

    return Error(EGL_SUCCESS);
}

// This is called when a texture object associated with this stream is destroyed. Even if multiple
// textures are bound, one being destroyed invalidates the stream, so all the remaining textures
// will be released and the stream will be invalidated.
void Stream::releaseTextures()
{
    for (int i = 0; i < 3; i++)
    {
        if (mPlanes[i].texture != nullptr)
        {
            mPlanes[i].texture->releaseStream();
            mPlanes[i].texture = nullptr;
        }
    }
    mProducerType = NoProducer;
    mConsumerType = NoConsumer;
    mState        = EGL_STREAM_STATE_DISCONNECTED_KHR;
}

}  // namespace egl
