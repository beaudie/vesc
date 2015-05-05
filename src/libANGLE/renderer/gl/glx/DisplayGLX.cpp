//
// Copyright (c) 2015 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//

// DisplayGLX.h: GLX implementation of egl::Display

#define GLX_GLXEXT_PROTOTYPES
#include "libANGLE/renderer/gl/glx/DisplayGLX.h"

#include <GL/glxext.h>
#include <EGL/eglext.h>
#include <algorithm>

#include "common/debug.h"
#include "libANGLE/Config.h"
#include "libANGLE/Display.h"
#include "libANGLE/Surface.h"
#include "libANGLE/renderer/gl/renderergl_utils.h"
#include "libANGLE/renderer/gl/glx/WindowSurfaceGLX.h"

namespace rx
{

class FunctionsGLX : public FunctionsGL
{
  public:
    FunctionsGLX()
    {
    }

    virtual ~FunctionsGLX()
    {
    }

  private:
    void *loadProcAddress(const std::string &function) override
    {
        return reinterpret_cast<void*>(glXGetProcAddress(reinterpret_cast<const unsigned char*>(function.c_str())));
    }
};

DisplayGLX::DisplayGLX()
    : DisplayGL(),
      mFunctionsGL(nullptr),
      mContext(nullptr),
      mEGLDisplay(nullptr),
      mXDisplay(nullptr)
{
}

DisplayGLX::~DisplayGLX()
{
}

egl::Error DisplayGLX::initialize(egl::Display *display)
{
    mEGLDisplay = display;
    mXDisplay = display->getNativeDisplayId();

    // Check we have GLX
    {
        int errorBase;
        int eventBase;
        if (!glXQueryExtension(mXDisplay, &errorBase, &eventBase))
        {
            return egl::Error(EGL_NOT_INITIALIZED, "GLX is not present.");
        }
    }

    // Check we have a supported version of GLX
    {
        int major;
        int minor;
        if (!glXQueryVersion(mXDisplay, &major, &minor))
        {
            return egl::Error(EGL_NOT_INITIALIZED, "Could not query the GLX version.");
        }
        if (major != 1 || minor < 3)
        {
            return egl::Error(EGL_NOT_INITIALIZED, "Unsupported GLX version (requires at least 1.3).");
        }

        const char *extensions = glXQueryExtensionsString(mXDisplay, DefaultScreen(mXDisplay));
        ASSERT(extensions);
        mGLXExtensions = ParseGLStyleExtensions(extensions);

        if (minor == 3 && !hasGLXExtension("GLX_ARB_get_proc_address"))
        {
            return egl::Error(EGL_NOT_INITIALIZED, "GLX doesn't support glXGetProcAddress.");
        }
        if (minor == 3 && !hasGLXExtension("GLX_ARB_multisample"))
        {
            return egl::Error(EGL_NOT_INITIALIZED, "GLX doesn't support ARB_multisample.");
        }
        // Require ARB_create_context which has been supported since Mesa 9 unconditionnaly
        // and is present in Mesa 8 in an almost always on compile flag. Also assume proprietary
        // drivers have it.
        if (!hasGLXExtension("GLX_ARB_create_context"))
        {
            return egl::Error(EGL_NOT_INITIALIZED, "GLX doesn't support ARB_create_context.");
        }
    }

    GLXFBConfig contextConfig;
    // When glXMakeCurrent is called the visual of the context FBConfig and of
    // the drawable must match. This means that when generating the list of EGL
    // configs, they must all have the same visual id as our unique GL context.
    // Here we find a GLX framebuffer config we like to create our GL context
    // so that we are sure there is a decent config given back to the application
    // when it queries EGL.
    {
        int nConfigs;
        int attribList[] =
        {
            // We want at least RGBA8 and DEPTH24_STENCIL8
            GLX_RED_SIZE, 8,
            GLX_GREEN_SIZE, 8,
            GLX_BLUE_SIZE, 8,
            GLX_ALPHA_SIZE, 8,
            GLX_DEPTH_SIZE, 24,
            GLX_STENCIL_SIZE, 8,
            // We want RGBA rendering (vs COLOR_INDEX)
            GLX_RENDER_TYPE, GLX_RGBA_BIT,
            // All of these must be supported for full EGL support
            GLX_DRAWABLE_TYPE, GLX_WINDOW_BIT | GLX_PBUFFER_BIT | GLX_PIXMAP_BIT,
            // This makes sure the config have an associated visual Id
            GLX_X_RENDERABLE, True,
            GLX_CONFIG_CAVEAT, GLX_NONE,
            None, None
        };
        GLXFBConfig* candidates = glXChooseFBConfig(mXDisplay, DefaultScreen(mXDisplay), attribList, &nConfigs);
        if (nConfigs == 0)
        {
            XFree(candidates);
            return egl::Error(EGL_NOT_INITIALIZED, "Could not find a decent GLX FBConfig to create the context.");
        }
        contextConfig = candidates[0];
        XFree(candidates);
    }
    mContextVisualId = getGLXFBConfigAttrib(contextConfig, GLX_VISUAL_ID);

    mContext = glXCreateContextAttribsARB(mXDisplay, contextConfig, nullptr, True, nullptr);
    if (!mContext)
    {
        return egl::Error(EGL_NOT_INITIALIZED, "Could not create GL context.");
    }

    // FunctionsGL and DisplayGL need to make a few GL calls, for example to
    // query the version of the context so we need to make the context current.
    // glXMakeCurrent requires a GLXDrawable so we create a temporary Pbuffer
    // (of size 0, 0) for the duration of these calls.
    GLXPbuffer pbuffer = glXCreatePbuffer(mXDisplay, contextConfig, nullptr);
    glXMakeCurrent(mXDisplay, pbuffer, mContext);

    mFunctionsGL = new FunctionsGLX;
    mFunctionsGL->initialize();

    egl::Error result = DisplayGL::initialize(display);

    glXMakeCurrent(mXDisplay, None, nullptr);
    glXDestroyPbuffer(mXDisplay, pbuffer);

    return result;
}

void DisplayGLX::terminate()
{
    DisplayGL::terminate();

    if (mContext)
    {
        glXDestroyContext(mXDisplay, mContext);
        mContext = nullptr;
    }

    SafeDelete(mFunctionsGL);
}

SurfaceImpl *DisplayGLX::createWindowSurface(const egl::Config *configuration,
                                             EGLNativeWindowType window,
                                             const egl::AttributeMap &attribs)
{
    ASSERT(configIdToGLXConfig.count(configuration->configID) > 0);
    GLXFBConfig fbConfig = configIdToGLXConfig[configuration->configID];

    return new WindowSurfaceGLX(window, mXDisplay, mContext, fbConfig);
}

SurfaceImpl *DisplayGLX::createPbufferSurface(const egl::Config *configuration,
                                              const egl::AttributeMap &attribs)
{
    //TODO(cwallez) WGL implements it
    UNIMPLEMENTED();
    return nullptr;
}

SurfaceImpl* DisplayGLX::createPbufferFromClientBuffer(const egl::Config *configuration,
                                                       EGLClientBuffer shareHandle,
                                                       const egl::AttributeMap &attribs)
{
    UNIMPLEMENTED();
    return nullptr;
}

SurfaceImpl *DisplayGLX::createPixmapSurface(const egl::Config *configuration,
                                             NativePixmapType nativePixmap,
                                             const egl::AttributeMap &attribs)
{
    UNIMPLEMENTED();
    return nullptr;
}

egl::Error DisplayGLX::getDevice(DeviceImpl **device)
{
    UNIMPLEMENTED();
    return egl::Error(EGL_BAD_DISPLAY);
}

egl::ConfigSet DisplayGLX::generateConfigs() const
{
    egl::ConfigSet configs;
    configIdToGLXConfig.clear();

    // GLX_EXT_texture_from_pixmap is required for the "bind to rgb(a)" attributes
    bool hasTextureFromPixmap = hasGLXExtension("GLX_EXT_texture_from_pixmap");

    int glxConfigCount;
    GLXFBConfig *glxConfigs = glXGetFBConfigs(mXDisplay, DefaultScreen(mXDisplay), &glxConfigCount);

    for (int i = 0; i < glxConfigCount; i++)
    {
        GLXFBConfig glxConfig = glxConfigs[i];
        egl::Config config;

        // Native stuff
        int visualId = getGLXFBConfigAttrib(glxConfig, GLX_VISUAL_ID);
        if (visualId != mContextVisualId)
        {
            // Filter out the configs that are incompatible with our GL context
            continue;
        }
        config.nativeVisualID = visualId;
        config.nativeVisualType = getGLXFBConfigAttrib(glxConfig, GLX_X_VISUAL_TYPE);
        config.nativeRenderable = EGL_TRUE;

        // Buffer sizes
        config.redSize = getGLXFBConfigAttrib(glxConfig, GLX_RED_SIZE);
        config.greenSize = getGLXFBConfigAttrib(glxConfig, GLX_GREEN_SIZE);
        config.blueSize = getGLXFBConfigAttrib(glxConfig, GLX_BLUE_SIZE);
        config.alphaSize = getGLXFBConfigAttrib(glxConfig, GLX_ALPHA_SIZE);
        config.depthSize = getGLXFBConfigAttrib(glxConfig, GLX_DEPTH_SIZE);
        config.stencilSize = getGLXFBConfigAttrib(glxConfig, GLX_STENCIL_SIZE);

        config.colorBufferType = EGL_RGB_BUFFER;
        config.luminanceSize = 0;
        config.alphaMaskSize = 0;

        config.bufferSize = config.redSize + config.greenSize + config.blueSize + config.alphaSize;

        // Transparency
        if (getGLXFBConfigAttrib(glxConfig, GLX_TRANSPARENT_TYPE) == GLX_TRANSPARENT_RGB)
        {
            config.transparentType = EGL_TRANSPARENT_RGB;
            config.transparentRedValue = getGLXFBConfigAttrib(glxConfig, GLX_TRANSPARENT_RED_VALUE);
            config.transparentGreenValue = getGLXFBConfigAttrib(glxConfig, GLX_TRANSPARENT_GREEN_VALUE);
            config.transparentBlueValue = getGLXFBConfigAttrib(glxConfig, GLX_TRANSPARENT_BLUE_VALUE);
        }
        else
        {
            config.transparentType = EGL_NONE;
        }

        // Pbuffer
        config.maxPBufferWidth = getGLXFBConfigAttrib(glxConfig, GLX_MAX_PBUFFER_WIDTH);
        config.maxPBufferHeight = getGLXFBConfigAttrib(glxConfig, GLX_MAX_PBUFFER_HEIGHT);
        config.maxPBufferPixels = getGLXFBConfigAttrib(glxConfig, GLX_MAX_PBUFFER_PIXELS);

        // Caveat
        config.configCaveat = EGL_NONE;

        int caveat = getGLXFBConfigAttrib(glxConfig, GLX_CONFIG_CAVEAT);
        if (caveat == GLX_SLOW_CONFIG)
        {
            config.configCaveat = EGL_SLOW_CONFIG;
        }
        else if (caveat == GLX_NON_CONFORMANT_CONFIG)
        {
            continue;
        }

        // Misc
        config.sampleBuffers = getGLXFBConfigAttrib(glxConfig, GLX_SAMPLE_BUFFERS);
        config.samples = getGLXFBConfigAttrib(glxConfig, GLX_SAMPLES);
        config.level = getGLXFBConfigAttrib(glxConfig, GLX_LEVEL);

        config.bindToTextureRGB = EGL_FALSE;
        config.bindToTextureRGBA = EGL_FALSE;
        if (hasTextureFromPixmap)
        {
            config.bindToTextureRGB = getGLXFBConfigAttrib(glxConfig, GLX_BIND_TO_TEXTURE_RGB_EXT);
            config.bindToTextureRGBA = getGLXFBConfigAttrib(glxConfig, GLX_BIND_TO_TEXTURE_RGBA_EXT);
        }

        int glxDrawable = getGLXFBConfigAttrib(glxConfig, GLX_DRAWABLE_TYPE);
        config.surfaceType = 0 |
            (glxDrawable & GLX_WINDOW_BIT ? EGL_WINDOW_BIT : 0) |
            (glxDrawable & GLX_PBUFFER_BIT ? EGL_PBUFFER_BIT : 0) |
            (glxDrawable & GLX_PIXMAP_BIT ? EGL_PIXMAP_BIT : 0);

        // In GLX_EXT_swap_control querying these is done on a GLXWindow so we just set a default value.
        config.maxSwapInterval = 1;
        config.minSwapInterval = 1;
        // TODO(cwallez) wildly guessing these formats, another TODO says they should be removed anyway
        config.renderTargetFormat = GL_RGBA8;
        config.depthStencilFormat = GL_DEPTH24_STENCIL8;
        // TODO(cwallez) Fill after determining the GL version we are using and what ES version it supports
        config.conformant = EGL_OPENGL_ES2_BIT | EGL_OPENGL_ES3_BIT_KHR;
        config.renderableType = EGL_OPENGL_ES2_BIT | EGL_OPENGL_ES3_BIT_KHR;
        // TODO(cwallez) I have no idea what this is
        config.matchNativePixmap = EGL_NONE;

        int id = configs.add(config);
        configIdToGLXConfig[id] = glxConfig;
    }
    return configs;
}

bool DisplayGLX::isDeviceLost() const
{
    // UNIMPLEMENTED();
    return false;
}

bool DisplayGLX::testDeviceLost()
{
    // UNIMPLEMENTED();
    return false;
}

egl::Error DisplayGLX::restoreLostDevice()
{
    UNIMPLEMENTED();
    return egl::Error(EGL_BAD_DISPLAY);
}

bool DisplayGLX::isValidNativeWindow(EGLNativeWindowType window) const
{
    // There is no function in Xlib to check the validity of a Window directly.
    // However a small number of functions used to obtain window information
    // return a status code (0 meaning failure) and guarantee that they will
    // fail if the window doesn't exist (the rational is that these function
    // are used by window managers). Out of these function we use XQueryTree
    // as it seems to be the simplest; a drawback is that it will allocate
    // memory for the list of children, becasue we use a child window for
    // WindowSurface.
    Window root;
    Window parent;
    Window *children = nullptr;
    unsigned nChildren;
    int status = XQueryTree(mXDisplay, window, &root, &parent, &children, &nChildren);
    if (children)
    {
        XFree(children);
    }
    return status != 0;
}

std::string DisplayGLX::getVendorString() const
{
    // UNIMPLEMENTED();
    return "";
}

const FunctionsGL *DisplayGLX::getFunctionsGL() const
{
    return mFunctionsGL;
}

void DisplayGLX::generateExtensions(egl::DisplayExtensions *outExtensions) const
{
    // UNIMPLEMENTED();
}

void DisplayGLX::generateCaps(egl::Caps *outCaps) const
{
    // UNIMPLEMENTED();
    outCaps->textureNPOT = true;
}

bool DisplayGLX::hasGLXExtension(const char *extension) const
{
    return std::find(mGLXExtensions.begin(), mGLXExtensions.end(), extension) != mGLXExtensions.end();
}

int DisplayGLX::getGLXFBConfigAttrib(GLXFBConfig config, int attrib) const
{
    int result;
    glXGetFBConfigAttrib(mXDisplay, config, attrib, &result);
    return result;
}

}
