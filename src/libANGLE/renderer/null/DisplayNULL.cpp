//
// Copyright 2016 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//
// DisplayNULL.cpp:
//    Implements the class methods for DisplayNULL.
//

#include "libANGLE/renderer/null/DisplayNULL.h"

#include "common/debug.h"

#include "libANGLE/renderer/null/ContextNULL.h"
#include "libANGLE/renderer/null/DeviceNULL.h"
#include "libANGLE/renderer/null/ImageNULL.h"
#include "libANGLE/renderer/null/SurfaceNULL.h"

namespace rx
{

DisplayNULL::DisplayNULL() : DisplayImpl()
{
}

DisplayNULL::~DisplayNULL()
{
}

egl::Error DisplayNULL::initialize(egl::Display *display)
{
    mDevice = new DeviceNULL();
    return egl::Success();
}

void DisplayNULL::terminate()
{
    SafeDelete(mDevice);
}

egl::Error DisplayNULL::makeCurrent(egl::Surface *drawSurface,
                                    egl::Surface *readSurface,
                                    gl::Context *context)
{
    return egl::Success();
}

egl::ConfigSet DisplayNULL::generateConfigs()
{
    UNIMPLEMENTED();
    return egl::ConfigSet();
}

bool DisplayNULL::testDeviceLost()
{
    return false;
}

egl::Error DisplayNULL::restoreLostDevice()
{
    return egl::Success();
}

bool DisplayNULL::isValidNativeWindow(EGLNativeWindowType window) const
{
    return true;
}

std::string DisplayNULL::getVendorString() const
{
    return "NULL";
}

egl::Error DisplayNULL::getDevice(DeviceImpl **device)
{
    *device = mDevice;
    return egl::Success();
}

egl::Error DisplayNULL::waitClient() const
{
    return egl::Success();
}

egl::Error DisplayNULL::waitNative(EGLint engine,
                                   egl::Surface *drawSurface,
                                   egl::Surface *readSurface) const
{
    return egl::Success();
}

gl::Version DisplayNULL::getMaxSupportedESVersion() const
{
    return gl::Version(3, 2);
}

SurfaceImpl *DisplayNULL::createWindowSurface(const egl::SurfaceState &state,
                                              const egl::Config *configuration,
                                              EGLNativeWindowType window,
                                              const egl::AttributeMap &attribs)
{
    return new SurfaceNULL(state);
}

SurfaceImpl *DisplayNULL::createPbufferSurface(const egl::SurfaceState &state,
                                               const egl::Config *configuration,
                                               const egl::AttributeMap &attribs)
{
    return new SurfaceNULL(state);
}

SurfaceImpl *DisplayNULL::createPbufferFromClientBuffer(const egl::SurfaceState &state,
                                                        const egl::Config *configuration,
                                                        EGLenum buftype,
                                                        EGLClientBuffer buffer,
                                                        const egl::AttributeMap &attribs)
{
    return new SurfaceNULL(state);
}

SurfaceImpl *DisplayNULL::createPixmapSurface(const egl::SurfaceState &state,
                                              const egl::Config *configuration,
                                              NativePixmapType nativePixmap,
                                              const egl::AttributeMap &attribs)
{
    return new SurfaceNULL(state);
}

ImageImpl *DisplayNULL::createImage(EGLenum target,
                                    egl::ImageSibling *buffer,
                                    const egl::AttributeMap &attribs)
{
    return new ImageNULL();
}

ContextImpl *DisplayNULL::createContext(const gl::ContextState &state)
{
    return new ContextNULL(state);
}

StreamProducerImpl *DisplayNULL::createStreamProducerD3DTextureNV12(
    egl::Stream::ConsumerType consumerType,
    const egl::AttributeMap &attribs)
{
    UNIMPLEMENTED();
    return nullptr;
}

void DisplayNULL::generateExtensions(egl::DisplayExtensions *outExtensions) const
{
    outExtensions->createContextRobustness            = true;
    outExtensions->d3dShareHandleClientBuffer         = false;
    outExtensions->d3dTextureClientBuffer             = false;
    outExtensions->surfaceD3DTexture2DShareHandle     = false;
    outExtensions->querySurfacePointer                = false;
    outExtensions->windowFixedSize                    = false;
    outExtensions->keyedMutex                         = false;
    outExtensions->surfaceOrientation                 = false;
    outExtensions->postSubBuffer                      = true;
    outExtensions->createContext                      = true;
    outExtensions->deviceQuery                        = true;
    outExtensions->image                              = true;
    outExtensions->imageBase                          = true;
    outExtensions->imagePixmap                        = false;
    outExtensions->glTexture2DImage                   = true;
    outExtensions->glTextureCubemapImage              = true;
    outExtensions->glTexture3DImage                   = true;
    outExtensions->glRenderbufferImage                = true;
    outExtensions->getAllProcAddresses                = true;
    outExtensions->flexibleSurfaceCompatibility       = true;
    outExtensions->directComposition                  = true;
    outExtensions->createContextNoError               = true;
    outExtensions->stream                             = false;
    outExtensions->streamConsumerGLTexture            = false;
    outExtensions->streamConsumerGLTextureYUV         = false;
    outExtensions->streamProducerD3DTextureNV12       = false;
    outExtensions->createContextWebGLCompatibility    = true;
    outExtensions->createContextBindGeneratesResource = true;
    outExtensions->getSyncValues                      = false;
    outExtensions->swapBuffersWithDamage              = true;
}

void DisplayNULL::generateCaps(egl::Caps *outCaps) const
{
    outCaps->textureNPOT = true;
}

}  // namespace rx
