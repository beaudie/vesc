//
// Copyright (c) 2016 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//

// DisplayDRM.cpp: DRM/KMS/GBM/Surfaceless implementation of egl::Display

#include "libANGLE/renderer/gl/cros/DisplayDRM.h"

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
#include "libANGLE/renderer/gl/cros/PbufferSurfaceCros.h"
#include "libANGLE/renderer/gl/cros/WindowSurfaceCros.h"

#define SHADER_SOURCE(...) #__VA_ARGS__

#ifndef EGL_SYNC_PRIOR_COMMANDS_IMPLICIT_EXTERNAL_ARM
#define EGL_SYNC_PRIOR_COMMANDS_IMPLICIT_EXTERNAL_ARM 0x328A
#endif

#ifndef EGL_NO_CONFIG_MESA
#define EGL_NO_CONFIG_MESA ((EGLConfig)0)
#endif

namespace rx
{

DisplayDRM::Buffer::Buffer(DisplayDRM *display, const NativeWindow *native, uint32_t useFlags, uint32_t gbmFormat, uint32_t drmFormat, uint32_t drmFormatFB, int depthBits, int stencilBits)
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

DisplayDRM::Buffer::~Buffer()
{
    mDisplay->mFunctionsGL->deleteFramebuffers(1, &mGLFB);
    reset();
}

void DisplayDRM::Buffer::reset()
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

static EGLint unsignedToSigned(uint32_t u)
{
    return *reinterpret_cast<const EGLint *>(&u);
}

bool DisplayDRM::Buffer::resize()
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

    const EGLint attr[] =
    {
        EGL_WIDTH, mNative->width,
        EGL_HEIGHT, mNative->height,
        EGL_LINUX_DRM_FOURCC_EXT, unsignedToSigned(mDRMFormat),
        EGL_DMA_BUF_PLANE0_FD_EXT, mDMABuf,
        EGL_DMA_BUF_PLANE0_PITCH_EXT, unsignedToSigned(gbm_bo_get_stride(mBO)),
        EGL_DMA_BUF_PLANE0_OFFSET_EXT, 0,
        EGL_NONE,
    };

    mImage = mDisplay->mEGL.createImageKHR(EGL_NO_CONTEXT, EGL_LINUX_DMA_BUF_EXT, NULL, attr);
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

    if (mDepthBits || mStencilBits)
    {
        gl->genRenderbuffers(1, &mDSBuffer);
        sm->bindRenderbuffer(GL_RENDERBUFFER, mDSBuffer);
        gl->renderbufferStorage(GL_RENDERBUFFER, GL_DEPTH24_STENCIL8_OES, mNative->width, mNative->height);
    }

    if (mDepthBits)
    {
        gl->framebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, mDSBuffer);
    }

    if (mStencilBits)
    {
        gl->framebufferRenderbuffer(GL_FRAMEBUFFER, GL_STENCIL_ATTACHMENT, GL_RENDERBUFFER, mDSBuffer);
    }

    mWidth = mNative->width;
    mHeight = mNative->height;
    return true;
}

bool DisplayDRM::Buffer::initialize()
{
    mDisplay->mFunctionsGL->genFramebuffers(1, &mGLFB);
    return resize();
}

GLuint DisplayDRM::Buffer::getTexture()
{
    // TODO(fjhenigman) Try not to create a new texture every time.  That already works on Intel and almost works on ARM.
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
    return mTexture;
}

uint32_t DisplayDRM::Buffer::getDRMFB()
{
    if (!mHasDRMFB)
    {
        int fd = gbm_device_get_fd(mDisplay->mGBM);
        uint32_t handles[4] = { gbm_bo_get_handle(mBO).u32 };
        uint32_t pitches[4] = { gbm_bo_get_stride(mBO) };
        uint32_t offsets[4] = { 0 };
        if (drmModeAddFB2(fd, mWidth, mHeight, mDRMFormatFB, handles, pitches, offsets, &mDRMFB, 0))
        {
            std::cout << "drmModeAddFB2 failed\n" << std::flush;
        }
        else
        {
            mHasDRMFB = true;
        }
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
    resize();
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
    FunctionsGLEGL(const FunctionsEGL& egl)
      : mEGL(egl)
    {
    }

    virtual ~FunctionsGLEGL()
    {
    }

  private:
    void *loadProcAddress(const std::string &function) override
    {
        return mEGL.getProcAddress(function.c_str());
    }

    const FunctionsEGL& mEGL;
};

DisplayDRM::DisplayDRM()
    : mFunctionsGL(nullptr),
      mContextConfig(nullptr),
      mContext(nullptr),
      mIsMesa(false),
      mHasMultisample(false),
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

    mConnector = NULL;
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
        int n = ChooseCRTC(fd, resources->count_crtcs, mConnector);
        if (n < 0)
        {
            continue;
        }
        mCRTC = drmModeGetCrtc(fd, resources->crtcs[n]);
    }
    drmModeFreeResources(resources);

    if (mCRTC)
    {
        mNative.width = mMode->hdisplay;
        mNative.height = mMode->vdisplay;
    }
    else if (!monitorConnected)
    {
        // Even though there is no screen to show it, we still draw
        // everything the same as if there were a screen, so we need
        // some arbitrary size for the buffer.
        mNative.width = 1280;
        mNative.height = 1024;
    }
    else
    {
        return egl::Error(EGL_NOT_INITIALIZED, "Failed to choose mode/crtc.");
    }

    std::string eglInitError;
    if (!mEGL.initialize(display->getNativeDisplayId(), &eglInitError))
    {
        return egl::Error(EGL_NOT_INITIALIZED, eglInitError.c_str());
    }

    const char *necessaryExtensions[] =
    {
        "EGL_KHR_image_base",
        "EGL_EXT_image_dma_buf_import",
        "EGL_KHR_surfaceless_context",
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
        reinterpret_cast<const char*>(mFunctionsGL->getString(GL_RENDERER));
    mIsMesa = rendererString.find("Mesa") != std::string::npos;

    return DisplayGL::initialize(display);
}

void DisplayDRM::pageFlipHandler(int fd, unsigned int sequence, unsigned int tv_sec, unsigned int tv_usec, void *data)
{
    DisplayDRM *display = reinterpret_cast<DisplayDRM *>(data);
    uint64_t tv = tv_sec;
    display->pageFlipHandler(sequence, tv * 1000000 + tv_usec);
}

void DisplayDRM::pageFlipHandler(unsigned int sequence, uint64_t tv)
{
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
            std::cout << "poll failed " << errno << std::endl << std::flush;
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
        flushGL();
        if (mSetCRTC)
        {
            if (drmModeSetCrtc(fd, mCRTC->crtc_id, mDrawing->getDRMFB(), 0, 0, &mConnector->connector_id, 1, mMode))
            {
                std::cout << "set crtc failed: " << errno << " " << strerror(errno) << std::endl << std::flush;
            }
            mSetCRTC = false;
        }
        if (drmModePageFlip(fd, mCRTC->crtc_id, mDrawing->getDRMFB(), DRM_MODE_PAGE_FLIP_EVENT, this))
        {
            std::cout << "page flip failed: " << errno << " " << strerror(errno) << std::endl << std::flush;
        }
        mPending = mDrawing;
        mDrawing = nullptr;
    }
}

GLuint DisplayDRM::makeShader(GLuint type, const char *src)
{
    FunctionsGL *gl = mFunctionsGL;
    GLuint shader = gl->createShader(type);
    gl->shaderSource(shader, 1, &src, NULL);
    gl->compileShader(shader);
    if (0)
    {
        GLchar buf[999];
        GLsizei len;
        GLint compiled;
        gl->getShaderInfoLog(shader, sizeof(buf), &len, buf);
        gl->getShaderiv(shader, GL_COMPILE_STATUS, &compiled);
        std::cout << "shader compiled=" << compiled << " log:\n" << buf << std::endl << std::flush;
    }
    return shader;
}

void DisplayDRM::drawWithTexture(Buffer *buffer)
{
    FunctionsGL *gl = mFunctionsGL;
    StateManagerGL *sm = getRenderer()->getStateManager();

    if (!mProgram)
    {

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
        gl->genBuffers(1, &mVertexBuffer);
        sm->bindBuffer(GL_ARRAY_BUFFER, mVertexBuffer);
        gl->bufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

        // window border triangle strip
        const GLuint borderStrip[] = { 5,0,4,2,6,3,7,1,5,0 };

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
    GLuint tex = buffer->getTexture();
    sm->bindTexture(GL_TEXTURE_2D, tex);
    gl->vertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, 0);
    gl->enableVertexAttribArray(0);
    sm->bindFramebuffer(GL_DRAW_FRAMEBUFFER, mDrawing->mGLFB);
    gl->drawArrays(GL_TRIANGLE_STRIP, 0, 4);
    gl->drawElements(GL_TRIANGLE_STRIP, 10, GL_UNSIGNED_INT, 0);
    sm->deleteTexture(tex);
}

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

void DisplayDRM::drawBuffer(Buffer *buffer)
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
            mUnused = nullptr;
        }
        else
        {
            mDrawing = new Buffer(this, &mNative, GBM_BO_USE_RENDERING|GBM_BO_USE_SCANOUT, GBM_FORMAT_ARGB8888, DRM_FORMAT_ARGB8888, DRM_FORMAT_XRGB8888, true, true);
            if (!mDrawing || !mDrawing->initialize())
            {
                return;
            }
        }

        StateManagerGL *sm = getRenderer()->getStateManager();
        sm->bindFramebuffer(GL_DRAW_FRAMEBUFFER, mDrawing->mGLFB);
        sm->setClearColor(gl::ColorF(.5,0,.5,1));
        sm->setClearDepth(1);
        sm->setScissorTestEnabled(false);
        sm->setColorMask(true, true, true, true);
        sm->setDepthMask(true);
        mFunctionsGL->clear(GL_COLOR_BUFFER_BIT|GL_DEPTH_BUFFER_BIT);
    }

    drawWithTexture(buffer);
    presentScreen();
}

void DisplayDRM::flushGL()
{
    mFunctionsGL->flush();
    if (mEGL.hasExtension("EGL_KHR_fence_sync"))
    {
        const EGLint attrib[] =
        {
            EGL_SYNC_CONDITION_KHR,
            EGL_SYNC_PRIOR_COMMANDS_IMPLICIT_EXTERNAL_ARM,
            EGL_NONE
        };
        EGLSyncKHR fence = mEGL.createSyncKHR(EGL_SYNC_FENCE_KHR, attrib);
        if (fence)
        {
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

    if (mCRTC)
    {
        drmModeFreeCrtc(mCRTC);
    }

    int fd = gbm_device_get_fd(mGBM);
    gbm_device_destroy(mGBM);
    close(fd);
}

SurfaceImpl *DisplayDRM::createWindowSurface(const egl::Config *configuration,
                                             EGLNativeWindowType window,
                                             const egl::AttributeMap &attribs)
{
    Buffer *buffer = new Buffer(this, reinterpret_cast<const NativeWindow *>(window), GBM_BO_USE_RENDERING, GBM_FORMAT_ARGB8888, DRM_FORMAT_ARGB8888, DRM_FORMAT_XRGB8888, true, true);
    if (!buffer || !buffer->initialize())
    {
        return nullptr;
    }
    return new WindowSurfaceCros(getRenderer(), buffer);
}

SurfaceImpl *DisplayDRM::createPbufferSurface(const egl::Config *configuration,
                                              const egl::AttributeMap &attribs)
{
    UNIMPLEMENTED();
    return nullptr;
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
    if (!mEGL.hasExtension("EGL_KHR_create_context"))
    {
        const EGLint attrib[] =
        {
            EGL_CONTEXT_CLIENT_VERSION, 2,
            EGL_NONE
        };
        return mEGL.createContext(config, EGL_NO_CONTEXT, attrib);
    }

    std::vector<gl::Version> versions;
    versions.push_back(gl::Version(3, 0));
    versions.push_back(gl::Version(2, 0));

    // Give first priority to requested version, if any.
    EGLint major =
        eglAttributes.get(EGL_PLATFORM_ANGLE_MAX_VERSION_MAJOR_ANGLE, EGL_DONT_CARE);
    EGLint minor =
        eglAttributes.get(EGL_PLATFORM_ANGLE_MAX_VERSION_MINOR_ANGLE, EGL_DONT_CARE);
    if (major != EGL_DONT_CARE && minor != EGL_DONT_CARE)
    {
        versions.insert(versions.begin(), gl::Version(major, minor));
    }

    for (auto &version : versions)
    {
        const EGLint attrib[] =
        {
            EGL_CONTEXT_MAJOR_VERSION_KHR, unsignedToSigned(version.major),
            EGL_CONTEXT_MINOR_VERSION_KHR, unsignedToSigned(version.minor),
            EGL_NONE
        };
        auto context = mEGL.createContext(config, EGL_NO_CONTEXT, attrib);
        if (context != EGL_NO_CONTEXT)
        {
            return context;
        }
    }

    return EGL_NO_CONTEXT;
}

egl::ConfigSet DisplayDRM::generateConfigs() const
{
    egl::ConfigSet configs;

    egl::Config config;
    config.redSize = 8;
    config.greenSize = 8;
    config.blueSize = 8;
    config.alphaSize = 8;
    config.depthSize = 24;
    config.stencilSize = 8;
    config.surfaceType = EGL_WINDOW_BIT;

    configs.add(config);
    return configs;
}

bool DisplayDRM::isDeviceLost() const
{
    return false;
}

bool DisplayDRM::testDeviceLost()
{
    return false;
}

egl::Error DisplayDRM::restoreLostDevice()
{
    UNIMPLEMENTED();
    return egl::Error(EGL_BAD_DISPLAY);
}

bool DisplayDRM::isValidNativeWindow(EGLNativeWindowType window) const
{
    return true;
}

std::string DisplayDRM::getVendorString() const
{
    return "";
}

egl::Error DisplayDRM::waitClient() const
{
    return egl::Error(EGL_SUCCESS);
}

egl::Error DisplayDRM::waitNative(EGLint engine, egl::Surface *drawSurface, egl::Surface *readSurface) const
{
    return egl::Error(EGL_SUCCESS);
}

void DisplayDRM::setSwapInterval(EGLSurface drawable, SwapControlData *data)
{
    ASSERT(data != nullptr);
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
    outCaps->textureNPOT = true;
}

}
