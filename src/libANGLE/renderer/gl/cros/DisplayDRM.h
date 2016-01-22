//
// Copyright (c) 2016 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//

// DisplayDRM.h: DRM implementation of egl::Display

#ifndef LIBANGLE_RENDERER_GL_CROS_DISPLAYDRM_H_
#define LIBANGLE_RENDERER_GL_CROS_DISPLAYDRM_H_

#include <xf86drm.h>
#include <xf86drmMode.h>

#include <string>
#include <vector>

#include "libANGLE/renderer/gl/DisplayGL.h"
#include "libANGLE/renderer/gl/cros/FunctionsEGL.h"

struct gbm_device;
struct gbm_bo;

namespace rx
{

class FramebufferGL;

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
    struct Buffer
    {
        Buffer(DisplayDRM *display);
        ~Buffer();
        bool initialize(int width, int height);
        FramebufferGL *framebufferGL(const gl::Framebuffer::Data &data);
        bool present();
        uint32_t getDRMFB();

        int mWidth, mHeight;
        DisplayDRM *mDisplay;
        gbm_bo *mBO;
        bool mHasDRMFB;
        uint32_t mDRMFB;
        GLuint mGLFB;

        int x, y, dx, dy;
    };

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

    egl::Error waitClient() const override;
    egl::Error waitNative(EGLint engine) const override;

    // Swap interval can be set globally or per drawable.
    // This function will make sure the drawable's swap interval is the
    // one required so that the subsequent swapBuffers acts as expected.
    void setSwapInterval(EGLSurface drawable, SwapControlData *data);

    egl::Error present(GLuint fb, GLint width, GLint height);

  private:
    const FunctionsGL *getFunctionsGL() const override;

    EGLContext initializeContext(EGLConfig config, const egl::AttributeMap &eglAttributes);

    void generateExtensions(egl::DisplayExtensions *outExtensions) const override;
    void generateCaps(egl::Caps *outCaps) const override;

    int getDRMFBConfigAttrib(EGLConfig config, int attrib) const;
    EGLContext createContextAttribs(EGLConfig config, const std::vector<int> &attribs) const;

    static void pageFlipHandler(int fd, unsigned int sequence, unsigned int tv_sec, unsigned int tv_usec, void *data);
    bool present(Buffer *buffer);

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

    FunctionsEGL mEGL;
    gbm_device *mGBM;
    drmModeConnectorPtr mConnector;
    drmModeModeInfoPtr mMode;
    drmModeCrtcPtr mCRTC;
    bool mSetCRTC;
    bool mFlipPending;

    int32_t mWidth;
    int32_t mHeight;

    Buffer mScanout[2];
    int mBack;
};

}

#endif // LIBANGLE_RENDERER_GL_CROS_DISPLAYDRM_H_
