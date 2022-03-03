//
// Copyright 2014 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//

// DisplayImpl.cpp: Implementation methods of egl::Display

#include "libANGLE/renderer/DisplayImpl.h"

#include "libANGLE/Display.h"
#include "libANGLE/Surface.h"
#include "libANGLE/renderer/DeviceImpl.h"

namespace rx
{
namespace
{
// For back-ends that do not implement EGLDevice.
class MockDevice : public DeviceImpl
{
  public:
    MockDevice() = default;
    egl::Error initialize() override { return egl::NoError(); }
    egl::Error getAttribute(const egl::Display *display, EGLint attribute, void **outValue) override
    {
        UNREACHABLE();
        return egl::EglBadAttribute();
    }
    EGLint getType() override
    {
        UNREACHABLE();
        return EGL_NONE;
    }
    void generateExtensions(egl::DeviceExtensions *outExtensions) const override
    {
        *outExtensions = egl::DeviceExtensions();
    }
};
}  // anonymous namespace

DisplayImpl::DisplayImpl(const egl::DisplayState &state)
    : mState(state), mExtensionsInitialized(false), mCapsInitialized(false), mBlobCache(nullptr)
{}

DisplayImpl::~DisplayImpl()
{
    ASSERT(mState.surfaceSet.empty());
}

egl::Error DisplayImpl::prepareForCall()
{
    return egl::NoError();
}

egl::Error DisplayImpl::releaseThread()
{
    return egl::NoError();
}

const egl::DisplayExtensions &DisplayImpl::getExtensions() const
{
    if (!mExtensionsInitialized)
    {
        generateExtensions(&mExtensions);
        mExtensionsInitialized = true;
    }

    return mExtensions;
}

egl::Error DisplayImpl::handleGPUSwitch()
{
    return egl::NoError();
}

egl::Error DisplayImpl::validateClientBuffer(const egl::Config *configuration,
                                             EGLenum buftype,
                                             EGLClientBuffer clientBuffer,
                                             const egl::AttributeMap &attribs) const
{
    UNREACHABLE();
    return egl::EglBadDisplay() << "DisplayImpl::validateClientBuffer unimplemented.";
}

egl::Error DisplayImpl::validateImageClientBuffer(const gl::Context *context,
                                                  EGLenum target,
                                                  EGLClientBuffer clientBuffer,
                                                  const egl::AttributeMap &attribs) const
{
    UNREACHABLE();
    return egl::EglBadDisplay() << "DisplayImpl::validateImageClientBuffer unimplemented.";
}

egl::Error DisplayImpl::validatePixmap(const egl::Config *config,
                                       EGLNativePixmapType pixmap,
                                       const egl::AttributeMap &attributes) const
{
    UNREACHABLE();
    return egl::EglBadDisplay() << "DisplayImpl::valdiatePixmap unimplemented.";
}

const egl::Caps &DisplayImpl::getCaps() const
{
    if (!mCapsInitialized)
    {
        generateCaps(&mCaps);
        mCapsInitialized = true;
    }

    return mCaps;
}

gl::Version DisplayImpl::getMaxSupportedESVersion() const
{
    gl::Version version = getMaxSupportedESVersionImpl();

    // if the backend reports the largest possible version, don't bother
    // doing further checks
    if (version.major == 3 && version.minor == 2)
    {
        return version;
    }

    const gl::Extensions &extensions = getNativeExtensions();

    if (version.major == 3 && version.minor >= 1 && extensions.gpuShader5EXT &&
        extensions.blendEquationAdvancedKHR && extensions.colorBufferFloatEXT &&
        extensions.copyImageOES && extensions.debugKHR && extensions.drawBuffersIndexedOES &&
        extensions.drawElementsBaseVertexOES && extensions.geometryShaderOES &&
        extensions.primitiveBoundingBoxOES && extensions.robustnessEXT &&
        extensions.sampleShadingOES && extensions.sampleVariablesOES &&
        extensions.shaderMultisampleInterpolationOES && extensions.shaderImageAtomicOES &&
        extensions.tessellationShaderEXT && extensions.shaderIoBlocksAny() &&
        extensions.textureBorderClampOES && extensions.textureBufferOES &&
        extensions.textureCompressionAstcHdrKHR && extensions.textureCubeMapArrayOES &&
        extensions.textureStencil8OES && extensions.textureStorageMultisample2dArrayOES)
        version.minor = 2;

    return version;
}

DeviceImpl *DisplayImpl::createDevice()
{
    return new MockDevice();
}

bool DisplayImpl::isX11() const
{
    return false;
}

}  // namespace rx
