//
// Copyright (c) 2015 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//

// DisplayDRM.cpp: DRM implementation of egl::Display

#include "libANGLE/renderer/gl/drm/DisplayDRM.h"

#include <EGL/eglext.h>
#include <algorithm>

#include "common/debug.h"
#include "libANGLE/Config.h"
#include "libANGLE/Display.h"
#include "libANGLE/Surface.h"
#include "libANGLE/renderer/gl/drm/PbufferSurfaceDRM.h"
#include "libANGLE/renderer/gl/drm/WindowSurfaceDRM.h"

#define EGL_NO_CONFIG_MESA ((EGLConfig)0)

namespace rx
{

SwapControlData::SwapControlData()
  : targetSwapInterval(0),
    maxSwapInterval(-1),
    currentSwapInterval(-1)
{
}

class FunctionsGLDRM : public FunctionsGL
{
  public:
    FunctionsGLDRM(FunctionsDRM::GETPROC getProc)
      : mGetProc(getProc)
    {
    }

    virtual ~FunctionsGLDRM()
    {
    }

  private:
    void *loadProcAddress(const std::string &function) override
    {
        return reinterpret_cast<void*>(mGetProc(function.c_str()));
    }

    FunctionsDRM::GETPROC mGetProc;
};

DisplayDRM::DisplayDRM()
    : DisplayGL(),
      mFunctionsGL(nullptr),
      mRequestedVisual(-1),
      mContextConfig(nullptr),
      mContext(nullptr),
      mDummyPbuffer(0),
      mUsesNewXDisplay(false),
      mIsMesa(false),
      mHasMultisample(false),
      mHasARBCreateContext(false),
      mSwapControl(SwapControl::Absent),
      mMinSwapInterval(0),
      mMaxSwapInterval(0),
      mCurrentSwapInterval(-1),
      mEGLDisplay(nullptr)
{
}

DisplayDRM::~DisplayDRM()
{
}

egl::Error DisplayDRM::initialize(egl::Display *display)
{
    mEGLDisplay = display;
    EGLNativeDisplayType nativeDisplay = display->getNativeDisplayId();
    const auto &attribMap = display->getAttributeMap();

    std::string drmInitError;
    if (!mDRM.initialize(nativeDisplay, &drmInitError))
    {
        return egl::Error(EGL_NOT_INITIALIZED, drmInitError.c_str());
    }

    mHasMultisample      = mDRM.minorVersion > 3 || mDRM.hasExtension("DRM_ARB_multisample");
    mHasARBCreateContext = mDRM.hasExtension("DRM_ARB_create_context");

#ifdef XXX
    if (attribMap.contains(EGL_X11_VISUAL_ID_ANGLE))
    {
        mRequestedVisual = attribMap.get(EGL_X11_VISUAL_ID_ANGLE, -1);

        // There is no direct way to get the DRMFBConfig matching an X11 visual ID
        // so we have to iterate over all the DRMFBConfigs to find the right one.
        int nConfigs;
        int attribList[] = {
            None,
        };
        drm::FBConfig *allConfigs = mDRM.chooseFBConfig(attribList, &nConfigs);

        for (int i = 0; i < nConfigs; ++i)
        {
            if (getDRMFBConfigAttrib(allConfigs[i], DRM_VISUAL_ID) == mRequestedVisual)
            {
                mContextConfig = allConfigs[i];
                break;
            }
        }
        XFree(allConfigs);

        if (mContextConfig == nullptr)
        {
            return egl::Error(EGL_NOT_INITIALIZED, "Invalid visual ID requested.");
        }
    }
    else
    {
        // When glXMakeCurrent is called, the context and the surface must be
        // compatible which in glX-speak means that their config have the same
        // color buffer type, are both RGBA or ColorIndex, and their buffers have
        // the same depth, if they exist.
        // Since our whole EGL implementation is backed by only one GL context, this
        // context must be compatible with all the DRMFBConfig corresponding to the
        // EGLconfigs that we will be exposing.
        int nConfigs;
        int attribList[] =
        {
            // We want RGBA8 and DEPTH24_STENCIL8
            DRM_RED_SIZE, 8,
            DRM_GREEN_SIZE, 8,
            DRM_BLUE_SIZE, 8,
            DRM_ALPHA_SIZE, 8,
            DRM_DEPTH_SIZE, 24,
            DRM_STENCIL_SIZE, 8,
            // We want RGBA rendering (vs COLOR_INDEX) and doublebuffer
            DRM_RENDER_TYPE, DRM_RGBA_BIT,
            // Double buffer is not strictly required as a non-doublebuffer
            // context can work with a doublebuffered surface, but it still
            // flickers and all applications want doublebuffer anyway.
            DRM_DOUBLEBUFFER, True,
            // All of these must be supported for full EGL support
            DRM_DRAWABLE_TYPE, DRM_WINDOW_BIT | DRM_PBUFFER_BIT | DRM_PIXMAP_BIT,
            // This makes sure the config have an associated visual Id
            DRM_X_RENDERABLE, True,
            DRM_CONFIG_CAVEAT, DRM_NONE,
            None
        };
        drm::FBConfig *candidates = mDRM.chooseFBConfig(attribList, &nConfigs);
        if (nConfigs == 0)
        {
            XFree(candidates);
            return egl::Error(EGL_NOT_INITIALIZED, "Could not find a decent DRM FBConfig to create the context.");
        }
        mContextConfig = candidates[0];
        XFree(candidates);
    }

    if (mHasARBCreateContext)
    {
        mContext = initializeContext(nullptr, display->getAttributeMap());
    }
    else
    {
        XVisualInfo visualTemplate;
        visualTemplate.visualid = getDRMFBConfigAttrib(mContextConfig, DRM_VISUAL_ID);

        int numVisuals       = 0;
        XVisualInfo *visuals = XGetVisualInfo(xDisplay, VisualIDMask, &visualTemplate, &numVisuals);
        if (numVisuals <= 0)
        {
            return egl::Error(EGL_NOT_INITIALIZED,
                              "Could not get the visual info from the fb config");
        }
        ASSERT(numVisuals == 1);

        mContext = mDRM.createContext(&visuals[0], nullptr, true);
        XFree(visuals);
    }
#endif
    mContext = initializeContext(nullptr, attribMap);
    if (!mContext)
    {
        return egl::Error(EGL_NOT_INITIALIZED, "Could not create GLES context.");
    }

#ifdef XXX
    // FunctionsGL and DisplayGL need to make a few GL calls, for example to
    // query the version of the context so we need to make the context current.
    // glXMakeCurrent requires a DRMDrawable so we create a temporary Pbuffer
    // (of size 1, 1) for the duration of these calls.
    // Ideally we would want to unset the current context and destroy the pbuffer
    // before going back to the application but this is TODO
    // We could use a pbuffer of size (0, 0) but it fails on the Intel Mesa driver
    // as commented on https://bugs.freedesktop.org/show_bug.cgi?id=38869 so we
    // use (1, 1) instead.

    int dummyPbufferAttribs[] =
    {
        DRM_PBUFFER_WIDTH, 1,
        DRM_PBUFFER_HEIGHT, 1,
        None,
    };
    mDummyPbuffer = mDRM.createPbuffer(mContextConfig, dummyPbufferAttribs);
    if (!mDummyPbuffer)
    {
        return egl::Error(EGL_NOT_INITIALIZED, "Could not create the dummy pbuffer.");
    }
#endif
    if (!mDRM.makeCurrent(EGL_NO_SURFACE, mContext))
    {
        return egl::Error(EGL_NOT_INITIALIZED, "Could not make context current.");
    }

    mFunctionsGL = new FunctionsGLDRM(mDRM.getProc);
    mFunctionsGL->initialize();

    std::string rendererString =
        reinterpret_cast<const char*>(mFunctionsGL->getString(GL_RENDERER));
    mIsMesa = rendererString.find("Mesa") != std::string::npos;

    return DisplayGL::initialize(display);
}

void DisplayDRM::terminate()
{
    DisplayGL::terminate();

    if (mDummyPbuffer)
    {
        //XXX mDRM.destroyPbuffer(mDummyPbuffer);
        mDummyPbuffer = 0;
    }

    if (mContext)
    {
        mDRM.destroyContext(mContext);
        mContext = nullptr;
    }

    mDRM.terminate();

    SafeDelete(mFunctionsGL);
}

SurfaceImpl *DisplayDRM::createWindowSurface(const egl::Config *configuration,
                                             EGLNativeWindowType window,
                                             const egl::AttributeMap &attribs)
{
    ASSERT(configIdToDRMConfig.count(configuration->configID) > 0);
    drm::FBConfig fbConfig = configIdToDRMConfig[configuration->configID];

    return new WindowSurfaceDRM(mDRM, this, this->getRenderer(), window, mDRM.getDisplay(),
                                mContext, fbConfig);
}

SurfaceImpl *DisplayDRM::createPbufferSurface(const egl::Config *configuration,
                                              const egl::AttributeMap &attribs)
{
    ASSERT(configIdToDRMConfig.count(configuration->configID) > 0);
    drm::FBConfig fbConfig = configIdToDRMConfig[configuration->configID];

    EGLint width = attribs.get(EGL_WIDTH, 0);
    EGLint height = attribs.get(EGL_HEIGHT, 0);
    bool largest = (attribs.get(EGL_LARGEST_PBUFFER, EGL_FALSE) == EGL_TRUE);

    return new PbufferSurfaceDRM(this->getRenderer(), width, height, largest, mDRM, mContext,
                                 fbConfig);
}

SurfaceImpl* DisplayDRM::createPbufferFromClientBuffer(const egl::Config *configuration,
                                                       EGLClientBuffer shareHandle,
                                                       const egl::AttributeMap &attribs)
{
    UNIMPLEMENTED();
    return nullptr;
}

SurfaceImpl *DisplayDRM::createPixmapSurface(const egl::Config *configuration,
                                             NativePixmapType nativePixmap,
                                             const egl::AttributeMap &attribs)
{
    UNIMPLEMENTED();
    return nullptr;
}

egl::Error DisplayDRM::getDevice(DeviceImpl **device)
{
    UNIMPLEMENTED();
    return egl::Error(EGL_BAD_DISPLAY);
}

drm::Context DisplayDRM::initializeContext(drm::FBConfig config,
                                           const egl::AttributeMap &eglAttributes)
{
    // Create a context of the requested version, if any.
    EGLint requestedMajorVersion =
        eglAttributes.get(EGL_PLATFORM_ANGLE_MAX_VERSION_MAJOR_ANGLE, EGL_DONT_CARE);
    EGLint requestedMinorVersion =
        eglAttributes.get(EGL_PLATFORM_ANGLE_MAX_VERSION_MINOR_ANGLE, EGL_DONT_CARE);
    if (requestedMajorVersion != EGL_DONT_CARE && requestedMinorVersion != EGL_DONT_CARE)
    {
        std::vector<int> contextAttributes;
        contextAttributes.push_back(EGL_CONTEXT_MAJOR_VERSION_KHR);
        contextAttributes.push_back(requestedMajorVersion);

        contextAttributes.push_back(EGL_CONTEXT_MINOR_VERSION_KHR);
        contextAttributes.push_back(requestedMinorVersion);

        contextAttributes.push_back(EGL_NONE);
        return mDRM.createContext(config, EGL_NO_CONTEXT, contextAttributes.data());
    }

    //XXX It is commonly assumed that glXCreateContextAttrib will create a context
    // of the highest version possible but it is not specified in the spec and
    // is not true on the Mesa drivers. Instead we try to create a context per
    // desktop GL version until we succeed, starting from newer version.
    // clang-format off
    const gl::Version esVersions[] = {
        gl::Version(2, 0),
    };
    // clang-format on

    for (size_t i = 0; i < ArraySize(esVersions); ++i)
    {
        const auto &version = esVersions[i];

        std::vector<int> contextAttributes;
        contextAttributes.push_back(EGL_CONTEXT_MAJOR_VERSION_KHR);
        contextAttributes.push_back(version.major);

        contextAttributes.push_back(EGL_CONTEXT_MINOR_VERSION_KHR);
        contextAttributes.push_back(version.minor);

        contextAttributes.push_back(EGL_NONE);
        auto context = mDRM.createContext(config, EGL_NO_CONTEXT, contextAttributes.data());

        if (context)
        {
            return context;
        }
    }

    return nullptr;
}

egl::ConfigSet DisplayDRM::generateConfigs() const
{
    egl::ConfigSet configs;
    configIdToDRMConfig.clear();

#ifdef XXX
    const gl::Version &maxVersion = getMaxSupportedESVersion();
    ASSERT(maxVersion >= gl::Version(2, 0));
    bool supportsES3 = maxVersion >= gl::Version(3, 0);

    int contextRedSize   = getDRMFBConfigAttrib(mContextConfig, DRM_RED_SIZE);
    int contextGreenSize = getDRMFBConfigAttrib(mContextConfig, DRM_GREEN_SIZE);
    int contextBlueSize  = getDRMFBConfigAttrib(mContextConfig, DRM_BLUE_SIZE);
    int contextAlphaSize = getDRMFBConfigAttrib(mContextConfig, DRM_ALPHA_SIZE);

    int contextDepthSize   = getDRMFBConfigAttrib(mContextConfig, DRM_DEPTH_SIZE);
    int contextStencilSize = getDRMFBConfigAttrib(mContextConfig, DRM_STENCIL_SIZE);

    int contextSamples = mHasMultisample ? getDRMFBConfigAttrib(mContextConfig, DRM_SAMPLES) : 0;
    int contextSampleBuffers =
        mHasMultisample ? getDRMFBConfigAttrib(mContextConfig, DRM_SAMPLE_BUFFERS) : 0;

    int contextAccumRedSize = getDRMFBConfigAttrib(mContextConfig, DRM_ACCUM_RED_SIZE);
    int contextAccumGreenSize = getDRMFBConfigAttrib(mContextConfig, DRM_ACCUM_GREEN_SIZE);
    int contextAccumBlueSize = getDRMFBConfigAttrib(mContextConfig, DRM_ACCUM_BLUE_SIZE);
    int contextAccumAlphaSize = getDRMFBConfigAttrib(mContextConfig, DRM_ACCUM_ALPHA_SIZE);

    int attribList[] =
    {
        DRM_RENDER_TYPE, DRM_RGBA_BIT,
        DRM_X_RENDERABLE, True,
        DRM_DOUBLEBUFFER, True,
        None,
    };

    int drmConfigCount;
    drm::FBConfig *drmConfigs = mDRM.chooseFBConfig(attribList, &drmConfigCount);

    for (int i = 0; i < drmConfigCount; i++)
    {
        drm::FBConfig drmConfig = drmConfigs[i];
        egl::Config config;

        // Native stuff
        config.nativeVisualID = getDRMFBConfigAttrib(drmConfig, DRM_VISUAL_ID);
        config.nativeVisualType = getDRMFBConfigAttrib(drmConfig, DRM_X_VISUAL_TYPE);
        config.nativeRenderable = EGL_TRUE;

        // When a visual ID has been specified with EGL_ANGLE_x11_visual we should
        // only return configs with this visual: it will maximize performance by avoid
        // blits in the driver when showing the window on the screen.
        if (mRequestedVisual != -1 && config.nativeVisualID != mRequestedVisual)
        {
            continue;
        }

        // Buffer sizes
        config.redSize = getDRMFBConfigAttrib(drmConfig, DRM_RED_SIZE);
        config.greenSize = getDRMFBConfigAttrib(drmConfig, DRM_GREEN_SIZE);
        config.blueSize = getDRMFBConfigAttrib(drmConfig, DRM_BLUE_SIZE);
        config.alphaSize = getDRMFBConfigAttrib(drmConfig, DRM_ALPHA_SIZE);
        config.depthSize = getDRMFBConfigAttrib(drmConfig, DRM_DEPTH_SIZE);
        config.stencilSize = getDRMFBConfigAttrib(drmConfig, DRM_STENCIL_SIZE);

        // We require RGBA8 and the D24S8 (or no DS buffer)
        if (config.redSize != contextRedSize || config.greenSize != contextGreenSize ||
            config.blueSize != contextBlueSize || config.alphaSize != contextAlphaSize)
        {
            continue;
        }
        // The DRM spec says that it is ok for a whole buffer to not be present
        // however the Mesa Intel driver (and probably on other Mesa drivers)
        // fails to make current when the Depth stencil doesn't exactly match the
        // configuration.
        bool hasSameDepthStencil =
            config.depthSize == contextDepthSize && config.stencilSize == contextStencilSize;
        bool hasNoDepthStencil = config.depthSize == 0 && config.stencilSize == 0;
        if (!hasSameDepthStencil && (mIsMesa || !hasNoDepthStencil))
        {
            continue;
        }

        config.colorBufferType = EGL_RGB_BUFFER;
        config.luminanceSize = 0;
        config.alphaMaskSize = 0;

        config.bufferSize = config.redSize + config.greenSize + config.blueSize + config.alphaSize;

        // Multisample and accumulation buffers
        int samples = mHasMultisample ? getDRMFBConfigAttrib(drmConfig, DRM_SAMPLES) : 0;
        int sampleBuffers =
            mHasMultisample ? getDRMFBConfigAttrib(drmConfig, DRM_SAMPLE_BUFFERS) : 0;

        int accumRedSize = getDRMFBConfigAttrib(drmConfig, DRM_ACCUM_RED_SIZE);
        int accumGreenSize = getDRMFBConfigAttrib(drmConfig, DRM_ACCUM_GREEN_SIZE);
        int accumBlueSize = getDRMFBConfigAttrib(drmConfig, DRM_ACCUM_BLUE_SIZE);
        int accumAlphaSize = getDRMFBConfigAttrib(drmConfig, DRM_ACCUM_ALPHA_SIZE);

        if (samples != contextSamples ||
            sampleBuffers != contextSampleBuffers ||
            accumRedSize != contextAccumRedSize ||
            accumGreenSize != contextAccumGreenSize ||
            accumBlueSize != contextAccumBlueSize ||
            accumAlphaSize != contextAccumAlphaSize)
        {
            continue;
        }

        config.samples = samples;
        config.sampleBuffers = sampleBuffers;

        // Transparency
        if (getDRMFBConfigAttrib(drmConfig, DRM_TRANSPARENT_TYPE) == DRM_TRANSPARENT_RGB)
        {
            config.transparentType = EGL_TRANSPARENT_RGB;
            config.transparentRedValue = getDRMFBConfigAttrib(drmConfig, DRM_TRANSPARENT_RED_VALUE);
            config.transparentGreenValue = getDRMFBConfigAttrib(drmConfig, DRM_TRANSPARENT_GREEN_VALUE);
            config.transparentBlueValue = getDRMFBConfigAttrib(drmConfig, DRM_TRANSPARENT_BLUE_VALUE);
        }
        else
        {
            config.transparentType = EGL_NONE;
        }

        // Pbuffer
        config.maxPBufferWidth = getDRMFBConfigAttrib(drmConfig, DRM_MAX_PBUFFER_WIDTH);
        config.maxPBufferHeight = getDRMFBConfigAttrib(drmConfig, DRM_MAX_PBUFFER_HEIGHT);
        config.maxPBufferPixels = getDRMFBConfigAttrib(drmConfig, DRM_MAX_PBUFFER_PIXELS);

        // Caveat
        config.configCaveat = EGL_NONE;

        int caveat = getDRMFBConfigAttrib(drmConfig, DRM_CONFIG_CAVEAT);
        if (caveat == DRM_SLOW_CONFIG)
        {
            config.configCaveat = EGL_SLOW_CONFIG;
        }
        else if (caveat == DRM_NON_CONFORMANT_CONFIG)
        {
            continue;
        }

        // Misc
        config.level = getDRMFBConfigAttrib(drmConfig, DRM_LEVEL);

        config.bindToTextureRGB = EGL_FALSE;
        config.bindToTextureRGBA = EGL_FALSE;

        int drmDrawable = getDRMFBConfigAttrib(drmConfig, DRM_DRAWABLE_TYPE);
        config.surfaceType = 0 |
            (drmDrawable & DRM_WINDOW_BIT ? EGL_WINDOW_BIT : 0) |
            (drmDrawable & DRM_PBUFFER_BIT ? EGL_PBUFFER_BIT : 0) |
            (drmDrawable & DRM_PIXMAP_BIT ? EGL_PIXMAP_BIT : 0);

        config.minSwapInterval = mMinSwapInterval;
        config.maxSwapInterval = mMaxSwapInterval;

        // TODO(cwallez) wildly guessing these formats, another TODO says they should be removed anyway
        config.renderTargetFormat = GL_RGBA8;
        config.depthStencilFormat = GL_DEPTH24_STENCIL8;

        config.conformant = EGL_OPENGL_ES2_BIT | (supportsES3 ? EGL_OPENGL_ES3_BIT_KHR : 0);
        config.renderableType = config.conformant;

        // TODO(cwallez) I have no idea what this is
        config.matchNativePixmap = EGL_NONE;

        int id = configs.add(config);
        configIdToDRMConfig[id] = drmConfig;
    }

    XFree(drmConfigs);
#endif
    return configs;
}

bool DisplayDRM::isDeviceLost() const
{
    // UNIMPLEMENTED();
    return false;
}

bool DisplayDRM::testDeviceLost()
{
    // UNIMPLEMENTED();
    return false;
}

egl::Error DisplayDRM::restoreLostDevice()
{
    UNIMPLEMENTED();
    return egl::Error(EGL_BAD_DISPLAY);
}

bool DisplayDRM::isValidNativeWindow(EGLNativeWindowType window) const
{
    // There is no function in Xlib to check the validity of a Window directly.
    // However a small number of functions used to obtain window information
    // return a status code (0 meaning failure) and guarantee that they will
    // fail if the window doesn't exist (the rational is that these function
    // are used by window managers). Out of these function we use XQueryTree
    // as it seems to be the simplest; a drawback is that it will allocate
    // memory for the list of children, becasue we use a child window for
    // WindowSurface.
    /*XXX
    Window root;
    Window parent;
    unsigned nChildren;
    */
    Window *children = nullptr;
    int status = 0;//XXX XQueryTree(mDRM.getDisplay(), window, &root, &parent, &children, &nChildren);
    if (children)
    {
        //XXX XFree(children);
    }
    return status != 0;
}

std::string DisplayDRM::getVendorString() const
{
    // UNIMPLEMENTED();
    return "";
}

void DisplayDRM::syncXCommands() const
{
    if (mUsesNewXDisplay)
    {
        //XXX XSync(mDRM.getDisplay(), False);
    }
}

void DisplayDRM::setSwapInterval(drm::Drawable drawable, SwapControlData *data)
{
    ASSERT(data != nullptr);

#ifdef XXX
    // TODO(cwallez) error checking?
    if (mSwapControl == SwapControl::EXT)
    {
        // Prefer the EXT extension, it gives per-drawable swap intervals, which will
        // minimize the number of driver calls.
        if (data->maxSwapInterval < 0)
        {
            unsigned int maxSwapInterval = 0;
            mDRM.queryDrawable(drawable, DRM_MAX_SWAP_INTERVAL_EXT, &maxSwapInterval);
            data->maxSwapInterval = static_cast<int>(maxSwapInterval);
        }

        // When the egl configs were generated we had to guess what the max swap interval
        // was because we didn't have a window to query it one (and that this max could
        // depend on the monitor). This means that the target interval might be higher
        // than the max interval and needs to be clamped.
        const int realInterval = std::min(data->targetSwapInterval, data->maxSwapInterval);
        if (data->currentSwapInterval != realInterval)
        {
            mDRM.swapIntervalEXT(drawable, realInterval);
            data->currentSwapInterval = realInterval;
        }
    }
    else if (mCurrentSwapInterval != data->targetSwapInterval)
    {
        // With the Mesa or SGI extensions we can still do per-drawable swap control
        // manually but it is more expensive in number of driver calls.
        if (mSwapControl == SwapControl::Mesa)
        {
            mDRM.swapIntervalMESA(data->targetSwapInterval);
        }
        else if (mSwapControl == SwapControl::Mesa)
        {
            mDRM.swapIntervalSGI(data->targetSwapInterval);
        }
        mCurrentSwapInterval = data->targetSwapInterval;
    }
#endif
}

bool DisplayDRM::isValidWindowVisualId(int visualId) const
{
    return mRequestedVisual == -1 || mRequestedVisual == visualId;
}

const FunctionsGL *DisplayDRM::getFunctionsGL() const
{
    return mFunctionsGL;
}

void DisplayDRM::generateExtensions(egl::DisplayExtensions *outExtensions) const
{
    outExtensions->createContext = true;
}

void DisplayDRM::generateCaps(egl::Caps *outCaps) const
{
    // UNIMPLEMENTED();
    outCaps->textureNPOT = true;
}

int DisplayDRM::getDRMFBConfigAttrib(drm::FBConfig config, int attrib) const
{
    int result;
    mDRM.getFBConfigAttrib(config, attrib, &result);
    return result;
}

drm::Context DisplayDRM::createContextAttribs(drm::FBConfig, const std::vector<int> &attribs) const
{
    // When creating a context with glXCreateContextAttribsARB, a variety of X11 errors can
    // be generated. To prevent these errors from crashing our process, we simply ignore
    // them and only look if DRMContext was created.
    //XXX auto oldErrorHandler = XSetErrorHandler(IgnoreX11Errors);
    auto context = mDRM.createContextAttribsARB(mContextConfig, nullptr, True, attribs.data());
    //XXX XSetErrorHandler(oldErrorHandler);

    return context;
}
}
