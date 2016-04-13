//
// Copyright (c) 2016 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//

// DisplayOzone.cpp: Ozone implementation of egl::Display

#include "libANGLE/renderer/gl/egl/ozone/DisplayOzone.h"

#include <fcntl.h>
#include <poll.h>
#include <iostream>
#include <unistd.h>
#include <sys/time.h>

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
#include "libANGLE/renderer/gl/egl/ozone/PbufferSurfaceOzone.h"
#include "libANGLE/renderer/gl/egl/ozone/WindowSurfaceOzone.h"

#define SHADER_SOURCE(...) #__VA_ARGS__

// ARM-specific extension needed to make Mali GPU behave - not in any
// published header file.
#ifndef EGL_SYNC_PRIOR_COMMANDS_IMPLICIT_EXTERNAL_ARM
#define EGL_SYNC_PRIOR_COMMANDS_IMPLICIT_EXTERNAL_ARM 0x328A
#endif

#ifndef EGL_NO_CONFIG_MESA
#define EGL_NO_CONFIG_MESA ((EGLConfig)0)
#endif

namespace
{

EGLint UnsignedToSigned(uint32_t u)
{
    return *reinterpret_cast<const EGLint *>(&u);
}

drmModeModeInfoPtr ChooseMode(drmModeConnectorPtr conn)
{
    drmModeModeInfoPtr mode = nullptr;
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

int ChooseCRTC(int fd, drmModeConnectorPtr conn)
{
    for (int i = 0; i < conn->count_encoders; ++i)
    {
        drmModeEncoderPtr enc = drmModeGetEncoder(fd, conn->encoders[i]);
        unsigned long crtcs = enc->possible_crtcs;
        drmModeFreeEncoder(enc);
        if (crtcs)
        {
            return __builtin_ctzl(crtcs);
        }
    }
    return -1;
}
}  // namespace

namespace rx
{

DisplayOzone::Buffer::Buffer(DisplayOzone *display,
                             const NativeWindow *native,
                             uint32_t useFlags,
                             uint32_t gbmFormat,
                             uint32_t drmFormat,
                             uint32_t drmFormatFB,
                             int depthBits,
                             int stencilBits)
    : mDisplay(display),
      mNative(native),
      mWidth(0),
      mHeight(0),
      mDepthBits(depthBits),
      mStencilBits(stencilBits),
      mUseFlags(useFlags),
      mGBMFormat(gbmFormat),
      mDRMFormat(drmFormat),
      mDRMFormatFB(drmFormatFB),
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

DisplayOzone::Buffer::~Buffer()
{
    mDisplay->mFunctionsGL->deleteFramebuffers(1, &mGLFB);
    reset();
}

void DisplayOzone::Buffer::reset()
{
    if (mHasDRMFB)
    {
        int fd = gbm_device_get_fd(mDisplay->mGBM);
        drmModeRmFB(fd, mDRMFB);
        mHasDRMFB = false;
    }

    FunctionsGL *gl = mDisplay->mFunctionsGL;
    gl->deleteRenderbuffers(1, &mColorBuffer);
    mColorBuffer = 0;
    gl->deleteRenderbuffers(1, &mDSBuffer);
    mDSBuffer = 0;

    // Unlike every other resource in the buffer, the GL framebuffer (mGLFB)
    // is not destroyed (and recreated) because when it is the default
    // framebuffer for an Angle surface Angle expects it to have the same
    // lifetime as that surface.

    if (mImage != EGL_NO_IMAGE_KHR)
    {
        mDisplay->mEGL.destroyImageKHR(mImage);
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

bool DisplayOzone::Buffer::resize()
{
    if (mWidth == mNative->width && mHeight == mNative->height)
    {
        return true;
    }

    reset();

    mBO = gbm_bo_create(mDisplay->mGBM, mNative->width, mNative->height, mGBMFormat, mUseFlags);
    if (!mBO)
    {
        return false;
    }

    mDMABuf = gbm_bo_get_fd(mBO);
    if (mDMABuf < 0)
    {
        return false;
    }

    // clang-format off
    const EGLint attr[] =
    {
        EGL_WIDTH, mNative->width,
        EGL_HEIGHT, mNative->height,
        EGL_LINUX_DRM_FOURCC_EXT, UnsignedToSigned(mDRMFormat),
        EGL_DMA_BUF_PLANE0_FD_EXT, mDMABuf,
        EGL_DMA_BUF_PLANE0_PITCH_EXT, UnsignedToSigned(gbm_bo_get_stride(mBO)),
        EGL_DMA_BUF_PLANE0_OFFSET_EXT, 0,
        EGL_NONE,
    };
    // clang-format on

    mImage = mDisplay->mEGL.createImageKHR(EGL_NO_CONTEXT, EGL_LINUX_DMA_BUF_EXT, nullptr, attr);
    if (mImage == EGL_NO_IMAGE_KHR)
    {
        return false;
    }

    FunctionsGL *gl    = mDisplay->mFunctionsGL;
    StateManagerGL *sm = mDisplay->getRenderer()->getStateManager();

    gl->genRenderbuffers(1, &mColorBuffer);
    sm->bindRenderbuffer(GL_RENDERBUFFER, mColorBuffer);
    gl->eglImageTargetRenderbufferStorageOES(GL_RENDERBUFFER, mImage);

    sm->bindFramebuffer(GL_FRAMEBUFFER, mGLFB);
    gl->framebufferRenderbuffer(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0_EXT, GL_RENDERBUFFER,
                                mColorBuffer);

    if (mDepthBits || mStencilBits)
    {
        gl->genRenderbuffers(1, &mDSBuffer);
        sm->bindRenderbuffer(GL_RENDERBUFFER, mDSBuffer);
        gl->renderbufferStorage(GL_RENDERBUFFER, GL_DEPTH24_STENCIL8_OES, mNative->width,
                                mNative->height);
    }

    if (mDepthBits)
    {
        gl->framebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER,
                                    mDSBuffer);
    }

    if (mStencilBits)
    {
        gl->framebufferRenderbuffer(GL_FRAMEBUFFER, GL_STENCIL_ATTACHMENT, GL_RENDERBUFFER,
                                    mDSBuffer);
    }

    mWidth  = mNative->width;
    mHeight = mNative->height;
    return true;
}

bool DisplayOzone::Buffer::initialize()
{
    mDisplay->mFunctionsGL->genFramebuffers(1, &mGLFB);
    return resize();
}

GLuint DisplayOzone::Buffer::getTexture()
{
    // TODO(fjhenigman) Try not to create a new texture every time.  That already works on Intel and
    // almost works on ARM.
    FunctionsGL *gl    = mDisplay->mFunctionsGL;
    StateManagerGL *sm = mDisplay->getRenderer()->getStateManager();

    gl->genTextures(1, &mTexture);
    sm->bindTexture(GL_TEXTURE_2D, mTexture);
    gl->texParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    gl->texParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    gl->texParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    gl->texParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    ASSERT(mImage != EGL_NO_IMAGE_KHR);
    gl->eglImageTargetTexture2DOES(GL_TEXTURE_2D, mImage);
    return mTexture;
}

uint32_t DisplayOzone::Buffer::getDRMFB()
{
    if (!mHasDRMFB)
    {
        int fd              = gbm_device_get_fd(mDisplay->mGBM);
        uint32_t handles[4] = {gbm_bo_get_handle(mBO).u32};
        uint32_t pitches[4] = {gbm_bo_get_stride(mBO)};
        uint32_t offsets[4] = {0};
        if (drmModeAddFB2(fd, mWidth, mHeight, mDRMFormatFB, handles, pitches, offsets, &mDRMFB, 0))
        {
            std::cerr << "drmModeAddFB2 failed" << std::endl;
        }
        else
        {
            mHasDRMFB = true;
        }
    }

    return mDRMFB;
}

FramebufferGL *DisplayOzone::Buffer::framebufferGL(const gl::Framebuffer::Data &data)
{
    return new FramebufferGL(mGLFB, data, mDisplay->mFunctionsGL,
                             mDisplay->getRenderer()->getWorkarounds(),
                             mDisplay->getRenderer()->getStateManager());
}

void DisplayOzone::Buffer::present()
{
    mDisplay->drawBuffer(this);
    resize();
}

// TODO(fjhenigman) Implement swap control.  Until then this is unused.
SwapControlData::SwapControlData()
    : targetSwapInterval(0), maxSwapInterval(-1), currentSwapInterval(-1)
{
}

class FunctionsGLEGL : public FunctionsGL
{
  public:
    FunctionsGLEGL(const FunctionsEGL &egl) : mEGL(egl) {}

    ~FunctionsGLEGL() override {}

  private:
    void *loadProcAddress(const std::string &function) override
    {
        return mEGL.getProcAddress(function.c_str());
    }

    const FunctionsEGL &mEGL;
};

DisplayOzone::DisplayOzone()
    : mFunctionsGL(nullptr),
      mContextConfig(nullptr),
      mContext(nullptr),
      mSwapControl(SwapControl::Absent),
      mMinSwapInterval(0),
      mMaxSwapInterval(0),
      mCurrentSwapInterval(-1),
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

DisplayOzone::~DisplayOzone()
{
}

egl::Error DisplayOzone::initialize(egl::Display *display)
{
    int fd;
    char deviceName[30];
    drmModeResPtr resources = nullptr;

    for (int i = 0; i < 9; ++i)
    {
        snprintf(deviceName, sizeof(deviceName), "/dev/dri/card%d", i);
        fd = open(deviceName, O_RDWR | O_CLOEXEC);
        if (fd >= 0)
        {
            resources = drmModeGetResources(fd);
            if (resources)
            {
                if (resources->count_connectors > 0)
                {
                    break;
                }
                drmModeFreeResources(resources);
                resources = nullptr;
            }
            close(fd);
        }
    }
    if (!resources)
    {
        return egl::Error(EGL_NOT_INITIALIZED, "Could not open drm device.");
    }

    mGBM = gbm_create_device(fd);
    if (!mGBM)
    {
        close(fd);
        drmModeFreeResources(resources);
        return egl::Error(EGL_NOT_INITIALIZED, "Could not create gbm device.");
    }

    mConnector            = nullptr;
    bool monitorConnected = false;
    for (int i = 0; !mCRTC && i < resources->count_connectors; ++i)
    {
        drmModeFreeConnector(mConnector);
        mConnector = drmModeGetConnector(fd, resources->connectors[i]);
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
        int n = ChooseCRTC(fd, mConnector);
        if (n < 0)
        {
            continue;
        }
        mCRTC = drmModeGetCrtc(fd, resources->crtcs[n]);
    }
    drmModeFreeResources(resources);

    if (mCRTC)
    {
        mNative.width  = mMode->hdisplay;
        mNative.height = mMode->vdisplay;
    }
    else if (!monitorConnected)
    {
        // Even though there is no monitor to show it, we still do
        // everything the same as if there were one, so we need an
        // arbitrary size for our buffers.
        mNative.width  = 1280;
        mNative.height = 1024;
    }
    else
    {
        return egl::Error(EGL_NOT_INITIALIZED, "Failed to choose mode/crtc.");
    }

    egl::Error result = mEGL.initialize(display->getNativeDisplayId());
    if (result.isError())
    {
        return result;
    }

    const char *necessaryExtensions[] = {
        "EGL_KHR_image_base", "EGL_EXT_image_dma_buf_import", "EGL_KHR_surfaceless_context",
    };
    for (auto &ext : necessaryExtensions)
    {
        if (!mEGL.hasExtension(ext))
        {
            return egl::Error(EGL_NOT_INITIALIZED, "need %s", ext);
        }
    }

    if (mEGL.hasExtension("EGL_MESA_configless_context"))
    {
        mContextConfig = EGL_NO_CONFIG_MESA;
    }
    else
    {
        // clang-format off
        const EGLint attrib[] =
        {
            // We want RGBA8 and DEPTH24_STENCIL8
            EGL_RED_SIZE, 8,
            EGL_GREEN_SIZE, 8,
            EGL_BLUE_SIZE, 8,
            EGL_ALPHA_SIZE, 8,
            EGL_DEPTH_SIZE, 24,
            EGL_STENCIL_SIZE, 8,
            EGL_NONE,
        };
        // clang-format on
        EGLint numConfig;
        EGLConfig config[1];
        if (!mEGL.chooseConfig(attrib, config, 1, &numConfig) || numConfig < 1)
        {
            return egl::Error(EGL_NOT_INITIALIZED, "Could not get EGL config.");
        }
        mContextConfig = config[0];
    }

    mContext = initializeContext(mContextConfig, display->getAttributeMap());
    if (mContext == EGL_NO_CONTEXT)
    {
        return egl::Error(EGL_NOT_INITIALIZED, "Could not create GLES context.");
    }

    if (!mEGL.makeCurrent(EGL_NO_SURFACE, mContext))
    {
        return egl::Error(EGL_NOT_INITIALIZED, "Could not make context current.");
    }

    mFunctionsGL = new FunctionsGLEGL(mEGL);
    mFunctionsGL->initialize();

    std::string rendererString =
        reinterpret_cast<const char *>(mFunctionsGL->getString(GL_RENDERER));

    return DisplayGL::initialize(display);
}

void DisplayOzone::pageFlipHandler(int fd,
                                   unsigned int sequence,
                                   unsigned int tv_sec,
                                   unsigned int tv_usec,
                                   void *data)
{
    DisplayOzone *display = reinterpret_cast<DisplayOzone *>(data);
    uint64_t tv = tv_sec;
    display->pageFlipHandler(sequence, tv * 1000000 + tv_usec);
}

void DisplayOzone::pageFlipHandler(unsigned int sequence, uint64_t tv)
{
    ASSERT(mPending);
    mUnused   = mScanning;
    mScanning = mPending;
    mPending  = nullptr;
}

void DisplayOzone::presentScreen()
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
        pfd.fd     = fd;
        pfd.events = POLLIN;
        if (poll(&pfd, 1, 0) < 0)
        {
            std::cerr << "poll failed: " << errno << " " << strerror(errno) << std::endl;
        }
        if (pfd.revents & POLLIN)
        {
            drmEventContext event;
            event.version           = DRM_EVENT_CONTEXT_VERSION;
            event.page_flip_handler = pageFlipHandler;
            drmHandleEvent(fd, &event);
        }
    }

    // if pending flip has finished, schedule next one
    if (!mPending && mDrawing)
    {
        flushGL();
        if (mSetCRTC)
        {
            if (drmModeSetCrtc(fd, mCRTC->crtc_id, mDrawing->getDRMFB(), 0, 0,
                               &mConnector->connector_id, 1, mMode))
            {
                std::cerr << "set crtc failed: " << errno << " " << strerror(errno) << std::endl;
            }
            mSetCRTC = false;
        }
        if (drmModePageFlip(fd, mCRTC->crtc_id, mDrawing->getDRMFB(), DRM_MODE_PAGE_FLIP_EVENT,
                            this))
        {
            std::cerr << "page flip failed: " << errno << " " << strerror(errno) << std::endl;
        }
        mPending = mDrawing;
        mDrawing = nullptr;
    }
}

GLuint DisplayOzone::makeShader(GLuint type, const char *src)
{
    FunctionsGL *gl = mFunctionsGL;
    GLuint shader = gl->createShader(type);
    gl->shaderSource(shader, 1, &src, nullptr);
    gl->compileShader(shader);

#if 0  // development aid
    GLchar buf[999];
    GLsizei len;
    GLint compiled;
    gl->getShaderInfoLog(shader, sizeof(buf), &len, buf);
    gl->getShaderiv(shader, GL_COMPILE_STATUS, &compiled);
    std::cerr << "shader compiled=" << compiled << " log:\n" << buf << std::endl;
#endif

    return shader;
}

void DisplayOzone::drawWithTexture(Buffer *buffer)
{
    FunctionsGL *gl    = mFunctionsGL;
    StateManagerGL *sm = getRenderer()->getStateManager();

    if (!mProgram)
    {
        // clang-format off
        const GLchar *vertexSource = "#version 100\n" SHADER_SOURCE
        (
            attribute vec3 vertex;
            uniform vec2 center;
            uniform vec2 windowSize;
            uniform vec2 borderSize;
            uniform float depth;
            varying vec3 texCoord;
            void main()
            {
                vec2 pos = vertex.xy * (windowSize + borderSize * vertex.z);
                gl_Position = vec4(center + pos, depth, 1);
                texCoord = vec3(pos / windowSize * vec2(.5, -.5) + vec2(.5, .5), vertex.z);
            }
        );

        const GLchar *fragmentSource = "#version 100\n" SHADER_SOURCE
        (
            precision mediump float;
            uniform sampler2D tex;
            varying vec3 texCoord;
            void main()
            {
                if (texCoord.z > 0.)
                {
                    float c = abs((texCoord.z * 2.) - 1.);
                    gl_FragColor = vec4(c, c, c, 1);
                }
                else
                {
                    gl_FragColor = texture2D(tex, texCoord.xy);
                }
            }
        );
        // clang-format on

        mVertexShader   = makeShader(GL_VERTEX_SHADER, vertexSource);
        mFragmentShader = makeShader(GL_FRAGMENT_SHADER, fragmentSource);
        mProgram = gl->createProgram();
        gl->attachShader(mProgram, mVertexShader);
        gl->attachShader(mProgram, mFragmentShader);
        gl->bindAttribLocation(mProgram, 0, "vertex");
        gl->linkProgram(mProgram);
        GLint linked;
        gl->getProgramiv(mProgram, GL_LINK_STATUS, &linked);
        ASSERT(linked);
        mCenterUniform     = gl->getUniformLocation(mProgram, "center");
        mWindowSizeUniform = gl->getUniformLocation(mProgram, "windowSize");
        mBorderSizeUniform = gl->getUniformLocation(mProgram, "borderSize");
        mDepthUniform      = gl->getUniformLocation(mProgram, "depth");
        GLint texUniform = gl->getUniformLocation(mProgram, "tex");
        sm->useProgram(mProgram);
        gl->uniform1i(texUniform, 0);

        // clang-format off
        const GLfloat vertices[] =
        {
             // window corners, and window border inside corners
             1, -1, 0,
            -1, -1, 0,
             1,  1, 0,
            -1,  1, 0,
             // window border outside corners
             1, -1, 1,
            -1, -1, 1,
             1,  1, 1,
            -1,  1, 1,
        };
        // clang-format on
        gl->genBuffers(1, &mVertexBuffer);
        sm->bindBuffer(GL_ARRAY_BUFFER, mVertexBuffer);
        gl->bufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

        // window border triangle strip
        const GLuint borderStrip[] = {5, 0, 4, 2, 6, 3, 7, 1, 5, 0};

        gl->genBuffers(1, &mIndexBuffer);
        sm->bindBuffer(GL_ELEMENT_ARRAY_BUFFER, mIndexBuffer);
        gl->bufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(borderStrip), borderStrip, GL_STATIC_DRAW);
    }
    else
    {
        sm->useProgram(mProgram);
        sm->bindBuffer(GL_ARRAY_BUFFER, mVertexBuffer);
        sm->bindBuffer(GL_ELEMENT_ARRAY_BUFFER, mIndexBuffer);
    }

    // convert from pixels to "-1 to 1" space
    const NativeWindow *n = buffer->mNative;
    double x              = n->x * 2. / mNative.width - 1;
    double y              = n->y * 2. / mNative.height - 1;
    double halfw          = n->width * 1. / mNative.width;
    double halfh          = n->height * 1. / mNative.height;
    double borderw        = n->borderWidth * 2. / mNative.width;
    double borderh        = n->borderHeight * 2. / mNative.height;

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
    GLuint tex = buffer->getTexture();
    sm->bindTexture(GL_TEXTURE_2D, tex);
    gl->vertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, 0);
    gl->enableVertexAttribArray(0);
    sm->bindFramebuffer(GL_DRAW_FRAMEBUFFER, mDrawing->mGLFB);
    gl->drawArrays(GL_TRIANGLE_STRIP, 0, 4);
    gl->drawElements(GL_TRIANGLE_STRIP, 10, GL_UNSIGNED_INT, 0);
    sm->deleteTexture(tex);
}

void DisplayOzone::drawBuffer(Buffer *buffer)
{
    if (!buffer->mNative->visible)
    {
        return;
    }

    if (!mDrawing)
    {
        // get buffer on which to draw window
        if (mUnused)
        {
            mDrawing = mUnused;
            mUnused  = nullptr;
        }
        else
        {
            mDrawing = new Buffer(this, &mNative, GBM_BO_USE_RENDERING | GBM_BO_USE_SCANOUT,
                                  GBM_FORMAT_ARGB8888, DRM_FORMAT_ARGB8888, DRM_FORMAT_XRGB8888,
                                  true, true);
            if (!mDrawing || !mDrawing->initialize())
            {
                return;
            }
        }

        StateManagerGL *sm = getRenderer()->getStateManager();
        sm->bindFramebuffer(GL_DRAW_FRAMEBUFFER, mDrawing->mGLFB);
        sm->setClearColor(gl::ColorF(0, 0, 0, 1));
        sm->setClearDepth(1);
        sm->setScissorTestEnabled(false);
        sm->setColorMask(true, true, true, true);
        sm->setDepthMask(true);
        mFunctionsGL->clear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    }

    drawWithTexture(buffer);
    presentScreen();
}

void DisplayOzone::flushGL()
{
    mFunctionsGL->flush();
    if (mEGL.hasExtension("EGL_KHR_fence_sync"))
    {
        const EGLint attrib[] = {EGL_SYNC_CONDITION_KHR,
                                 EGL_SYNC_PRIOR_COMMANDS_IMPLICIT_EXTERNAL_ARM, EGL_NONE};
        EGLSyncKHR fence = mEGL.createSyncKHR(EGL_SYNC_FENCE_KHR, attrib);
        if (fence)
        {
            // TODO(fjhenigman) Figure out the right way to use fences on
            // Mali GPU so maximize throughput and avoid crashes.
            // Polling the fence and sleeping is an attempt to reduce crashes
            // when interrupted by SIGINT.
            for (;;)
            {
                EGLint r = mEGL.clientWaitSyncKHR(fence, EGL_SYNC_FLUSH_COMMANDS_BIT_KHR, 0);
                if (r != EGL_TIMEOUT_EXPIRED_KHR)
                {
                    break;
                }
                usleep(99);
            }
            mEGL.destroySyncKHR(fence);
            return;
        }
    }
}

void DisplayOzone::terminate()
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

    if (mContext)
    {
        // Mesa might crash if you terminate EGL with a context current
        // then re-initialize EGL, so make our context not current.
        mEGL.makeCurrent(EGL_NO_SURFACE, EGL_NO_CONTEXT);
        mEGL.destroyContext(mContext);
        mContext = nullptr;
    }

    SafeDelete(mFunctionsGL);

    mEGL.terminate();

    drmModeFreeCrtc(mCRTC);

    int fd = gbm_device_get_fd(mGBM);
    gbm_device_destroy(mGBM);
    close(fd);
}

SurfaceImpl *DisplayOzone::createWindowSurface(const egl::Config *configuration,
                                               EGLNativeWindowType window,
                                               const egl::AttributeMap &attribs)
{
    Buffer *buffer =
        new Buffer(this, reinterpret_cast<const NativeWindow *>(window), GBM_BO_USE_RENDERING,
                   GBM_FORMAT_ARGB8888, DRM_FORMAT_ARGB8888, DRM_FORMAT_XRGB8888, true, true);
    if (!buffer || !buffer->initialize())
    {
        return nullptr;
    }
    return new WindowSurfaceOzone(getRenderer(), buffer);
}

// TODO(fjhenigman) Implement pbuffers and/or pixmaps.

SurfaceImpl *DisplayOzone::createPbufferSurface(const egl::Config *configuration,
                                                const egl::AttributeMap &attribs)
{
    UNIMPLEMENTED();
    return nullptr;
}

SurfaceImpl *DisplayOzone::createPbufferFromClientBuffer(const egl::Config *configuration,
                                                         EGLClientBuffer shareHandle,
                                                         const egl::AttributeMap &attribs)
{
    UNIMPLEMENTED();
    return nullptr;
}

SurfaceImpl *DisplayOzone::createPixmapSurface(const egl::Config *configuration,
                                               NativePixmapType nativePixmap,
                                               const egl::AttributeMap &attribs)
{
    UNIMPLEMENTED();
    return nullptr;
}

egl::Error DisplayOzone::getDevice(DeviceImpl **device)
{
    UNIMPLEMENTED();
    return egl::Error(EGL_BAD_DISPLAY);
}

EGLContext DisplayOzone::initializeContext(EGLConfig config, const egl::AttributeMap &eglAttributes)
{
    if (!(mEGL.majorVersion > 1 || mEGL.minorVersion > 4 ||
          mEGL.hasExtension("EGL_KHR_create_context")))
    {
        const EGLint attrib[] = {EGL_CONTEXT_CLIENT_VERSION, 2, EGL_NONE};
        return mEGL.createContext(config, EGL_NO_CONTEXT, attrib);
    }

    std::vector<gl::Version> versions;

    EGLint major = eglAttributes.get(EGL_PLATFORM_ANGLE_MAX_VERSION_MAJOR_ANGLE, EGL_DONT_CARE);
    EGLint minor = eglAttributes.get(EGL_PLATFORM_ANGLE_MAX_VERSION_MINOR_ANGLE, EGL_DONT_CARE);
    if (major != EGL_DONT_CARE && minor != EGL_DONT_CARE)
    {
        // If specific version requested, only try that one.
        versions.push_back(gl::Version(major, minor));
    }
    else
    {
        // Acceptable versions, from most to least preferred.
        versions.push_back(gl::Version(3, 0));
        versions.push_back(gl::Version(2, 0));
    }

    for (auto &version : versions)
    {
        const EGLint attrib[] = {EGL_CONTEXT_MAJOR_VERSION_KHR, UnsignedToSigned(version.major),
                                 EGL_CONTEXT_MINOR_VERSION_KHR, UnsignedToSigned(version.minor),
                                 EGL_NONE};
        auto context = mEGL.createContext(config, EGL_NO_CONTEXT, attrib);
        if (context != EGL_NO_CONTEXT)
        {
            return context;
        }
    }

    return EGL_NO_CONTEXT;
}

egl::ConfigSet DisplayOzone::generateConfigs() const
{
    egl::ConfigSet configs;

    egl::Config config;
    config.redSize     = 8;
    config.greenSize   = 8;
    config.blueSize    = 8;
    config.alphaSize   = 8;
    config.depthSize   = 24;
    config.stencilSize = 8;
    config.surfaceType = EGL_WINDOW_BIT;

    configs.add(config);
    return configs;
}

bool DisplayOzone::isDeviceLost() const
{
    return false;
}

bool DisplayOzone::testDeviceLost()
{
    return false;
}

egl::Error DisplayOzone::restoreLostDevice()
{
    UNIMPLEMENTED();
    return egl::Error(EGL_BAD_DISPLAY);
}

bool DisplayOzone::isValidNativeWindow(EGLNativeWindowType window) const
{
    return true;
}

std::string DisplayOzone::getVendorString() const
{
    return "";
}

egl::Error DisplayOzone::getDriverVersion(std::string *version) const
{
    *version = "";
    return egl::Error(EGL_SUCCESS);
}

egl::Error DisplayOzone::waitClient() const
{
    // TODO(fjhenigman) Implement this.
    return egl::Error(EGL_SUCCESS);
}

egl::Error DisplayOzone::waitNative(EGLint engine,
                                    egl::Surface *drawSurface,
                                    egl::Surface *readSurface) const
{
    // TODO(fjhenigman) Implement this.
    return egl::Error(EGL_SUCCESS);
}

void DisplayOzone::setSwapInterval(EGLSurface drawable, SwapControlData *data)
{
    ASSERT(data != nullptr);
}

const FunctionsGL *DisplayOzone::getFunctionsGL() const
{
    return mFunctionsGL;
}

void DisplayOzone::generateExtensions(egl::DisplayExtensions *outExtensions) const
{
    outExtensions->createContext        = true;
    outExtensions->createContextNoError = true;
}

void DisplayOzone::generateCaps(egl::Caps *outCaps) const
{
    outCaps->textureNPOT = true;
}
}  // namespace rx
