//
// Copyright (c) 2016 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//

// DisplayDRM.cpp: DRM/KMS/GBM implementation of egl::Display

#include "libANGLE/renderer/gl/cros/DisplayDRM.h"

#include <fcntl.h>
#include <poll.h>
#include <iostream>
#include <unistd.h>

#include <EGL/eglext.h>

#include <gbm.h>
#include <drm_fourcc.h>

#include "common/debug.h"
#include "libANGLE/Config.h"
#include "libANGLE/Display.h"
#include "libANGLE/Surface.h"
#include "libANGLE/renderer/gl/FramebufferGL.h"
#include "libANGLE/renderer/gl/RendererGL.h"
#include "libANGLE/renderer/gl/StateManagerGL.h"
#include "libANGLE/renderer/gl/cros/PbufferSurfaceCros.h"
#include "libANGLE/renderer/gl/cros/WindowSurfaceCros.h"

static void br(){}
namespace rx
{

DisplayDRM::Buffer::Buffer(DisplayDRM *display, const NativeWindow *native)
  : mDisplay(display),
    mNative(native),
    mWidth(0),
    mHeight(0),
    mBO(nullptr),
    mDMABuf(-1),
    mHasDRMFB(false),
    mDRMFB(0),
    mImage(EGL_NO_IMAGE_KHR),
    mColorBuffer(0),
    mDSBuffer(0),
    mGLFB(0),
    mTexture(0)
{
}

DisplayDRM::Buffer::~Buffer()
{
    cleanup(true);
}

void DisplayDRM::Buffer::cleanup(bool cleanupGLFB)
{
    if (mHasDRMFB)
    {
        int fd = gbm_device_get_fd(mDisplay->mGBM);
        drmModeRmFB(fd, mDRMFB);
        mHasDRMFB = false;
    }

    FunctionsGL *gl = mDisplay->mFunctionsGL;
    if (cleanupGLFB)
    {
        gl->deleteFramebuffers(1, &mGLFB);
        mGLFB = 0;
    }
    gl->deleteRenderbuffers(1, &mColorBuffer);
    mColorBuffer = 0;
    gl->deleteRenderbuffers(1, &mDSBuffer);
    mDSBuffer = 0;

    if (mImage != EGL_NO_IMAGE_KHR)
    {
        mDisplay->mEGL.destroyImage(mImage);
        mImage = EGL_NO_IMAGE_KHR;
    }

    if (mTexture)
    {
        gl->deleteTextures(1, &mTexture);
        mTexture = 0;
    }

    if (mDMABuf >= 0)
    {
        close(mDMABuf);
        mDMABuf = -1;
    }

    if (mBO)
    {
        gbm_bo_destroy(mBO);
        mBO = nullptr;
    }
}

bool DisplayDRM::Buffer::resize(uint32_t format, uint32_t flags, bool depthStencil)
{

    if (mWidth == mNative->width && mHeight == mNative->height)
    {
        return true;
    }

    std::cout << "RESIZE" << std::endl;

    cleanup(false);

    mBO = gbm_bo_create(mDisplay->mGBM, mNative->width, mNative->height, format, flags);
    if (!mBO)
    {
        return false;
    }

    mDMABuf = gbm_bo_get_fd(mBO);
    if (mDMABuf < 0)
    {
        return false;
    }

    const EGLint attr[] = {
        EGL_WIDTH, mNative->width,
        EGL_HEIGHT, mNative->height,
        EGL_LINUX_DRM_FOURCC_EXT, DRM_FORMAT_ARGB8888,
        EGL_DMA_BUF_PLANE0_FD_EXT, mDMABuf,
        EGL_DMA_BUF_PLANE0_OFFSET_EXT, 0,
        EGL_DMA_BUF_PLANE0_PITCH_EXT, (EGLint) gbm_bo_get_stride(mBO),
        EGL_NONE,
    };

    mImage = mDisplay->mEGL.createImage(EGL_NO_CONTEXT, EGL_LINUX_DMA_BUF_EXT, NULL, attr);
    if (mImage == EGL_NO_IMAGE_KHR)
    {
        return false;
    }

    FunctionsGL *gl = mDisplay->mFunctionsGL;
    StateManagerGL *sm = mDisplay->getRenderer()->getStateManager();

    gl->genRenderbuffers(1, &mColorBuffer);
    sm->bindRenderbuffer(GL_RENDERBUFFER, mColorBuffer);
    gl->eglImageTargetRenderbufferStorageOES(GL_RENDERBUFFER, mImage);

    sm->bindFramebuffer(GL_FRAMEBUFFER, mGLFB);
    gl->framebufferRenderbuffer(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0_EXT, GL_RENDERBUFFER, mColorBuffer);

    bool depth = depthStencil, stencil = depthStencil;
    if (depth || stencil)
    {
        gl->genRenderbuffers(1, &mDSBuffer);
        sm->bindRenderbuffer(GL_RENDERBUFFER, mDSBuffer);
        gl->renderbufferStorage(GL_RENDERBUFFER, GL_DEPTH24_STENCIL8_OES, mNative->width, mNative->height);
    }

    if (depth)
    {
        gl->framebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, mDSBuffer);
    }

    if (stencil)
    {
        gl->framebufferRenderbuffer(GL_FRAMEBUFFER, GL_STENCIL_ATTACHMENT, GL_RENDERBUFFER, mDSBuffer);
    }

    mWidth = mNative->width;
    mHeight = mNative->height;
    mFormat = format;
    mFlags = flags;
    mDepthStencil = depthStencil;

    return true;
}

bool DisplayDRM::Buffer::initialize(uint32_t format, uint32_t flags, bool depthStencil)
{
    mDisplay->mFunctionsGL->genFramebuffers(1, &mGLFB);
    return resize(format, flags, depthStencil);
}

GLuint DisplayDRM::Buffer::getTexture()
{
    if (!mTexture)
    {
        FunctionsGL *gl = mDisplay->mFunctionsGL;
        StateManagerGL *sm = mDisplay->getRenderer()->getStateManager();

        gl->genTextures(1, &mTexture);
        sm->bindTexture(GL_TEXTURE_2D, mTexture);
        gl->texParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
        gl->texParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
        gl->texParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
        gl->texParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
        ASSERT(mImage != EGL_NO_IMAGE_KHR);
        gl->eglImageTargetTexture2DOES(GL_TEXTURE_2D, mImage);
    }
    return mTexture;
}

uint32_t DisplayDRM::Buffer::getDRMFB()
{
    if (!mHasDRMFB)
    {
        int fd = gbm_device_get_fd(mDisplay->mGBM);
        //XXX use addfb2
        if (drmModeAddFB(fd, mWidth, mHeight, 24, 32, gbm_bo_get_stride(mBO), gbm_bo_get_handle(mBO).u32, &mDRMFB))
        {
            std::cout << "drmModeAddFB failed" << std::endl;
        }
        mHasDRMFB = true;
    }

    return mDRMFB;
}

FramebufferGL *DisplayDRM::Buffer::framebufferGL(const gl::Framebuffer::Data &data)
{
    return new FramebufferGL(mGLFB, data, mDisplay->mFunctionsGL, mDisplay->getRenderer()->getWorkarounds(), mDisplay->getRenderer()->getStateManager());
}

void DisplayDRM::Buffer::present()
{
    mDisplay->drawBuffer(this);
    resize(mFormat, mFlags, mDepthStencil);
}

SwapControlData::SwapControlData()
  : targetSwapInterval(0),
    maxSwapInterval(-1),
    currentSwapInterval(-1)
{
}

class FunctionsGLEGL : public FunctionsGL
{
  public:
    FunctionsGLEGL(FunctionsEGL::GETPROC getProc)
      : mGetProc(getProc)
    {
    }

    virtual ~FunctionsGLEGL()
    {
    }

  private:
    void *loadProcAddress(const std::string &function) override
    {
        return reinterpret_cast<void*>(mGetProc(function.c_str()));
    }

    FunctionsEGL::GETPROC mGetProc;
};

DisplayDRM::DisplayDRM()
    : mFunctionsGL(nullptr),
      mContextConfig(nullptr),
      mContext(nullptr),
      mDummyPbuffer(0),
      mIsMesa(false),
      mHasMultisample(false),
      mHasARBCreateContext(false),
      mSwapControl(SwapControl::Absent),
      mMinSwapInterval(0),
      mMaxSwapInterval(0),
      mCurrentSwapInterval(-1),
      mEGLDisplay(nullptr),
      mGBM(nullptr),
      mConnector(nullptr),
      mMode(nullptr),
      mCRTC(nullptr),
      mSetCRTC(true),
      mScanning(nullptr),
      mPending(nullptr),
      mDrawing(nullptr),
      mUnused(nullptr),
      mProgram(0),
      mVertexShader(0),
      mFragmentShader(0),
      mVertexBuffer(0),
      mIndexBuffer(0),
      mCenterUniform(0),
      mWindowSizeUniform(0),
      mBorderSizeUniform(0),
      mDepthUniform(0)
{
}

DisplayDRM::~DisplayDRM()
{
}

static drmModeModeInfoPtr ChooseMode(drmModeConnectorPtr conn)
{
    drmModeModeInfoPtr mode = NULL;
    ASSERT(conn);
    ASSERT(conn->connection == DRM_MODE_CONNECTED);
    // use first preferred mode if any, else end up with last mode in list
    for (int i = 0; i < conn->count_modes; ++i)
    {
        mode = conn->modes + i;
        if (mode->type & DRM_MODE_TYPE_PREFERRED)
        {
            break;
        }
    }
    return mode;
}

static int ChooseCRTC(int fd, unsigned count_crtcs, drmModeConnectorPtr conn)
{
    for (int i = 0; i < conn->count_encoders; ++i)
    {
        drmModeEncoderPtr enc = drmModeGetEncoder(fd, conn->encoders[i]);
        unsigned b = enc->possible_crtcs;
        drmModeFreeEncoder(enc);
        for (unsigned j = 0; b && j < count_crtcs; b >>= 1, ++j)
        {
            if (b & 1)
            {
                return j;
            }
        }
    }
    return -1;
}

egl::Error DisplayDRM::initialize(egl::Display *display)
{
    //XXX be smarter about which device to open
    int fd = open("/dev/dri/card0", O_RDWR | O_CLOEXEC);
    if (fd < 0)
    {
        return egl::Error(EGL_NOT_INITIALIZED, "Could not open drm device.");
    }
    mGBM = gbm_create_device(fd);
    if (!mGBM)
    {
        close(fd);
        return egl::Error(EGL_NOT_INITIALIZED, "Could not create gbm device.");
    }

    mConnector = NULL;
    drmModeResPtr mr = drmModeGetResources(fd);
    bool monitorConnected = false;
    for (int i = 0; !mCRTC && i < mr->count_connectors; ++i)
    {
        drmModeFreeConnector(mConnector);
        mConnector = drmModeGetConnector(fd, mr->connectors[i]);
        if (!mConnector || mConnector->connection != DRM_MODE_CONNECTED)
        {
            continue;
        }
        monitorConnected = true;
        mMode = ChooseMode(mConnector);
        if (!mMode)
        {
            continue;
        }
        int n = ChooseCRTC(fd, mr->count_crtcs, mConnector);
        if (n < 0)
        {
            continue;
        }
        mCRTC = drmModeGetCrtc(fd, mr->crtcs[n]);
    }
    drmModeFreeResources(mr);

    if (mCRTC)
    {
        mNative.width = mMode->hdisplay;
        mNative.height = mMode->vdisplay;
    }
    else if (!monitorConnected)
    {
        // arbitrary
        //XXX may not need this - did it in waffle so window size could be based on display size
        mNative.width = 1280;
        mNative.height = 1024;
    }
    else
    {
        return egl::Error(EGL_NOT_INITIALIZED, "Failed to choose mode/crtc.");
    }

    mEGLDisplay = display;
    EGLNativeDisplayType nativeDisplay = display->getNativeDisplayId();
    const auto &attribMap = display->getAttributeMap();

    std::string eglInitError;
    if (!mEGL.initialize(nativeDisplay, &eglInitError))
    {
        return egl::Error(EGL_NOT_INITIALIZED, eglInitError.c_str());
    }

#ifdef XXX
    mHasMultisample      = mEGL.minorVersion > 3 || mEGL.hasExtension("DRM_ARB_multisample");
    mHasARBCreateContext = mEGL.hasExtension("DRM_ARB_create_context");

    if (attribMap.contains(EGL_X11_VISUAL_ID_ANGLE))
    {
        mRequestedVisual = attribMap.get(EGL_X11_VISUAL_ID_ANGLE, -1);

        // There is no direct way to get the FBConfig matching an X11 visual ID
        // so we have to iterate over all the FBConfigs to find the right one.
        int nConfigs;
        int attribList[] = {
            None,
        };
        EGLConfig *allConfigs = mEGL.chooseFBConfig(attribList, &nConfigs);

        for (int i = 0; i < nConfigs; ++i)
        {
            if (getFBConfigAttrib(allConfigs[i], DRM_VISUAL_ID) == mRequestedVisual)
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
        // context must be compatible with all the FBConfig corresponding to the
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
        EGLConfig *candidates = mEGL.chooseFBConfig(attribList, &nConfigs);
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
        visualTemplate.visualid = getFBConfigAttrib(mContextConfig, DRM_VISUAL_ID);

        int numVisuals       = 0;
        XVisualInfo *visuals = XGetVisualInfo(xDisplay, VisualIDMask, &visualTemplate, &numVisuals);
        if (numVisuals <= 0)
        {
            return egl::Error(EGL_NOT_INITIALIZED,
                              "Could not get the visual info from the fb config");
        }
        ASSERT(numVisuals == 1);

        mContext = mEGL.createContext(&visuals[0], nullptr, true);
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
    mDummyPbuffer = mEGL.createPbuffer(mContextConfig, dummyPbufferAttribs);
    if (!mDummyPbuffer)
    {
        return egl::Error(EGL_NOT_INITIALIZED, "Could not create the dummy pbuffer.");
    }
#endif

    if (!mEGL.makeCurrent(EGL_NO_SURFACE, mContext))
    {
        return egl::Error(EGL_NOT_INITIALIZED, "Could not make context current.");
    }

    mFunctionsGL = new FunctionsGLEGL(mEGL.getProc);
    mFunctionsGL->initialize();

    std::string rendererString =
        reinterpret_cast<const char*>(mFunctionsGL->getString(GL_RENDERER));
    mIsMesa = rendererString.find("Mesa") != std::string::npos;

    return DisplayGL::initialize(display);
}

static double hat(double x)
{
    x=fmod(x, 3);
    if (x<1) return x;
    if (x<2) return 2-x;
    return 0;
}

static gl::ColorF wheel(double x)
{
    return gl::ColorF(hat(x+0), hat(x+1), hat(x+2), 1);
}

void DisplayDRM::pageFlipHandler(int fd, unsigned int sequence, unsigned int tv_sec, unsigned int tv_usec, void *data)
{
    DisplayDRM *display = reinterpret_cast<DisplayDRM *>(data);
    uint64_t tv = tv_sec;
    display->pageFlipHandler(sequence, tv * 1000000 + tv_usec);
}

void DisplayDRM::pageFlipHandler(unsigned int sequence, uint64_t tv)
{
    //std::cout << "FLIP ---------------- " << sequence << " " << tv << std::endl;
    ASSERT(mPending);
    mUnused = mScanning;
    mScanning = mPending;
    mPending = nullptr;
}

void DisplayDRM::presentScreen()
{
    if (!mCRTC)
    {
        // no monitor
        return;
    }

    // see if pending flip has finished, without blocking
    int fd = gbm_device_get_fd(mGBM);
    if (mPending)
    {
        pollfd pfd;
        pfd.fd = fd;
        pfd.events = POLLIN;
        if (poll(&pfd, 1, 0) < 0)
        {
            std::cout << "poll failed " << errno << std::endl;
        }
        if (pfd.revents & POLLIN)
        {
            drmEventContext event;
            event.version = DRM_EVENT_CONTEXT_VERSION;
            event.page_flip_handler = pageFlipHandler;
            drmHandleEvent(fd, &event);
        }
    }

    // if pending flip has finished, schedule next one
    if (!mPending && mDrawing)
    {
        if (mSetCRTC)
        {
            if (drmModeSetCrtc(fd, mCRTC->crtc_id, mDrawing->getDRMFB(), 0, 0, &mConnector->connector_id, 1, mMode))
            {
                std::cout << "set crtc failed" << std::endl;
            }
            mSetCRTC = false;
        }
        if (drmModePageFlip(fd, mCRTC->crtc_id, mDrawing->getDRMFB(), DRM_MODE_PAGE_FLIP_EVENT, this))
        {
            std::cout << "page flip failed" << std::endl;
        }
        mPending = mDrawing;
        mDrawing = nullptr;
        //std::cout << "FLIP SCHEDULED ***************" << std::endl;
    }
}

//XXX blit is ES 3.0 only, and we probably need to support 2.0
void DisplayDRM::drawWithBlit(Buffer *buffer)
{
    StateManagerGL *sm = getRenderer()->getStateManager();
    sm->bindFramebuffer(GL_READ_FRAMEBUFFER, buffer->mGLFB);
    sm->bindFramebuffer(GL_DRAW_FRAMEBUFFER, mDrawing->mGLFB);
    mFunctionsGL->blitFramebuffer(0, 0, buffer->mWidth, buffer->mHeight,
                                  buffer->mNative->x,
                                  buffer->mNative->y,
                                  buffer->mWidth  + buffer->mNative->x,
                                  buffer->mHeight + buffer->mNative->y,
                                  GL_COLOR_BUFFER_BIT, GL_NEAREST);
}

GLuint DisplayDRM::makeShader(GLuint type, const char *src)
{
    FunctionsGL *gl = mFunctionsGL;
    GLuint shader = gl->createShader(type);
    gl->shaderSource(shader, 1, &src, NULL);
    gl->compileShader(shader);
    if (1)
    {
        GLchar buf[999];
        GLsizei len;
        GLint compiled;
        gl->getShaderInfoLog(shader, sizeof(buf), &len, buf);
        gl->getShaderiv(shader, GL_COMPILE_STATUS, &compiled);
        std::cout << "shader compiled=" << compiled << " log:\n" << buf << std::endl;
    }
    return shader;
}

void DisplayDRM::drawWithTexture(Buffer *buffer)
{
    FunctionsGL *gl = mFunctionsGL;
    StateManagerGL *sm = getRenderer()->getStateManager();

    if (!mProgram)
    {
        const GLchar *vertexSource =
            "#version 100\n"
            "attribute vec3 vertex;"
            "uniform vec2 center;"
            "uniform vec2 windowSize;"
            "uniform vec2 borderSize;"
            "uniform float depth;"
            "varying vec3 texCoord;"
            "void main() {"
            "    vec2 pos = vertex.xy * (windowSize + borderSize * vertex.z);"
            "    gl_Position = vec4(center + pos, depth, 1);"
            "    texCoord = vec3(pos / windowSize * vec2(.5, -.5) + vec2(.5, .5), vertex.z);"
            "}";
        const GLchar *fragmentSource =
            "#version 100\n"
            "precision mediump float;"
            "uniform sampler2D tex;"
            "varying vec3 texCoord;"
            "void main() {"
            "    if (texCoord.z > 0.) {"
            "        float c = abs((texCoord.z * 2.) - 1.);"
            "        gl_FragColor = vec4(c,c,c,1);"
            "    } else {"
            "        gl_FragColor = texture2D(tex, texCoord.xy);"
            "    }"
            "}";
        mVertexShader = makeShader(GL_VERTEX_SHADER, vertexSource);
        mFragmentShader = makeShader(GL_FRAGMENT_SHADER, fragmentSource);
        mProgram = gl->createProgram();
        gl->attachShader(mProgram, mVertexShader);
        gl->attachShader(mProgram, mFragmentShader);
        gl->bindAttribLocation(mProgram, 0, "vertex");
        gl->linkProgram(mProgram);
        GLint linked;
        gl->getProgramiv(mProgram, GL_LINK_STATUS, &linked);
        ASSERT(linked);
        mCenterUniform = gl->getUniformLocation(mProgram, "center");
        mWindowSizeUniform = gl->getUniformLocation(mProgram, "windowSize");
        mBorderSizeUniform = gl->getUniformLocation(mProgram, "borderSize");
        mDepthUniform = gl->getUniformLocation(mProgram, "depth");
        GLint texUniform = gl->getUniformLocation(mProgram, "tex");
        sm->useProgram(mProgram);
        gl->uniform1i(texUniform, 0);

        const GLfloat vertices[] = {
             // window corners
             1, -1, 0,
            -1, -1, 0,
             1,  1, 0,
            -1,  1, 0,
             // window plus border corners
             1, -1, 1,
            -1, -1, 1,
             1,  1, 1,
            -1,  1, 1,
        };
        gl->genBuffers(1, &mVertexBuffer);
        sm->bindBuffer(GL_ARRAY_BUFFER, mVertexBuffer);
        gl->bufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

        // window border triangle strip
        const GLuint indices[] = { 5,0,4,2,6,3,7,1,5,0 };

        gl->genBuffers(1, &mIndexBuffer);
        sm->bindBuffer(GL_ELEMENT_ARRAY_BUFFER, mIndexBuffer);
        gl->bufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW);
    }
    else
    {
        sm->useProgram(mProgram);
        sm->bindBuffer(GL_ARRAY_BUFFER, mVertexBuffer);
        sm->bindBuffer(GL_ELEMENT_ARRAY_BUFFER, mIndexBuffer);
    }

    // convert from pixels to "-1 to 1" space
    const NativeWindow *n = buffer->mNative;
    double x = n->x * 2. / mNative.width - 1;
    double y = n->y * 2. / mNative.height - 1;
    double halfw = n->width * 1. / mNative.width;
    double halfh = n->height * 1. / mNative.height;
    double borderw = n->borderWidth * 2. / mNative.width;
    double borderh = n->borderHeight * 2. / mNative.height;

    gl->uniform2f(mCenterUniform, x + halfw, y + halfh);
    gl->uniform2f(mWindowSizeUniform, halfw, halfh);
    gl->uniform2f(mBorderSizeUniform, borderw, borderh);
    gl->uniform1f(mDepthUniform, n->depth / 1e6);

    sm->setBlendEnabled(false);
    sm->setCullFaceEnabled(false);
    sm->setStencilTestEnabled(false);
    sm->setScissorTestEnabled(false);
    sm->setDepthTestEnabled(true);
    sm->setColorMask(true, true, true, true);
    sm->setDepthMask(true);
    sm->setDepthRange(0, 1);
    sm->setDepthFunc(GL_LESS);
    sm->setViewport(gl::Rectangle(0, 0, mNative.width, mNative.height));
    sm->activeTexture(0);
    sm->bindTexture(GL_TEXTURE_2D, buffer->getTexture());
    gl->vertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, 0);
    gl->enableVertexAttribArray(0);
    sm->bindFramebuffer(GL_DRAW_FRAMEBUFFER, mDrawing->mGLFB);
    gl->drawArrays(GL_TRIANGLE_STRIP, 0, 4);
    gl->drawElements(GL_TRIANGLE_STRIP, 10, GL_UNSIGNED_INT, 0);
}

void DisplayDRM::drawBuffer(Buffer *buffer)
{
    if (!buffer->mNative->visible)
    {
        return;
    }

    static double f=0;
    StateManagerGL *sm = getRenderer()->getStateManager();

    // get buffer on which to draw window
    if (!mDrawing)
    {
        if (mUnused)
        {
            mDrawing = mUnused;
        }
        else
        {
            mDrawing = new Buffer(this, &mNative);
            mDrawing->initialize(GBM_FORMAT_ARGB8888, GBM_BO_USE_RENDERING|GBM_BO_USE_SCANOUT, true);
        }
        sm->bindFramebuffer(GL_DRAW_FRAMEBUFFER, mDrawing->mGLFB);
        sm->setClearColor(wheel(f));
        sm->setClearDepth(1);
        sm->setScissorTestEnabled(false);
        sm->setColorMask(true, true, true, true);
        sm->setDepthMask(true);
        mFunctionsGL->clear(GL_COLOR_BUFFER_BIT|GL_DEPTH_BUFFER_BIT);
        f+=.01;
    }

    // draw the window
    drawWithTexture(buffer);
    presentScreen();
}

void DisplayDRM::terminate()
{
    SafeDelete(mScanning);
    SafeDelete(mPending);
    SafeDelete(mDrawing);
    SafeDelete(mUnused);

    if (mProgram)
    {
        mFunctionsGL->deleteProgram(mProgram);
        mFunctionsGL->deleteShader(mVertexShader);
        mFunctionsGL->deleteShader(mFragmentShader);
        mFunctionsGL->deleteBuffers(1, &mVertexBuffer);
        mFunctionsGL->deleteBuffers(1, &mIndexBuffer);
        mProgram = 0;
    }

    DisplayGL::terminate();

    if (mDummyPbuffer)
    {
        //XXX mEGL.destroyPbuffer(mDummyPbuffer);
        mDummyPbuffer = 0;
    }

    if (mContext)
    {
        mEGL.destroyContext(mContext);
        mContext = nullptr;
    }

    mEGL.terminate();

    SafeDelete(mFunctionsGL);
}

SurfaceImpl *DisplayDRM::createWindowSurface(const egl::Config *configuration,
                                             EGLNativeWindowType window,
                                             const egl::AttributeMap &attribs)
{
    Buffer *buffer = new Buffer(this, reinterpret_cast<const NativeWindow *>(window));
    buffer->initialize(GBM_FORMAT_ARGB8888, GBM_BO_USE_RENDERING, true);
    return new WindowSurfaceCros(getRenderer(), buffer);
}

SurfaceImpl *DisplayDRM::createPbufferSurface(const egl::Config *configuration,
                                              const egl::AttributeMap &attribs)
{
    ASSERT(configIdToConfig.count(configuration->configID) > 0);
    EGLConfig fbConfig = configIdToConfig[configuration->configID];

    EGLint width = attribs.get(EGL_WIDTH, 0);
    EGLint height = attribs.get(EGL_HEIGHT, 0);
    bool largest = (attribs.get(EGL_LARGEST_PBUFFER, EGL_FALSE) == EGL_TRUE);

    return new PbufferSurfaceCros(getRenderer(), width, height, largest, mEGL, mContext,
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

EGLContext DisplayDRM::initializeContext(EGLConfig config,
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
        return mEGL.createContext(config, EGL_NO_CONTEXT, contextAttributes.data());
    }

    //XXX do we need to do this for gles?
    // It is commonly assumed that glXCreateContextAttrib will create a context
    // of the highest version possible but it is not specified in the spec and
    // is not true on the Mesa drivers. Instead we try to create a context per
    // desktop GL version until we succeed, starting from newer version.
    // clang-format off
    const gl::Version esVersions[] = {
        gl::Version(3, 0),
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
        auto context = mEGL.createContext(config, EGL_NO_CONTEXT, contextAttributes.data());

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
    configIdToConfig.clear();

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
    EGLConfig *drmConfigs = mEGL.chooseFBConfig(attribList, &drmConfigCount);

    for (int i = 0; i < drmConfigCount; i++)
    {
        EGLConfig drmConfig = drmConfigs[i];
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
        configIdToConfig[id] = drmConfig;
    }

    XFree(drmConfigs);
#else
    //XXX just hard code one config for now
    egl::Config config;
    config.redSize = 8;
    config.greenSize = 8;
    config.blueSize = 8;
    config.alphaSize = 8;
    config.depthSize = 24;
    config.stencilSize = 8;
    config.surfaceType = EGL_WINDOW_BIT;
    configs.add(config);
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
    //XXX could put a magic number or something in the native window, but is it really worth it?
    return true;
}

std::string DisplayDRM::getVendorString() const
{
    // UNIMPLEMENTED();
    return "";
}

egl::Error DisplayDRM::waitClient() const
{
    //XXX mGLX.waitGL();
    return egl::Error(EGL_SUCCESS);
}

egl::Error DisplayDRM::waitNative(EGLint engine, egl::Surface *drawSurface, egl::Surface *readSurface) const
{
    return egl::Error(EGL_SUCCESS);
}

void DisplayDRM::setSwapInterval(EGLSurface drawable, SwapControlData *data)
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
            mEGL.queryDrawable(drawable, DRM_MAX_SWAP_INTERVAL_EXT, &maxSwapInterval);
            data->maxSwapInterval = static_cast<int>(maxSwapInterval);
        }

        // When the egl configs were generated we had to guess what the max swap interval
        // was because we didn't have a window to query it one (and that this max could
        // depend on the monitor). This means that the target interval might be higher
        // than the max interval and needs to be clamped.
        const int realInterval = std::min(data->targetSwapInterval, data->maxSwapInterval);
        if (data->currentSwapInterval != realInterval)
        {
            mEGL.swapIntervalEXT(drawable, realInterval);
            data->currentSwapInterval = realInterval;
        }
    }
    else if (mCurrentSwapInterval != data->targetSwapInterval)
    {
        // With the Mesa or SGI extensions we can still do per-drawable swap control
        // manually but it is more expensive in number of driver calls.
        if (mSwapControl == SwapControl::Mesa)
        {
            mEGL.swapIntervalMESA(data->targetSwapInterval);
        }
        else if (mSwapControl == SwapControl::Mesa)
        {
            mEGL.swapIntervalSGI(data->targetSwapInterval);
        }
        mCurrentSwapInterval = data->targetSwapInterval;
    }
#endif
}

const FunctionsGL *DisplayDRM::getFunctionsGL() const
{
    return mFunctionsGL;
}

void DisplayDRM::generateExtensions(egl::DisplayExtensions *outExtensions) const
{
    outExtensions->createContext = true;
    outExtensions->createContextNoError = true;
}

void DisplayDRM::generateCaps(egl::Caps *outCaps) const
{
    // UNIMPLEMENTED();
    outCaps->textureNPOT = true;
}

int DisplayDRM::getDRMFBConfigAttrib(EGLConfig config, int attrib) const
{
    UNIMPLEMENTED();
    return 0;
}

}
