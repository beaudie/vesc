//
// Copyright (c) 2016 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//

// DisplayDRM.h: DRM implementation of egl::Display

#ifndef LIBANGLE_RENDERER_GL_DRM_DISPLAYDRM_H_
#define LIBANGLE_RENDERER_GL_DRM_DISPLAYDRM_H_

#include <xf86drm.h>
#include <xf86drmMode.h>

#include <string>
#include <vector>

#include "libANGLE/renderer/gl/DisplayGL.h"
#include "libANGLE/renderer/gl/drm/FunctionsDRM.h"

struct gbm_device;

namespace rx
{

class FunctionsDRM;

// State-tracking data for the swap control to allow DisplayDRM to remember per
// drawable information for swap control.
struct SwapControlData
{
    SwapControlData();

    // Set by the drawable
    int targetSwapInterval;

    // DisplayDRM-side state-tracking
    int maxSwapInterval;
    int currentSwapInterval;
};

class DisplayDRM : public DisplayGL
{
  public:
    DisplayDRM();
    ~DisplayDRM() override;

    egl::Error initialize(egl::Display *display) override;
    void terminate() override;

    SurfaceImpl *createWindowSurface(const egl::Config *configuration,
                                     EGLNativeWindowType window,
                                     const egl::AttributeMap &attribs) override;
    SurfaceImpl *createPbufferSurface(const egl::Config *configuration,
                                      const egl::AttributeMap &attribs) override;
    SurfaceImpl *createPbufferFromClientBuffer(const egl::Config *configuration,
                                               EGLClientBuffer shareHandle,
                                               const egl::AttributeMap &attribs) override;
    SurfaceImpl *createPixmapSurface(const egl::Config *configuration,
                                     NativePixmapType nativePixmap,
                                     const egl::AttributeMap &attribs) override;

    egl::ConfigSet generateConfigs() const override;

    bool isDeviceLost() const override;
    bool testDeviceLost() override;
    egl::Error restoreLostDevice() override;

    bool isValidNativeWindow(EGLNativeWindowType window) const override;

    egl::Error getDevice(DeviceImpl **device) override;

    std::string getVendorString() const override;

    // Depending on the supported DRM extension, swap interval can be set
    // globally or per drawable. This function will make sure the drawable's
    // swap interval is the one required so that the subsequent swapBuffers
    // acts as expected.
    void setSwapInterval(EGLSurface drawable, SwapControlData *data);

  private:
    const FunctionsGL *getFunctionsGL() const override;

    EGLContext initializeContext(EGLConfig config, const egl::AttributeMap &eglAttributes);

    void generateExtensions(egl::DisplayExtensions *outExtensions) const override;
    void generateCaps(egl::Caps *outCaps) const override;

    int getDRMFBConfigAttrib(EGLConfig config, int attrib) const;
    EGLContext createContextAttribs(EGLConfig config, const std::vector<int> &attribs) const;

    FunctionsGL *mFunctionsGL;

    //TODO(cwallez) yuck, change generateConfigs to be non-const or add a userdata member to egl::Config?
    mutable std::map<int, EGLConfig> configIdToConfig;

    EGLConfig mContextConfig;
    EGLContext mContext;
    // A pbuffer the context is current on during ANGLE initialization
    EGLSurface mDummyPbuffer;

    bool mIsMesa;
    bool mHasMultisample;
    bool mHasARBCreateContext;

    enum class SwapControl
    {
        Absent,
        EXT,
        Mesa,
        SGI,
    };
    SwapControl mSwapControl;
    int mMinSwapInterval;
    int mMaxSwapInterval;
    int mCurrentSwapInterval;

    egl::Display *mEGLDisplay;

    FunctionsDRM mDRM;
    gbm_device *mGBM;
    drmModeConnectorPtr mConnector;
    drmModeModeInfoPtr mMode;
    drmModeCrtcPtr mCRTC;

    int32_t mWidth;
    int32_t mHeight;
};

}

#endif // LIBANGLE_RENDERER_GL_DRM_DISPLAYDRM_H_
