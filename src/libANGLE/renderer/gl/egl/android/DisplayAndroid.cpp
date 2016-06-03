//
// Copyright (c) 2016 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//

// DisplayAndroid.cpp: Android implementation of egl::Display

#include <android/native_window.h>

#include "common/debug.h"
#include "libANGLE/Display.h"
#include "libANGLE/renderer/gl/renderergl_utils.h"
#include "libANGLE/renderer/gl/egl/android/DisplayAndroid.h"
#include "libANGLE/renderer/gl/egl/FunctionsEGLDL.h"
#include "libANGLE/renderer/gl/egl/PbufferSurfaceEGL.h"
#include "libANGLE/renderer/gl/egl/WindowSurfaceEGL.h"

namespace rx
{

DisplayAndroid::DisplayAndroid() : DisplayEGL(), mDummyPbuffer(EGL_NO_SURFACE)
{
}

DisplayAndroid::~DisplayAndroid()
{
}

egl::Error DisplayAndroid::initialize(egl::Display *display)
{
    FunctionsEGLDL *egl = new FunctionsEGLDL();
    ANGLE_TRY(egl->initialize(display->getNativeDisplayId(), "libEGL.so"));
    mEGL = egl;

    gl::Version eglVersion(mEGL->majorVersion, mEGL->minorVersion);
    ASSERT(eglVersion >= gl::Version(1, 4));

    EGLint esBit = EGL_OPENGL_ES2_BIT;
    if (eglVersion >= gl::Version(1, 5))
    {
        esBit = EGL_OPENGL_ES3_BIT;
    }
    else if (mEGL->hasExtension("EGL_KHR_create_context"))
    {
        esBit = EGL_OPENGL_ES3_BIT_KHR;
    }

    // clang-format off
    mConfigAttribList =
    {
        // Choose RGBA8888
        EGL_COLOR_BUFFER_TYPE, EGL_RGB_BUFFER,
        EGL_RED_SIZE, 8,
        EGL_GREEN_SIZE, 8,
        EGL_BLUE_SIZE, 8,
        EGL_ALPHA_SIZE, 8,
        // EGL1.5 spec §2.2 says that depth, multisample and stencil buffer depths
        // must match for contexts to be compatible.
        EGL_DEPTH_SIZE, 24,
        EGL_STENCIL_SIZE, 8,
        EGL_SAMPLE_BUFFERS, 0,
        // Android doesn't support pixmaps
        EGL_SURFACE_TYPE, EGL_WINDOW_BIT | EGL_PBUFFER_BIT,
        EGL_CONFIG_CAVEAT, EGL_NONE,
        EGL_CONFORMANT, esBit,
        EGL_RENDERABLE_TYPE, esBit,
        EGL_NONE
    };
    // clang-format on
    EGLint numConfig;

    EGLBoolean success = mEGL->chooseConfig(mConfigAttribList.data(), &mConfig, 1, &numConfig);
    if (success == EGL_FALSE)
    {
        return egl::Error(mEGL->getError(), "eglChooseConfig failed");
    }

    ANGLE_TRY(initializeContext(display->getAttributeMap()));

    // clang-format off
    int dummyPbufferAttribs[] =
    {
            EGL_WIDTH, 1,
            EGL_HEIGHT, 1,
            EGL_NONE,
    };
    // clang-format on
    mDummyPbuffer = mEGL->createPbufferSurface(mConfig, dummyPbufferAttribs);
    if (mDummyPbuffer == EGL_NO_SURFACE)
    {
        return egl::Error(mEGL->getError(), "eglCreatePbufferSurface failed");
    }

    success = mEGL->makeCurrent(mDummyPbuffer, mContext);
    if (success == EGL_FALSE)
    {
        return egl::Error(mEGL->getError(), "eglMakeCurrent failed");
    }

    mFunctionsGL = mEGL->makeFunctionsGL();
    mFunctionsGL->initialize();

    return DisplayGL::initialize(display);
}

void DisplayAndroid::terminate()
{
    DisplayGL::terminate();

    EGLBoolean success = mEGL->makeCurrent(EGL_NO_SURFACE, EGL_NO_CONTEXT);
    if (success == EGL_FALSE)
    {
        ERR("eglMakeCurrent error 0x%04x", mEGL->getError());
    }

    if (mDummyPbuffer != EGL_NO_SURFACE)
    {
        success = mEGL->destroySurface(mDummyPbuffer);
        mDummyPbuffer = EGL_NO_SURFACE;
        if (success == EGL_FALSE)
        {
            ERR("eglDestroySurface error 0x%04x", mEGL->getError());
        }
    }

    if (mContext != EGL_NO_CONTEXT)
    {
        success = mEGL->destroyContext(mContext);
        mContext = EGL_NO_CONTEXT;
        if (success == EGL_FALSE)
        {
            ERR("eglDestroyContext error 0x%04x", mEGL->getError());
        }
    }

    egl::Error result = mEGL->terminate();
    if (result.isError())
    {
        ERR("eglTerminate error 0x%04x", result.getCode());
    }

    SafeDelete(mEGL);
    SafeDelete(mFunctionsGL);
}

SurfaceImpl *DisplayAndroid::createWindowSurface(const egl::SurfaceState &state,
                                                 const egl::Config *configuration,
                                                 EGLNativeWindowType window,
                                                 const egl::AttributeMap &attribs)
{
    EGLConfig config;
    EGLint numConfig;
    EGLBoolean success;

    // clang-format off
    const EGLint configAttribList[] =
    {
        EGL_CONFIG_ID, mConfigIds[configuration->configID],
        EGL_NONE
    };
    // clang-format on
    success = mEGL->chooseConfig(configAttribList, &config, 1, &numConfig);
    ASSERT(success && numConfig == 1);

    std::vector<EGLint> surfaceAttribList;
    for (egl::AttributeMap::const_iterator i = attribs.begin(); i != attribs.end(); i++)
    {
        surfaceAttribList.push_back(i->first);
        surfaceAttribList.push_back(i->second);
    }
    surfaceAttribList.push_back(EGL_NONE);

    return new WindowSurfaceEGL(state, mEGL, config, window, surfaceAttribList, mContext,
                                getRenderer());
}

SurfaceImpl *DisplayAndroid::createPbufferSurface(const egl::SurfaceState &state,
                                                  const egl::Config *configuration,
                                                  const egl::AttributeMap &attribs)
{
    EGLConfig config;
    EGLint numConfig;
    EGLBoolean success;

    // clang-format off
    const EGLint configAttribList[] =
    {
        EGL_CONFIG_ID, mConfigIds[configuration->configID],
        EGL_NONE
    };
    // clang-format on
    success = mEGL->chooseConfig(configAttribList, &config, 1, &numConfig);
    ASSERT(success && numConfig == 1);

    std::vector<EGLint> surfaceAttribList;
    for (egl::AttributeMap::const_iterator i = attribs.begin(); i != attribs.end(); i++)
    {
        surfaceAttribList.push_back(i->first);
        surfaceAttribList.push_back(i->second);
    }
    surfaceAttribList.push_back(EGL_NONE);

    return new PbufferSurfaceEGL(state, mEGL, config, surfaceAttribList, mContext, getRenderer());
}

SurfaceImpl *DisplayAndroid::createPbufferFromClientBuffer(const egl::SurfaceState &state,
                                                           const egl::Config *configuration,
                                                           EGLClientBuffer shareHandle,
                                                           const egl::AttributeMap &attribs)
{
    UNIMPLEMENTED();
    return nullptr;
}

SurfaceImpl *DisplayAndroid::createPixmapSurface(const egl::SurfaceState &state,
                                                 const egl::Config *configuration,
                                                 NativePixmapType nativePixmap,
                                                 const egl::AttributeMap &attribs)
{
    UNIMPLEMENTED();
    return nullptr;
}

ImageImpl *DisplayAndroid::createImage(EGLenum target,
                                       egl::ImageSibling *buffer,
                                       const egl::AttributeMap &attribs)
{
    UNIMPLEMENTED();
    return DisplayGL::createImage(target, buffer, attribs);
}

egl::ConfigSet DisplayAndroid::generateConfigs() const
{
    egl::ConfigSet configSet;
    mConfigIds.clear();

    EGLint numConfigs;
    EGLBoolean success = mEGL->chooseConfig(mConfigAttribList.data(), nullptr, 0, &numConfigs);
    ASSERT(success == EGL_TRUE && numConfigs > 0);

    EGLConfig *configs = new EGLConfig[numConfigs];
    EGLint numConfigs2;
    success = mEGL->chooseConfig(mConfigAttribList.data(), configs, numConfigs, &numConfigs2);
    ASSERT(success == EGL_TRUE && numConfigs2 == numConfigs);

    for (int i = 0; i < numConfigs; i++)
    {
        egl::Config config;
        EGLint value;

#define GET_ATTRIB(member, attrib)                                   \
        success = mEGL->getConfigAttrib(configs[i], attrib, &value); \
        ASSERT(success == EGL_TRUE);                                 \
        config.member = value;

        GET_ATTRIB(bufferSize, EGL_BUFFER_SIZE);
        GET_ATTRIB(redSize, EGL_RED_SIZE);
        GET_ATTRIB(greenSize, EGL_GREEN_SIZE);
        GET_ATTRIB(blueSize, EGL_BLUE_SIZE);
        GET_ATTRIB(luminanceSize, EGL_LUMINANCE_SIZE);
        GET_ATTRIB(alphaSize, EGL_ALPHA_SIZE);
        GET_ATTRIB(alphaMaskSize, EGL_ALPHA_MASK_SIZE);
        GET_ATTRIB(bindToTextureRGB, EGL_BIND_TO_TEXTURE_RGB);
        GET_ATTRIB(bindToTextureRGBA, EGL_BIND_TO_TEXTURE_RGBA);
        GET_ATTRIB(colorBufferType, EGL_COLOR_BUFFER_TYPE);
        GET_ATTRIB(configCaveat, EGL_CONFIG_CAVEAT);
        GET_ATTRIB(configID, EGL_CONFIG_ID);
        GET_ATTRIB(conformant, EGL_CONFORMANT);
        GET_ATTRIB(depthSize, EGL_DEPTH_SIZE);
        GET_ATTRIB(level, EGL_LEVEL);
        GET_ATTRIB(maxPBufferWidth, EGL_MAX_PBUFFER_WIDTH);
        GET_ATTRIB(maxPBufferHeight, EGL_MAX_PBUFFER_HEIGHT);
        GET_ATTRIB(maxPBufferPixels, EGL_MAX_PBUFFER_PIXELS);
        GET_ATTRIB(maxSwapInterval, EGL_MAX_SWAP_INTERVAL);
        GET_ATTRIB(minSwapInterval, EGL_MIN_SWAP_INTERVAL);
        GET_ATTRIB(nativeRenderable, EGL_NATIVE_RENDERABLE);
        GET_ATTRIB(nativeVisualID, EGL_NATIVE_VISUAL_ID);
        GET_ATTRIB(nativeVisualType, EGL_NATIVE_VISUAL_TYPE);
        GET_ATTRIB(renderableType, EGL_RENDERABLE_TYPE);
        GET_ATTRIB(sampleBuffers, EGL_SAMPLE_BUFFERS);
        GET_ATTRIB(samples, EGL_SAMPLES);
        GET_ATTRIB(stencilSize, EGL_STENCIL_SIZE);
        GET_ATTRIB(surfaceType, EGL_SURFACE_TYPE);
        GET_ATTRIB(transparentType, EGL_TRANSPARENT_TYPE);
        GET_ATTRIB(transparentRedValue, EGL_TRANSPARENT_RED_VALUE);
        GET_ATTRIB(transparentGreenValue, EGL_TRANSPARENT_GREEN_VALUE);
        GET_ATTRIB(transparentBlueValue, EGL_TRANSPARENT_BLUE_VALUE);

        if (config.colorBufferType == EGL_RGB_BUFFER)
        {
            if (config.redSize == 8 && config.greenSize == 8 && config.blueSize == 8 &&
                config.alphaSize == 8)
            {
                config.renderTargetFormat = GL_RGBA8;
            }
            else if (config.redSize == 8 && config.greenSize == 8 && config.blueSize == 8 &&
                     config.alphaSize == 0)
            {
                config.renderTargetFormat = GL_RGB8;
            }
            else if (config.redSize == 5 && config.greenSize == 6 && config.blueSize == 5 &&
                     config.alphaSize == 0)
            {
                config.renderTargetFormat = GL_RGB565;
            }
            else
            {
                UNREACHABLE();
            }
        }
        else
        {
            UNREACHABLE();
        }

        if (config.depthSize == 0 && config.stencilSize == 0)
        {
            config.depthStencilFormat = GL_ZERO;
        }
        else if (config.depthSize == 16 && config.stencilSize == 0)
        {
            config.depthStencilFormat = GL_DEPTH_COMPONENT16;
        }
        else if (config.depthSize == 24 && config.stencilSize == 0)
        {
            config.depthStencilFormat = GL_DEPTH_COMPONENT24;
        }
        else if (config.depthSize == 24 && config.stencilSize == 8)
        {
            config.depthStencilFormat = GL_DEPTH24_STENCIL8;
        }
        else if (config.depthSize == 0 && config.stencilSize == 8)
        {
            config.depthStencilFormat = GL_STENCIL_INDEX8;
        }
        else
        {
            UNREACHABLE();
        }

        config.matchNativePixmap = EGL_NONE;
        config.optimalOrientation = 0;

        int internalId = configSet.add(config);
        mConfigIds[internalId] = config.configID;
    }

    delete[] configs;
    return configSet;
}

bool DisplayAndroid::isDeviceLost() const
{
    return false;
}

bool DisplayAndroid::testDeviceLost()
{
    return false;
}

egl::Error DisplayAndroid::restoreLostDevice()
{
    UNIMPLEMENTED();
    return egl::Error(EGL_SUCCESS);
}

bool DisplayAndroid::isValidNativeWindow(EGLNativeWindowType window) const
{
    return ANativeWindow_getFormat(window) >= 0;
}

egl::Error DisplayAndroid::getDevice(DeviceImpl **device)
{
    UNIMPLEMENTED();
    return egl::Error(EGL_SUCCESS);
}

egl::Error DisplayAndroid::waitClient() const
{
    UNIMPLEMENTED();
    return egl::Error(EGL_SUCCESS);
}

egl::Error DisplayAndroid::waitNative(EGLint engine,
                                      egl::Surface *drawSurface,
                                      egl::Surface *readSurface) const
{
    UNIMPLEMENTED();
    return egl::Error(EGL_SUCCESS);
}

egl::Error DisplayAndroid::getDriverVersion(std::string *version) const
{
    VendorID vendor = GetVendorID(mFunctionsGL);

    switch (vendor)
    {
        case VENDOR_ID_QUALCOMM:
            *version = reinterpret_cast<const char *>(mFunctionsGL->getString(GL_VERSION));
            return egl::Error(EGL_SUCCESS);
        default:
            *version = "";
            return egl::Error(EGL_SUCCESS);
    }
}

}  // namespace rx
