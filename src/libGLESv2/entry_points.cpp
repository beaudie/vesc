//
// Copyright(c) 2014 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//

// entry_points.cpp : Implements the egl and gl entry points.

#include "libGLESv2/entry_points.h"

#include "libANGLE/formatutils.h"
#include "libANGLE/Buffer.h"
#include "libANGLE/Context.h"
#include "libANGLE/Display.h"
#include "libANGLE/Error.h"
#include "libANGLE/Fence.h"
#include "libANGLE/Framebuffer.h"
#include "libANGLE/Renderbuffer.h"
#include "libANGLE/Shader.h"
#include "libANGLE/Surface.h"
#include "libANGLE/Program.h"
#include "libANGLE/Texture.h"
#include "libANGLE/Query.h"
#include "libANGLE/Context.h"
#include "libANGLE/VertexArray.h"
#include "libANGLE/VertexAttribute.h"
#include "libANGLE/TransformFeedback.h"
#include "libANGLE/FramebufferAttachment.h"
#include "libANGLE/renderer/SwapChain.h"

#include "libANGLE/validationES.h"
#include "libANGLE/validationES2.h"
#include "libANGLE/validationES3.h"
#include "libANGLE/queryconversions.h"

#include "common/debug.h"
#include "common/platform.h"
#include "common/tls.h"
#include "common/utilities.h"
#include "common/version.h"

namespace
{

struct Current
{
    EGLint error;
    EGLenum API;
    egl::Display *display;
    egl::Surface *drawSurface;
    egl::Surface *readSurface;
    gl::Context *context;
};

static TLSIndex currentTLS = TLS_OUT_OF_INDEXES;

Current *AllocateCurrent()
{
    ASSERT(currentTLS != TLS_OUT_OF_INDEXES);
    if (currentTLS == TLS_OUT_OF_INDEXES)
    {
        return NULL;
    }

    Current *current = new Current();
    current->error = EGL_SUCCESS;
    current->API = EGL_OPENGL_ES_API;
    current->display = reinterpret_cast<egl::Display*>(EGL_NO_DISPLAY);
    current->drawSurface = reinterpret_cast<egl::Surface*>(EGL_NO_SURFACE);
    current->readSurface = reinterpret_cast<egl::Surface*>(EGL_NO_SURFACE);
    current->context = reinterpret_cast<gl::Context*>(EGL_NO_CONTEXT);

    if (!SetTLSValue(currentTLS, current))
    {
        ERR("Could not set thread local storage.");
        return NULL;
    }

    return current;
}

void DeallocateCurrent()
{
    Current *current = reinterpret_cast<Current*>(GetTLSValue(currentTLS));
    SafeDelete(current);
    SetTLSValue(currentTLS, NULL);
}

Current *GetCurrentData()
{
    // Create a TLS index if one has not been created for this DLL
    if (currentTLS == TLS_OUT_OF_INDEXES)
    {
        currentTLS = CreateTLSIndex();
    }

    Current *current = reinterpret_cast<Current*>(GetTLSValue(currentTLS));

    // ANGLE issue 488: when the dll is loaded after thread initialization,
    // thread local storage (current) might not exist yet.
    return (current ? current : AllocateCurrent());
}

gl::Context *getCurrentContext()
{
    Current *current = GetCurrentData();

    return current->context;
}

#ifdef ANGLE_PLATFORM_WINDOWS
extern "C" BOOL WINAPI DllMain(HINSTANCE, DWORD reason, LPVOID)
{
    switch (reason)
    {
      case DLL_PROCESS_ATTACH:
        currentTLS = CreateTLSIndex();
        if (currentTLS == TLS_INVALID_INDEX)
        {
            return FALSE;
        }
        AllocateCurrent();
        break;

      case DLL_THREAD_ATTACH:
        AllocateCurrent();
        break;

      case DLL_THREAD_DETACH:
        DeallocateCurrent();
        break;

      case DLL_PROCESS_DETACH:
        DeallocateCurrent();
        if (currentTLS != TLS_INVALID_INDEX)
        {
            DestroyTLSIndex(currentTLS);
            currentTLS = TLS_INVALID_INDEX;
        }
        break;
    }

    return TRUE;
}
#endif

}

namespace egl
{

// Current state manipulation
void recordError(const egl::Error &error)
{
    Current *current = GetCurrentData();

    current->error = error.getCode();
}

EGLint getCurrentError()
{
    Current *current = GetCurrentData();

    return current->error;
}

void setCurrentAPI(EGLenum API)
{
    Current *current = GetCurrentData();

    current->API = API;
}

EGLenum getCurrentAPI()
{
    Current *current = GetCurrentData();

    return current->API;
}

void setCurrentDisplay(Display *dpy)
{
    Current *current = GetCurrentData();

    current->display = dpy;
}

Display *getCurrentDisplay()
{
    Current *current = GetCurrentData();

    return current->display;
}

void setCurrentDrawSurface(Surface *surface)
{
    Current *current = GetCurrentData();

    current->drawSurface = surface;
}

Surface *getCurrentDrawSurface()
{
    Current *current = GetCurrentData();

    return current->drawSurface;
}

void setCurrentReadSurface(Surface *surface)
{
    Current *current = GetCurrentData();

    current->readSurface = surface;
}

Surface *getCurrentReadSurface()
{
    Current *current = GetCurrentData();

    return current->readSurface;
}

void makeCurrent(gl::Context *context, egl::Display *display, egl::Surface *surface)
{
    Current *current = GetCurrentData();

    current->context = context;
    current->display = display;

    if (context && display && surface)
    {
        context->makeCurrent(surface);
    }
}

// EGL object validation
static bool ValidateDisplay(Display *display)
{
    if (display == EGL_NO_DISPLAY)
    {
        recordError(Error(EGL_BAD_DISPLAY));
        return false;
    }

    if (!display->isInitialized())
    {
        recordError(Error(EGL_NOT_INITIALIZED));
        return false;
    }

    return true;
}

static bool ValidateConfig(Display *display, EGLConfig config)
{
    if (!ValidateDisplay(display))
    {
        return false;
    }

    if (!display->isValidConfig(config))
    {
        recordError(Error(EGL_BAD_CONFIG));
        return false;
    }

    return true;
}

static bool ValidateContext(Display *display, gl::Context *context)
{
    if (!ValidateDisplay(display))
    {
        return false;
    }

    if (!display->isValidContext(context))
    {
        recordError(Error(EGL_BAD_CONTEXT));
        return false;
    }

    return true;
}

static bool ValidateSurface(Display *display, Surface *surface)
{
    if (!ValidateDisplay(display))
    {
        return false;
    }

    if (!display->isValidSurface(surface))
    {
        recordError(Error(EGL_BAD_SURFACE));
        return false;
    }

    return true;
}

// EGL 1.0
EGLint GetError(void)
{
    EVENT("()");

    EGLint error = getCurrentError();
    recordError(Error(EGL_SUCCESS));
    return error;
}

EGLDisplay GetDisplay(EGLNativeDisplayType display_id)
{
    EVENT("(EGLNativeDisplayType display_id = 0x%0.8p)", display_id);

    return Display::getDisplay(display_id, AttributeMap());
}

EGLBoolean Initialize(EGLDisplay dpy, EGLint *major, EGLint *minor)
{
    EVENT("(EGLDisplay dpy = 0x%0.8p, EGLint *major = 0x%0.8p, EGLint *minor = 0x%0.8p)",
          dpy, major, minor);

    if (dpy == EGL_NO_DISPLAY)
    {
        recordError(Error(EGL_BAD_DISPLAY));
        return EGL_FALSE;
    }

    Display *display = static_cast<Display*>(dpy);

    Error error = display->initialize();
    if (error.isError())
    {
        recordError(error);
        return EGL_FALSE;
    }

    if (major) *major = 1;
    if (minor) *minor = 4;

    recordError(Error(EGL_SUCCESS));
    return EGL_TRUE;
}

EGLBoolean Terminate(EGLDisplay dpy)
{
    EVENT("(EGLDisplay dpy = 0x%0.8p)", dpy);

    if (dpy == EGL_NO_DISPLAY)
    {
        recordError(Error(EGL_BAD_DISPLAY));
        return EGL_FALSE;
    }

    Display *display = static_cast<Display*>(dpy);

    if (display->isValidContext(getCurrentContext()))
    {
        makeCurrent(NULL, NULL, NULL);
    }

    display->terminate();

    recordError(Error(EGL_SUCCESS));
    return EGL_TRUE;
}

const char *QueryString(EGLDisplay dpy, EGLint name)
{
    EVENT("(EGLDisplay dpy = 0x%0.8p, EGLint name = %d)", dpy, name);

    Display *display = static_cast<Display*>(dpy);
    if (!(display == EGL_NO_DISPLAY && name == EGL_EXTENSIONS) && !ValidateDisplay(display))
    {
        return NULL;
    }

    const char *result;
    switch (name)
    {
      case EGL_CLIENT_APIS:
        result = "OpenGL_ES";
        break;
      case EGL_EXTENSIONS:
        result = Display::getExtensionString(display);
        break;
      case EGL_VENDOR:
        result = display->getVendorString();
        break;
      case EGL_VERSION:
        result = "1.4 (ANGLE " ANGLE_VERSION_STRING ")";
        break;
      default:
        recordError(Error(EGL_BAD_PARAMETER));
        return NULL;
    }

    recordError(Error(EGL_SUCCESS));
    return result;
}

EGLBoolean GetConfigs(EGLDisplay dpy, EGLConfig *configs, EGLint config_size, EGLint *num_config)
{
    EVENT("(EGLDisplay dpy = 0x%0.8p, EGLConfig *configs = 0x%0.8p, "
          "EGLint config_size = %d, EGLint *num_config = 0x%0.8p)",
          dpy, configs, config_size, num_config);

    Display *display = static_cast<Display*>(dpy);

    if (!ValidateDisplay(display))
    {
        return EGL_FALSE;
    }

    if (!num_config)
    {
        recordError(Error(EGL_BAD_PARAMETER));
        return EGL_FALSE;
    }

    const EGLint attribList[] =    {EGL_NONE};

    if (!display->getConfigs(configs, attribList, config_size, num_config))
    {
        recordError(Error(EGL_BAD_ATTRIBUTE));
        return EGL_FALSE;
    }

    recordError(Error(EGL_SUCCESS));
    return EGL_TRUE;
}

EGLBoolean ChooseConfig(EGLDisplay dpy, const EGLint *attrib_list, EGLConfig *configs, EGLint config_size, EGLint *num_config)
{
    EVENT("(EGLDisplay dpy = 0x%0.8p, const EGLint *attrib_list = 0x%0.8p, "
          "EGLConfig *configs = 0x%0.8p, EGLint config_size = %d, EGLint *num_config = 0x%0.8p)",
          dpy, attrib_list, configs, config_size, num_config);

    Display *display = static_cast<Display*>(dpy);

    if (!ValidateDisplay(display))
    {
        return EGL_FALSE;
    }

    if (!num_config)
    {
        recordError(Error(EGL_BAD_PARAMETER));
        return EGL_FALSE;
    }

    const EGLint attribList[] =    {EGL_NONE};

    if (!attrib_list)
    {
        attrib_list = attribList;
    }

    display->getConfigs(configs, attrib_list, config_size, num_config);

    recordError(Error(EGL_SUCCESS));
    return EGL_TRUE;
}

EGLBoolean GetConfigAttrib(EGLDisplay dpy, EGLConfig config, EGLint attribute, EGLint *value)
{
    EVENT("(EGLDisplay dpy = 0x%0.8p, EGLConfig config = 0x%0.8p, EGLint attribute = %d, EGLint *value = 0x%0.8p)",
          dpy, config, attribute, value);

    Display *display = static_cast<Display*>(dpy);

    if (!ValidateConfig(display, config))
    {
        return EGL_FALSE;
    }

    if (!display->getConfigAttrib(config, attribute, value))
    {
        recordError(Error(EGL_BAD_ATTRIBUTE));
        return EGL_FALSE;
    }

    recordError(Error(EGL_SUCCESS));
    return EGL_TRUE;
}

EGLSurface CreateWindowSurface(EGLDisplay dpy, EGLConfig config, EGLNativeWindowType win, const EGLint *attrib_list)
{
    EVENT("(EGLDisplay dpy = 0x%0.8p, EGLConfig config = 0x%0.8p, EGLNativeWindowType win = 0x%0.8p, "
          "const EGLint *attrib_list = 0x%0.8p)", dpy, config, win, attrib_list);

    Display *display = static_cast<Display*>(dpy);

    if (!ValidateConfig(display, config))
    {
        return EGL_NO_SURFACE;
    }

    if (!rx::IsValidEGLNativeWindowType(win))
    {
        recordError(Error(EGL_BAD_NATIVE_WINDOW));
        return EGL_NO_SURFACE;
    }

    EGLSurface surface = EGL_NO_SURFACE;
    Error error = display->createWindowSurface(win, config, attrib_list, &surface);
    if (error.isError())
    {
        recordError(error);
        return EGL_NO_SURFACE;
    }

    return surface;
}

EGLSurface CreatePbufferSurface(EGLDisplay dpy, EGLConfig config, const EGLint *attrib_list)
{
    EVENT("(EGLDisplay dpy = 0x%0.8p, EGLConfig config = 0x%0.8p, const EGLint *attrib_list = 0x%0.8p)",
          dpy, config, attrib_list);

    Display *display = static_cast<Display*>(dpy);

    if (!ValidateConfig(display, config))
    {
        return EGL_NO_SURFACE;
    }

    EGLSurface surface = EGL_NO_SURFACE;
    Error error = display->createOffscreenSurface(config, NULL, attrib_list, &surface);
    if (error.isError())
    {
        recordError(error);
        return EGL_NO_SURFACE;
    }

    return surface;
}

EGLSurface CreatePixmapSurface(EGLDisplay dpy, EGLConfig config, EGLNativePixmapType pixmap, const EGLint *attrib_list)
{
    EVENT("(EGLDisplay dpy = 0x%0.8p, EGLConfig config = 0x%0.8p, EGLNativePixmapType pixmap = 0x%0.8p, "
          "const EGLint *attrib_list = 0x%0.8p)", dpy, config, pixmap, attrib_list);

    Display *display = static_cast<Display*>(dpy);

    if (!ValidateConfig(display, config))
    {
        return EGL_NO_SURFACE;
    }

    UNIMPLEMENTED();   // FIXME

    recordError(Error(EGL_SUCCESS));
    return EGL_NO_SURFACE;
}

EGLBoolean DestroySurface(EGLDisplay dpy, EGLSurface surface)
{
    EVENT("(EGLDisplay dpy = 0x%0.8p, EGLSurface surface = 0x%0.8p)", dpy, surface);

    Display *display = static_cast<Display*>(dpy);
    Surface *eglSurface = static_cast<Surface*>(surface);

    if (!ValidateSurface(display, eglSurface))
    {
        return EGL_FALSE;
    }

    if (surface == EGL_NO_SURFACE)
    {
        recordError(Error(EGL_BAD_SURFACE));
        return EGL_FALSE;
    }

    display->destroySurface((Surface*)surface);

    recordError(Error(EGL_SUCCESS));
    return EGL_TRUE;
}

EGLBoolean QuerySurface(EGLDisplay dpy, EGLSurface surface, EGLint attribute, EGLint *value)
{
    EVENT("(EGLDisplay dpy = 0x%0.8p, EGLSurface surface = 0x%0.8p, EGLint attribute = %d, EGLint *value = 0x%0.8p)",
          dpy, surface, attribute, value);

    Display *display = static_cast<Display*>(dpy);
    Surface *eglSurface = (Surface*)surface;

    if (!ValidateSurface(display, eglSurface))
    {
        return EGL_FALSE;
    }

    if (surface == EGL_NO_SURFACE)
    {
        recordError(Error(EGL_BAD_SURFACE));
        return EGL_FALSE;
    }

    switch (attribute)
    {
      case EGL_VG_ALPHA_FORMAT:
        UNIMPLEMENTED();   // FIXME
        break;
      case EGL_VG_COLORSPACE:
        UNIMPLEMENTED();   // FIXME
        break;
      case EGL_CONFIG_ID:
        *value = eglSurface->getConfigID();
        break;
      case EGL_HEIGHT:
        *value = eglSurface->getHeight();
        break;
      case EGL_HORIZONTAL_RESOLUTION:
        UNIMPLEMENTED();   // FIXME
        break;
      case EGL_LARGEST_PBUFFER:
        UNIMPLEMENTED();   // FIXME
        break;
      case EGL_MIPMAP_TEXTURE:
        UNIMPLEMENTED();   // FIXME
        break;
      case EGL_MIPMAP_LEVEL:
        UNIMPLEMENTED();   // FIXME
        break;
      case EGL_MULTISAMPLE_RESOLVE:
        UNIMPLEMENTED();   // FIXME
        break;
      case EGL_PIXEL_ASPECT_RATIO:
        *value = eglSurface->getPixelAspectRatio();
        break;
      case EGL_RENDER_BUFFER:
        *value = eglSurface->getRenderBuffer();
        break;
      case EGL_SWAP_BEHAVIOR:
        *value = eglSurface->getSwapBehavior();
        break;
      case EGL_TEXTURE_FORMAT:
        *value = eglSurface->getTextureFormat();
        break;
      case EGL_TEXTURE_TARGET:
        *value = eglSurface->getTextureTarget();
        break;
      case EGL_VERTICAL_RESOLUTION:
        UNIMPLEMENTED();   // FIXME
        break;
      case EGL_WIDTH:
        *value = eglSurface->getWidth();
        break;
      case EGL_POST_SUB_BUFFER_SUPPORTED_NV:
        *value = eglSurface->isPostSubBufferSupported();
        break;
      case EGL_FIXED_SIZE_ANGLE:
        *value = eglSurface->isFixedSize();
        break;
      default:
        recordError(Error(EGL_BAD_ATTRIBUTE));
        return EGL_FALSE;
    }

    recordError(Error(EGL_SUCCESS));
    return EGL_TRUE;
}

EGLContext CreateContext(EGLDisplay dpy, EGLConfig config, EGLContext share_context, const EGLint *attrib_list)
{
    EVENT("(EGLDisplay dpy = 0x%0.8p, EGLConfig config = 0x%0.8p, EGLContext share_context = 0x%0.8p, "
          "const EGLint *attrib_list = 0x%0.8p)", dpy, config, share_context, attrib_list);

    // Get the requested client version (default is 1) and check it is 2 or 3.
    EGLint client_version = 1;
    bool reset_notification = false;
    bool robust_access = false;

    if (attrib_list)
    {
        for (const EGLint* attribute = attrib_list; attribute[0] != EGL_NONE; attribute += 2)
        {
            switch (attribute[0])
            {
              case EGL_CONTEXT_CLIENT_VERSION:
                client_version = attribute[1];
                break;
              case EGL_CONTEXT_OPENGL_ROBUST_ACCESS_EXT:
                if (attribute[1] == EGL_TRUE)
                {
                    recordError(Error(EGL_BAD_CONFIG));   // Unimplemented
                    return EGL_NO_CONTEXT;
                    // robust_access = true;
                }
                else if (attribute[1] != EGL_FALSE)
                {
                    recordError(Error(EGL_BAD_ATTRIBUTE));
                    return EGL_NO_CONTEXT;
                }
                break;
              case EGL_CONTEXT_OPENGL_RESET_NOTIFICATION_STRATEGY_EXT:
                if (attribute[1] == EGL_LOSE_CONTEXT_ON_RESET_EXT)
                {
                    reset_notification = true;
                }
                else if (attribute[1] != EGL_NO_RESET_NOTIFICATION_EXT)
                {
                    recordError(Error(EGL_BAD_ATTRIBUTE));
                    return EGL_NO_CONTEXT;
                }
                break;
              default:
                recordError(Error(EGL_BAD_ATTRIBUTE));
                return EGL_NO_CONTEXT;
            }
        }
    }

    if (client_version != 2 && client_version != 3)
    {
        recordError(Error(EGL_BAD_CONFIG));
        return EGL_NO_CONTEXT;
    }

    Display *display = static_cast<Display*>(dpy);

    if (share_context)
    {
        gl::Context* sharedGLContext = static_cast<gl::Context*>(share_context);

        if (sharedGLContext->isResetNotificationEnabled() != reset_notification)
        {
            recordError(Error(EGL_BAD_MATCH));
            return EGL_NO_CONTEXT;
        }

        if (sharedGLContext->getClientVersion() != client_version)
        {
            recordError(Error(EGL_BAD_CONTEXT));
            return EGL_NO_CONTEXT;
        }

        // Can not share contexts between displays
        if (sharedGLContext->getRenderer() != display->getRenderer())
        {
            recordError(Error(EGL_BAD_MATCH));
            return EGL_NO_CONTEXT;
        }
    }

    if (!ValidateConfig(display, config))
    {
        return EGL_NO_CONTEXT;
    }

    EGLContext context = EGL_NO_CONTEXT;
    Error error =  display->createContext(config, client_version, static_cast<gl::Context*>(share_context),
                                               reset_notification, robust_access, &context);
    if (error.isError())
    {
        recordError(error);
        return EGL_NO_CONTEXT;
    }

    return context;
}

EGLBoolean DestroyContext(EGLDisplay dpy, EGLContext ctx)
{
    EVENT("(EGLDisplay dpy = 0x%0.8p, EGLContext ctx = 0x%0.8p)", dpy, ctx);

    Display *display = static_cast<Display*>(dpy);
    gl::Context *context = static_cast<gl::Context*>(ctx);

    if (!ValidateContext(display, context))
    {
        return EGL_FALSE;
    }

    if (ctx == EGL_NO_CONTEXT)
    {
        recordError(Error(EGL_BAD_CONTEXT));
        return EGL_FALSE;
    }

    if (context == getCurrentContext())
    {
        makeCurrent(NULL, NULL, NULL);
    }

    display->destroyContext(context);

    recordError(Error(EGL_SUCCESS));
    return EGL_TRUE;
}

EGLBoolean MakeCurrent(EGLDisplay dpy, EGLSurface draw, EGLSurface read, EGLContext ctx)
{
    EVENT("(EGLDisplay dpy = 0x%0.8p, EGLSurface draw = 0x%0.8p, EGLSurface read = 0x%0.8p, EGLContext ctx = 0x%0.8p)",
          dpy, draw, read, ctx);

    Display *display = static_cast<Display*>(dpy);
    gl::Context *context = static_cast<gl::Context*>(ctx);

    bool noContext = (ctx == EGL_NO_CONTEXT);
    bool noSurface = (draw == EGL_NO_SURFACE || read == EGL_NO_SURFACE);
    if (noContext != noSurface)
    {
        recordError(Error(EGL_BAD_MATCH));
        return EGL_FALSE;
    }

    if (ctx != EGL_NO_CONTEXT && !ValidateContext(display, context))
    {
        return EGL_FALSE;
    }

    if (dpy != EGL_NO_DISPLAY && display->isInitialized())
    {
        rx::Renderer *renderer = display->getRenderer();
        if (renderer->testDeviceLost())
        {
            display->notifyDeviceLost();
            return EGL_FALSE;
        }

        if (renderer->isDeviceLost())
        {
            recordError(Error(EGL_CONTEXT_LOST));
            return EGL_FALSE;
        }
    }

    Surface *drawSurface = static_cast<Surface*>(draw);
    Surface *readSurface = static_cast<Surface*>(read);

    if ((draw != EGL_NO_SURFACE && !ValidateSurface(display, drawSurface)) ||
        (read != EGL_NO_SURFACE && !ValidateSurface(display, readSurface)))
    {
        return EGL_FALSE;
    }

    if (draw != read)
    {
        UNIMPLEMENTED();   // FIXME
    }

    setCurrentDisplay(display);
    setCurrentDrawSurface(drawSurface);
    setCurrentReadSurface(readSurface);

    makeCurrent(context, display, drawSurface);

    recordError(Error(EGL_SUCCESS));
    return EGL_TRUE;
}

EGLSurface GetCurrentSurface(EGLint readdraw)
{
    EVENT("(EGLint readdraw = %d)", readdraw);

    if (readdraw == EGL_READ)
    {
        recordError(Error(EGL_SUCCESS));
        return getCurrentReadSurface();
    }
    else if (readdraw == EGL_DRAW)
    {
        recordError(Error(EGL_SUCCESS));
        return getCurrentDrawSurface();
    }
    else
    {
        recordError(Error(EGL_BAD_PARAMETER));
        return EGL_NO_SURFACE;
    }
}

EGLDisplay GetCurrentDisplay(void)
{
    EVENT("()");

    EGLDisplay dpy = getCurrentDisplay();

    recordError(Error(EGL_SUCCESS));
    return dpy;
}

EGLBoolean QueryContext(EGLDisplay dpy, EGLContext ctx, EGLint attribute, EGLint *value)
{
    EVENT("(EGLDisplay dpy = 0x%0.8p, EGLContext ctx = 0x%0.8p, EGLint attribute = %d, EGLint *value = 0x%0.8p)",
          dpy, ctx, attribute, value);

    Display *display = static_cast<Display*>(dpy);
    gl::Context *context = static_cast<gl::Context*>(ctx);

    if (!ValidateContext(display, context))
    {
        return EGL_FALSE;
    }

    UNIMPLEMENTED();   // FIXME

    recordError(Error(EGL_SUCCESS));
    return 0;
}

EGLBoolean WaitGL(void)
{
    EVENT("()");

    UNIMPLEMENTED();   // FIXME

    recordError(Error(EGL_SUCCESS));
    return 0;
}

EGLBoolean WaitNative(EGLint engine)
{
    EVENT("(EGLint engine = %d)", engine);

    UNIMPLEMENTED();   // FIXME

    recordError(Error(EGL_SUCCESS));
    return 0;
}

EGLBoolean SwapBuffers(EGLDisplay dpy, EGLSurface surface)
{
    EVENT("(EGLDisplay dpy = 0x%0.8p, EGLSurface surface = 0x%0.8p)", dpy, surface);

    Display *display = static_cast<Display*>(dpy);
    Surface *eglSurface = (Surface*)surface;

    if (!ValidateSurface(display, eglSurface))
    {
        return EGL_FALSE;
    }

    if (display->getRenderer()->isDeviceLost())
    {
        recordError(Error(EGL_CONTEXT_LOST));
        return EGL_FALSE;
    }

    if (surface == EGL_NO_SURFACE)
    {
        recordError(Error(EGL_BAD_SURFACE));
        return EGL_FALSE;
    }

    Error error = eglSurface->swap();
    if (error.isError())
    {
        recordError(error);
        return EGL_FALSE;
    }

    recordError(Error(EGL_SUCCESS));
    return EGL_TRUE;
}

EGLBoolean CopyBuffers(EGLDisplay dpy, EGLSurface surface, EGLNativePixmapType target)
{
    EVENT("(EGLDisplay dpy = 0x%0.8p, EGLSurface surface = 0x%0.8p, EGLNativePixmapType target = 0x%0.8p)", dpy, surface, target);

    Display *display = static_cast<Display*>(dpy);
    Surface *eglSurface = static_cast<Surface*>(surface);

    if (!ValidateSurface(display, eglSurface))
    {
        return EGL_FALSE;
    }

    if (display->getRenderer()->isDeviceLost())
    {
        recordError(Error(EGL_CONTEXT_LOST));
        return EGL_FALSE;
    }

    UNIMPLEMENTED();   // FIXME

    recordError(Error(EGL_SUCCESS));
    return 0;
}

// EGL 1.1
EGLBoolean BindTexImage(EGLDisplay dpy, EGLSurface surface, EGLint buffer)
{
    EVENT("(EGLDisplay dpy = 0x%0.8p, EGLSurface surface = 0x%0.8p, EGLint buffer = %d)", dpy, surface, buffer);

    Display *display = static_cast<Display*>(dpy);
    Surface *eglSurface = static_cast<Surface*>(surface);

    if (!ValidateSurface(display, eglSurface))
    {
        return EGL_FALSE;
    }

    if (buffer != EGL_BACK_BUFFER)
    {
        recordError(Error(EGL_BAD_PARAMETER));
        return EGL_FALSE;
    }

    if (surface == EGL_NO_SURFACE || eglSurface->getWindowHandle())
    {
        recordError(Error(EGL_BAD_SURFACE));
        return EGL_FALSE;
    }

    if (eglSurface->getBoundTexture())
    {
        recordError(Error(EGL_BAD_ACCESS));
        return EGL_FALSE;
    }

    if (eglSurface->getTextureFormat() == EGL_NO_TEXTURE)
    {
        recordError(Error(EGL_BAD_MATCH));
        return EGL_FALSE;
    }

    gl::Context *context = getCurrentContext();
    if (context)
    {
        gl::Texture2D *textureObject = context->getTexture2D();
        ASSERT(textureObject != NULL);

        if (textureObject->isImmutable())
        {
            recordError(Error(EGL_BAD_MATCH));
            return EGL_FALSE;
        }

        eglSurface->bindTexImage(textureObject, buffer);
    }

    recordError(Error(EGL_SUCCESS));
    return EGL_TRUE;
}

EGLBoolean SurfaceAttrib(EGLDisplay dpy, EGLSurface surface, EGLint attribute, EGLint value)
{
    EVENT("(EGLDisplay dpy = 0x%0.8p, EGLSurface surface = 0x%0.8p, EGLint attribute = %d, EGLint value = %d)",
        dpy, surface, attribute, value);

    Display *display = static_cast<Display*>(dpy);
    Surface *eglSurface = static_cast<Surface*>(surface);

    if (!ValidateSurface(display, eglSurface))
    {
        return EGL_FALSE;
    }

    UNIMPLEMENTED();   // FIXME

    recordError(Error(EGL_SUCCESS));
    return EGL_TRUE;
}

EGLBoolean ReleaseTexImage(EGLDisplay dpy, EGLSurface surface, EGLint buffer)
{
    EVENT("(EGLDisplay dpy = 0x%0.8p, EGLSurface surface = 0x%0.8p, EGLint buffer = %d)", dpy, surface, buffer);

    Display *display = static_cast<Display*>(dpy);
    Surface *eglSurface = static_cast<Surface*>(surface);

    if (!ValidateSurface(display, eglSurface))
    {
        return EGL_FALSE;
    }

    if (buffer != EGL_BACK_BUFFER)
    {
        recordError(Error(EGL_BAD_PARAMETER));
        return EGL_FALSE;
    }

    if (surface == EGL_NO_SURFACE || eglSurface->getWindowHandle())
    {
        recordError(Error(EGL_BAD_SURFACE));
        return EGL_FALSE;
    }

    if (eglSurface->getTextureFormat() == EGL_NO_TEXTURE)
    {
        recordError(Error(EGL_BAD_MATCH));
        return EGL_FALSE;
    }

    gl::Texture2D *texture = eglSurface->getBoundTexture();

    if (texture)
    {
        eglSurface->releaseTexImage(buffer);
    }

    recordError(Error(EGL_SUCCESS));
    return EGL_TRUE;
}

EGLBoolean SwapInterval(EGLDisplay dpy, EGLint interval)
{
    EVENT("(EGLDisplay dpy = 0x%0.8p, EGLint interval = %d)", dpy, interval);

    Display *display = static_cast<Display*>(dpy);

    if (!ValidateDisplay(display))
    {
        return EGL_FALSE;
    }

    Surface *draw_surface = static_cast<Surface*>(getCurrentDrawSurface());

    if (draw_surface == NULL)
    {
        recordError(Error(EGL_BAD_SURFACE));
        return EGL_FALSE;
    }

    draw_surface->setSwapInterval(interval);

    recordError(Error(EGL_SUCCESS));
    return EGL_TRUE;
}


// EGL 1.2
EGLBoolean BindAPI(EGLenum api)
{
    EVENT("(EGLenum api = 0x%X)", api);

    switch (api)
    {
      case EGL_OPENGL_API:
      case EGL_OPENVG_API:
        recordError(Error(EGL_BAD_PARAMETER));
        return EGL_FALSE;   // Not supported by this implementation
      case EGL_OPENGL_ES_API:
        break;
      default:
        recordError(Error(EGL_BAD_PARAMETER));
        return EGL_FALSE;
    }

    setCurrentAPI(api);

    recordError(Error(EGL_SUCCESS));
    return EGL_TRUE;
}

EGLenum QueryAPI(void)
{
    EVENT("()");

    EGLenum API = getCurrentAPI();

    recordError(Error(EGL_SUCCESS));
    return API;
}

EGLSurface CreatePbufferFromClientBuffer(EGLDisplay dpy, EGLenum buftype, EGLClientBuffer buffer, EGLConfig config, const EGLint *attrib_list)
{
    EVENT("(EGLDisplay dpy = 0x%0.8p, EGLenum buftype = 0x%X, EGLClientBuffer buffer = 0x%0.8p, "
        "EGLConfig config = 0x%0.8p, const EGLint *attrib_list = 0x%0.8p)",
        dpy, buftype, buffer, config, attrib_list);

    Display *display = static_cast<Display*>(dpy);

    if (!ValidateConfig(display, config))
    {
        return EGL_NO_SURFACE;
    }

    if (buftype != EGL_D3D_TEXTURE_2D_SHARE_HANDLE_ANGLE || !buffer)
    {
        recordError(Error(EGL_BAD_PARAMETER));
        return EGL_NO_SURFACE;
    }

    EGLSurface surface = EGL_NO_SURFACE;
    Error error = display->createOffscreenSurface(config, buffer, attrib_list, &surface);
    if (error.isError())
    {
        recordError(error);
        return EGL_NO_SURFACE;
    }

    return surface;
}

EGLBoolean ReleaseThread(void)
{
    EVENT("()");

    MakeCurrent(EGL_NO_DISPLAY, EGL_NO_CONTEXT, EGL_NO_SURFACE, EGL_NO_SURFACE);

    recordError(Error(EGL_SUCCESS));
    return EGL_TRUE;
}

EGLBoolean WaitClient(void)
{
    EVENT("()");

    UNIMPLEMENTED();   // FIXME

    recordError(Error(EGL_SUCCESS));
    return 0;
}

// EGL 1.4
EGLContext GetCurrentContext(void)
{
    EVENT("()");

    gl::Context *context = getCurrentContext();

    recordError(Error(EGL_SUCCESS));
    return static_cast<EGLContext>(context);
}

// EGL 1.5
EGLSync CreateSync(EGLDisplay dpy, EGLenum type, const EGLAttrib *attrib_list)
{
    UNIMPLEMENTED();
    return EGL_NO_SYNC;
}

EGLBoolean DestroySync(EGLDisplay dpy, EGLSync sync)
{
    UNIMPLEMENTED();
    return EGL_FALSE;
}

EGLint ClientWaitSync(EGLDisplay dpy, EGLSync sync, EGLint flags, EGLTime timeout)
{
    UNIMPLEMENTED();
    return 0;
}

EGLBoolean GetSyncAttrib(EGLDisplay dpy, EGLSync sync, EGLint attribute, EGLAttrib *value)
{
    UNIMPLEMENTED();
    return EGL_FALSE;
}

EGLDisplay GetPlatformDisplay(EGLenum platform, void *native_display, const EGLAttrib *attrib_list)
{
    UNIMPLEMENTED();
    return EGL_NO_DISPLAY;
}

EGLSurface CreatePlatformWindowSurface(EGLDisplay dpy, EGLConfig config, void *native_window, const EGLAttrib *attrib_list)
{
    UNIMPLEMENTED();
    return EGL_NO_SURFACE;
}

EGLSurface CreatePlatformPixmapSurface(EGLDisplay dpy, EGLConfig config, void *native_pixmap, const EGLAttrib *attrib_list)
{
    UNIMPLEMENTED();
    return EGL_NO_SURFACE;
}

EGLBoolean WaitSync(EGLDisplay dpy, EGLSync sync, EGLint flags)
{
    UNIMPLEMENTED();
    return EGL_FALSE;
}

// EGL_ANGLE_query_surface_pointer
EGLBoolean QuerySurfacePointerANGLE(EGLDisplay dpy, EGLSurface surface, EGLint attribute, void **value)
{
    TRACE("(EGLDisplay dpy = 0x%0.8p, EGLSurface surface = 0x%0.8p, EGLint attribute = %d, void **value = 0x%0.8p)",
          dpy, surface, attribute, value);

    Display *display = static_cast<Display*>(dpy);
    Surface *eglSurface = (Surface*)surface;

    if (!ValidateSurface(display, eglSurface))
    {
        return EGL_FALSE;
    }

    if (surface == EGL_NO_SURFACE)
    {
        recordError(Error(EGL_BAD_SURFACE));
        return EGL_FALSE;
    }

    // validate the attribute parameter
    switch (attribute)
    {
      case EGL_D3D_TEXTURE_2D_SHARE_HANDLE_ANGLE:
        break;
      default:
        recordError(Error(EGL_BAD_ATTRIBUTE));
        return EGL_FALSE;
    }

    Error error = eglSurface->querySurfacePointerANGLE(attribute, value);
    recordError(error);
    return (error.isError() ? EGL_FALSE : EGL_TRUE);
}


// EGL_NV_post_sub_buffer
EGLBoolean PostSubBufferNV(EGLDisplay dpy, EGLSurface surface, EGLint x, EGLint y, EGLint width, EGLint height)
{
    EVENT("(EGLDisplay dpy = 0x%0.8p, EGLSurface surface = 0x%0.8p, EGLint x = %d, EGLint y = %d, EGLint width = %d, EGLint height = %d)", dpy, surface, x, y, width, height);

    if (x < 0 || y < 0 || width < 0 || height < 0)
    {
        recordError(Error(EGL_BAD_PARAMETER));
        return EGL_FALSE;
    }

    Display *display = static_cast<Display*>(dpy);
    Surface *eglSurface = static_cast<Surface*>(surface);

    if (!ValidateSurface(display, eglSurface))
    {
        return EGL_FALSE;
    }

    if (display->getRenderer()->isDeviceLost())
    {
        recordError(Error(EGL_CONTEXT_LOST));
        return EGL_FALSE;
    }

    if (surface == EGL_NO_SURFACE)
    {
        recordError(Error(EGL_BAD_SURFACE));
        return EGL_FALSE;
    }

    Error error = eglSurface->postSubBuffer(x, y, width, height);
    if (error.isError())
    {
        recordError(error);
        return EGL_FALSE;
    }

    recordError(Error(EGL_SUCCESS));
    return EGL_TRUE;
}

// EGL_EXT_platform_base
EGLDisplay GetPlatformDisplayEXT(EGLenum platform, void *native_display, const EGLint *attrib_list)
{
    EVENT("(EGLenum platform = %d, void* native_display = 0x%0.8p, const EGLint* attrib_list = 0x%0.8p)",
          platform, native_display, attrib_list);

    switch (platform)
    {
      case EGL_PLATFORM_ANGLE_ANGLE:
        break;

      default:
        recordError(Error(EGL_BAD_CONFIG));
        return EGL_NO_DISPLAY;
    }

    EGLNativeDisplayType displayId = static_cast<EGLNativeDisplayType>(native_display);

#if !defined(ANGLE_ENABLE_WINDOWS_STORE)
    // Validate the display device context
    if (WindowFromDC(displayId) == NULL)
    {
        recordError(Error(EGL_SUCCESS));
        return EGL_NO_DISPLAY;
    }
#endif

    EGLint platformType = EGL_PLATFORM_ANGLE_TYPE_DEFAULT_ANGLE;
    bool majorVersionSpecified = false;
    bool minorVersionSpecified = false;
    bool requestedWARP = false;

    if (attrib_list)
    {
        for (const EGLint *curAttrib = attrib_list; curAttrib[0] != EGL_NONE; curAttrib += 2)
        {
            switch (curAttrib[0])
            {
              case EGL_PLATFORM_ANGLE_TYPE_ANGLE:
                switch (curAttrib[1])
                {
                  case EGL_PLATFORM_ANGLE_TYPE_DEFAULT_ANGLE:
                    break;

                  case EGL_PLATFORM_ANGLE_TYPE_D3D9_ANGLE:
                  case EGL_PLATFORM_ANGLE_TYPE_D3D11_ANGLE:
                    if (!Display::supportsPlatformD3D())
                    {
                        recordError(Error(EGL_SUCCESS));
                        return EGL_NO_DISPLAY;
                    }
                    break;

                  case EGL_PLATFORM_ANGLE_TYPE_OPENGL_ANGLE:
                  case EGL_PLATFORM_ANGLE_TYPE_OPENGLES_ANGLE:
                    if (!Display::supportsPlatformOpenGL())
                    {
                        recordError(Error(EGL_SUCCESS));
                        return EGL_NO_DISPLAY;
                    }
                    break;

                  default:
                    recordError(Error(EGL_SUCCESS));
                    return EGL_NO_DISPLAY;
                }
                platformType = curAttrib[1];
                break;

              case EGL_PLATFORM_ANGLE_MAX_VERSION_MAJOR_ANGLE:
                if (curAttrib[1] != EGL_DONT_CARE)
                {
                    majorVersionSpecified = true;
                }
                break;

              case EGL_PLATFORM_ANGLE_MAX_VERSION_MINOR_ANGLE:
                if (curAttrib[1] != EGL_DONT_CARE)
                {
                    minorVersionSpecified = true;
                }
                break;

              case EGL_PLATFORM_ANGLE_USE_WARP_ANGLE:
                if (!Display::supportsPlatformD3D())
                {
                    recordError(Error(EGL_SUCCESS));
                    return EGL_NO_DISPLAY;
                }

                switch (curAttrib[1])
                {
                  case EGL_FALSE:
                  case EGL_TRUE:
                    break;

                  default:
                    recordError(Error(EGL_SUCCESS));
                    return EGL_NO_DISPLAY;
                }

                requestedWARP = (curAttrib[1] == EGL_TRUE);
                break;

              default:
                break;
            }
        }
    }

    if (!majorVersionSpecified && minorVersionSpecified)
    {
        recordError(Error(EGL_BAD_ATTRIBUTE));
        return EGL_NO_DISPLAY;
    }

    if (platformType != EGL_PLATFORM_ANGLE_TYPE_D3D11_ANGLE && requestedWARP)
    {
        recordError(Error(EGL_BAD_ATTRIBUTE));
        return EGL_NO_DISPLAY;
    }

    recordError(Error(EGL_SUCCESS));
    return Display::getDisplay(displayId, AttributeMap(attrib_list));
}

}

namespace gl
{

gl::Context *getNonLostContext()
{
    gl::Context *context = getCurrentContext();
    if (context)
    {
        if (context->isContextLost())
        {
            context->recordError(gl::Error(GL_OUT_OF_MEMORY, "Context has been lost."));
            return NULL;
        }
        else
        {
            return context;
        }
    }
    return NULL;
}

// OpenGL ES 2.0 functions

void ActiveTexture(GLenum texture)
{
    EVENT("(GLenum texture = 0x%X)", texture);

    Context *context = getNonLostContext();
    if (context)
    {
        if (texture < GL_TEXTURE0 || texture > GL_TEXTURE0 + context->getCaps().maxCombinedTextureImageUnits - 1)
        {
            context->recordError(Error(GL_INVALID_ENUM));
            return;
        }

        context->getState().setActiveSampler(texture - GL_TEXTURE0);
    }
}

void AttachShader(GLuint program, GLuint shader)
{
    EVENT("(GLuint program = %d, GLuint shader = %d)", program, shader);

    Context *context = getNonLostContext();
    if (context)
    {
        Program *programObject = context->getProgram(program);
        Shader *shaderObject = context->getShader(shader);

        if (!programObject)
        {
            if (context->getShader(program))
            {
                context->recordError(Error(GL_INVALID_OPERATION));
                return;
            }
            else
            {
                context->recordError(Error(GL_INVALID_VALUE));
                return;
            }
        }

        if (!shaderObject)
        {
            if (context->getProgram(shader))
            {
                context->recordError(Error(GL_INVALID_OPERATION));
                return;
            }
            else
            {
                context->recordError(Error(GL_INVALID_VALUE));
                return;
            }
        }

        if (!programObject->attachShader(shaderObject))
        {
            context->recordError(Error(GL_INVALID_OPERATION));
            return;
        }
    }
}

void BeginQueryEXT(GLenum target, GLuint id)
{
    EVENT("(GLenum target = 0x%X, GLuint %d)", target, id);

    Context *context = getNonLostContext();
    if (context)
    {
        if (!ValidateBeginQuery(context, target, id))
        {
            return;
        }

        Error error = context->beginQuery(target, id);
        if (error.isError())
        {
            context->recordError(error);
            return;
        }
    }
}

void BindAttribLocation(GLuint program, GLuint index, const GLchar* name)
{
    EVENT("(GLuint program = %d, GLuint index = %d, const GLchar* name = 0x%0.8p)", program, index, name);

    Context *context = getNonLostContext();
    if (context)
    {
        if (index >= MAX_VERTEX_ATTRIBS)
        {
            context->recordError(Error(GL_INVALID_VALUE));
            return;
        }

        Program *programObject = context->getProgram(program);

        if (!programObject)
        {
            if (context->getShader(program))
            {
                context->recordError(Error(GL_INVALID_OPERATION));
                return;
            }
            else
            {
                context->recordError(Error(GL_INVALID_VALUE));
                return;
            }
        }

        if (strncmp(name, "gl_", 3) == 0)
        {
            context->recordError(Error(GL_INVALID_OPERATION));
            return;
        }

        programObject->bindAttributeLocation(index, name);
    }
}

void BindBuffer(GLenum target, GLuint buffer)
{
    EVENT("(GLenum target = 0x%X, GLuint buffer = %d)", target, buffer);

    Context *context = getNonLostContext();
    if (context)
    {
        if (!ValidBufferTarget(context, target))
        {
            context->recordError(Error(GL_INVALID_ENUM));
            return;
        }

        switch (target)
        {
          case GL_ARRAY_BUFFER:
            context->bindArrayBuffer(buffer);
            return;
          case GL_ELEMENT_ARRAY_BUFFER:
            context->bindElementArrayBuffer(buffer);
            return;
          case GL_COPY_READ_BUFFER:
            context->bindCopyReadBuffer(buffer);
            return;
          case GL_COPY_WRITE_BUFFER:
            context->bindCopyWriteBuffer(buffer);
            return;
          case GL_PIXEL_PACK_BUFFER:
            context->bindPixelPackBuffer(buffer);
            return;
          case GL_PIXEL_UNPACK_BUFFER:
            context->bindPixelUnpackBuffer(buffer);
            return;
          case GL_UNIFORM_BUFFER:
            context->bindGenericUniformBuffer(buffer);
            return;
          case GL_TRANSFORM_FEEDBACK_BUFFER:
            context->bindGenericTransformFeedbackBuffer(buffer);
            return;

          default:
            context->recordError(Error(GL_INVALID_ENUM));
            return;
        }
    }
}

void BindFramebuffer(GLenum target, GLuint framebuffer)
{
    EVENT("(GLenum target = 0x%X, GLuint framebuffer = %d)", target, framebuffer);

    Context *context = getNonLostContext();
    if (context)
    {
        if (!ValidFramebufferTarget(target))
        {
            context->recordError(Error(GL_INVALID_ENUM));
            return;
        }

        if (target == GL_READ_FRAMEBUFFER_ANGLE || target == GL_FRAMEBUFFER)
        {
            context->bindReadFramebuffer(framebuffer);
        }

        if (target == GL_DRAW_FRAMEBUFFER_ANGLE || target == GL_FRAMEBUFFER)
        {
            context->bindDrawFramebuffer(framebuffer);
        }
    }
}

void BindRenderbuffer(GLenum target, GLuint renderbuffer)
{
    EVENT("(GLenum target = 0x%X, GLuint renderbuffer = %d)", target, renderbuffer);

    Context *context = getNonLostContext();
    if (context)
    {
        if (target != GL_RENDERBUFFER)
        {
            context->recordError(Error(GL_INVALID_ENUM));
            return;
        }

        context->bindRenderbuffer(renderbuffer);
    }
}

void BindTexture(GLenum target, GLuint texture)
{
    EVENT("(GLenum target = 0x%X, GLuint texture = %d)", target, texture);

    Context *context = getNonLostContext();
    if (context)
    {
        Texture *textureObject = context->getTexture(texture);

        if (textureObject && textureObject->getTarget() != target && texture != 0)
        {
            context->recordError(Error(GL_INVALID_OPERATION));
            return;
        }

        switch (target)
        {
          case GL_TEXTURE_2D:
          case GL_TEXTURE_CUBE_MAP:
            break;

          case GL_TEXTURE_3D:
          case GL_TEXTURE_2D_ARRAY:
            if (context->getClientVersion() < 3)
            {
                context->recordError(Error(GL_INVALID_ENUM));
                return;
            }
            break;

          default:
            context->recordError(Error(GL_INVALID_ENUM));
            return;
        }

        context->bindTexture(target, texture);
    }
}

void BlendColor(GLclampf red, GLclampf green, GLclampf blue, GLclampf alpha)
{
    EVENT("(GLclampf red = %f, GLclampf green = %f, GLclampf blue = %f, GLclampf alpha = %f)",
          red, green, blue, alpha);

    Context* context = getNonLostContext();

    if (context)
    {
        context->getState().setBlendColor(clamp01(red), clamp01(green), clamp01(blue), clamp01(alpha));
    }
}

void BlendEquation(GLenum mode)
{
    BlendEquationSeparate(mode, mode);
}

void BlendEquationSeparate(GLenum modeRGB, GLenum modeAlpha)
{
    EVENT("(GLenum modeRGB = 0x%X, GLenum modeAlpha = 0x%X)", modeRGB, modeAlpha);

    Context *context = getNonLostContext();
    if (context)
    {
        switch (modeRGB)
        {
          case GL_FUNC_ADD:
          case GL_FUNC_SUBTRACT:
          case GL_FUNC_REVERSE_SUBTRACT:
          case GL_MIN:
          case GL_MAX:
            break;

          default:
            context->recordError(Error(GL_INVALID_ENUM));
            return;
        }

        switch (modeAlpha)
        {
          case GL_FUNC_ADD:
          case GL_FUNC_SUBTRACT:
          case GL_FUNC_REVERSE_SUBTRACT:
          case GL_MIN:
          case GL_MAX:
            break;

          default:
            context->recordError(Error(GL_INVALID_ENUM));
            return;
        }

        context->getState().setBlendEquation(modeRGB, modeAlpha);
    }
}

void BlendFunc(GLenum sfactor, GLenum dfactor)
{
    BlendFuncSeparate(sfactor, dfactor, sfactor, dfactor);
}

void BlendFuncSeparate(GLenum srcRGB, GLenum dstRGB, GLenum srcAlpha, GLenum dstAlpha)
{
    EVENT("(GLenum srcRGB = 0x%X, GLenum dstRGB = 0x%X, GLenum srcAlpha = 0x%X, GLenum dstAlpha = 0x%X)",
          srcRGB, dstRGB, srcAlpha, dstAlpha);

    Context *context = getNonLostContext();
    if (context)
    {
        switch (srcRGB)
        {
          case GL_ZERO:
          case GL_ONE:
          case GL_SRC_COLOR:
          case GL_ONE_MINUS_SRC_COLOR:
          case GL_DST_COLOR:
          case GL_ONE_MINUS_DST_COLOR:
          case GL_SRC_ALPHA:
          case GL_ONE_MINUS_SRC_ALPHA:
          case GL_DST_ALPHA:
          case GL_ONE_MINUS_DST_ALPHA:
          case GL_CONSTANT_COLOR:
          case GL_ONE_MINUS_CONSTANT_COLOR:
          case GL_CONSTANT_ALPHA:
          case GL_ONE_MINUS_CONSTANT_ALPHA:
          case GL_SRC_ALPHA_SATURATE:
            break;

          default:
              context->recordError(Error(GL_INVALID_ENUM));
              return;
        }

        switch (dstRGB)
        {
          case GL_ZERO:
          case GL_ONE:
          case GL_SRC_COLOR:
          case GL_ONE_MINUS_SRC_COLOR:
          case GL_DST_COLOR:
          case GL_ONE_MINUS_DST_COLOR:
          case GL_SRC_ALPHA:
          case GL_ONE_MINUS_SRC_ALPHA:
          case GL_DST_ALPHA:
          case GL_ONE_MINUS_DST_ALPHA:
          case GL_CONSTANT_COLOR:
          case GL_ONE_MINUS_CONSTANT_COLOR:
          case GL_CONSTANT_ALPHA:
          case GL_ONE_MINUS_CONSTANT_ALPHA:
            break;

          case GL_SRC_ALPHA_SATURATE:
            if (context->getClientVersion() < 3)
            {
                context->recordError(Error(GL_INVALID_ENUM));
                return;
            }
            break;

          default:
            context->recordError(Error(GL_INVALID_ENUM));
            return;
        }

        switch (srcAlpha)
        {
          case GL_ZERO:
          case GL_ONE:
          case GL_SRC_COLOR:
          case GL_ONE_MINUS_SRC_COLOR:
          case GL_DST_COLOR:
          case GL_ONE_MINUS_DST_COLOR:
          case GL_SRC_ALPHA:
          case GL_ONE_MINUS_SRC_ALPHA:
          case GL_DST_ALPHA:
          case GL_ONE_MINUS_DST_ALPHA:
          case GL_CONSTANT_COLOR:
          case GL_ONE_MINUS_CONSTANT_COLOR:
          case GL_CONSTANT_ALPHA:
          case GL_ONE_MINUS_CONSTANT_ALPHA:
          case GL_SRC_ALPHA_SATURATE:
            break;

          default:
              context->recordError(Error(GL_INVALID_ENUM));
              return;
        }

        switch (dstAlpha)
        {
          case GL_ZERO:
          case GL_ONE:
          case GL_SRC_COLOR:
          case GL_ONE_MINUS_SRC_COLOR:
          case GL_DST_COLOR:
          case GL_ONE_MINUS_DST_COLOR:
          case GL_SRC_ALPHA:
          case GL_ONE_MINUS_SRC_ALPHA:
          case GL_DST_ALPHA:
          case GL_ONE_MINUS_DST_ALPHA:
          case GL_CONSTANT_COLOR:
          case GL_ONE_MINUS_CONSTANT_COLOR:
          case GL_CONSTANT_ALPHA:
          case GL_ONE_MINUS_CONSTANT_ALPHA:
            break;

          case GL_SRC_ALPHA_SATURATE:
            if (context->getClientVersion() < 3)
            {
                context->recordError(Error(GL_INVALID_ENUM));
                return;
            }
            break;

          default:
            context->recordError(Error(GL_INVALID_ENUM));
            return;
        }

        bool constantColorUsed = (srcRGB == GL_CONSTANT_COLOR || srcRGB == GL_ONE_MINUS_CONSTANT_COLOR ||
                                  dstRGB == GL_CONSTANT_COLOR || dstRGB == GL_ONE_MINUS_CONSTANT_COLOR);

        bool constantAlphaUsed = (srcRGB == GL_CONSTANT_ALPHA || srcRGB == GL_ONE_MINUS_CONSTANT_ALPHA ||
                                  dstRGB == GL_CONSTANT_ALPHA || dstRGB == GL_ONE_MINUS_CONSTANT_ALPHA);

        if (constantColorUsed && constantAlphaUsed)
        {
            ERR("Simultaneous use of GL_CONSTANT_ALPHA/GL_ONE_MINUS_CONSTANT_ALPHA and GL_CONSTANT_COLOR/GL_ONE_MINUS_CONSTANT_COLOR invalid under WebGL");
            context->recordError(Error(GL_INVALID_OPERATION));
            return;
        }

        context->getState().setBlendFactors(srcRGB, dstRGB, srcAlpha, dstAlpha);
    }
}

void BufferData(GLenum target, GLsizeiptr size, const GLvoid* data, GLenum usage)
{
    EVENT("(GLenum target = 0x%X, GLsizeiptr size = %d, const GLvoid* data = 0x%0.8p, GLenum usage = %d)",
          target, size, data, usage);

    Context *context = getNonLostContext();
    if (context)
    {
        if (size < 0)
        {
            context->recordError(Error(GL_INVALID_VALUE));
            return;
        }

        switch (usage)
        {
          case GL_STREAM_DRAW:
          case GL_STATIC_DRAW:
          case GL_DYNAMIC_DRAW:
            break;

          case GL_STREAM_READ:
          case GL_STREAM_COPY:
          case GL_STATIC_READ:
          case GL_STATIC_COPY:
          case GL_DYNAMIC_READ:
          case GL_DYNAMIC_COPY:
            if (context->getClientVersion() < 3)
            {
                context->recordError(Error(GL_INVALID_ENUM));
                return;
            }
            break;

          default:
              context->recordError(Error(GL_INVALID_ENUM));
              return;
        }

        if (!ValidBufferTarget(context, target))
        {
            context->recordError(Error(GL_INVALID_ENUM));
            return;
        }

        Buffer *buffer = context->getState().getTargetBuffer(target);

        if (!buffer)
        {
            context->recordError(Error(GL_INVALID_OPERATION));
            return;
        }

        Error error = buffer->bufferData(data, size, usage);
        if (error.isError())
        {
            context->recordError(error);
            return;
        }
    }
}

void BufferSubData(GLenum target, GLintptr offset, GLsizeiptr size, const GLvoid* data)
{
    EVENT("(GLenum target = 0x%X, GLintptr offset = %d, GLsizeiptr size = %d, const GLvoid* data = 0x%0.8p)",
          target, offset, size, data);

    Context *context = getNonLostContext();
    if (context)
    {
        if (size < 0 || offset < 0)
        {
            context->recordError(Error(GL_INVALID_VALUE));
            return;
        }

        if (data == NULL)
        {
            return;
        }

        if (!ValidBufferTarget(context, target))
        {
            context->recordError(Error(GL_INVALID_ENUM));
            return;
        }

        Buffer *buffer = context->getState().getTargetBuffer(target);

        if (!buffer)
        {
            context->recordError(Error(GL_INVALID_OPERATION));
            return;
        }

        if (buffer->isMapped())
        {
            context->recordError(Error(GL_INVALID_OPERATION));
            return;
        }

        // Check for possible overflow of size + offset
        if (!rx::IsUnsignedAdditionSafe<size_t>(size, offset))
        {
            context->recordError(Error(GL_OUT_OF_MEMORY));
            return;
        }

        if (size + offset > buffer->getSize())
        {
            context->recordError(Error(GL_INVALID_VALUE));
            return;
        }

        Error error = buffer->bufferSubData(data, size, offset);
        if (error.isError())
        {
            context->recordError(error);
            return;
        }
    }
}

GLenum CheckFramebufferStatus(GLenum target)
{
    EVENT("(GLenum target = 0x%X)", target);

    Context *context = getNonLostContext();
    if (context)
    {
        if (!ValidFramebufferTarget(target))
        {
            context->recordError(Error(GL_INVALID_ENUM));
            return 0;
        }

        Framebuffer *framebuffer = context->getState().getTargetFramebuffer(target);
        ASSERT(framebuffer);

        return framebuffer->completeness(context->getData());
    }

    return 0;
}

void Clear(GLbitfield mask)
{
    EVENT("(GLbitfield mask = 0x%X)", mask);

    Context *context = getNonLostContext();
    if (context)
    {
        Framebuffer *framebufferObject = context->getState().getDrawFramebuffer();
        ASSERT(framebufferObject);

        if (framebufferObject->completeness(context->getData()) != GL_FRAMEBUFFER_COMPLETE)
        {
            context->recordError(Error(GL_INVALID_FRAMEBUFFER_OPERATION));
            return;
        }

        if ((mask & ~(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT)) != 0)
        {
            context->recordError(Error(GL_INVALID_VALUE));
            return;
        }

        Error error = context->clear(mask);
        if (error.isError())
        {
            context->recordError(error);
            return;
        }
    }
}

void ClearColor(GLclampf red, GLclampf green, GLclampf blue, GLclampf alpha)
{
    EVENT("(GLclampf red = %f, GLclampf green = %f, GLclampf blue = %f, GLclampf alpha = %f)",
          red, green, blue, alpha);

    Context *context = getNonLostContext();
    if (context)
    {
        context->getState().setClearColor(red, green, blue, alpha);
    }
}

void ClearDepthf(GLclampf depth)
{
    EVENT("(GLclampf depth = %f)", depth);

    Context *context = getNonLostContext();
    if (context)
    {
        context->getState().setClearDepth(depth);
    }
}

void ClearStencil(GLint s)
{
    EVENT("(GLint s = %d)", s);

    Context *context = getNonLostContext();
    if (context)
    {
        context->getState().setClearStencil(s);
    }
}

void ColorMask(GLboolean red, GLboolean green, GLboolean blue, GLboolean alpha)
{
    EVENT("(GLboolean red = %d, GLboolean green = %u, GLboolean blue = %u, GLboolean alpha = %u)",
          red, green, blue, alpha);

    Context *context = getNonLostContext();
    if (context)
    {
        context->getState().setColorMask(red == GL_TRUE, green == GL_TRUE, blue == GL_TRUE, alpha == GL_TRUE);
    }
}

void CompileShader(GLuint shader)
{
    EVENT("(GLuint shader = %d)", shader);

    Context *context = getNonLostContext();
    if (context)
    {
        Shader *shaderObject = context->getShader(shader);

        if (!shaderObject)
        {
            if (context->getProgram(shader))
            {
                context->recordError(Error(GL_INVALID_OPERATION));
                return;
            }
            else
            {
                context->recordError(Error(GL_INVALID_VALUE));
                return;
            }
        }

        shaderObject->compile(context->getData());
    }
}

void CompressedTexImage2D(GLenum target, GLint level, GLenum internalformat, GLsizei width, GLsizei height,
                                      GLint border, GLsizei imageSize, const GLvoid* data)
{
    EVENT("(GLenum target = 0x%X, GLint level = %d, GLenum internalformat = 0x%X, GLsizei width = %d, "
          "GLsizei height = %d, GLint border = %d, GLsizei imageSize = %d, const GLvoid* data = 0x%0.8p)",
          target, level, internalformat, width, height, border, imageSize, data);

    Context *context = getNonLostContext();
    if (context)
    {
        if (context->getClientVersion() < 3 &&
            !ValidateES2TexImageParameters(context, target, level, internalformat, true, false,
                                           0, 0, width, height, border, GL_NONE, GL_NONE, data))
        {
            return;
        }

        if (context->getClientVersion() >= 3 &&
            !ValidateES3TexImageParameters(context, target, level, internalformat, true, false,
                                           0, 0, 0, width, height, 1, border, GL_NONE, GL_NONE, data))
        {
            return;
        }

        const InternalFormat &formatInfo = GetInternalFormatInfo(internalformat);
        if (imageSize < 0 || static_cast<GLuint>(imageSize) != formatInfo.computeBlockSize(GL_UNSIGNED_BYTE, width, height))
        {
            context->recordError(Error(GL_INVALID_VALUE));
            return;
        }

        switch (target)
        {
          case GL_TEXTURE_2D:
            {
                Texture2D *texture = context->getTexture2D();
                Error error = texture->setCompressedImage(level, internalformat, width, height, imageSize, context->getState().getUnpackState(), data);
                if (error.isError())
                {
                    context->recordError(error);
                    return;
                }
            }
            break;

          case GL_TEXTURE_CUBE_MAP_POSITIVE_X:
          case GL_TEXTURE_CUBE_MAP_NEGATIVE_X:
          case GL_TEXTURE_CUBE_MAP_POSITIVE_Y:
          case GL_TEXTURE_CUBE_MAP_NEGATIVE_Y:
          case GL_TEXTURE_CUBE_MAP_POSITIVE_Z:
          case GL_TEXTURE_CUBE_MAP_NEGATIVE_Z:
            {
                TextureCubeMap *texture = context->getTextureCubeMap();
                Error error = texture->setCompressedImage(target, level, internalformat, width, height, imageSize, context->getState().getUnpackState(), data);
                if (error.isError())
                {
                    context->recordError(error);
                    return;
                }
            }
            break;

          default:
            context->recordError(Error(GL_INVALID_ENUM));
            return;
        }
    }
}

void CompressedTexSubImage2D(GLenum target, GLint level, GLint xoffset, GLint yoffset, GLsizei width, GLsizei height,
                                         GLenum format, GLsizei imageSize, const GLvoid* data)
{
    EVENT("(GLenum target = 0x%X, GLint level = %d, GLint xoffset = %d, GLint yoffset = %d, "
          "GLsizei width = %d, GLsizei height = %d, GLenum format = 0x%X, "
          "GLsizei imageSize = %d, const GLvoid* data = 0x%0.8p)",
          target, level, xoffset, yoffset, width, height, format, imageSize, data);

    Context *context = getNonLostContext();
    if (context)
    {
        if (context->getClientVersion() < 3 &&
            !ValidateES2TexImageParameters(context, target, level, GL_NONE, true, true,
                                           xoffset, yoffset, width, height, 0, GL_NONE, GL_NONE, data))
        {
            return;
        }

        if (context->getClientVersion() >= 3 &&
            !ValidateES3TexImageParameters(context, target, level, GL_NONE, true, true,
                                           xoffset, yoffset, 0, width, height, 1, 0, GL_NONE, GL_NONE, data))
        {
            return;
        }

        const InternalFormat &formatInfo = GetInternalFormatInfo(format);
        if (imageSize < 0 || static_cast<GLuint>(imageSize) != formatInfo.computeBlockSize(GL_UNSIGNED_BYTE, width, height))
        {
            context->recordError(Error(GL_INVALID_VALUE));
            return;
        }

        switch (target)
        {
          case GL_TEXTURE_2D:
            {
                Texture2D *texture = context->getTexture2D();
                Error error = texture->subImageCompressed(level, xoffset, yoffset, width, height, format, imageSize, context->getState().getUnpackState(), data);
                if (error.isError())
                {
                    context->recordError(error);
                    return;
                }
            }
            break;

          case GL_TEXTURE_CUBE_MAP_POSITIVE_X:
          case GL_TEXTURE_CUBE_MAP_NEGATIVE_X:
          case GL_TEXTURE_CUBE_MAP_POSITIVE_Y:
          case GL_TEXTURE_CUBE_MAP_NEGATIVE_Y:
          case GL_TEXTURE_CUBE_MAP_POSITIVE_Z:
          case GL_TEXTURE_CUBE_MAP_NEGATIVE_Z:
            {
                TextureCubeMap *texture = context->getTextureCubeMap();
                Error error = texture->subImageCompressed(target, level, xoffset, yoffset, width, height, format, imageSize, context->getState().getUnpackState(), data);
                if (error.isError())
                {
                    context->recordError(error);
                    return;
                }
            }
            break;

          default:
            context->recordError(Error(GL_INVALID_ENUM));
            return;
        }
    }
}

void CopyTexImage2D(GLenum target, GLint level, GLenum internalformat, GLint x, GLint y, GLsizei width, GLsizei height, GLint border)
{
    EVENT("(GLenum target = 0x%X, GLint level = %d, GLenum internalformat = 0x%X, "
          "GLint x = %d, GLint y = %d, GLsizei width = %d, GLsizei height = %d, GLint border = %d)",
          target, level, internalformat, x, y, width, height, border);

    Context *context = getNonLostContext();
    if (context)
    {
        if (context->getClientVersion() < 3 &&
            !ValidateES2CopyTexImageParameters(context, target, level, internalformat, false,
                                               0, 0, x, y, width, height, border))
        {
            return;
        }

        if (context->getClientVersion() >= 3 &&
            !ValidateES3CopyTexImageParameters(context, target, level, internalformat, false,
                                               0, 0, 0, x, y, width, height, border))
        {
            return;
        }

        Framebuffer *framebuffer = context->getState().getReadFramebuffer();

        switch (target)
        {
          case GL_TEXTURE_2D:
            {
                Texture2D *texture = context->getTexture2D();
                Error error = texture->copyImage(level, internalformat, x, y, width, height, framebuffer);
                if (error.isError())
                {
                    context->recordError(error);
                    return;
                }
            }
            break;

          case GL_TEXTURE_CUBE_MAP_POSITIVE_X:
          case GL_TEXTURE_CUBE_MAP_NEGATIVE_X:
          case GL_TEXTURE_CUBE_MAP_POSITIVE_Y:
          case GL_TEXTURE_CUBE_MAP_NEGATIVE_Y:
          case GL_TEXTURE_CUBE_MAP_POSITIVE_Z:
          case GL_TEXTURE_CUBE_MAP_NEGATIVE_Z:
            {
                TextureCubeMap *texture = context->getTextureCubeMap();
                Error error = texture->copyImage(target, level, internalformat, x, y, width, height, framebuffer);
                if (error.isError())
                {
                    context->recordError(error);
                    return;
                }
            }
            break;

          default:
            context->recordError(Error(GL_INVALID_ENUM));
            return;
        }
    }
}

void CopyTexSubImage2D(GLenum target, GLint level, GLint xoffset, GLint yoffset, GLint x, GLint y, GLsizei width, GLsizei height)
{
    EVENT("(GLenum target = 0x%X, GLint level = %d, GLint xoffset = %d, GLint yoffset = %d, "
          "GLint x = %d, GLint y = %d, GLsizei width = %d, GLsizei height = %d)",
          target, level, xoffset, yoffset, x, y, width, height);

    Context *context = getNonLostContext();
    if (context)
    {
        if (context->getClientVersion() < 3 &&
            !ValidateES2CopyTexImageParameters(context, target, level, GL_NONE, true,
                                               xoffset, yoffset, x, y, width, height, 0))
        {
            return;
        }

        if (context->getClientVersion() >= 3 &&
            !ValidateES3CopyTexImageParameters(context, target, level, GL_NONE, true,
                                               xoffset, yoffset, 0, x, y, width, height, 0))
        {
            return;
        }

        Framebuffer *framebuffer = context->getState().getReadFramebuffer();

        switch (target)
        {
          case GL_TEXTURE_2D:
            {
                Texture2D *texture = context->getTexture2D();
                Error error = texture->copySubImage(target, level, xoffset, yoffset, 0, x, y, width, height, framebuffer);
                if (error.isError())
                {
                    context->recordError(error);
                    return;
                }
            }
            break;

          case GL_TEXTURE_CUBE_MAP_POSITIVE_X:
          case GL_TEXTURE_CUBE_MAP_NEGATIVE_X:
          case GL_TEXTURE_CUBE_MAP_POSITIVE_Y:
          case GL_TEXTURE_CUBE_MAP_NEGATIVE_Y:
          case GL_TEXTURE_CUBE_MAP_POSITIVE_Z:
          case GL_TEXTURE_CUBE_MAP_NEGATIVE_Z:
            {
                TextureCubeMap *texture = context->getTextureCubeMap();
                Error error = texture->copySubImage(target, level, xoffset, yoffset, 0, x, y, width, height, framebuffer);
                if (error.isError())
                {
                    context->recordError(error);
                    return;
                }
            }
            break;

          default:
            context->recordError(Error(GL_INVALID_ENUM));
            return;
        }
    }
}

GLuint CreateProgram(void)
{
    EVENT("()");

    Context *context = getNonLostContext();
    if (context)
    {
        return context->createProgram();
    }

    return 0;
}

GLuint CreateShader(GLenum type)
{
    EVENT("(GLenum type = 0x%X)", type);

    Context *context = getNonLostContext();
    if (context)
    {
        switch (type)
        {
          case GL_FRAGMENT_SHADER:
          case GL_VERTEX_SHADER:
            return context->createShader(type);

          default:
            context->recordError(Error(GL_INVALID_ENUM));
            return 0;
        }
    }

    return 0;
}

void CullFace(GLenum mode)
{
    EVENT("(GLenum mode = 0x%X)", mode);

    Context *context = getNonLostContext();
    if (context)
    {
        switch (mode)
        {
          case GL_FRONT:
          case GL_BACK:
          case GL_FRONT_AND_BACK:
            break;

          default:
            context->recordError(Error(GL_INVALID_ENUM));
            return;
        }

        context->getState().setCullMode(mode);
    }
}

void DeleteBuffers(GLsizei n, const GLuint* buffers)
{
    EVENT("(GLsizei n = %d, const GLuint* buffers = 0x%0.8p)", n, buffers);

    Context *context = getNonLostContext();
    if (context)
    {
        if (n < 0)
        {
            context->recordError(Error(GL_INVALID_VALUE));
            return;
        }

        for (int i = 0; i < n; i++)
        {
            context->deleteBuffer(buffers[i]);
        }
    }
}

void DeleteFencesNV(GLsizei n, const GLuint* fences)
{
    EVENT("(GLsizei n = %d, const GLuint* fences = 0x%0.8p)", n, fences);

    Context *context = getNonLostContext();
    if (context)
    {
        if (n < 0)
        {
            context->recordError(Error(GL_INVALID_VALUE));
            return;
        }

        for (int i = 0; i < n; i++)
        {
            context->deleteFenceNV(fences[i]);
        }
    }
}

void DeleteFramebuffers(GLsizei n, const GLuint* framebuffers)
{
    EVENT("(GLsizei n = %d, const GLuint* framebuffers = 0x%0.8p)", n, framebuffers);

    Context *context = getNonLostContext();
    if (context)
    {
        if (n < 0)
        {
            context->recordError(Error(GL_INVALID_VALUE));
            return;
        }

        for (int i = 0; i < n; i++)
        {
            if (framebuffers[i] != 0)
            {
                context->deleteFramebuffer(framebuffers[i]);
            }
        }
    }
}

void DeleteProgram(GLuint program)
{
    EVENT("(GLuint program = %d)", program);

    Context *context = getNonLostContext();
    if (context)
    {
        if (program == 0)
        {
            return;
        }

        if (!context->getProgram(program))
        {
            if(context->getShader(program))
            {
                context->recordError(Error(GL_INVALID_OPERATION));
                return;
            }
            else
            {
                context->recordError(Error(GL_INVALID_VALUE));
                return;
            }
        }

        context->deleteProgram(program);
    }
}

void DeleteQueriesEXT(GLsizei n, const GLuint *ids)
{
    EVENT("(GLsizei n = %d, const GLuint *ids = 0x%0.8p)", n, ids);

    Context *context = getNonLostContext();
    if (context)
    {
        if (n < 0)
        {
            context->recordError(Error(GL_INVALID_VALUE));
            return;
        }

        for (int i = 0; i < n; i++)
        {
            context->deleteQuery(ids[i]);
        }
    }
}

void DeleteRenderbuffers(GLsizei n, const GLuint* renderbuffers)
{
    EVENT("(GLsizei n = %d, const GLuint* renderbuffers = 0x%0.8p)", n, renderbuffers);

    Context *context = getNonLostContext();
    if (context)
    {
        if (n < 0)
        {
            context->recordError(Error(GL_INVALID_VALUE));
            return;
        }

        for (int i = 0; i < n; i++)
        {
            context->deleteRenderbuffer(renderbuffers[i]);
        }
    }
}

void DeleteShader(GLuint shader)
{
    EVENT("(GLuint shader = %d)", shader);

    Context *context = getNonLostContext();
    if (context)
    {
        if (shader == 0)
        {
            return;
        }

        if (!context->getShader(shader))
        {
            if(context->getProgram(shader))
            {
                context->recordError(Error(GL_INVALID_OPERATION));
                return;
            }
            else
            {
                context->recordError(Error(GL_INVALID_VALUE));
                return;
            }
        }

        context->deleteShader(shader);
    }
}

void DeleteTextures(GLsizei n, const GLuint* textures)
{
    EVENT("(GLsizei n = %d, const GLuint* textures = 0x%0.8p)", n, textures);

    Context *context = getNonLostContext();
    if (context)
    {
        if (n < 0)
        {
            context->recordError(Error(GL_INVALID_VALUE));
            return;
        }

        for (int i = 0; i < n; i++)
        {
            if (textures[i] != 0)
            {
                context->deleteTexture(textures[i]);
            }
        }
    }
}

void DepthFunc(GLenum func)
{
    EVENT("(GLenum func = 0x%X)", func);

    Context *context = getNonLostContext();
    if (context)
    {
        switch (func)
        {
          case GL_NEVER:
          case GL_ALWAYS:
          case GL_LESS:
          case GL_LEQUAL:
          case GL_EQUAL:
          case GL_GREATER:
          case GL_GEQUAL:
          case GL_NOTEQUAL:
            context->getState().setDepthFunc(func);
            break;

          default:
            context->recordError(Error(GL_INVALID_ENUM));
            return;
        }
    }
}

void DepthMask(GLboolean flag)
{
    EVENT("(GLboolean flag = %u)", flag);

    Context *context = getNonLostContext();
    if (context)
    {
        context->getState().setDepthMask(flag != GL_FALSE);
    }
}

void DepthRangef(GLclampf zNear, GLclampf zFar)
{
    EVENT("(GLclampf zNear = %f, GLclampf zFar = %f)", zNear, zFar);

    Context *context = getNonLostContext();
    if (context)
    {
        context->getState().setDepthRange(zNear, zFar);
    }
}

void DetachShader(GLuint program, GLuint shader)
{
    EVENT("(GLuint program = %d, GLuint shader = %d)", program, shader);

    Context *context = getNonLostContext();
    if (context)
    {
        Program *programObject = context->getProgram(program);
        Shader *shaderObject = context->getShader(shader);

        if (!programObject)
        {
            Shader *shaderByProgramHandle;
            shaderByProgramHandle = context->getShader(program);
            if (!shaderByProgramHandle)
            {
                context->recordError(Error(GL_INVALID_VALUE));
                return;
            }
            else
            {
                context->recordError(Error(GL_INVALID_OPERATION));
                return;
            }
        }

        if (!shaderObject)
        {
            Program *programByShaderHandle = context->getProgram(shader);
            if (!programByShaderHandle)
            {
                context->recordError(Error(GL_INVALID_VALUE));
                return;
            }
            else
            {
                context->recordError(Error(GL_INVALID_OPERATION));
                return;
            }
        }

        if (!programObject->detachShader(shaderObject))
        {
            context->recordError(Error(GL_INVALID_OPERATION));
            return;
        }
    }
}

void Disable(GLenum cap)
{
    EVENT("(GLenum cap = 0x%X)", cap);

    Context *context = getNonLostContext();
    if (context)
    {
        if (!ValidCap(context, cap))
        {
            context->recordError(Error(GL_INVALID_ENUM));
            return;
        }

        context->getState().setEnableFeature(cap, false);
    }
}

void DisableVertexAttribArray(GLuint index)
{
    EVENT("(GLuint index = %d)", index);

    Context *context = getNonLostContext();
    if (context)
    {
        if (index >= MAX_VERTEX_ATTRIBS)
        {
            context->recordError(Error(GL_INVALID_VALUE));
            return;
        }

        context->getState().setEnableVertexAttribArray(index, false);
    }
}

void DrawArrays(GLenum mode, GLint first, GLsizei count)
{
    EVENT("(GLenum mode = 0x%X, GLint first = %d, GLsizei count = %d)", mode, first, count);

    Context *context = getNonLostContext();
    if (context)
    {
        if (!ValidateDrawArrays(context, mode, first, count, 0))
        {
            return;
        }

        Error error = context->drawArrays(mode, first, count, 0);
        if (error.isError())
        {
            context->recordError(error);
            return;
        }
    }
}

void DrawArraysInstancedANGLE(GLenum mode, GLint first, GLsizei count, GLsizei primcount)
{
    EVENT("(GLenum mode = 0x%X, GLint first = %d, GLsizei count = %d, GLsizei primcount = %d)", mode, first, count, primcount);

    Context *context = getNonLostContext();
    if (context)
    {
        if (!ValidateDrawArraysInstancedANGLE(context, mode, first, count, primcount))
        {
            return;
        }

        Error error = context->drawArrays(mode, first, count, primcount);
        if (error.isError())
        {
            context->recordError(error);
            return;
        }
    }
}

void DrawElements(GLenum mode, GLsizei count, GLenum type, const GLvoid* indices)
{
    EVENT("(GLenum mode = 0x%X, GLsizei count = %d, GLenum type = 0x%X, const GLvoid* indices = 0x%0.8p)",
          mode, count, type, indices);

    Context *context = getNonLostContext();
    if (context)
    {
        rx::RangeUI indexRange;
        if (!ValidateDrawElements(context, mode, count, type, indices, 0, &indexRange))
        {
            return;
        }

        Error error = context->drawElements(mode, count, type, indices, 0, indexRange);
        if (error.isError())
        {
            context->recordError(error);
            return;
        }
    }
}

void DrawElementsInstancedANGLE(GLenum mode, GLsizei count, GLenum type, const GLvoid *indices, GLsizei primcount)
{
    EVENT("(GLenum mode = 0x%X, GLsizei count = %d, GLenum type = 0x%X, const GLvoid* indices = 0x%0.8p, GLsizei primcount = %d)",
          mode, count, type, indices, primcount);

    Context *context = getNonLostContext();
    if (context)
    {
        rx::RangeUI indexRange;
        if (!ValidateDrawElementsInstancedANGLE(context, mode, count, type, indices, primcount, &indexRange))
        {
            return;
        }

        Error error = context->drawElements(mode, count, type, indices, primcount, indexRange);
        if (error.isError())
        {
            context->recordError(error);
            return;
        }
    }
}

void Enable(GLenum cap)
{
    EVENT("(GLenum cap = 0x%X)", cap);

    Context *context = getNonLostContext();
    if (context)
    {
        if (!ValidCap(context, cap))
        {
            context->recordError(Error(GL_INVALID_ENUM));
            return;
        }

        context->getState().setEnableFeature(cap, true);
    }
}

void EnableVertexAttribArray(GLuint index)
{
    EVENT("(GLuint index = %d)", index);

    Context *context = getNonLostContext();
    if (context)
    {
        if (index >= MAX_VERTEX_ATTRIBS)
        {
            context->recordError(Error(GL_INVALID_VALUE));
            return;
        }

        context->getState().setEnableVertexAttribArray(index, true);
    }
}

void EndQueryEXT(GLenum target)
{
    EVENT("GLenum target = 0x%X)", target);

    Context *context = getNonLostContext();
    if (context)
    {
        if (!ValidateEndQuery(context, target))
        {
            return;
        }

        Error error = context->endQuery(target);
        if (error.isError())
        {
            context->recordError(error);
            return;
        }
    }
}

void FinishFenceNV(GLuint fence)
{
    EVENT("(GLuint fence = %d)", fence);

    Context *context = getNonLostContext();
    if (context)
    {
        FenceNV *fenceObject = context->getFenceNV(fence);

        if (fenceObject == NULL)
        {
            context->recordError(Error(GL_INVALID_OPERATION));
            return;
        }

        if (fenceObject->isFence() != GL_TRUE)
        {
            context->recordError(Error(GL_INVALID_OPERATION));
            return;
        }

        fenceObject->finishFence();
    }
}

void Finish(void)
{
    EVENT("()");

    Context *context = getNonLostContext();
    if (context)
    {
        Error error = context->sync(true);
        if (error.isError())
        {
            context->recordError(error);
            return;
        }
    }
}

void Flush(void)
{
    EVENT("()");

    Context *context = getNonLostContext();
    if (context)
    {
        Error error = context->sync(false);
        if (error.isError())
        {
            context->recordError(error);
            return;
        }
    }
}

void FramebufferRenderbuffer(GLenum target, GLenum attachment, GLenum renderbuffertarget, GLuint renderbuffer)
{
    EVENT("(GLenum target = 0x%X, GLenum attachment = 0x%X, GLenum renderbuffertarget = 0x%X, "
          "GLuint renderbuffer = %d)", target, attachment, renderbuffertarget, renderbuffer);

    Context *context = getNonLostContext();
    if (context)
    {
        if (!ValidFramebufferTarget(target) || (renderbuffertarget != GL_RENDERBUFFER && renderbuffer != 0))
        {
            context->recordError(Error(GL_INVALID_ENUM));
            return;
        }

        if (!ValidateFramebufferRenderbufferParameters(context, target, attachment, renderbuffertarget, renderbuffer))
        {
            return;
        }

        Framebuffer *framebuffer = context->getState().getTargetFramebuffer(target);
        ASSERT(framebuffer);

        if (renderbuffer != 0)
        {
            Renderbuffer *renderbufferObject = context->getRenderbuffer(renderbuffer);
            framebuffer->setRenderbufferAttachment(attachment, renderbufferObject);
        }
        else
        {
            framebuffer->setNULLAttachment(attachment);
        }
    }
}

void FramebufferTexture2D(GLenum target, GLenum attachment, GLenum textarget, GLuint texture, GLint level)
{
    EVENT("(GLenum target = 0x%X, GLenum attachment = 0x%X, GLenum textarget = 0x%X, "
          "GLuint texture = %d, GLint level = %d)", target, attachment, textarget, texture, level);

    Context *context = getNonLostContext();
    if (context)
    {
        if (!ValidateFramebufferTexture2D(context, target, attachment, textarget, texture, level))
        {
            return;
        }

        Framebuffer *framebuffer = context->getState().getTargetFramebuffer(target);
        ASSERT(framebuffer);

        if (texture != 0)
        {
            Texture *textureObj = context->getTexture(texture);
            ImageIndex index(textarget, level, ImageIndex::ENTIRE_LEVEL);
            framebuffer->setTextureAttachment(attachment, textureObj, index);
        }
        else
        {
            framebuffer->setNULLAttachment(attachment);
        }
    }
}

void FrontFace(GLenum mode)
{
    EVENT("(GLenum mode = 0x%X)", mode);

    Context *context = getNonLostContext();
    if (context)
    {
        switch (mode)
        {
          case GL_CW:
          case GL_CCW:
            context->getState().setFrontFace(mode);
            break;
          default:
            context->recordError(Error(GL_INVALID_ENUM));
            return;
        }
    }
}

void GenBuffers(GLsizei n, GLuint* buffers)
{
    EVENT("(GLsizei n = %d, GLuint* buffers = 0x%0.8p)", n, buffers);

    Context *context = getNonLostContext();
    if (context)
    {
        if (n < 0)
        {
            context->recordError(Error(GL_INVALID_VALUE));
            return;
        }

        for (int i = 0; i < n; i++)
        {
            buffers[i] = context->createBuffer();
        }
    }
}

void GenerateMipmap(GLenum target)
{
    EVENT("(GLenum target = 0x%X)", target);

    Context *context = getNonLostContext();
    if (context)
    {
        if (!ValidTextureTarget(context, target))
        {
            context->recordError(Error(GL_INVALID_ENUM));
            return;
        }

        Texture *texture = context->getTargetTexture(target);

        if (texture == NULL)
        {
            context->recordError(Error(GL_INVALID_OPERATION));
            return;
        }

        GLenum internalFormat = texture->getBaseLevelInternalFormat();
        const TextureCaps &formatCaps = context->getTextureCaps().get(internalFormat);
        const InternalFormat &formatInfo = GetInternalFormatInfo(internalFormat);

        // GenerateMipmap should not generate an INVALID_OPERATION for textures created with
        // unsized formats or that are color renderable and filterable.  Since we do not track if
        // the texture was created with sized or unsized format (only sized formats are stored),
        // it is not possible to make sure the the LUMA formats can generate mipmaps (they should
        // be able to) because they aren't color renderable.  Simply do a special case for LUMA
        // textures since they're the only texture format that can be created with unsized formats
        // that is not color renderable.  New unsized formats are unlikely to be added, since ES2
        // was the last version to use add them.
        bool isLUMA = internalFormat == GL_LUMINANCE8_EXT ||
                      internalFormat == GL_LUMINANCE8_ALPHA8_EXT ||
                      internalFormat == GL_ALPHA8_EXT;

        if (formatInfo.depthBits > 0 || formatInfo.stencilBits > 0 || !formatCaps.filterable ||
            (!formatCaps.renderable && !isLUMA) || formatInfo.compressed)
        {
            context->recordError(Error(GL_INVALID_OPERATION));
            return;
        }

        // GL_EXT_sRGB does not support mipmap generation on sRGB textures
        if (context->getClientVersion() == 2 && formatInfo.colorEncoding == GL_SRGB)
        {
            context->recordError(Error(GL_INVALID_OPERATION));
            return;
        }

        // Non-power of 2 ES2 check
        if (!context->getExtensions().textureNPOT && (!isPow2(texture->getBaseLevelWidth()) || !isPow2(texture->getBaseLevelHeight())))
        {
            ASSERT(context->getClientVersion() <= 2 && (target == GL_TEXTURE_2D || target == GL_TEXTURE_CUBE_MAP));
            context->recordError(Error(GL_INVALID_OPERATION));
            return;
        }

        // Cube completeness check
        if (target == GL_TEXTURE_CUBE_MAP)
        {
            TextureCubeMap *textureCube = static_cast<TextureCubeMap *>(texture);
            if (!textureCube->isCubeComplete())
            {
                context->recordError(Error(GL_INVALID_OPERATION));
                return;
            }
        }

        Error error = texture->generateMipmaps();
        if (error.isError())
        {
            context->recordError(error);
            return;
        }
    }
}

void GenFencesNV(GLsizei n, GLuint* fences)
{
    EVENT("(GLsizei n = %d, GLuint* fences = 0x%0.8p)", n, fences);

    Context *context = getNonLostContext();
    if (context)
    {
        if (n < 0)
        {
            context->recordError(Error(GL_INVALID_VALUE));
            return;
        }

        for (int i = 0; i < n; i++)
        {
            fences[i] = context->createFenceNV();
        }
    }
}

void GenFramebuffers(GLsizei n, GLuint* framebuffers)
{
    EVENT("(GLsizei n = %d, GLuint* framebuffers = 0x%0.8p)", n, framebuffers);

    Context *context = getNonLostContext();
    if (context)
    {
        if (n < 0)
        {
            context->recordError(Error(GL_INVALID_VALUE));
            return;
        }

        for (int i = 0; i < n; i++)
        {
            framebuffers[i] = context->createFramebuffer();
        }
    }
}

void GenQueriesEXT(GLsizei n, GLuint* ids)
{
    EVENT("(GLsizei n = %d, GLuint* ids = 0x%0.8p)", n, ids);

    Context *context = getNonLostContext();
    if (context)
    {
        if (n < 0)
        {
            context->recordError(Error(GL_INVALID_VALUE));
            return;
        }

        for (GLsizei i = 0; i < n; i++)
        {
            ids[i] = context->createQuery();
        }
    }
}

void GenRenderbuffers(GLsizei n, GLuint* renderbuffers)
{
    EVENT("(GLsizei n = %d, GLuint* renderbuffers = 0x%0.8p)", n, renderbuffers);

    Context *context = getNonLostContext();
    if (context)
    {
        if (n < 0)
        {
            context->recordError(Error(GL_INVALID_VALUE));
            return;
        }

        for (int i = 0; i < n; i++)
        {
            renderbuffers[i] = context->createRenderbuffer();
        }
    }
}

void GenTextures(GLsizei n, GLuint* textures)
{
    EVENT("(GLsizei n = %d, GLuint* textures = 0x%0.8p)", n, textures);

    Context *context = getNonLostContext();
    if (context)
    {
        if (n < 0)
        {
            context->recordError(Error(GL_INVALID_VALUE));
            return;
        }

        for (int i = 0; i < n; i++)
        {
            textures[i] = context->createTexture();
        }
    }
}

void GetActiveAttrib(GLuint program, GLuint index, GLsizei bufsize, GLsizei *length, GLint *size, GLenum *type, GLchar *name)
{
    EVENT("(GLuint program = %d, GLuint index = %d, GLsizei bufsize = %d, GLsizei *length = 0x%0.8p, "
          "GLint *size = 0x%0.8p, GLenum *type = %0.8p, GLchar *name = %0.8p)",
          program, index, bufsize, length, size, type, name);

    Context *context = getNonLostContext();
    if (context)
    {
        if (bufsize < 0)
        {
            context->recordError(Error(GL_INVALID_VALUE));
            return;
        }

        Program *programObject = context->getProgram(program);

        if (!programObject)
        {
            if (context->getShader(program))
            {
                context->recordError(Error(GL_INVALID_OPERATION));
                return;
            }
            else
            {
                context->recordError(Error(GL_INVALID_VALUE));
                return;
            }
        }

        if (index >= (GLuint)programObject->getActiveAttributeCount())
        {
            context->recordError(Error(GL_INVALID_VALUE));
            return;
        }

        programObject->getActiveAttribute(index, bufsize, length, size, type, name);
    }
}

void GetActiveUniform(GLuint program, GLuint index, GLsizei bufsize, GLsizei* length, GLint* size, GLenum* type, GLchar* name)
{
    EVENT("(GLuint program = %d, GLuint index = %d, GLsizei bufsize = %d, "
          "GLsizei* length = 0x%0.8p, GLint* size = 0x%0.8p, GLenum* type = 0x%0.8p, GLchar* name = 0x%0.8p)",
          program, index, bufsize, length, size, type, name);


    Context *context = getNonLostContext();
    if (context)
    {
        if (bufsize < 0)
        {
            context->recordError(Error(GL_INVALID_VALUE));
            return;
        }

        Program *programObject = context->getProgram(program);

        if (!programObject)
        {
            if (context->getShader(program))
            {
                context->recordError(Error(GL_INVALID_OPERATION));
                return;
            }
            else
            {
                context->recordError(Error(GL_INVALID_VALUE));
                return;
            }
        }

        if (index >= (GLuint)programObject->getActiveUniformCount())
        {
            context->recordError(Error(GL_INVALID_VALUE));
            return;
        }

        programObject->getActiveUniform(index, bufsize, length, size, type, name);
    }
}

void GetAttachedShaders(GLuint program, GLsizei maxcount, GLsizei* count, GLuint* shaders)
{
    EVENT("(GLuint program = %d, GLsizei maxcount = %d, GLsizei* count = 0x%0.8p, GLuint* shaders = 0x%0.8p)",
          program, maxcount, count, shaders);

    Context *context = getNonLostContext();
    if (context)
    {
        if (maxcount < 0)
        {
            context->recordError(Error(GL_INVALID_VALUE));
            return;
        }

        Program *programObject = context->getProgram(program);

        if (!programObject)
        {
            if (context->getShader(program))
            {
                context->recordError(Error(GL_INVALID_OPERATION));
                return;
            }
            else
            {
                context->recordError(Error(GL_INVALID_VALUE));
                return;
            }
        }

        return programObject->getAttachedShaders(maxcount, count, shaders);
    }
}

GLint GetAttribLocation(GLuint program, const GLchar* name)
{
    EVENT("(GLuint program = %d, const GLchar* name = %s)", program, name);

    Context *context = getNonLostContext();
    if (context)
    {
        Program *programObject = context->getProgram(program);

        if (!programObject)
        {
            if (context->getShader(program))
            {
                context->recordError(Error(GL_INVALID_OPERATION));
                return -1;
            }
            else
            {
                context->recordError(Error(GL_INVALID_VALUE));
                return -1;
            }
        }

        if (!programObject->isLinked())
        {
            context->recordError(Error(GL_INVALID_OPERATION));
            return -1;
        }

        return programObject->getAttributeLocation(name);
    }

    return -1;
}

void GetBooleanv(GLenum pname, GLboolean* params)
{
    EVENT("(GLenum pname = 0x%X, GLboolean* params = 0x%0.8p)",  pname, params);

    Context *context = getNonLostContext();
    if (context)
    {
        GLenum nativeType;
        unsigned int numParams = 0;
        if (!ValidateStateQuery(context, pname, &nativeType, &numParams))
        {
            return;
        }

        if (nativeType == GL_BOOL)
        {
            context->getBooleanv(pname, params);
        }
        else
        {
            CastStateValues(context, nativeType, pname, numParams, params);
        }
    }
}

void GetBufferParameteriv(GLenum target, GLenum pname, GLint* params)
{
    EVENT("(GLenum target = 0x%X, GLenum pname = 0x%X, GLint* params = 0x%0.8p)", target, pname, params);

    Context *context = getNonLostContext();
    if (context)
    {
        if (!ValidBufferTarget(context, target))
        {
            context->recordError(Error(GL_INVALID_ENUM));
            return;
        }

        if (!ValidBufferParameter(context, pname))
        {
            context->recordError(Error(GL_INVALID_ENUM));
            return;
        }

        Buffer *buffer = context->getState().getTargetBuffer(target);

        if (!buffer)
        {
            // A null buffer means that "0" is bound to the requested buffer target
            context->recordError(Error(GL_INVALID_OPERATION));
            return;
        }

        switch (pname)
        {
          case GL_BUFFER_USAGE:
            *params = static_cast<GLint>(buffer->getUsage());
            break;
          case GL_BUFFER_SIZE:
            *params = clampCast<GLint>(buffer->getSize());
            break;
          case GL_BUFFER_ACCESS_FLAGS:
            *params = buffer->getAccessFlags();
            break;
          case GL_BUFFER_MAPPED:
            *params = static_cast<GLint>(buffer->isMapped());
            break;
          case GL_BUFFER_MAP_OFFSET:
            *params = clampCast<GLint>(buffer->getMapOffset());
            break;
          case GL_BUFFER_MAP_LENGTH:
            *params = clampCast<GLint>(buffer->getMapLength());
            break;
          default: UNREACHABLE(); break;
        }
    }
}

GLenum GetError(void)
{
    EVENT("()");

    Context *context = getCurrentContext();

    if (context)
    {
        return context->getError();
    }

    return GL_NO_ERROR;
}

void GetFenceivNV(GLuint fence, GLenum pname, GLint *params)
{
    EVENT("(GLuint fence = %d, GLenum pname = 0x%X, GLint *params = 0x%0.8p)", fence, pname, params);

    Context *context = getNonLostContext();
    if (context)
    {
        FenceNV *fenceObject = context->getFenceNV(fence);

        if (fenceObject == NULL)
        {
            context->recordError(Error(GL_INVALID_OPERATION));
            return;
        }

        if (fenceObject->isFence() != GL_TRUE)
        {
            context->recordError(Error(GL_INVALID_OPERATION));
            return;
        }

        switch (pname)
        {
          case GL_FENCE_STATUS_NV:
            {
                // GL_NV_fence spec:
                // Once the status of a fence has been finished (via FinishFenceNV) or tested and the returned status is TRUE (via either TestFenceNV
                // or GetFenceivNV querying the FENCE_STATUS_NV), the status remains TRUE until the next SetFenceNV of the fence.
                GLboolean status = GL_TRUE;
                if (fenceObject->getStatus() != GL_TRUE)
                {
                    Error error = fenceObject->testFence(&status);
                    if (error.isError())
                    {
                        context->recordError(error);
                        return;
                    }
                }
                *params = status;
                break;
            }

          case GL_FENCE_CONDITION_NV:
            {
                *params = fenceObject->getCondition();
                break;
            }

          default:
            {
                context->recordError(Error(GL_INVALID_ENUM));
                return;
            }
        }
    }
}

void GetFloatv(GLenum pname, GLfloat* params)
{
    EVENT("(GLenum pname = 0x%X, GLfloat* params = 0x%0.8p)", pname, params);

    Context *context = getNonLostContext();
    if (context)
    {
        GLenum nativeType;
        unsigned int numParams = 0;
        if (!ValidateStateQuery(context, pname, &nativeType, &numParams))
        {
            return;
        }

        if (nativeType == GL_FLOAT)
        {
            context->getFloatv(pname, params);
        }
        else
        {
            CastStateValues(context, nativeType, pname, numParams, params);
        }
    }
}

void GetFramebufferAttachmentParameteriv(GLenum target, GLenum attachment, GLenum pname, GLint* params)
{
    EVENT("(GLenum target = 0x%X, GLenum attachment = 0x%X, GLenum pname = 0x%X, GLint* params = 0x%0.8p)",
          target, attachment, pname, params);

    Context *context = getNonLostContext();
    if (context)
    {
        if (!ValidFramebufferTarget(target))
        {
            context->recordError(Error(GL_INVALID_ENUM));
            return;
        }

        int clientVersion = context->getClientVersion();

        switch (pname)
        {
          case GL_FRAMEBUFFER_ATTACHMENT_OBJECT_TYPE:
          case GL_FRAMEBUFFER_ATTACHMENT_OBJECT_NAME:
          case GL_FRAMEBUFFER_ATTACHMENT_TEXTURE_LEVEL:
          case GL_FRAMEBUFFER_ATTACHMENT_TEXTURE_CUBE_MAP_FACE:
            break;

          case GL_FRAMEBUFFER_ATTACHMENT_COLOR_ENCODING:
            if (clientVersion < 3 && !context->getExtensions().sRGB)
            {
                context->recordError(Error(GL_INVALID_ENUM));
                return;
            }
            break;

          case GL_FRAMEBUFFER_ATTACHMENT_RED_SIZE:
          case GL_FRAMEBUFFER_ATTACHMENT_GREEN_SIZE:
          case GL_FRAMEBUFFER_ATTACHMENT_BLUE_SIZE:
          case GL_FRAMEBUFFER_ATTACHMENT_ALPHA_SIZE:
          case GL_FRAMEBUFFER_ATTACHMENT_DEPTH_SIZE:
          case GL_FRAMEBUFFER_ATTACHMENT_STENCIL_SIZE:
          case GL_FRAMEBUFFER_ATTACHMENT_COMPONENT_TYPE:
          case GL_FRAMEBUFFER_ATTACHMENT_TEXTURE_LAYER:
            if (clientVersion < 3)
            {
                context->recordError(Error(GL_INVALID_ENUM));
                return;
            }
            break;

          default:
            context->recordError(Error(GL_INVALID_ENUM));
            return;
        }

        // Determine if the attachment is a valid enum
        switch (attachment)
        {
          case GL_BACK:
          case GL_FRONT:
          case GL_DEPTH:
          case GL_STENCIL:
          case GL_DEPTH_STENCIL_ATTACHMENT:
            if (clientVersion < 3)
            {
                context->recordError(Error(GL_INVALID_ENUM));
                return;
            }
            break;

          case GL_DEPTH_ATTACHMENT:
          case GL_STENCIL_ATTACHMENT:
            break;

          default:
            if (attachment < GL_COLOR_ATTACHMENT0_EXT ||
                (attachment - GL_COLOR_ATTACHMENT0_EXT) >= context->getCaps().maxColorAttachments)
            {
                context->recordError(Error(GL_INVALID_ENUM));
                return;
            }
            break;
        }

        Framebuffer *framebuffer = context->getState().getTargetFramebuffer(target);
        ASSERT(framebuffer);

        if (framebuffer->id() == 0)
        {
            if (clientVersion < 3)
            {
                context->recordError(Error(GL_INVALID_OPERATION));
                return;
            }

            switch (attachment)
            {
              case GL_BACK:
              case GL_DEPTH:
              case GL_STENCIL:
                break;

              default:
                context->recordError(Error(GL_INVALID_OPERATION));
                return;
            }
        }
        else
        {
            if (attachment >= GL_COLOR_ATTACHMENT0_EXT && attachment <= GL_COLOR_ATTACHMENT15_EXT)
            {
                // Valid attachment query
            }
            else
            {
                switch (attachment)
                {
                  case GL_DEPTH_ATTACHMENT:
                  case GL_STENCIL_ATTACHMENT:
                    break;

                  case GL_DEPTH_STENCIL_ATTACHMENT:
                    if (framebuffer->hasValidDepthStencil())
                    {
                        context->recordError(Error(GL_INVALID_OPERATION));
                        return;
                    }
                    break;

                  default:
                    context->recordError(Error(GL_INVALID_OPERATION));
                    return;
                }
            }
        }

        const FramebufferAttachment *attachmentObject = framebuffer->getAttachment(attachment);
        if (attachmentObject)
        {
            ASSERT(attachmentObject->type() == GL_RENDERBUFFER ||
                   attachmentObject->type() == GL_TEXTURE ||
                   attachmentObject->type() == GL_FRAMEBUFFER_DEFAULT);

            switch (pname)
            {
              case GL_FRAMEBUFFER_ATTACHMENT_OBJECT_TYPE:
                *params = attachmentObject->type();
                break;

              case GL_FRAMEBUFFER_ATTACHMENT_OBJECT_NAME:
                if (attachmentObject->type() != GL_RENDERBUFFER && attachmentObject->type() != GL_TEXTURE)
                {
                    context->recordError(Error(GL_INVALID_ENUM));
                    return;
                }
                *params = attachmentObject->id();
                break;

              case GL_FRAMEBUFFER_ATTACHMENT_TEXTURE_LEVEL:
                if (attachmentObject->type() != GL_TEXTURE)
                {
                    context->recordError(Error(GL_INVALID_ENUM));
                    return;
                }
                *params = attachmentObject->mipLevel();
                break;

              case GL_FRAMEBUFFER_ATTACHMENT_TEXTURE_CUBE_MAP_FACE:
                if (attachmentObject->type() != GL_TEXTURE)
                {
                    context->recordError(Error(GL_INVALID_ENUM));
                    return;
                }
                *params = attachmentObject->cubeMapFace();
                break;

              case GL_FRAMEBUFFER_ATTACHMENT_RED_SIZE:
                *params = attachmentObject->getRedSize();
                break;

              case GL_FRAMEBUFFER_ATTACHMENT_GREEN_SIZE:
                *params = attachmentObject->getGreenSize();
                break;

              case GL_FRAMEBUFFER_ATTACHMENT_BLUE_SIZE:
                *params = attachmentObject->getBlueSize();
                break;

              case GL_FRAMEBUFFER_ATTACHMENT_ALPHA_SIZE:
                *params = attachmentObject->getAlphaSize();
                break;

              case GL_FRAMEBUFFER_ATTACHMENT_DEPTH_SIZE:
                *params = attachmentObject->getDepthSize();
                break;

              case GL_FRAMEBUFFER_ATTACHMENT_STENCIL_SIZE:
                *params = attachmentObject->getStencilSize();
                break;

              case GL_FRAMEBUFFER_ATTACHMENT_COMPONENT_TYPE:
                if (attachment == GL_DEPTH_STENCIL_ATTACHMENT)
                {
                    context->recordError(Error(GL_INVALID_OPERATION));
                    return;
                }
                *params = attachmentObject->getComponentType();
                break;

              case GL_FRAMEBUFFER_ATTACHMENT_COLOR_ENCODING:
                *params = attachmentObject->getColorEncoding();
                break;

              case GL_FRAMEBUFFER_ATTACHMENT_TEXTURE_LAYER:
                if (attachmentObject->type() != GL_TEXTURE)
                {
                    context->recordError(Error(GL_INVALID_ENUM));
                    return;
                }
                *params = attachmentObject->layer();
                break;

              default:
                UNREACHABLE();
                break;
            }
        }
        else
        {
            // ES 2.0.25 spec pg 127 states that if the value of FRAMEBUFFER_ATTACHMENT_OBJECT_TYPE
            // is NONE, then querying any other pname will generate INVALID_ENUM.

            // ES 3.0.2 spec pg 235 states that if the attachment type is none,
            // GL_FRAMEBUFFER_ATTACHMENT_OBJECT_NAME will return zero and be an
            // INVALID_OPERATION for all other pnames

            switch (pname)
            {
              case GL_FRAMEBUFFER_ATTACHMENT_OBJECT_TYPE:
                *params = GL_NONE;
                break;

              case GL_FRAMEBUFFER_ATTACHMENT_OBJECT_NAME:
                if (clientVersion < 3)
                {
                    context->recordError(Error(GL_INVALID_ENUM));
                    return;
                }
                *params = 0;
                break;

              default:
                if (clientVersion < 3)
                {
                    context->recordError(Error(GL_INVALID_ENUM));
                    return;
                }
                else
                {
                    context->recordError(Error(GL_INVALID_OPERATION));
                    return;
                }
            }
        }
    }
}

GLenum GetGraphicsResetStatusEXT(void)
{
    EVENT("()");

    Context *context = getCurrentContext();

    if (context)
    {
        return context->getResetStatus();
    }

    return GL_NO_ERROR;
}

void GetIntegerv(GLenum pname, GLint* params)
{
    EVENT("(GLenum pname = 0x%X, GLint* params = 0x%0.8p)", pname, params);

    Context *context = getNonLostContext();
    if (context)
    {
        GLenum nativeType;
        unsigned int numParams = 0;

        if (!ValidateStateQuery(context, pname, &nativeType, &numParams))
        {
            return;
        }

        if (nativeType == GL_INT)
        {
            context->getIntegerv(pname, params);
        }
        else
        {
            CastStateValues(context, nativeType, pname, numParams, params);
        }
    }
}

void GetProgramiv(GLuint program, GLenum pname, GLint* params)
{
    EVENT("(GLuint program = %d, GLenum pname = %d, GLint* params = 0x%0.8p)", program, pname, params);

    Context *context = getNonLostContext();
    if (context)
    {
        Program *programObject = context->getProgram(program);

        if (!programObject)
        {
            context->recordError(Error(GL_INVALID_VALUE));
            return;
        }

        if (context->getClientVersion() < 3)
        {
            switch (pname)
            {
              case GL_ACTIVE_UNIFORM_BLOCKS:
              case GL_ACTIVE_UNIFORM_BLOCK_MAX_NAME_LENGTH:
              case GL_TRANSFORM_FEEDBACK_BUFFER_MODE:
              case GL_TRANSFORM_FEEDBACK_VARYINGS:
              case GL_TRANSFORM_FEEDBACK_VARYING_MAX_LENGTH:
                context->recordError(Error(GL_INVALID_ENUM));
                return;
            }
        }

        switch (pname)
        {
          case GL_DELETE_STATUS:
            *params = programObject->isFlaggedForDeletion();
            return;
          case GL_LINK_STATUS:
            *params = programObject->isLinked();
            return;
          case GL_VALIDATE_STATUS:
            *params = programObject->isValidated();
            return;
          case GL_INFO_LOG_LENGTH:
            *params = programObject->getInfoLogLength();
            return;
          case GL_ATTACHED_SHADERS:
            *params = programObject->getAttachedShadersCount();
            return;
          case GL_ACTIVE_ATTRIBUTES:
            *params = programObject->getActiveAttributeCount();
            return;
          case GL_ACTIVE_ATTRIBUTE_MAX_LENGTH:
            *params = programObject->getActiveAttributeMaxLength();
            return;
          case GL_ACTIVE_UNIFORMS:
            *params = programObject->getActiveUniformCount();
            return;
          case GL_ACTIVE_UNIFORM_MAX_LENGTH:
            *params = programObject->getActiveUniformMaxLength();
            return;
          case GL_PROGRAM_BINARY_LENGTH_OES:
            *params = programObject->getBinaryLength();
            return;
          case GL_ACTIVE_UNIFORM_BLOCKS:
            *params = programObject->getActiveUniformBlockCount();
            return;
          case GL_ACTIVE_UNIFORM_BLOCK_MAX_NAME_LENGTH:
            *params = programObject->getActiveUniformBlockMaxLength();
            break;
          case GL_TRANSFORM_FEEDBACK_BUFFER_MODE:
            *params = programObject->getTransformFeedbackBufferMode();
            break;
          case GL_TRANSFORM_FEEDBACK_VARYINGS:
            *params = programObject->getTransformFeedbackVaryingCount();
            break;
          case GL_TRANSFORM_FEEDBACK_VARYING_MAX_LENGTH:
            *params = programObject->getTransformFeedbackVaryingMaxLength();
            break;

          default:
            context->recordError(Error(GL_INVALID_ENUM));
            return;
        }
    }
}

void GetProgramInfoLog(GLuint program, GLsizei bufsize, GLsizei* length, GLchar* infolog)
{
    EVENT("(GLuint program = %d, GLsizei bufsize = %d, GLsizei* length = 0x%0.8p, GLchar* infolog = 0x%0.8p)",
          program, bufsize, length, infolog);

    Context *context = getNonLostContext();
    if (context)
    {
        if (bufsize < 0)
        {
            context->recordError(Error(GL_INVALID_VALUE));
            return;
        }

        Program *programObject = context->getProgram(program);

        if (!programObject)
        {
            context->recordError(Error(GL_INVALID_VALUE));
            return;
        }

        programObject->getInfoLog(bufsize, length, infolog);
    }
}

void GetQueryivEXT(GLenum target, GLenum pname, GLint *params)
{
    EVENT("GLenum target = 0x%X, GLenum pname = 0x%X, GLint *params = 0x%0.8p)", target, pname, params);

    Context *context = getNonLostContext();
    if (context)
    {
        if (!ValidQueryType(context, target))
        {
            context->recordError(Error(GL_INVALID_ENUM));
            return;
        }

        switch (pname)
        {
          case GL_CURRENT_QUERY_EXT:
            params[0] = context->getState().getActiveQueryId(target);
            break;

          default:
            context->recordError(Error(GL_INVALID_ENUM));
            return;
        }
    }
}

void GetQueryObjectuivEXT(GLuint id, GLenum pname, GLuint *params)
{
    EVENT("(GLuint id = %d, GLenum pname = 0x%X, GLuint *params = 0x%0.8p)", id, pname, params);

    Context *context = getNonLostContext();
    if (context)
    {
        Query *queryObject = context->getQuery(id, false, GL_NONE);

        if (!queryObject)
        {
            context->recordError(Error(GL_INVALID_OPERATION));
            return;
        }

        if (context->getState().getActiveQueryId(queryObject->getType()) == id)
        {
            context->recordError(Error(GL_INVALID_OPERATION));
            return;
        }

        switch(pname)
        {
          case GL_QUERY_RESULT_EXT:
            {
                Error error = queryObject->getResult(params);
                if (error.isError())
                {
                    context->recordError(error);
                    return;
                }
            }
            break;

          case GL_QUERY_RESULT_AVAILABLE_EXT:
            {
                Error error = queryObject->isResultAvailable(params);
                if (error.isError())
                {
                    context->recordError(error);
                    return;
                }
            }
            break;

          default:
            context->recordError(Error(GL_INVALID_ENUM));
            return;
        }
    }
}

void GetRenderbufferParameteriv(GLenum target, GLenum pname, GLint* params)
{
    EVENT("(GLenum target = 0x%X, GLenum pname = 0x%X, GLint* params = 0x%0.8p)", target, pname, params);

    Context *context = getNonLostContext();
    if (context)
    {
        if (target != GL_RENDERBUFFER)
        {
            context->recordError(Error(GL_INVALID_ENUM));
            return;
        }

        if (context->getState().getRenderbufferId() == 0)
        {
            context->recordError(Error(GL_INVALID_OPERATION));
            return;
        }

        Renderbuffer *renderbuffer = context->getRenderbuffer(context->getState().getRenderbufferId());

        switch (pname)
        {
          case GL_RENDERBUFFER_WIDTH:           *params = renderbuffer->getWidth();          break;
          case GL_RENDERBUFFER_HEIGHT:          *params = renderbuffer->getHeight();         break;
          case GL_RENDERBUFFER_INTERNAL_FORMAT: *params = renderbuffer->getInternalFormat(); break;
          case GL_RENDERBUFFER_RED_SIZE:        *params = renderbuffer->getRedSize();        break;
          case GL_RENDERBUFFER_GREEN_SIZE:      *params = renderbuffer->getGreenSize();      break;
          case GL_RENDERBUFFER_BLUE_SIZE:       *params = renderbuffer->getBlueSize();       break;
          case GL_RENDERBUFFER_ALPHA_SIZE:      *params = renderbuffer->getAlphaSize();      break;
          case GL_RENDERBUFFER_DEPTH_SIZE:      *params = renderbuffer->getDepthSize();      break;
          case GL_RENDERBUFFER_STENCIL_SIZE:    *params = renderbuffer->getStencilSize();    break;

          case GL_RENDERBUFFER_SAMPLES_ANGLE:
            if (!context->getExtensions().framebufferMultisample)
            {
                context->recordError(Error(GL_INVALID_ENUM));
                return;
            }
            *params = renderbuffer->getSamples();
            break;

          default:
            context->recordError(Error(GL_INVALID_ENUM));
            return;
        }
    }
}

void GetShaderiv(GLuint shader, GLenum pname, GLint* params)
{
    EVENT("(GLuint shader = %d, GLenum pname = %d, GLint* params = 0x%0.8p)", shader, pname, params);

    Context *context = getNonLostContext();
    if (context)
    {
        Shader *shaderObject = context->getShader(shader);

        if (!shaderObject)
        {
            context->recordError(Error(GL_INVALID_VALUE));
            return;
        }

        switch (pname)
        {
          case GL_SHADER_TYPE:
            *params = shaderObject->getType();
            return;
          case GL_DELETE_STATUS:
            *params = shaderObject->isFlaggedForDeletion();
            return;
          case GL_COMPILE_STATUS:
            *params = shaderObject->isCompiled() ? GL_TRUE : GL_FALSE;
            return;
          case GL_INFO_LOG_LENGTH:
            *params = shaderObject->getInfoLogLength();
            return;
          case GL_SHADER_SOURCE_LENGTH:
            *params = shaderObject->getSourceLength();
            return;
          case GL_TRANSLATED_SHADER_SOURCE_LENGTH_ANGLE:
            *params = shaderObject->getTranslatedSourceLength();
            return;

          default:
            context->recordError(Error(GL_INVALID_ENUM));
            return;
        }
    }
}

void GetShaderInfoLog(GLuint shader, GLsizei bufsize, GLsizei* length, GLchar* infolog)
{
    EVENT("(GLuint shader = %d, GLsizei bufsize = %d, GLsizei* length = 0x%0.8p, GLchar* infolog = 0x%0.8p)",
          shader, bufsize, length, infolog);

    Context *context = getNonLostContext();
    if (context)
    {
        if (bufsize < 0)
        {
            context->recordError(Error(GL_INVALID_VALUE));
            return;
        }

        Shader *shaderObject = context->getShader(shader);

        if (!shaderObject)
        {
            context->recordError(Error(GL_INVALID_VALUE));
            return;
        }

        shaderObject->getInfoLog(bufsize, length, infolog);
    }
}

void GetShaderPrecisionFormat(GLenum shadertype, GLenum precisiontype, GLint* range, GLint* precision)
{
    EVENT("(GLenum shadertype = 0x%X, GLenum precisiontype = 0x%X, GLint* range = 0x%0.8p, GLint* precision = 0x%0.8p)",
          shadertype, precisiontype, range, precision);

    Context *context = getNonLostContext();
    if (context)
    {
        switch (shadertype)
        {
          case GL_VERTEX_SHADER:
          case GL_FRAGMENT_SHADER:
            break;

          default:
            context->recordError(Error(GL_INVALID_ENUM));
            return;
        }

        switch (precisiontype)
        {
          case GL_LOW_FLOAT:
          case GL_MEDIUM_FLOAT:
          case GL_HIGH_FLOAT:
            // Assume IEEE 754 precision
            range[0] = 127;
            range[1] = 127;
            *precision = 23;
            break;

          case GL_LOW_INT:
          case GL_MEDIUM_INT:
          case GL_HIGH_INT:
            // Some (most) hardware only supports single-precision floating-point numbers,
            // which can accurately represent integers up to +/-16777216
            range[0] = 24;
            range[1] = 24;
            *precision = 0;
            break;

          default:
            context->recordError(Error(GL_INVALID_ENUM));
            return;
        }
    }
}

void GetShaderSource(GLuint shader, GLsizei bufsize, GLsizei* length, GLchar* source)
{
    EVENT("(GLuint shader = %d, GLsizei bufsize = %d, GLsizei* length = 0x%0.8p, GLchar* source = 0x%0.8p)",
          shader, bufsize, length, source);

    Context *context = getNonLostContext();
    if (context)
    {
        if (bufsize < 0)
        {
            context->recordError(Error(GL_INVALID_VALUE));
            return;
        }

        Shader *shaderObject = context->getShader(shader);

        if (!shaderObject)
        {
            context->recordError(Error(GL_INVALID_OPERATION));
            return;
        }

        shaderObject->getSource(bufsize, length, source);
    }
}

void GetTranslatedShaderSourceANGLE(GLuint shader, GLsizei bufsize, GLsizei* length, GLchar* source)
{
    EVENT("(GLuint shader = %d, GLsizei bufsize = %d, GLsizei* length = 0x%0.8p, GLchar* source = 0x%0.8p)",
          shader, bufsize, length, source);

    Context *context = getNonLostContext();
    if (context)
    {
        if (bufsize < 0)
        {
            context->recordError(Error(GL_INVALID_VALUE));
            return;
        }

        Shader *shaderObject = context->getShader(shader);

        if (!shaderObject)
        {
            context->recordError(Error(GL_INVALID_OPERATION));
            return;
        }

        // Only returns extra info if ANGLE_GENERATE_SHADER_DEBUG_INFO is defined
        shaderObject->getTranslatedSourceWithDebugInfo(bufsize, length, source);
    }
}

const GLubyte* GetString(GLenum name)
{
    EVENT("(GLenum name = 0x%X)", name);

    Context *context = getNonLostContext();

    switch (name)
    {
      case GL_VENDOR:
        return (GLubyte*)"Google Inc.";

      case GL_RENDERER:
        return (GLubyte*)((context != NULL) ? context->getRendererString().c_str() : "ANGLE");

      case GL_VERSION:
        if (context->getClientVersion() == 2)
        {
            return (GLubyte*)"OpenGL ES 2.0 (ANGLE " ANGLE_VERSION_STRING ")";
        }
        else
        {
            return (GLubyte*)"OpenGL ES 3.0 (ANGLE " ANGLE_VERSION_STRING ")";
        }

      case GL_SHADING_LANGUAGE_VERSION:
        if (context->getClientVersion() == 2)
        {
            return (GLubyte*)"OpenGL ES GLSL ES 1.00 (ANGLE " ANGLE_VERSION_STRING ")";
        }
        else
        {
            return (GLubyte*)"OpenGL ES GLSL ES 3.00 (ANGLE " ANGLE_VERSION_STRING ")";
        }

      case GL_EXTENSIONS:
        return (GLubyte*)((context != NULL) ? context->getExtensionString().c_str() : "");

      default:
        if (context)
        {
            context->recordError(Error(GL_INVALID_ENUM));
        }
        return NULL;
    }
}

void GetTexParameterfv(GLenum target, GLenum pname, GLfloat* params)
{
    EVENT("(GLenum target = 0x%X, GLenum pname = 0x%X, GLfloat* params = 0x%0.8p)", target, pname, params);

    Context *context = getNonLostContext();
    if (context)
    {
        Texture *texture = context->getTargetTexture(target);

        if (!texture)
        {
            context->recordError(Error(GL_INVALID_ENUM));
            return;
        }

        switch (pname)
        {
          case GL_TEXTURE_MAG_FILTER:
            *params = (GLfloat)texture->getSamplerState().magFilter;
            break;
          case GL_TEXTURE_MIN_FILTER:
            *params = (GLfloat)texture->getSamplerState().minFilter;
            break;
          case GL_TEXTURE_WRAP_S:
            *params = (GLfloat)texture->getSamplerState().wrapS;
            break;
          case GL_TEXTURE_WRAP_T:
            *params = (GLfloat)texture->getSamplerState().wrapT;
            break;
          case GL_TEXTURE_WRAP_R:
            if (context->getClientVersion() < 3)
            {
                context->recordError(Error(GL_INVALID_ENUM));
                return;
            }
            *params = (GLfloat)texture->getSamplerState().wrapR;
            break;
          case GL_TEXTURE_IMMUTABLE_FORMAT:
            // Exposed to ES2.0 through EXT_texture_storage, no client version validation.
            *params = (GLfloat)(texture->isImmutable() ? GL_TRUE : GL_FALSE);
            break;
          case GL_TEXTURE_IMMUTABLE_LEVELS:
            if (context->getClientVersion() < 3)
            {
                context->recordError(Error(GL_INVALID_ENUM));
                return;
            }
            *params = (GLfloat)texture->immutableLevelCount();
            break;
          case GL_TEXTURE_USAGE_ANGLE:
            *params = (GLfloat)texture->getUsage();
            break;
          case GL_TEXTURE_MAX_ANISOTROPY_EXT:
            if (!context->getExtensions().textureFilterAnisotropic)
            {
                context->recordError(Error(GL_INVALID_ENUM));
                return;
            }
            *params = (GLfloat)texture->getSamplerState().maxAnisotropy;
            break;
          case GL_TEXTURE_SWIZZLE_R:
            if (context->getClientVersion() < 3)
            {
                context->recordError(Error(GL_INVALID_ENUM));
                return;
            }
            *params = (GLfloat)texture->getSamplerState().swizzleRed;
            break;
          case GL_TEXTURE_SWIZZLE_G:
            if (context->getClientVersion() < 3)
            {
                context->recordError(Error(GL_INVALID_ENUM));
                return;
            }
            *params = (GLfloat)texture->getSamplerState().swizzleGreen;
            break;
          case GL_TEXTURE_SWIZZLE_B:
            if (context->getClientVersion() < 3)
            {
                context->recordError(Error(GL_INVALID_ENUM));
                return;
            }
            *params = (GLfloat)texture->getSamplerState().swizzleBlue;
            break;
          case GL_TEXTURE_SWIZZLE_A:
            if (context->getClientVersion() < 3)
            {
                context->recordError(Error(GL_INVALID_ENUM));
                return;
            }
            *params = (GLfloat)texture->getSamplerState().swizzleAlpha;
            break;
          case GL_TEXTURE_BASE_LEVEL:
            if (context->getClientVersion() < 3)
            {
                context->recordError(Error(GL_INVALID_ENUM));
                return;
            }
            *params = (GLfloat)texture->getSamplerState().baseLevel;
            break;
          case GL_TEXTURE_MAX_LEVEL:
            if (context->getClientVersion() < 3)
            {
                context->recordError(Error(GL_INVALID_ENUM));
                return;
            }
            *params = (GLfloat)texture->getSamplerState().maxLevel;
            break;
          case GL_TEXTURE_MIN_LOD:
            if (context->getClientVersion() < 3)
            {
                context->recordError(Error(GL_INVALID_ENUM));
                return;
            }
            *params = texture->getSamplerState().minLod;
            break;
          case GL_TEXTURE_MAX_LOD:
            if (context->getClientVersion() < 3)
            {
                context->recordError(Error(GL_INVALID_ENUM));
                return;
            }
            *params = texture->getSamplerState().maxLod;
            break;

          default:
            context->recordError(Error(GL_INVALID_ENUM));
            return;
        }
    }
}

void GetTexParameteriv(GLenum target, GLenum pname, GLint* params)
{
    EVENT("(GLenum target = 0x%X, GLenum pname = 0x%X, GLint* params = 0x%0.8p)", target, pname, params);

    Context *context = getNonLostContext();
    if (context)
    {
        Texture *texture = context->getTargetTexture(target);

        if (!texture)
        {
            context->recordError(Error(GL_INVALID_ENUM));
            return;
        }

        switch (pname)
        {
          case GL_TEXTURE_MAG_FILTER:
            *params = texture->getSamplerState().magFilter;
            break;
          case GL_TEXTURE_MIN_FILTER:
            *params = texture->getSamplerState().minFilter;
            break;
          case GL_TEXTURE_WRAP_S:
            *params = texture->getSamplerState().wrapS;
            break;
          case GL_TEXTURE_WRAP_T:
            *params = texture->getSamplerState().wrapT;
            break;
          case GL_TEXTURE_WRAP_R:
            if (context->getClientVersion() < 3)
            {
                context->recordError(Error(GL_INVALID_ENUM));
                return;
            }
            *params = texture->getSamplerState().wrapR;
            break;
          case GL_TEXTURE_IMMUTABLE_FORMAT:
            // Exposed to ES2.0 through EXT_texture_storage, no client version validation.
            *params = texture->isImmutable() ? GL_TRUE : GL_FALSE;
            break;
          case GL_TEXTURE_IMMUTABLE_LEVELS:
            if (context->getClientVersion() < 3)
            {
                context->recordError(Error(GL_INVALID_ENUM));
                return;
            }
            *params = static_cast<GLint>(texture->immutableLevelCount());
            break;
          case GL_TEXTURE_USAGE_ANGLE:
            *params = texture->getUsage();
            break;
          case GL_TEXTURE_MAX_ANISOTROPY_EXT:
            if (!context->getExtensions().textureFilterAnisotropic)
            {
                context->recordError(Error(GL_INVALID_ENUM));
                return;
            }
            *params = (GLint)texture->getSamplerState().maxAnisotropy;
            break;
          case GL_TEXTURE_SWIZZLE_R:
            if (context->getClientVersion() < 3)
            {
                context->recordError(Error(GL_INVALID_ENUM));
                return;
            }
            *params = texture->getSamplerState().swizzleRed;
            break;
          case GL_TEXTURE_SWIZZLE_G:
            if (context->getClientVersion() < 3)
            {
                context->recordError(Error(GL_INVALID_ENUM));
                return;
            }
            *params = texture->getSamplerState().swizzleGreen;
            break;
          case GL_TEXTURE_SWIZZLE_B:
            if (context->getClientVersion() < 3)
            {
                context->recordError(Error(GL_INVALID_ENUM));
                return;
            }
            *params = texture->getSamplerState().swizzleBlue;
            break;
          case GL_TEXTURE_SWIZZLE_A:
            if (context->getClientVersion() < 3)
            {
                context->recordError(Error(GL_INVALID_ENUM));
                return;
            }
            *params = texture->getSamplerState().swizzleAlpha;
            break;
          case GL_TEXTURE_BASE_LEVEL:
            if (context->getClientVersion() < 3)
            {
                context->recordError(Error(GL_INVALID_ENUM));
                return;
            }
            *params = texture->getSamplerState().baseLevel;
            break;
          case GL_TEXTURE_MAX_LEVEL:
            if (context->getClientVersion() < 3)
            {
                context->recordError(Error(GL_INVALID_ENUM));
                return;
            }
            *params = texture->getSamplerState().maxLevel;
            break;
          case GL_TEXTURE_MIN_LOD:
            if (context->getClientVersion() < 3)
            {
                context->recordError(Error(GL_INVALID_ENUM));
                return;
            }
            *params = (GLint)texture->getSamplerState().minLod;
            break;
          case GL_TEXTURE_MAX_LOD:
            if (context->getClientVersion() < 3)
            {
                context->recordError(Error(GL_INVALID_ENUM));
                return;
            }
            *params = (GLint)texture->getSamplerState().maxLod;
            break;

          default:
            context->recordError(Error(GL_INVALID_ENUM));
            return;
        }
    }
}

void GetnUniformfvEXT(GLuint program, GLint location, GLsizei bufSize, GLfloat* params)
{
    EVENT("(GLuint program = %d, GLint location = %d, GLsizei bufSize = %d, GLfloat* params = 0x%0.8p)",
          program, location, bufSize, params);

    Context *context = getNonLostContext();
    if (context)
    {
        if (!ValidateGetnUniformfvEXT(context, program, location, bufSize, params))
        {
            return;
        }

        Program *programObject = context->getProgram(program);
        ASSERT(programObject);

        programObject->getUniformfv(location, params);
    }
}

void GetUniformfv(GLuint program, GLint location, GLfloat* params)
{
    EVENT("(GLuint program = %d, GLint location = %d, GLfloat* params = 0x%0.8p)", program, location, params);

    Context *context = getNonLostContext();
    if (context)
    {
        if (!ValidateGetUniformfv(context, program, location, params))
        {
            return;
        }

        Program *programObject = context->getProgram(program);
        ASSERT(programObject);

        programObject->getUniformfv(location, params);
    }
}

void GetnUniformivEXT(GLuint program, GLint location, GLsizei bufSize, GLint* params)
{
    EVENT("(GLuint program = %d, GLint location = %d, GLsizei bufSize = %d, GLint* params = 0x%0.8p)",
          program, location, bufSize, params);

    Context *context = getNonLostContext();
    if (context)
    {
        if (!ValidateGetnUniformivEXT(context, program, location, bufSize, params))
        {
            return;
        }

        Program *programObject = context->getProgram(program);
        ASSERT(programObject);

        programObject->getUniformiv(location, params);
    }
}

void GetUniformiv(GLuint program, GLint location, GLint* params)
{
    EVENT("(GLuint program = %d, GLint location = %d, GLint* params = 0x%0.8p)", program, location, params);

    Context *context = getNonLostContext();
    if (context)
    {
        if (!ValidateGetUniformiv(context, program, location, params))
        {
            return;
        }

        Program *programObject = context->getProgram(program);
        ASSERT(programObject);

        programObject->getUniformiv(location, params);
    }
}

GLint GetUniformLocation(GLuint program, const GLchar* name)
{
    EVENT("(GLuint program = %d, const GLchar* name = 0x%0.8p)", program, name);

    Context *context = getNonLostContext();
    if (context)
    {
        if (strstr(name, "gl_") == name)
        {
            return -1;
        }

        Program *programObject = context->getProgram(program);

        if (!programObject)
        {
            if (context->getShader(program))
            {
                context->recordError(Error(GL_INVALID_OPERATION));
                return -1;
            }
            else
            {
                context->recordError(Error(GL_INVALID_VALUE));
                return -1;
            }
        }

        if (!programObject->isLinked())
        {
            context->recordError(Error(GL_INVALID_OPERATION));
            return -1;
        }

        return programObject->getUniformLocation(name);
    }

    return -1;
}

void GetVertexAttribfv(GLuint index, GLenum pname, GLfloat* params)
{
    EVENT("(GLuint index = %d, GLenum pname = 0x%X, GLfloat* params = 0x%0.8p)", index, pname, params);

    Context *context = getNonLostContext();
    if (context)
    {
        if (index >= MAX_VERTEX_ATTRIBS)
        {
            context->recordError(Error(GL_INVALID_VALUE));
            return;
        }

        const VertexAttribute &attribState = context->getState().getVertexAttribState(index);
        if (!ValidateGetVertexAttribParameters(context, pname))
        {
            return;
        }

        if (pname == GL_CURRENT_VERTEX_ATTRIB)
        {
            const VertexAttribCurrentValueData &currentValueData = context->getState().getVertexAttribCurrentValue(index);
            for (int i = 0; i < 4; ++i)
            {
                params[i] = currentValueData.FloatValues[i];
            }
        }
        else
        {
            *params = QuerySingleVertexAttributeParameter<GLfloat>(attribState, pname);
        }
    }
}

void GetVertexAttribiv(GLuint index, GLenum pname, GLint* params)
{
    EVENT("(GLuint index = %d, GLenum pname = 0x%X, GLint* params = 0x%0.8p)", index, pname, params);

    Context *context = getNonLostContext();
    if (context)
    {
        if (index >= MAX_VERTEX_ATTRIBS)
        {
            context->recordError(Error(GL_INVALID_VALUE));
            return;
        }

        const VertexAttribute &attribState = context->getState().getVertexAttribState(index);

        if (!ValidateGetVertexAttribParameters(context, pname))
        {
            return;
        }

        if (pname == GL_CURRENT_VERTEX_ATTRIB)
        {
            const VertexAttribCurrentValueData &currentValueData = context->getState().getVertexAttribCurrentValue(index);
            for (int i = 0; i < 4; ++i)
            {
                float currentValue = currentValueData.FloatValues[i];
                params[i] = iround<GLint>(currentValue);
            }
        }
        else
        {
            *params = QuerySingleVertexAttributeParameter<GLint>(attribState, pname);
        }
    }
}

void GetVertexAttribPointerv(GLuint index, GLenum pname, GLvoid** pointer)
{
    EVENT("(GLuint index = %d, GLenum pname = 0x%X, GLvoid** pointer = 0x%0.8p)", index, pname, pointer);

    Context *context = getNonLostContext();
    if (context)
    {
        if (index >= MAX_VERTEX_ATTRIBS)
        {
            context->recordError(Error(GL_INVALID_VALUE));
            return;
        }

        if (pname != GL_VERTEX_ATTRIB_ARRAY_POINTER)
        {
            context->recordError(Error(GL_INVALID_ENUM));
            return;
        }

        *pointer = const_cast<GLvoid*>(context->getState().getVertexAttribPointer(index));
    }
}

void Hint(GLenum target, GLenum mode)
{
    EVENT("(GLenum target = 0x%X, GLenum mode = 0x%X)", target, mode);

    Context *context = getNonLostContext();
    if (context)
    {
        switch (mode)
        {
          case GL_FASTEST:
          case GL_NICEST:
          case GL_DONT_CARE:
            break;

          default:
            context->recordError(Error(GL_INVALID_ENUM));
            return;
        }

        switch (target)
        {
          case GL_GENERATE_MIPMAP_HINT:
            context->getState().setGenerateMipmapHint(mode);
            break;

          case GL_FRAGMENT_SHADER_DERIVATIVE_HINT_OES:
            context->getState().setFragmentShaderDerivativeHint(mode);
            break;

          default:
            context->recordError(Error(GL_INVALID_ENUM));
            return;
        }
    }
}

GLboolean IsBuffer(GLuint buffer)
{
    EVENT("(GLuint buffer = %d)", buffer);

    Context *context = getNonLostContext();
    if (context && buffer)
    {
        Buffer *bufferObject = context->getBuffer(buffer);

        if (bufferObject)
        {
            return GL_TRUE;
        }
    }

    return GL_FALSE;
}

GLboolean IsEnabled(GLenum cap)
{
    EVENT("(GLenum cap = 0x%X)", cap);

    Context *context = getNonLostContext();
    if (context)
    {
        if (!ValidCap(context, cap))
        {
            context->recordError(Error(GL_INVALID_ENUM));
            return GL_FALSE;
        }

        return context->getState().getEnableFeature(cap);
    }

    return false;
}

GLboolean IsFenceNV(GLuint fence)
{
    EVENT("(GLuint fence = %d)", fence);

    Context *context = getNonLostContext();
    if (context)
    {
        FenceNV *fenceObject = context->getFenceNV(fence);

        if (fenceObject == NULL)
        {
            return GL_FALSE;
        }

        return fenceObject->isFence();
    }

    return GL_FALSE;
}

GLboolean IsFramebuffer(GLuint framebuffer)
{
    EVENT("(GLuint framebuffer = %d)", framebuffer);

    Context *context = getNonLostContext();
    if (context && framebuffer)
    {
        Framebuffer *framebufferObject = context->getFramebuffer(framebuffer);

        if (framebufferObject)
        {
            return GL_TRUE;
        }
    }

    return GL_FALSE;
}

GLboolean IsProgram(GLuint program)
{
    EVENT("(GLuint program = %d)", program);

    Context *context = getNonLostContext();
    if (context && program)
    {
        Program *programObject = context->getProgram(program);

        if (programObject)
        {
            return GL_TRUE;
        }
    }

    return GL_FALSE;
}

GLboolean IsQueryEXT(GLuint id)
{
    EVENT("(GLuint id = %d)", id);

    Context *context = getNonLostContext();
    if (context)
    {
        return (context->getQuery(id, false, GL_NONE) != NULL) ? GL_TRUE : GL_FALSE;
    }

    return GL_FALSE;
}

GLboolean IsRenderbuffer(GLuint renderbuffer)
{
    EVENT("(GLuint renderbuffer = %d)", renderbuffer);

    Context *context = getNonLostContext();
    if (context && renderbuffer)
    {
        Renderbuffer *renderbufferObject = context->getRenderbuffer(renderbuffer);

        if (renderbufferObject)
        {
            return GL_TRUE;
        }
    }

    return GL_FALSE;
}

GLboolean IsShader(GLuint shader)
{
    EVENT("(GLuint shader = %d)", shader);

    Context *context = getNonLostContext();
    if (context && shader)
    {
        Shader *shaderObject = context->getShader(shader);

        if (shaderObject)
        {
            return GL_TRUE;
        }
    }

    return GL_FALSE;
}

GLboolean IsTexture(GLuint texture)
{
    EVENT("(GLuint texture = %d)", texture);

    Context *context = getNonLostContext();
    if (context && texture)
    {
        Texture *textureObject = context->getTexture(texture);

        if (textureObject)
        {
            return GL_TRUE;
        }
    }

    return GL_FALSE;
}

void LineWidth(GLfloat width)
{
    EVENT("(GLfloat width = %f)", width);

    Context *context = getNonLostContext();
    if (context)
    {
        if (width <= 0.0f)
        {
            context->recordError(Error(GL_INVALID_VALUE));
            return;
        }

        context->getState().setLineWidth(width);
    }
}

void LinkProgram(GLuint program)
{
    EVENT("(GLuint program = %d)", program);

    Context *context = getNonLostContext();
    if (context)
    {
        Program *programObject = context->getProgram(program);

        if (!programObject)
        {
            if (context->getShader(program))
            {
                context->recordError(Error(GL_INVALID_OPERATION));
                return;
            }
            else
            {
                context->recordError(Error(GL_INVALID_VALUE));
                return;
            }
        }

        Error error = programObject->link(context->getData());
        if (error.isError())
        {
            context->recordError(error);
            return;
        }
    }
}

void PixelStorei(GLenum pname, GLint param)
{
    EVENT("(GLenum pname = 0x%X, GLint param = %d)", pname, param);

    Context *context = getNonLostContext();
    if (context)
    {
        switch (pname)
        {
          case GL_UNPACK_ALIGNMENT:
            if (param != 1 && param != 2 && param != 4 && param != 8)
            {
                context->recordError(Error(GL_INVALID_VALUE));
                return;
            }

            context->getState().setUnpackAlignment(param);
            break;

          case GL_PACK_ALIGNMENT:
            if (param != 1 && param != 2 && param != 4 && param != 8)
            {
                context->recordError(Error(GL_INVALID_VALUE));
                return;
            }

            context->getState().setPackAlignment(param);
            break;

          case GL_PACK_REVERSE_ROW_ORDER_ANGLE:
            context->getState().setPackReverseRowOrder(param != 0);
            break;

          case GL_UNPACK_IMAGE_HEIGHT:
          case GL_UNPACK_SKIP_IMAGES:
          case GL_UNPACK_ROW_LENGTH:
          case GL_UNPACK_SKIP_ROWS:
          case GL_UNPACK_SKIP_PIXELS:
          case GL_PACK_ROW_LENGTH:
          case GL_PACK_SKIP_ROWS:
          case GL_PACK_SKIP_PIXELS:
            if (context->getClientVersion() < 3)
            {
                context->recordError(Error(GL_INVALID_ENUM));
                return;
            }
            UNIMPLEMENTED();
            break;

          default:
            context->recordError(Error(GL_INVALID_ENUM));
            return;
        }
    }
}

void PolygonOffset(GLfloat factor, GLfloat units)
{
    EVENT("(GLfloat factor = %f, GLfloat units = %f)", factor, units);

    Context *context = getNonLostContext();
    if (context)
    {
        context->getState().setPolygonOffsetParams(factor, units);
    }
}

void ReadnPixelsEXT(GLint x, GLint y, GLsizei width, GLsizei height,
                                GLenum format, GLenum type, GLsizei bufSize,
                                GLvoid *data)
{
    EVENT("(GLint x = %d, GLint y = %d, GLsizei width = %d, GLsizei height = %d, "
          "GLenum format = 0x%X, GLenum type = 0x%X, GLsizei bufSize = 0x%d, GLvoid *data = 0x%0.8p)",
          x, y, width, height, format, type, bufSize, data);

    Context *context = getNonLostContext();
    if (context)
    {
        if (width < 0 || height < 0 || bufSize < 0)
        {
            context->recordError(Error(GL_INVALID_VALUE));
            return;
        }

        if (!ValidateReadPixelsParameters(context, x, y, width, height,
                                              format, type, &bufSize, data))
        {
            return;
        }

        Error error = context->readPixels(x, y, width, height, format, type, &bufSize, data);
        if (error.isError())
        {
            context->recordError(error);
            return;
        }
    }
}

void ReadPixels(GLint x, GLint y, GLsizei width, GLsizei height,
                            GLenum format, GLenum type, GLvoid* pixels)
{
    EVENT("(GLint x = %d, GLint y = %d, GLsizei width = %d, GLsizei height = %d, "
          "GLenum format = 0x%X, GLenum type = 0x%X, GLvoid* pixels = 0x%0.8p)",
          x, y, width, height, format, type,  pixels);

    Context *context = getNonLostContext();
    if (context)
    {
        if (width < 0 || height < 0)
        {
            context->recordError(Error(GL_INVALID_VALUE));
            return;
        }

        if (!ValidateReadPixelsParameters(context, x, y, width, height,
                                              format, type, NULL, pixels))
        {
            return;
        }

        Error error = context->readPixels(x, y, width, height, format, type, NULL, pixels);
        if (error.isError())
        {
            context->recordError(error);
            return;
        }
    }
}

void ReleaseShaderCompiler(void)
{
    EVENT("()");

    Context *context = getNonLostContext();

    if (context)
    {
        context->releaseShaderCompiler();
    }
}

void RenderbufferStorageMultisampleANGLE(GLenum target, GLsizei samples, GLenum internalformat, GLsizei width, GLsizei height)
{
    EVENT("(GLenum target = 0x%X, GLsizei samples = %d, GLenum internalformat = 0x%X, GLsizei width = %d, GLsizei height = %d)",
          target, samples, internalformat, width, height);

    Context *context = getNonLostContext();
    if (context)
    {
        if (!ValidateRenderbufferStorageParametersANGLE(context, target, samples, internalformat,
                                                        width, height))
        {
            return;
        }

        Renderbuffer *renderbuffer = context->getState().getCurrentRenderbuffer();
        Error error = renderbuffer->setStorage(width, height, internalformat, samples);
        if (error.isError())
        {
            context->recordError(error);
            return;
        }
    }
}

void RenderbufferStorage(GLenum target, GLenum internalformat, GLsizei width, GLsizei height)
{
    RenderbufferStorageMultisampleANGLE(target, 0, internalformat, width, height);
}

void SampleCoverage(GLclampf value, GLboolean invert)
{
    EVENT("(GLclampf value = %f, GLboolean invert = %u)", value, invert);

    Context* context = getNonLostContext();

    if (context)
    {
        context->getState().setSampleCoverageParams(clamp01(value), invert == GL_TRUE);
    }
}

void SetFenceNV(GLuint fence, GLenum condition)
{
    EVENT("(GLuint fence = %d, GLenum condition = 0x%X)", fence, condition);

    Context *context = getNonLostContext();
    if (context)
    {
        if (condition != GL_ALL_COMPLETED_NV)
        {
            context->recordError(Error(GL_INVALID_ENUM));
            return;
        }

        FenceNV *fenceObject = context->getFenceNV(fence);

        if (fenceObject == NULL)
        {
            context->recordError(Error(GL_INVALID_OPERATION));
            return;
        }

        Error error = fenceObject->setFence(condition);
        if (error.isError())
        {
            context->recordError(error);
            return;
        }
    }
}

void Scissor(GLint x, GLint y, GLsizei width, GLsizei height)
{
    EVENT("(GLint x = %d, GLint y = %d, GLsizei width = %d, GLsizei height = %d)", x, y, width, height);

    Context* context = getNonLostContext();
    if (context)
    {
        if (width < 0 || height < 0)
        {
            context->recordError(Error(GL_INVALID_VALUE));
            return;
        }

        context->getState().setScissorParams(x, y, width, height);
    }
}

void ShaderBinary(GLsizei n, const GLuint* shaders, GLenum binaryformat, const GLvoid* binary, GLsizei length)
{
    EVENT("(GLsizei n = %d, const GLuint* shaders = 0x%0.8p, GLenum binaryformat = 0x%X, "
          "const GLvoid* binary = 0x%0.8p, GLsizei length = %d)",
          n, shaders, binaryformat, binary, length);

    Context* context = getNonLostContext();
    if (context)
    {
        const std::vector<GLenum> &shaderBinaryFormats = context->getCaps().shaderBinaryFormats;
        if (std::find(shaderBinaryFormats.begin(), shaderBinaryFormats.end(), binaryformat) == shaderBinaryFormats.end())
        {
            context->recordError(Error(GL_INVALID_ENUM));
            return;
        }

        // No binary shader formats are supported.
        UNIMPLEMENTED();
    }
}

void ShaderSource(GLuint shader, GLsizei count, const GLchar* const* string, const GLint* length)
{
    EVENT("(GLuint shader = %d, GLsizei count = %d, const GLchar** string = 0x%0.8p, const GLint* length = 0x%0.8p)",
          shader, count, string, length);

    Context *context = getNonLostContext();
    if (context)
    {
        if (count < 0)
        {
            context->recordError(Error(GL_INVALID_VALUE));
            return;
        }

        Shader *shaderObject = context->getShader(shader);

        if (!shaderObject)
        {
            if (context->getProgram(shader))
            {
                context->recordError(Error(GL_INVALID_OPERATION));
                return;
            }
            else
            {
                context->recordError(Error(GL_INVALID_VALUE));
                return;
            }
        }

        shaderObject->setSource(count, string, length);
    }
}

void StencilFunc(GLenum func, GLint ref, GLuint mask)
{
    StencilFuncSeparate(GL_FRONT_AND_BACK, func, ref, mask);
}

void StencilFuncSeparate(GLenum face, GLenum func, GLint ref, GLuint mask)
{
    EVENT("(GLenum face = 0x%X, GLenum func = 0x%X, GLint ref = %d, GLuint mask = %d)", face, func, ref, mask);

    Context *context = getNonLostContext();
    if (context)
    {
        switch (face)
        {
          case GL_FRONT:
          case GL_BACK:
          case GL_FRONT_AND_BACK:
            break;

          default:
            context->recordError(Error(GL_INVALID_ENUM));
            return;
        }

        switch (func)
        {
          case GL_NEVER:
          case GL_ALWAYS:
          case GL_LESS:
          case GL_LEQUAL:
          case GL_EQUAL:
          case GL_GEQUAL:
          case GL_GREATER:
          case GL_NOTEQUAL:
            break;

          default:
            context->recordError(Error(GL_INVALID_ENUM));
            return;
        }

        if (face == GL_FRONT || face == GL_FRONT_AND_BACK)
        {
            context->getState().setStencilParams(func, ref, mask);
        }

        if (face == GL_BACK || face == GL_FRONT_AND_BACK)
        {
            context->getState().setStencilBackParams(func, ref, mask);
        }
    }
}

void StencilMask(GLuint mask)
{
    StencilMaskSeparate(GL_FRONT_AND_BACK, mask);
}

void StencilMaskSeparate(GLenum face, GLuint mask)
{
    EVENT("(GLenum face = 0x%X, GLuint mask = %d)", face, mask);

    Context *context = getNonLostContext();
    if (context)
    {
        switch (face)
        {
          case GL_FRONT:
          case GL_BACK:
          case GL_FRONT_AND_BACK:
            break;

          default:
            context->recordError(Error(GL_INVALID_ENUM));
            return;
        }

        if (face == GL_FRONT || face == GL_FRONT_AND_BACK)
        {
            context->getState().setStencilWritemask(mask);
        }

        if (face == GL_BACK || face == GL_FRONT_AND_BACK)
        {
            context->getState().setStencilBackWritemask(mask);
        }
    }
}

void StencilOp(GLenum fail, GLenum zfail, GLenum zpass)
{
    StencilOpSeparate(GL_FRONT_AND_BACK, fail, zfail, zpass);
}

void StencilOpSeparate(GLenum face, GLenum fail, GLenum zfail, GLenum zpass)
{
    EVENT("(GLenum face = 0x%X, GLenum fail = 0x%X, GLenum zfail = 0x%X, GLenum zpas = 0x%Xs)",
          face, fail, zfail, zpass);

    Context *context = getNonLostContext();
    if (context)
    {
        switch (face)
        {
          case GL_FRONT:
          case GL_BACK:
          case GL_FRONT_AND_BACK:
            break;

          default:
            context->recordError(Error(GL_INVALID_ENUM));
            return;
        }

        switch (fail)
        {
          case GL_ZERO:
          case GL_KEEP:
          case GL_REPLACE:
          case GL_INCR:
          case GL_DECR:
          case GL_INVERT:
          case GL_INCR_WRAP:
          case GL_DECR_WRAP:
            break;

          default:
            context->recordError(Error(GL_INVALID_ENUM));
            return;
        }

        switch (zfail)
        {
          case GL_ZERO:
          case GL_KEEP:
          case GL_REPLACE:
          case GL_INCR:
          case GL_DECR:
          case GL_INVERT:
          case GL_INCR_WRAP:
          case GL_DECR_WRAP:
            break;

          default:
            context->recordError(Error(GL_INVALID_ENUM));
            return;
        }

        switch (zpass)
        {
          case GL_ZERO:
          case GL_KEEP:
          case GL_REPLACE:
          case GL_INCR:
          case GL_DECR:
          case GL_INVERT:
          case GL_INCR_WRAP:
          case GL_DECR_WRAP:
            break;

          default:
            context->recordError(Error(GL_INVALID_ENUM));
            return;
        }

        if (face == GL_FRONT || face == GL_FRONT_AND_BACK)
        {
            context->getState().setStencilOperations(fail, zfail, zpass);
        }

        if (face == GL_BACK || face == GL_FRONT_AND_BACK)
        {
            context->getState().setStencilBackOperations(fail, zfail, zpass);
        }
    }
}

GLboolean TestFenceNV(GLuint fence)
{
    EVENT("(GLuint fence = %d)", fence);

    Context *context = getNonLostContext();
    if (context)
    {
        FenceNV *fenceObject = context->getFenceNV(fence);

        if (fenceObject == NULL)
        {
            context->recordError(Error(GL_INVALID_OPERATION));
            return GL_TRUE;
        }

        if (fenceObject->isFence() != GL_TRUE)
        {
            context->recordError(Error(GL_INVALID_OPERATION));
            return GL_TRUE;
        }

        GLboolean result;
        Error error = fenceObject->testFence(&result);
        if (error.isError())
        {
            context->recordError(error);
            return GL_TRUE;
        }

        return result;
    }

    return GL_TRUE;
}

void TexImage2D(GLenum target, GLint level, GLint internalformat, GLsizei width, GLsizei height,
                            GLint border, GLenum format, GLenum type, const GLvoid* pixels)
{
    EVENT("(GLenum target = 0x%X, GLint level = %d, GLint internalformat = %d, GLsizei width = %d, GLsizei height = %d, "
          "GLint border = %d, GLenum format = 0x%X, GLenum type = 0x%X, const GLvoid* pixels = 0x%0.8p)",
          target, level, internalformat, width, height, border, format, type, pixels);

    Context *context = getNonLostContext();
    if (context)
    {
        if (context->getClientVersion() < 3 &&
            !ValidateES2TexImageParameters(context, target, level, internalformat, false, false,
                                           0, 0, width, height, border, format, type, pixels))
        {
            return;
        }

        if (context->getClientVersion() >= 3 &&
            !ValidateES3TexImageParameters(context, target, level, internalformat, false, false,
                                           0, 0, 0, width, height, 1, border, format, type, pixels))
        {
            return;
        }

        switch (target)
        {
          case GL_TEXTURE_2D:
            {
                Texture2D *texture = context->getTexture2D();
                Error error = texture->setImage(level, width, height, internalformat, format, type, context->getState().getUnpackState(), pixels);
                if (error.isError())
                {
                    context->recordError(error);
                    return;
                }
            }
            break;

          case GL_TEXTURE_CUBE_MAP_POSITIVE_X:
          case GL_TEXTURE_CUBE_MAP_NEGATIVE_X:
          case GL_TEXTURE_CUBE_MAP_POSITIVE_Y:
          case GL_TEXTURE_CUBE_MAP_NEGATIVE_Y:
          case GL_TEXTURE_CUBE_MAP_POSITIVE_Z:
          case GL_TEXTURE_CUBE_MAP_NEGATIVE_Z:
            {
                TextureCubeMap *texture = context->getTextureCubeMap();
                Error error = texture->setImage(target, level, width, height, internalformat, format, type, context->getState().getUnpackState(), pixels);
                if (error.isError())
                {
                    context->recordError(error);
                    return;
                }
            }
            break;

          default: UNREACHABLE();
        }
    }
}

void TexParameterf(GLenum target, GLenum pname, GLfloat param)
{
    EVENT("(GLenum target = 0x%X, GLenum pname = 0x%X, GLint param = %f)", target, pname, param);

    Context *context = getNonLostContext();
    if (context)
    {
        if (!ValidateTexParamParameters(context, pname, static_cast<GLint>(param)))
        {
            return;
        }

        Texture *texture = context->getTargetTexture(target);

        if (!texture)
        {
            context->recordError(Error(GL_INVALID_ENUM));
            return;
        }

        switch (pname)
        {
          case GL_TEXTURE_WRAP_S:               texture->getSamplerState().wrapS = uiround<GLenum>(param);        break;
          case GL_TEXTURE_WRAP_T:               texture->getSamplerState().wrapT = uiround<GLenum>(param);        break;
          case GL_TEXTURE_WRAP_R:               texture->getSamplerState().wrapR = uiround<GLenum>(param);        break;
          case GL_TEXTURE_MIN_FILTER:           texture->getSamplerState().minFilter = uiround<GLenum>(param);    break;
          case GL_TEXTURE_MAG_FILTER:           texture->getSamplerState().magFilter = uiround<GLenum>(param);    break;
          case GL_TEXTURE_USAGE_ANGLE:          texture->setUsage(uiround<GLenum>(param));                        break;
          case GL_TEXTURE_MAX_ANISOTROPY_EXT:   texture->getSamplerState().maxAnisotropy = std::min(param, context->getExtensions().maxTextureAnisotropy); break;
          case GL_TEXTURE_COMPARE_MODE:         texture->getSamplerState().compareMode = uiround<GLenum>(param);  break;
          case GL_TEXTURE_COMPARE_FUNC:         texture->getSamplerState().compareFunc = uiround<GLenum>(param);  break;
          case GL_TEXTURE_SWIZZLE_R:            texture->getSamplerState().swizzleRed = uiround<GLenum>(param);   break;
          case GL_TEXTURE_SWIZZLE_G:            texture->getSamplerState().swizzleGreen = uiround<GLenum>(param); break;
          case GL_TEXTURE_SWIZZLE_B:            texture->getSamplerState().swizzleBlue = uiround<GLenum>(param);  break;
          case GL_TEXTURE_SWIZZLE_A:            texture->getSamplerState().swizzleAlpha = uiround<GLenum>(param); break;
          case GL_TEXTURE_BASE_LEVEL:           texture->getSamplerState().baseLevel = iround<GLint>(param);      break;
          case GL_TEXTURE_MAX_LEVEL:            texture->getSamplerState().maxLevel = iround<GLint>(param);       break;
          case GL_TEXTURE_MIN_LOD:              texture->getSamplerState().minLod = param;                            break;
          case GL_TEXTURE_MAX_LOD:              texture->getSamplerState().maxLod = param;                            break;
          default: UNREACHABLE(); break;
        }
    }
}

void TexParameterfv(GLenum target, GLenum pname, const GLfloat* params)
{
    TexParameterf(target, pname, (GLfloat)*params);
}

void TexParameteri(GLenum target, GLenum pname, GLint param)
{
    EVENT("(GLenum target = 0x%X, GLenum pname = 0x%X, GLint param = %d)", target, pname, param);

    Context *context = getNonLostContext();
    if (context)
    {
        if (!ValidateTexParamParameters(context, pname, param))
        {
            return;
        }

        Texture *texture = context->getTargetTexture(target);

        if (!texture)
        {
            context->recordError(Error(GL_INVALID_ENUM));
            return;
        }

        switch (pname)
        {
          case GL_TEXTURE_WRAP_S:               texture->getSamplerState().wrapS = (GLenum)param;        break;
          case GL_TEXTURE_WRAP_T:               texture->getSamplerState().wrapT = (GLenum)param;        break;
          case GL_TEXTURE_WRAP_R:               texture->getSamplerState().wrapR = (GLenum)param;        break;
          case GL_TEXTURE_MIN_FILTER:           texture->getSamplerState().minFilter = (GLenum)param;    break;
          case GL_TEXTURE_MAG_FILTER:           texture->getSamplerState().magFilter = (GLenum)param;    break;
          case GL_TEXTURE_USAGE_ANGLE:          texture->setUsage((GLenum)param);                        break;
          case GL_TEXTURE_MAX_ANISOTROPY_EXT:   texture->getSamplerState().maxAnisotropy = std::min((float)param, context->getExtensions().maxTextureAnisotropy); break;
          case GL_TEXTURE_COMPARE_MODE:         texture->getSamplerState().compareMode = (GLenum)param;  break;
          case GL_TEXTURE_COMPARE_FUNC:         texture->getSamplerState().compareFunc = (GLenum)param;  break;
          case GL_TEXTURE_SWIZZLE_R:            texture->getSamplerState().swizzleRed = (GLenum)param;   break;
          case GL_TEXTURE_SWIZZLE_G:            texture->getSamplerState().swizzleGreen = (GLenum)param; break;
          case GL_TEXTURE_SWIZZLE_B:            texture->getSamplerState().swizzleBlue = (GLenum)param;  break;
          case GL_TEXTURE_SWIZZLE_A:            texture->getSamplerState().swizzleAlpha = (GLenum)param; break;
          case GL_TEXTURE_BASE_LEVEL:           texture->getSamplerState().baseLevel = param;            break;
          case GL_TEXTURE_MAX_LEVEL:            texture->getSamplerState().maxLevel = param;             break;
          case GL_TEXTURE_MIN_LOD:              texture->getSamplerState().minLod = (GLfloat)param;      break;
          case GL_TEXTURE_MAX_LOD:              texture->getSamplerState().maxLod = (GLfloat)param;      break;
          default: UNREACHABLE(); break;
        }
    }
}

void TexParameteriv(GLenum target, GLenum pname, const GLint* params)
{
    TexParameteri(target, pname, *params);
}

void TexStorage2DEXT(GLenum target, GLsizei levels, GLenum internalformat, GLsizei width, GLsizei height)
{
    EVENT("(GLenum target = 0x%X, GLsizei levels = %d, GLenum internalformat = 0x%X, GLsizei width = %d, GLsizei height = %d)",
           target, levels, internalformat, width, height);

    Context *context = getNonLostContext();
    if (context)
    {
        if (!context->getExtensions().textureStorage)
        {
            context->recordError(Error(GL_INVALID_OPERATION));
            return;
        }

        if (context->getClientVersion() < 3 &&
            !ValidateES2TexStorageParameters(context, target, levels, internalformat, width, height))
        {
            return;
        }

        if (context->getClientVersion() >= 3 &&
            !ValidateES3TexStorageParameters(context, target, levels, internalformat, width, height, 1))
        {
            return;
        }

        switch (target)
        {
          case GL_TEXTURE_2D:
            {
                Texture2D *texture2d = context->getTexture2D();
                Error error = texture2d->storage(levels, internalformat, width, height);
                if (error.isError())
                {
                    context->recordError(error);
                    return;
                }
            }
            break;

          case GL_TEXTURE_CUBE_MAP:
            {
                TextureCubeMap *textureCube = context->getTextureCubeMap();
                Error error = textureCube->storage(levels, internalformat, width);
                if (error.isError())
                {
                    context->recordError(error);
                    return;
                }
            }
            break;

          default:
            context->recordError(Error(GL_INVALID_ENUM));
            return;
        }
    }
}

void TexSubImage2D(GLenum target, GLint level, GLint xoffset, GLint yoffset, GLsizei width, GLsizei height,
                               GLenum format, GLenum type, const GLvoid* pixels)
{
    EVENT("(GLenum target = 0x%X, GLint level = %d, GLint xoffset = %d, GLint yoffset = %d, "
          "GLsizei width = %d, GLsizei height = %d, GLenum format = 0x%X, GLenum type = 0x%X, "
          "const GLvoid* pixels = 0x%0.8p)",
           target, level, xoffset, yoffset, width, height, format, type, pixels);

    Context *context = getNonLostContext();
    if (context)
    {
        if (context->getClientVersion() < 3 &&
            !ValidateES2TexImageParameters(context, target, level, GL_NONE, false, true,
                                           xoffset, yoffset, width, height, 0, format, type, pixels))
        {
            return;
        }

        if (context->getClientVersion() >= 3 &&
            !ValidateES3TexImageParameters(context, target, level, GL_NONE, false, true,
                                           xoffset, yoffset, 0, width, height, 1, 0, format, type, pixels))
        {
            return;
        }

        // Zero sized uploads are valid but no-ops
        if (width == 0 || height == 0)
        {
            return;
        }

        switch (target)
        {
          case GL_TEXTURE_2D:
            {
                Texture2D *texture = context->getTexture2D();
                Error error = texture->subImage(level, xoffset, yoffset, width, height, format, type, context->getState().getUnpackState(), pixels);
                if (error.isError())
                {
                    context->recordError(error);
                    return;
                }
            }
            break;

          case GL_TEXTURE_CUBE_MAP_POSITIVE_X:
          case GL_TEXTURE_CUBE_MAP_NEGATIVE_X:
          case GL_TEXTURE_CUBE_MAP_POSITIVE_Y:
          case GL_TEXTURE_CUBE_MAP_NEGATIVE_Y:
          case GL_TEXTURE_CUBE_MAP_POSITIVE_Z:
          case GL_TEXTURE_CUBE_MAP_NEGATIVE_Z:
            {
                TextureCubeMap *texture = context->getTextureCubeMap();
                Error error = texture->subImage(target, level, xoffset, yoffset, width, height, format, type, context->getState().getUnpackState(), pixels);
                if (error.isError())
                {
                    context->recordError(error);
                    return;
                }
            }
            break;

          default:
            UNREACHABLE();
        }
    }
}

void Uniform1f(GLint location, GLfloat x)
{
    Uniform1fv(location, 1, &x);
}

void Uniform1fv(GLint location, GLsizei count, const GLfloat* v)
{
    EVENT("(GLint location = %d, GLsizei count = %d, const GLfloat* v = 0x%0.8p)", location, count, v);

    Context *context = getNonLostContext();
    if (context)
    {
        if (!ValidateUniform(context, GL_FLOAT, location, count))
        {
            return;
        }

        Program *program = context->getState().getProgram();
        program->setUniform1fv(location, count, v);
    }
}

void Uniform1i(GLint location, GLint x)
{
    Uniform1iv(location, 1, &x);
}

void Uniform1iv(GLint location, GLsizei count, const GLint* v)
{
    EVENT("(GLint location = %d, GLsizei count = %d, const GLint* v = 0x%0.8p)", location, count, v);

    Context *context = getNonLostContext();
    if (context)
    {
        if (!ValidateUniform(context, GL_INT, location, count))
        {
            return;
        }

        Program *program = context->getState().getProgram();
        program->setUniform1iv(location, count, v);
    }
}

void Uniform2f(GLint location, GLfloat x, GLfloat y)
{
    GLfloat xy[2] = {x, y};

    Uniform2fv(location, 1, xy);
}

void Uniform2fv(GLint location, GLsizei count, const GLfloat* v)
{
    EVENT("(GLint location = %d, GLsizei count = %d, const GLfloat* v = 0x%0.8p)", location, count, v);

    Context *context = getNonLostContext();
    if (context)
    {
        if (!ValidateUniform(context, GL_FLOAT_VEC2, location, count))
        {
            return;
        }

        Program *program = context->getState().getProgram();
        program->setUniform2fv(location, count, v);
    }
}

void Uniform2i(GLint location, GLint x, GLint y)
{
    GLint xy[2] = {x, y};

    Uniform2iv(location, 1, xy);
}

void Uniform2iv(GLint location, GLsizei count, const GLint* v)
{
    EVENT("(GLint location = %d, GLsizei count = %d, const GLint* v = 0x%0.8p)", location, count, v);

    Context *context = getNonLostContext();
    if (context)
    {
        if (!ValidateUniform(context, GL_INT_VEC2, location, count))
        {
            return;
        }

        Program *program = context->getState().getProgram();
        program->setUniform2iv(location, count, v);
    }
}

void Uniform3f(GLint location, GLfloat x, GLfloat y, GLfloat z)
{
    GLfloat xyz[3] = {x, y, z};

    Uniform3fv(location, 1, xyz);
}

void Uniform3fv(GLint location, GLsizei count, const GLfloat* v)
{
    EVENT("(GLint location = %d, GLsizei count = %d, const GLfloat* v = 0x%0.8p)", location, count, v);

    Context *context = getNonLostContext();
    if (context)
    {
        if (!ValidateUniform(context, GL_FLOAT_VEC3, location, count))
        {
            return;
        }

        Program *program = context->getState().getProgram();
        program->setUniform3fv(location, count, v);
    }
}

void Uniform3i(GLint location, GLint x, GLint y, GLint z)
{
    GLint xyz[3] = {x, y, z};

    Uniform3iv(location, 1, xyz);
}

void Uniform3iv(GLint location, GLsizei count, const GLint* v)
{
    EVENT("(GLint location = %d, GLsizei count = %d, const GLint* v = 0x%0.8p)", location, count, v);

    Context *context = getNonLostContext();
    if (context)
    {
        if (!ValidateUniform(context, GL_INT_VEC3, location, count))
        {
            return;
        }

        Program *program = context->getState().getProgram();
        program->setUniform3iv(location, count, v);
    }
}

void Uniform4f(GLint location, GLfloat x, GLfloat y, GLfloat z, GLfloat w)
{
    GLfloat xyzw[4] = {x, y, z, w};

    Uniform4fv(location, 1, xyzw);
}

void Uniform4fv(GLint location, GLsizei count, const GLfloat* v)
{
    EVENT("(GLint location = %d, GLsizei count = %d, const GLfloat* v = 0x%0.8p)", location, count, v);

    Context *context = getNonLostContext();
    if (context)
    {
        if (!ValidateUniform(context, GL_FLOAT_VEC4, location, count))
        {
            return;
        }

        Program *program = context->getState().getProgram();
        program->setUniform4fv(location, count, v);
    }
}

void Uniform4i(GLint location, GLint x, GLint y, GLint z, GLint w)
{
    GLint xyzw[4] = {x, y, z, w};

    Uniform4iv(location, 1, xyzw);
}

void Uniform4iv(GLint location, GLsizei count, const GLint* v)
{
    EVENT("(GLint location = %d, GLsizei count = %d, const GLint* v = 0x%0.8p)", location, count, v);

    Context *context = getNonLostContext();
    if (context)
    {
        if (!ValidateUniform(context, GL_INT_VEC4, location, count))
        {
            return;
        }

        Program *program = context->getState().getProgram();
        program->setUniform4iv(location, count, v);
    }
}

void UniformMatrix2fv(GLint location, GLsizei count, GLboolean transpose, const GLfloat* value)
{
    EVENT("(GLint location = %d, GLsizei count = %d, GLboolean transpose = %u, const GLfloat* value = 0x%0.8p)",
          location, count, transpose, value);

    Context *context = getNonLostContext();
    if (context)
    {
        if (!ValidateUniformMatrix(context, GL_FLOAT_MAT2, location, count, transpose))
        {
            return;
        }

        Program *program = context->getState().getProgram();
        program->setUniformMatrix2fv(location, count, transpose, value);
    }
}

void UniformMatrix3fv(GLint location, GLsizei count, GLboolean transpose, const GLfloat* value)
{
    EVENT("(GLint location = %d, GLsizei count = %d, GLboolean transpose = %u, const GLfloat* value = 0x%0.8p)",
          location, count, transpose, value);

    Context *context = getNonLostContext();
    if (context)
    {
        if (!ValidateUniformMatrix(context, GL_FLOAT_MAT3, location, count, transpose))
        {
            return;
        }

        Program *program = context->getState().getProgram();
        program->setUniformMatrix3fv(location, count, transpose, value);
    }
}

void UniformMatrix4fv(GLint location, GLsizei count, GLboolean transpose, const GLfloat* value)
{
    EVENT("(GLint location = %d, GLsizei count = %d, GLboolean transpose = %u, const GLfloat* value = 0x%0.8p)",
          location, count, transpose, value);

    Context *context = getNonLostContext();
    if (context)
    {
        if (!ValidateUniformMatrix(context, GL_FLOAT_MAT4, location, count, transpose))
        {
            return;
        }

        Program *program = context->getState().getProgram();
        program->setUniformMatrix4fv(location, count, transpose, value);
    }
}

void UseProgram(GLuint program)
{
    EVENT("(GLuint program = %d)", program);

    Context *context = getNonLostContext();
    if (context)
    {
        Program *programObject = context->getProgram(program);

        if (!programObject && program != 0)
        {
            if (context->getShader(program))
            {
                context->recordError(Error(GL_INVALID_OPERATION));
                return;
            }
            else
            {
                context->recordError(Error(GL_INVALID_VALUE));
                return;
            }
        }

        if (program != 0 && !programObject->isLinked())
        {
            context->recordError(Error(GL_INVALID_OPERATION));
            return;
        }

        context->useProgram(program);
    }
}

void ValidateProgram(GLuint program)
{
    EVENT("(GLuint program = %d)", program);

    Context *context = getNonLostContext();
    if (context)
    {
        Program *programObject = context->getProgram(program);

        if (!programObject)
        {
            if (context->getShader(program))
            {
                context->recordError(Error(GL_INVALID_OPERATION));
                return;
            }
            else
            {
                context->recordError(Error(GL_INVALID_VALUE));
                return;
            }
        }

        programObject->validate(context->getCaps());
    }
}

void VertexAttrib1f(GLuint index, GLfloat x)
{
    EVENT("(GLuint index = %d, GLfloat x = %f)", index, x);

    Context *context = getNonLostContext();
    if (context)
    {
        if (index >= MAX_VERTEX_ATTRIBS)
        {
            context->recordError(Error(GL_INVALID_VALUE));
            return;
        }

        GLfloat vals[4] = { x, 0, 0, 1 };
        context->getState().setVertexAttribf(index, vals);
    }
}

void VertexAttrib1fv(GLuint index, const GLfloat* values)
{
    EVENT("(GLuint index = %d, const GLfloat* values = 0x%0.8p)", index, values);

    Context *context = getNonLostContext();
    if (context)
    {
        if (index >= MAX_VERTEX_ATTRIBS)
        {
            context->recordError(Error(GL_INVALID_VALUE));
            return;
        }

        GLfloat vals[4] = { values[0], 0, 0, 1 };
        context->getState().setVertexAttribf(index, vals);
    }
}

void VertexAttrib2f(GLuint index, GLfloat x, GLfloat y)
{
    EVENT("(GLuint index = %d, GLfloat x = %f, GLfloat y = %f)", index, x, y);

    Context *context = getNonLostContext();
    if (context)
    {
        if (index >= MAX_VERTEX_ATTRIBS)
        {
            context->recordError(Error(GL_INVALID_VALUE));
            return;
        }

        GLfloat vals[4] = { x, y, 0, 1 };
        context->getState().setVertexAttribf(index, vals);
    }
}

void VertexAttrib2fv(GLuint index, const GLfloat* values)
{
    EVENT("(GLuint index = %d, const GLfloat* values = 0x%0.8p)", index, values);

    Context *context = getNonLostContext();
    if (context)
    {
        if (index >= MAX_VERTEX_ATTRIBS)
        {
            context->recordError(Error(GL_INVALID_VALUE));
            return;
        }

        GLfloat vals[4] = { values[0], values[1], 0, 1 };
        context->getState().setVertexAttribf(index, vals);
    }
}

void VertexAttrib3f(GLuint index, GLfloat x, GLfloat y, GLfloat z)
{
    EVENT("(GLuint index = %d, GLfloat x = %f, GLfloat y = %f, GLfloat z = %f)", index, x, y, z);

    Context *context = getNonLostContext();
    if (context)
    {
        if (index >= MAX_VERTEX_ATTRIBS)
        {
            context->recordError(Error(GL_INVALID_VALUE));
            return;
        }

        GLfloat vals[4] = { x, y, z, 1 };
        context->getState().setVertexAttribf(index, vals);
    }
}

void VertexAttrib3fv(GLuint index, const GLfloat* values)
{
    EVENT("(GLuint index = %d, const GLfloat* values = 0x%0.8p)", index, values);

    Context *context = getNonLostContext();
    if (context)
    {
        if (index >= MAX_VERTEX_ATTRIBS)
        {
            context->recordError(Error(GL_INVALID_VALUE));
            return;
        }

        GLfloat vals[4] = { values[0], values[1], values[2], 1 };
        context->getState().setVertexAttribf(index, vals);
    }
}

void VertexAttrib4f(GLuint index, GLfloat x, GLfloat y, GLfloat z, GLfloat w)
{
    EVENT("(GLuint index = %d, GLfloat x = %f, GLfloat y = %f, GLfloat z = %f, GLfloat w = %f)", index, x, y, z, w);

    Context *context = getNonLostContext();
    if (context)
    {
        if (index >= MAX_VERTEX_ATTRIBS)
        {
            context->recordError(Error(GL_INVALID_VALUE));
            return;
        }

        GLfloat vals[4] = { x, y, z, w };
        context->getState().setVertexAttribf(index, vals);
    }
}

void VertexAttrib4fv(GLuint index, const GLfloat* values)
{
    EVENT("(GLuint index = %d, const GLfloat* values = 0x%0.8p)", index, values);

    Context *context = getNonLostContext();
    if (context)
    {
        if (index >= MAX_VERTEX_ATTRIBS)
        {
            context->recordError(Error(GL_INVALID_VALUE));
            return;
        }

        context->getState().setVertexAttribf(index, values);
    }
}

void VertexAttribDivisorANGLE(GLuint index, GLuint divisor)
{
    EVENT("(GLuint index = %d, GLuint divisor = %d)", index, divisor);

    Context *context = getNonLostContext();
    if (context)
    {
        if (index >= MAX_VERTEX_ATTRIBS)
        {
            context->recordError(Error(GL_INVALID_VALUE));
            return;
        }

        context->setVertexAttribDivisor(index, divisor);
    }
}

void VertexAttribPointer(GLuint index, GLint size, GLenum type, GLboolean normalized, GLsizei stride, const GLvoid* ptr)
{
    EVENT("(GLuint index = %d, GLint size = %d, GLenum type = 0x%X, "
          "GLboolean normalized = %u, GLsizei stride = %d, const GLvoid* ptr = 0x%0.8p)",
          index, size, type, normalized, stride, ptr);

    Context *context = getNonLostContext();
    if (context)
    {
        if (index >= MAX_VERTEX_ATTRIBS)
        {
            context->recordError(Error(GL_INVALID_VALUE));
            return;
        }

        if (size < 1 || size > 4)
        {
            context->recordError(Error(GL_INVALID_VALUE));
            return;
        }

        switch (type)
        {
          case GL_BYTE:
          case GL_UNSIGNED_BYTE:
          case GL_SHORT:
          case GL_UNSIGNED_SHORT:
          case GL_FIXED:
          case GL_FLOAT:
            break;

          case GL_HALF_FLOAT:
          case GL_INT:
          case GL_UNSIGNED_INT:
          case GL_INT_2_10_10_10_REV:
          case GL_UNSIGNED_INT_2_10_10_10_REV:
            if (context->getClientVersion() < 3)
            {
                context->recordError(Error(GL_INVALID_ENUM));
                return;
            }
            break;

          default:
            context->recordError(Error(GL_INVALID_ENUM));
            return;
        }

        if (stride < 0)
        {
            context->recordError(Error(GL_INVALID_VALUE));
            return;
        }

        if ((type == GL_INT_2_10_10_10_REV || type == GL_UNSIGNED_INT_2_10_10_10_REV) && size != 4)
        {
            context->recordError(Error(GL_INVALID_OPERATION));
            return;
        }

        // [OpenGL ES 3.0.2] Section 2.8 page 24:
        // An INVALID_OPERATION error is generated when a non-zero vertex array object
        // is bound, zero is bound to the ARRAY_BUFFER buffer object binding point,
        // and the pointer argument is not NULL.
        if (context->getState().getVertexArray()->id() != 0 && context->getState().getArrayBufferId() == 0 && ptr != NULL)
        {
            context->recordError(Error(GL_INVALID_OPERATION));
            return;
        }

        context->getState().setVertexAttribState(index, context->getState().getTargetBuffer(GL_ARRAY_BUFFER), size, type,
                                                 normalized == GL_TRUE, false, stride, ptr);
    }
}

void Viewport(GLint x, GLint y, GLsizei width, GLsizei height)
{
    EVENT("(GLint x = %d, GLint y = %d, GLsizei width = %d, GLsizei height = %d)", x, y, width, height);

    Context *context = getNonLostContext();
    if (context)
    {
        if (width < 0 || height < 0)
        {
            context->recordError(Error(GL_INVALID_VALUE));
            return;
        }

        context->getState().setViewportParams(x, y, width, height);
    }
}

// OpenGL ES 3.0 functions

void ReadBuffer(GLenum mode)
{
    EVENT("(GLenum mode = 0x%X)", mode);

    Context *context = getNonLostContext();
    if (context)
    {
        if (context->getClientVersion() < 3)
        {
            context->recordError(Error(GL_INVALID_OPERATION));
            return;
        }

        // glReadBuffer
        UNIMPLEMENTED();
    }
}

void DrawRangeElements(GLenum mode, GLuint start, GLuint end, GLsizei count, GLenum type, const GLvoid* indices)
{
    EVENT("(GLenum mode = 0x%X, GLuint start = %u, GLuint end = %u, GLsizei count = %d, GLenum type = 0x%X, "
          "const GLvoid* indices = 0x%0.8p)", mode, start, end, count, type, indices);

    Context *context = getNonLostContext();
    if (context)
    {
        if (context->getClientVersion() < 3)
        {
            context->recordError(Error(GL_INVALID_OPERATION));
            return;
        }

        // glDrawRangeElements
        UNIMPLEMENTED();
    }
}

void TexImage3D(GLenum target, GLint level, GLint internalformat, GLsizei width, GLsizei height, GLsizei depth, GLint border, GLenum format, GLenum type, const GLvoid* pixels)
{
    EVENT("(GLenum target = 0x%X, GLint level = %d, GLint internalformat = %d, GLsizei width = %d, "
          "GLsizei height = %d, GLsizei depth = %d, GLint border = %d, GLenum format = 0x%X, "
          "GLenum type = 0x%X, const GLvoid* pixels = 0x%0.8p)",
          target, level, internalformat, width, height, depth, border, format, type, pixels);

    Context *context = getNonLostContext();
    if (context)
    {
        if (context->getClientVersion() < 3)
        {
            context->recordError(Error(GL_INVALID_OPERATION));
            return;
        }

        // validateES3TexImageFormat sets the error code if there is an error
        if (!ValidateES3TexImageParameters(context, target, level, internalformat, false, false,
                                           0, 0, 0, width, height, depth, border, format, type, pixels))
        {
            return;
        }

        switch(target)
        {
          case GL_TEXTURE_3D:
            {
                Texture3D *texture = context->getTexture3D();
                Error error = texture->setImage(level, width, height, depth, internalformat, format, type, context->getState().getUnpackState(), pixels);
                if (error.isError())
                {
                    context->recordError(error);
                    return;
                }
            }
            break;

          case GL_TEXTURE_2D_ARRAY:
            {
                Texture2DArray *texture = context->getTexture2DArray();
                Error error = texture->setImage(level, width, height, depth, internalformat, format, type, context->getState().getUnpackState(), pixels);
                if (error.isError())
                {
                    context->recordError(error);
                    return;
                }
            }
            break;

          default:
            context->recordError(Error(GL_INVALID_ENUM));
            return;
        }
    }
}

void TexSubImage3D(GLenum target, GLint level, GLint xoffset, GLint yoffset, GLint zoffset, GLsizei width, GLsizei height, GLsizei depth, GLenum format, GLenum type, const GLvoid* pixels)
{
    EVENT("(GLenum target = 0x%X, GLint level = %d, GLint xoffset = %d, GLint yoffset = %d, "
          "GLint zoffset = %d, GLsizei width = %d, GLsizei height = %d, GLsizei depth = %d, "
          "GLenum format = 0x%X, GLenum type = 0x%X, const GLvoid* pixels = 0x%0.8p)",
          target, level, xoffset, yoffset, zoffset, width, height, depth, format, type, pixels);

    Context *context = getNonLostContext();
    if (context)
    {
        if (context->getClientVersion() < 3)
        {
            context->recordError(Error(GL_INVALID_OPERATION));
            return;
        }

        // validateES3TexImageFormat sets the error code if there is an error
        if (!ValidateES3TexImageParameters(context, target, level, GL_NONE, false, true,
                                           xoffset, yoffset, zoffset, width, height, depth, 0,
                                           format, type, pixels))
        {
            return;
        }

        // Zero sized uploads are valid but no-ops
        if (width == 0 || height == 0 || depth == 0)
        {
            return;
        }

        switch(target)
        {
          case GL_TEXTURE_3D:
            {
                Texture3D *texture = context->getTexture3D();
                Error error = texture->subImage(level, xoffset, yoffset, zoffset, width, height, depth, format, type, context->getState().getUnpackState(), pixels);
                if (error.isError())
                {
                    context->recordError(error);
                    return;
                }
            }
            break;

          case GL_TEXTURE_2D_ARRAY:
            {
                Texture2DArray *texture = context->getTexture2DArray();
                Error error = texture->subImage(level, xoffset, yoffset, zoffset, width, height, depth, format, type, context->getState().getUnpackState(), pixels);
                if (error.isError())
                {
                    context->recordError(error);
                    return;
                }
            }
            break;

          default:
            context->recordError(Error(GL_INVALID_ENUM));
            return;
        }
    }
}

void CopyTexSubImage3D(GLenum target, GLint level, GLint xoffset, GLint yoffset, GLint zoffset, GLint x, GLint y, GLsizei width, GLsizei height)
{
    EVENT("(GLenum target = 0x%X, GLint level = %d, GLint xoffset = %d, GLint yoffset = %d, "
          "GLint zoffset = %d, GLint x = %d, GLint y = %d, GLsizei width = %d, GLsizei height = %d)",
          target, level, xoffset, yoffset, zoffset, x, y, width, height);

    Context *context = getNonLostContext();
    if (context)
    {
        if (context->getClientVersion() < 3)
        {
            context->recordError(Error(GL_INVALID_OPERATION));
            return;
        }

        if (!ValidateES3CopyTexImageParameters(context, target, level, GL_NONE, true, xoffset, yoffset, zoffset,
                                               x, y, width, height, 0))
        {
            return;
        }

        Framebuffer *framebuffer = context->getState().getReadFramebuffer();
        Texture *texture = NULL;
        switch (target)
        {
          case GL_TEXTURE_3D:
            texture = context->getTexture3D();
            break;

          case GL_TEXTURE_2D_ARRAY:
            texture = context->getTexture2DArray();
            break;

          default:
            context->recordError(Error(GL_INVALID_ENUM));
            return;
        }

        Error error = texture->copySubImage(target, level, xoffset, yoffset, zoffset, x, y, width, height, framebuffer);
        if (error.isError())
        {
            context->recordError(error);
            return;
        }
    }
}

void CompressedTexImage3D(GLenum target, GLint level, GLenum internalformat, GLsizei width, GLsizei height, GLsizei depth, GLint border, GLsizei imageSize, const GLvoid* data)
{
    EVENT("(GLenum target = 0x%X, GLint level = %d, GLenum internalformat = 0x%X, GLsizei width = %d, "
          "GLsizei height = %d, GLsizei depth = %d, GLint border = %d, GLsizei imageSize = %d, "
          "const GLvoid* data = 0x%0.8p)",
          target, level, internalformat, width, height, depth, border, imageSize, data);

    Context *context = getNonLostContext();
    if (context)
    {
        if (context->getClientVersion() < 3)
        {
            context->recordError(Error(GL_INVALID_OPERATION));
            return;
        }

        const InternalFormat &formatInfo = GetInternalFormatInfo(internalformat);
        if (imageSize < 0 || static_cast<GLuint>(imageSize) != formatInfo.computeBlockSize(GL_UNSIGNED_BYTE, width, height))
        {
            context->recordError(Error(GL_INVALID_VALUE));
            return;
        }

        // validateES3TexImageFormat sets the error code if there is an error
        if (!ValidateES3TexImageParameters(context, target, level, internalformat, true, false,
                                           0, 0, 0, width, height, depth, border, GL_NONE, GL_NONE, data))
        {
            return;
        }

        switch(target)
        {
          case GL_TEXTURE_3D:
            {
                Texture3D *texture = context->getTexture3D();
                Error error = texture->setCompressedImage(level, internalformat, width, height, depth, imageSize, context->getState().getUnpackState(), data);
                if (error.isError())
                {
                    context->recordError(error);
                    return;
                }
            }
            break;

          case GL_TEXTURE_2D_ARRAY:
            {
                Texture2DArray *texture = context->getTexture2DArray();
                Error error = texture->setCompressedImage(level, internalformat, width, height, depth, imageSize, context->getState().getUnpackState(), data);
                if (error.isError())
                {
                    context->recordError(error);
                    return;
                }
            }
            break;

          default:
            context->recordError(Error(GL_INVALID_ENUM));
            return;
        }
    }
}

void CompressedTexSubImage3D(GLenum target, GLint level, GLint xoffset, GLint yoffset, GLint zoffset, GLsizei width, GLsizei height, GLsizei depth, GLenum format, GLsizei imageSize, const GLvoid* data)
{
    EVENT("(GLenum target = 0x%X, GLint level = %d, GLint xoffset = %d, GLint yoffset = %d, "
        "GLint zoffset = %d, GLsizei width = %d, GLsizei height = %d, GLsizei depth = %d, "
        "GLenum format = 0x%X, GLsizei imageSize = %d, const GLvoid* data = 0x%0.8p)",
        target, level, xoffset, yoffset, zoffset, width, height, depth, format, imageSize, data);

    Context *context = getNonLostContext();
    if (context)
    {
        if (context->getClientVersion() < 3)
        {
            context->recordError(Error(GL_INVALID_OPERATION));
            return;
        }

        const InternalFormat &formatInfo = GetInternalFormatInfo(format);
        if (imageSize < 0 || static_cast<GLuint>(imageSize) != formatInfo.computeBlockSize(GL_UNSIGNED_BYTE, width, height))
        {
            context->recordError(Error(GL_INVALID_VALUE));
            return;
        }

        if (!data)
        {
            context->recordError(Error(GL_INVALID_VALUE));
            return;
        }

        // validateES3TexImageFormat sets the error code if there is an error
        if (!ValidateES3TexImageParameters(context, target, level, GL_NONE, true, true,
                                           0, 0, 0, width, height, depth, 0, GL_NONE, GL_NONE, data))
        {
            return;
        }

        // Zero sized uploads are valid but no-ops
        if (width == 0 || height == 0)
        {
            return;
        }

        switch(target)
        {
          case GL_TEXTURE_3D:
            {
                Texture3D *texture = context->getTexture3D();
                Error error = texture->subImageCompressed(level, xoffset, yoffset, zoffset, width, height, depth,
                                                              format, imageSize, context->getState().getUnpackState(), data);
                if (error.isError())
                {
                    context->recordError(error);
                    return;
                }
            }
            break;

          case GL_TEXTURE_2D_ARRAY:
            {
                Texture2DArray *texture = context->getTexture2DArray();
                Error error = texture->subImageCompressed(level, xoffset, yoffset, zoffset, width, height, depth,
                                                              format, imageSize, context->getState().getUnpackState(), data);
                if (error.isError())
                {
                    context->recordError(error);
                    return;
                }
            }
            break;

          default:
            context->recordError(Error(GL_INVALID_ENUM));
            return;
        }
    }
}

void GenQueries(GLsizei n, GLuint* ids)
{
    EVENT("(GLsizei n = %d, GLuint* ids = 0x%0.8p)", n, ids);

    Context *context = getNonLostContext();
    if (context)
    {
        if (context->getClientVersion() < 3)
        {
            context->recordError(Error(GL_INVALID_OPERATION));
            return;
        }

        if (n < 0)
        {
            context->recordError(Error(GL_INVALID_VALUE));
            return;
        }

        for (GLsizei i = 0; i < n; i++)
        {
            ids[i] = context->createQuery();
        }
    }
}

void DeleteQueries(GLsizei n, const GLuint* ids)
{
    EVENT("(GLsizei n = %d, GLuint* ids = 0x%0.8p)", n, ids);

    Context *context = getNonLostContext();
    if (context)
    {
        if (context->getClientVersion() < 3)
        {
            context->recordError(Error(GL_INVALID_OPERATION));
            return;
        }

        if (n < 0)
        {
            context->recordError(Error(GL_INVALID_VALUE));
            return;
        }

        for (GLsizei i = 0; i < n; i++)
        {
            context->deleteQuery(ids[i]);
        }
    }
}

GLboolean IsQuery(GLuint id)
{
    EVENT("(GLuint id = %u)", id);

    Context *context = getNonLostContext();
    if (context)
    {
        if (context->getClientVersion() < 3)
        {
            context->recordError(Error(GL_INVALID_OPERATION));
            return GL_FALSE;
        }

        return (context->getQuery(id, false, GL_NONE) != NULL) ? GL_TRUE : GL_FALSE;
    }

    return GL_FALSE;
}

void BeginQuery(GLenum target, GLuint id)
{
    EVENT("(GLenum target = 0x%X, GLuint id = %u)", target, id);

    Context *context = getNonLostContext();
    if (context)
    {
        if (context->getClientVersion() < 3)
        {
            context->recordError(Error(GL_INVALID_OPERATION));
            return;
        }

        if (!ValidateBeginQuery(context, target, id))
        {
            return;
        }

        Error error = context->beginQuery(target, id);
        if (error.isError())
        {
            context->recordError(error);
            return;
        }
    }
}

void EndQuery(GLenum target)
{
    EVENT("(GLenum target = 0x%X)", target);

    Context *context = getNonLostContext();
    if (context)
    {
        if (context->getClientVersion() < 3)
        {
            context->recordError(Error(GL_INVALID_OPERATION));
            return;
        }

        if (!ValidateEndQuery(context, target))
        {
            return;
        }

        Error error = context->endQuery(target);
        if (error.isError())
        {
            context->recordError(error);
            return;
        }
    }
}

void GetQueryiv(GLenum target, GLenum pname, GLint* params)
{
    EVENT("(GLenum target = 0x%X, GLenum pname = 0x%X, GLint* params = 0x%0.8p)", target, pname, params);

    Context *context = getNonLostContext();
    if (context)
    {
        if (context->getClientVersion() < 3)
        {
            context->recordError(Error(GL_INVALID_OPERATION));
            return;
        }

        if (!ValidQueryType(context, target))
        {
            context->recordError(Error(GL_INVALID_ENUM));
            return;
        }

        switch (pname)
        {
          case GL_CURRENT_QUERY:
            params[0] = static_cast<GLint>(context->getState().getActiveQueryId(target));
            break;

          default:
            context->recordError(Error(GL_INVALID_ENUM));
            return;
        }
    }
}

void GetQueryObjectuiv(GLuint id, GLenum pname, GLuint* params)
{
    EVENT("(GLuint id = %u, GLenum pname = 0x%X, GLint* params = 0x%0.8p)", id, pname, params);

    Context *context = getNonLostContext();
    if (context)
    {
        if (context->getClientVersion() < 3)
        {
            context->recordError(Error(GL_INVALID_OPERATION));
            return;
        }

        Query *queryObject = context->getQuery(id, false, GL_NONE);

        if (!queryObject)
        {
            context->recordError(Error(GL_INVALID_OPERATION));
            return;
        }

        if (context->getState().getActiveQueryId(queryObject->getType()) == id)
        {
            context->recordError(Error(GL_INVALID_OPERATION));
            return;
        }

        switch(pname)
        {
          case GL_QUERY_RESULT_EXT:
            {
                Error error = queryObject->getResult(params);
                if (error.isError())
                {
                    context->recordError(error);
                    return;
                }
            }
            break;

          case GL_QUERY_RESULT_AVAILABLE_EXT:
            {
                Error error = queryObject->isResultAvailable(params);
                if (error.isError())
                {
                    context->recordError(error);
                    return;
                }
            }
            break;

          default:
            context->recordError(Error(GL_INVALID_ENUM));
            return;
        }
    }
}

GLboolean UnmapBuffer(GLenum target)
{
    EVENT("(GLenum target = 0x%X)", target);

    Context *context = getNonLostContext();
    if (context)
    {
        if (context->getClientVersion() < 3)
        {
            context->recordError(Error(GL_INVALID_OPERATION));
            return GL_FALSE;
        }

        return UnmapBufferOES(target);
    }

    return GL_FALSE;
}

void GetBufferPointerv(GLenum target, GLenum pname, GLvoid** params)
{
    EVENT("(GLenum target = 0x%X, GLenum pname = 0x%X, GLvoid** params = 0x%0.8p)", target, pname, params);

    Context *context = getNonLostContext();
    if (context)
    {
        if (context->getClientVersion() < 3)
        {
            context->recordError(Error(GL_INVALID_OPERATION));
            return;
        }

        GetBufferPointervOES(target, pname, params);
    }
}

void DrawBuffers(GLsizei n, const GLenum* bufs)
{
    Context *context = getNonLostContext();
    if (context)
    {
        if (context->getClientVersion() < 3)
        {
            context->recordError(Error(GL_INVALID_OPERATION));
            return;
        }

        DrawBuffersEXT(n, bufs);
    }
}

void UniformMatrix2x3fv(GLint location, GLsizei count, GLboolean transpose, const GLfloat* value)
{
    EVENT("(GLint location = %d, GLsizei count = %d, GLboolean transpose = %u, const GLfloat* value = 0x%0.8p)",
          location, count, transpose, value);

    Context *context = getNonLostContext();
    if (context)
    {
        if (!ValidateUniformMatrix(context, GL_FLOAT_MAT2x3, location, count, transpose))
        {
            return;
        }

        Program *program = context->getState().getProgram();
        program->setUniformMatrix2x3fv(location, count, transpose, value);
    }
}

void UniformMatrix3x2fv(GLint location, GLsizei count, GLboolean transpose, const GLfloat* value)
{
    EVENT("(GLint location = %d, GLsizei count = %d, GLboolean transpose = %u, const GLfloat* value = 0x%0.8p)",
          location, count, transpose, value);

    Context *context = getNonLostContext();
    if (context)
    {
        if (!ValidateUniformMatrix(context, GL_FLOAT_MAT3x2, location, count, transpose))
        {
            return;
        }

        Program *program = context->getState().getProgram();
        program->setUniformMatrix3x2fv(location, count, transpose, value);
    }
}

void UniformMatrix2x4fv(GLint location, GLsizei count, GLboolean transpose, const GLfloat* value)
{
    EVENT("(GLint location = %d, GLsizei count = %d, GLboolean transpose = %u, const GLfloat* value = 0x%0.8p)",
          location, count, transpose, value);

    Context *context = getNonLostContext();
    if (context)
    {
        if (!ValidateUniformMatrix(context, GL_FLOAT_MAT2x4, location, count, transpose))
        {
            return;
        }

        Program *program = context->getState().getProgram();
        program->setUniformMatrix2x4fv(location, count, transpose, value);
    }
}

void UniformMatrix4x2fv(GLint location, GLsizei count, GLboolean transpose, const GLfloat* value)
{
    EVENT("(GLint location = %d, GLsizei count = %d, GLboolean transpose = %u, const GLfloat* value = 0x%0.8p)",
          location, count, transpose, value);

    Context *context = getNonLostContext();
    if (context)
    {
        if (!ValidateUniformMatrix(context, GL_FLOAT_MAT4x2, location, count, transpose))
        {
            return;
        }

        Program *program = context->getState().getProgram();
        program->setUniformMatrix4x2fv(location, count, transpose, value);
    }
}

void UniformMatrix3x4fv(GLint location, GLsizei count, GLboolean transpose, const GLfloat* value)
{
    EVENT("(GLint location = %d, GLsizei count = %d, GLboolean transpose = %u, const GLfloat* value = 0x%0.8p)",
          location, count, transpose, value);

    Context *context = getNonLostContext();
    if (context)
    {
        if (!ValidateUniformMatrix(context, GL_FLOAT_MAT3x4, location, count, transpose))
        {
            return;
        }

        Program *program = context->getState().getProgram();
        program->setUniformMatrix3x4fv(location, count, transpose, value);
    }
}

void UniformMatrix4x3fv(GLint location, GLsizei count, GLboolean transpose, const GLfloat* value)
{
    EVENT("(GLint location = %d, GLsizei count = %d, GLboolean transpose = %u, const GLfloat* value = 0x%0.8p)",
          location, count, transpose, value);

    Context *context = getNonLostContext();
    if (context)
    {
        if (!ValidateUniformMatrix(context, GL_FLOAT_MAT4x3, location, count, transpose))
        {
            return;
        }

        Program *program = context->getState().getProgram();
        program->setUniformMatrix4x3fv(location, count, transpose, value);
    }
}

void BlitFramebuffer(GLint srcX0, GLint srcY0, GLint srcX1, GLint srcY1, GLint dstX0, GLint dstY0, GLint dstX1, GLint dstY1, GLbitfield mask, GLenum filter)
{
    EVENT("(GLint srcX0 = %d, GLint srcY0 = %d, GLint srcX1 = %d, GLint srcY1 = %d, GLint dstX0 = %d, "
          "GLint dstY0 = %d, GLint dstX1 = %d, GLint dstY1 = %d, GLbitfield mask = 0x%X, GLenum filter = 0x%X)",
          srcX0, srcY0, srcX1, srcY1, dstX0, dstY0, dstX1, dstY1, mask, filter);

    Context *context = getNonLostContext();
    if (context)
    {
        if (context->getClientVersion() < 3)
        {
            context->recordError(Error(GL_INVALID_OPERATION));
            return;
        }

        if (!ValidateBlitFramebufferParameters(context, srcX0, srcY0, srcX1, srcY1,
                                               dstX0, dstY0, dstX1, dstY1, mask, filter,
                                               false))
        {
            return;
        }

        Error error = context->blitFramebuffer(srcX0, srcY0, srcX1, srcY1, dstX0, dstY0, dstX1, dstY1,
                                                   mask, filter);
        if (error.isError())
        {
            context->recordError(error);
            return;
        }
    }
}

void RenderbufferStorageMultisample(GLenum target, GLsizei samples, GLenum internalformat, GLsizei width, GLsizei height)
{
    EVENT("(GLenum target = 0x%X, GLsizei samples = %d, GLenum internalformat = 0x%X, GLsizei width = %d, GLsizei height = %d)",
        target, samples, internalformat, width, height);

    Context *context = getNonLostContext();
    if (context)
    {
        if (context->getClientVersion() < 3)
        {
            context->recordError(Error(GL_INVALID_OPERATION));
            return;
        }

        if (!ValidateES3RenderbufferStorageParameters(context, target, samples, internalformat, width, height))
        {
            return;
        }

        Renderbuffer *renderbuffer = context->getState().getCurrentRenderbuffer();
        renderbuffer->setStorage(width, height, internalformat, samples);
    }
}

void FramebufferTextureLayer(GLenum target, GLenum attachment, GLuint texture, GLint level, GLint layer)
{
    EVENT("(GLenum target = 0x%X, GLenum attachment = 0x%X, GLuint texture = %u, GLint level = %d, GLint layer = %d)",
        target, attachment, texture, level, layer);

    Context *context = getNonLostContext();
    if (context)
    {
        if (!ValidateFramebufferTextureLayer(context, target, attachment, texture,
                                             level, layer))
        {
            return;
        }

        Framebuffer *framebuffer = context->getState().getTargetFramebuffer(target);
        ASSERT(framebuffer);

        if (texture != 0)
        {
            Texture *textureObject = context->getTexture(texture);
            ImageIndex index(textureObject->getTarget(), level, layer);
            framebuffer->setTextureAttachment(attachment, textureObject, index);
        }
        else
        {
            framebuffer->setNULLAttachment(attachment);
        }
    }
}

GLvoid* MapBufferRange(GLenum target, GLintptr offset, GLsizeiptr length, GLbitfield access)
{
    EVENT("(GLenum target = 0x%X, GLintptr offset = %d, GLsizeiptr length = %d, GLbitfield access = 0x%X)",
          target, offset, length, access);

    Context *context = getNonLostContext();
    if (context)
    {
        if (context->getClientVersion() < 3)
        {
            context->recordError(Error(GL_INVALID_OPERATION));
            return NULL;
        }

        return MapBufferRangeEXT(target, offset, length, access);
    }

    return NULL;
}

void FlushMappedBufferRange(GLenum target, GLintptr offset, GLsizeiptr length)
{
    EVENT("(GLenum target = 0x%X, GLintptr offset = %d, GLsizeiptr length = %d)", target, offset, length);

    Context *context = getNonLostContext();
    if (context)
    {
        if (context->getClientVersion() < 3)
        {
            context->recordError(Error(GL_INVALID_OPERATION));
            return;
        }

        FlushMappedBufferRangeEXT(target, offset, length);
    }
}

void BindVertexArray(GLuint array)
{
    EVENT("(GLuint array = %u)", array);

    Context *context = getNonLostContext();
    if (context)
    {
        if (context->getClientVersion() < 3)
        {
            context->recordError(Error(GL_INVALID_OPERATION));
            return;
        }

        VertexArray *vao = context->getVertexArray(array);

        if (!vao)
        {
            // The default VAO should always exist
            ASSERT(array != 0);
            context->recordError(Error(GL_INVALID_OPERATION));
            return;
        }

        context->bindVertexArray(array);
    }
}

void DeleteVertexArrays(GLsizei n, const GLuint* arrays)
{
    EVENT("(GLsizei n = %d, const GLuint* arrays = 0x%0.8p)", n, arrays);

    Context *context = getNonLostContext();
    if (context)
    {
        if (context->getClientVersion() < 3)
        {
            context->recordError(Error(GL_INVALID_OPERATION));
            return;
        }

        if (n < 0)
        {
            context->recordError(Error(GL_INVALID_VALUE));
            return;
        }

        for (int arrayIndex = 0; arrayIndex < n; arrayIndex++)
        {
            if (arrays[arrayIndex] != 0)
            {
                context->deleteVertexArray(arrays[arrayIndex]);
            }
        }
    }
}

void GenVertexArrays(GLsizei n, GLuint* arrays)
{
    EVENT("(GLsizei n = %d, GLuint* arrays = 0x%0.8p)", n, arrays);

    Context *context = getNonLostContext();
    if (context)
    {
        if (context->getClientVersion() < 3)
        {
            context->recordError(Error(GL_INVALID_OPERATION));
            return;
        }

        if (n < 0)
        {
            context->recordError(Error(GL_INVALID_VALUE));
            return;
        }

        for (int arrayIndex = 0; arrayIndex < n; arrayIndex++)
        {
            arrays[arrayIndex] = context->createVertexArray();
        }
    }
}

GLboolean IsVertexArray(GLuint array)
{
    EVENT("(GLuint array = %u)", array);

    Context *context = getNonLostContext();
    if (context)
    {
        if (context->getClientVersion() < 3)
        {
            context->recordError(Error(GL_INVALID_OPERATION));
            return GL_FALSE;
        }

        if (array == 0)
        {
            return GL_FALSE;
        }

        VertexArray *vao = context->getVertexArray(array);

        return (vao != NULL ? GL_TRUE : GL_FALSE);
    }

    return GL_FALSE;
}

void GetIntegeri_v(GLenum target, GLuint index, GLint* data)
{
    EVENT("(GLenum target = 0x%X, GLuint index = %u, GLint* data = 0x%0.8p)",
          target, index, data);

    Context *context = getNonLostContext();
    if (context)
    {
        if (context->getClientVersion() < 3)
        {
            context->recordError(Error(GL_INVALID_OPERATION));
            return;
        }

        const Caps &caps = context->getCaps();
        switch (target)
        {
          case GL_TRANSFORM_FEEDBACK_BUFFER_START:
          case GL_TRANSFORM_FEEDBACK_BUFFER_SIZE:
          case GL_TRANSFORM_FEEDBACK_BUFFER_BINDING:
            if (index >= caps.maxTransformFeedbackSeparateAttributes)
            {
                context->recordError(Error(GL_INVALID_VALUE));
                return;
            }
            break;

          case GL_UNIFORM_BUFFER_START:
          case GL_UNIFORM_BUFFER_SIZE:
          case GL_UNIFORM_BUFFER_BINDING:
            if (index >= caps.maxCombinedUniformBlocks)
            {
                context->recordError(Error(GL_INVALID_VALUE));
                return;
            }
            break;

          default:
            context->recordError(Error(GL_INVALID_ENUM));
            return;
        }

        if (!(context->getIndexedIntegerv(target, index, data)))
        {
            GLenum nativeType;
            unsigned int numParams = 0;
            if (!context->getIndexedQueryParameterInfo(target, &nativeType, &numParams))
            {
                context->recordError(Error(GL_INVALID_ENUM));
                return;
            }

            if (numParams == 0)
            {
                return; // it is known that pname is valid, but there are no parameters to return
            }

            if (nativeType == GL_INT_64_ANGLEX)
            {
                GLint64 minIntValue = static_cast<GLint64>(std::numeric_limits<int>::min());
                GLint64 maxIntValue = static_cast<GLint64>(std::numeric_limits<int>::max());
                GLint64 *int64Params = new GLint64[numParams];

                context->getIndexedInteger64v(target, index, int64Params);

                for (unsigned int i = 0; i < numParams; ++i)
                {
                    GLint64 clampedValue = std::max(std::min(int64Params[i], maxIntValue), minIntValue);
                    data[i] = static_cast<GLint>(clampedValue);
                }

                delete [] int64Params;
            }
            else
            {
                UNREACHABLE();
            }
        }
    }
}

void BeginTransformFeedback(GLenum primitiveMode)
{
    EVENT("(GLenum primitiveMode = 0x%X)", primitiveMode);

    Context *context = getNonLostContext();
    if (context)
    {
        if (context->getClientVersion() < 3)
        {
            context->recordError(Error(GL_INVALID_OPERATION));
            return;
        }

        switch (primitiveMode)
        {
          case GL_TRIANGLES:
          case GL_LINES:
          case GL_POINTS:
            break;

          default:
            context->recordError(Error(GL_INVALID_ENUM));
            return;
        }

        TransformFeedback *transformFeedback = context->getState().getCurrentTransformFeedback();
        ASSERT(transformFeedback != NULL);

        if (transformFeedback->isStarted())
        {
            context->recordError(Error(GL_INVALID_OPERATION));
            return;
        }

        if (transformFeedback->isPaused())
        {
            transformFeedback->resume();
        }
        else
        {
            transformFeedback->start(primitiveMode);
        }
    }
}

void EndTransformFeedback(void)
{
    EVENT("(void)");

    Context *context = getNonLostContext();
    if (context)
    {
        if (context->getClientVersion() < 3)
        {
            context->recordError(Error(GL_INVALID_OPERATION));
            return;
        }

        TransformFeedback *transformFeedback = context->getState().getCurrentTransformFeedback();
        ASSERT(transformFeedback != NULL);

        if (!transformFeedback->isStarted())
        {
            context->recordError(Error(GL_INVALID_OPERATION));
            return;
        }

        transformFeedback->stop();
    }
}

void BindBufferRange(GLenum target, GLuint index, GLuint buffer, GLintptr offset, GLsizeiptr size)
{
    EVENT("(GLenum target = 0x%X, GLuint index = %u, GLuint buffer = %u, GLintptr offset = %d, GLsizeiptr size = %d)",
          target, index, buffer, offset, size);

    Context *context = getNonLostContext();
    if (context)
    {
        if (context->getClientVersion() < 3)
        {
            context->recordError(Error(GL_INVALID_OPERATION));
            return;
        }

        const Caps &caps = context->getCaps();
        switch (target)
        {
          case GL_TRANSFORM_FEEDBACK_BUFFER:
            if (index >= caps.maxTransformFeedbackSeparateAttributes)
            {
                context->recordError(Error(GL_INVALID_VALUE));
                return;
            }
            break;

          case GL_UNIFORM_BUFFER:
            if (index >= caps.maxUniformBufferBindings)
            {
                context->recordError(Error(GL_INVALID_VALUE));
                return;
            }
            break;

          default:
            context->recordError(Error(GL_INVALID_ENUM));
            return;
        }

        if (buffer != 0 && size <= 0)
        {
            context->recordError(Error(GL_INVALID_VALUE));
            return;
        }

        switch (target)
        {
          case GL_TRANSFORM_FEEDBACK_BUFFER:

            // size and offset must be a multiple of 4
            if (buffer != 0 && ((offset % 4) != 0 || (size % 4) != 0))
            {
                context->recordError(Error(GL_INVALID_VALUE));
                return;
            }

            context->bindIndexedTransformFeedbackBuffer(buffer, index, offset, size);
            context->bindGenericTransformFeedbackBuffer(buffer);
            break;

          case GL_UNIFORM_BUFFER:

            // it is an error to bind an offset not a multiple of the alignment
            if (buffer != 0 && (offset % caps.uniformBufferOffsetAlignment) != 0)
            {
                context->recordError(Error(GL_INVALID_VALUE));
                return;
            }

            context->bindIndexedUniformBuffer(buffer, index, offset, size);
            context->bindGenericUniformBuffer(buffer);
            break;

          default:
            UNREACHABLE();
        }
    }
}

void BindBufferBase(GLenum target, GLuint index, GLuint buffer)
{
    EVENT("(GLenum target = 0x%X, GLuint index = %u, GLuint buffer = %u)",
          target, index, buffer);

    Context *context = getNonLostContext();
    if (context)
    {
        if (context->getClientVersion() < 3)
        {
            context->recordError(Error(GL_INVALID_OPERATION));
            return;
        }

        const Caps &caps = context->getCaps();
        switch (target)
        {
          case GL_TRANSFORM_FEEDBACK_BUFFER:
            if (index >= caps.maxTransformFeedbackSeparateAttributes)
            {
                context->recordError(Error(GL_INVALID_VALUE));
                return;
            }
            break;

          case GL_UNIFORM_BUFFER:
            if (index >= caps.maxUniformBufferBindings)
            {
                context->recordError(Error(GL_INVALID_VALUE));
                return;
            }
            break;

          default:
            context->recordError(Error(GL_INVALID_ENUM));
            return;
        }

        switch (target)
        {
          case GL_TRANSFORM_FEEDBACK_BUFFER:
            context->bindIndexedTransformFeedbackBuffer(buffer, index, 0, 0);
            context->bindGenericTransformFeedbackBuffer(buffer);
            break;

          case GL_UNIFORM_BUFFER:
            context->bindIndexedUniformBuffer(buffer, index, 0, 0);
            context->bindGenericUniformBuffer(buffer);
            break;

          default:
            UNREACHABLE();
        }
    }
}

void TransformFeedbackVaryings(GLuint program, GLsizei count, const GLchar* const* varyings, GLenum bufferMode)
{
    EVENT("(GLuint program = %u, GLsizei count = %d, const GLchar* const* varyings = 0x%0.8p, GLenum bufferMode = 0x%X)",
          program, count, varyings, bufferMode);

    Context *context = getNonLostContext();
    if (context)
    {
        if (context->getClientVersion() < 3)
        {
            context->recordError(Error(GL_INVALID_OPERATION));
            return;
        }

        if (count < 0)
        {
            context->recordError(Error(GL_INVALID_VALUE));
            return;
        }

        const Caps &caps = context->getCaps();
        switch (bufferMode)
        {
          case GL_INTERLEAVED_ATTRIBS:
            break;
          case GL_SEPARATE_ATTRIBS:
            if (static_cast<GLuint>(count) > caps.maxTransformFeedbackSeparateAttributes)
            {
                context->recordError(Error(GL_INVALID_VALUE));
                return;
            }
            break;
          default:
            context->recordError(Error(GL_INVALID_ENUM));
            return;
        }

        if (!ValidProgram(context, program))
        {
            return;
        }

        Program *programObject = context->getProgram(program);
        ASSERT(programObject);

        programObject->setTransformFeedbackVaryings(count, varyings, bufferMode);
    }
}

void GetTransformFeedbackVarying(GLuint program, GLuint index, GLsizei bufSize, GLsizei* length, GLsizei* size, GLenum* type, GLchar* name)
{
    EVENT("(GLuint program = %u, GLuint index = %u, GLsizei bufSize = %d, GLsizei* length = 0x%0.8p, "
          "GLsizei* size = 0x%0.8p, GLenum* type = 0x%0.8p, GLchar* name = 0x%0.8p)",
          program, index, bufSize, length, size, type, name);

    Context *context = getNonLostContext();
    if (context)
    {
        if (context->getClientVersion() < 3)
        {
            context->recordError(Error(GL_INVALID_OPERATION));
            return;
        }

        if (bufSize < 0)
        {
            context->recordError(Error(GL_INVALID_VALUE));
            return;
        }

        if (!ValidProgram(context, program))
        {
            return;
        }

        Program *programObject = context->getProgram(program);
        ASSERT(programObject);

        if (index >= static_cast<GLuint>(programObject->getTransformFeedbackVaryingCount()))
        {
            context->recordError(Error(GL_INVALID_VALUE));
            return;
        }

        programObject->getTransformFeedbackVarying(index, bufSize, length, size, type, name);
    }
}

void VertexAttribIPointer(GLuint index, GLint size, GLenum type, GLsizei stride, const GLvoid* pointer)
{
    EVENT("(GLuint index = %u, GLint size = %d, GLenum type = 0x%X, GLsizei stride = %d, const GLvoid* pointer = 0x%0.8p)",
          index, size, type, stride, pointer);

    Context *context = getNonLostContext();
    if (context)
    {
        if (context->getClientVersion() < 3)
        {
            context->recordError(Error(GL_INVALID_OPERATION));
            return;
        }

        if (index >= MAX_VERTEX_ATTRIBS)
        {
            context->recordError(Error(GL_INVALID_VALUE));
            return;
        }

        if (size < 1 || size > 4)
        {
            context->recordError(Error(GL_INVALID_VALUE));
            return;
        }

        switch (type)
        {
          case GL_BYTE:
          case GL_UNSIGNED_BYTE:
          case GL_SHORT:
          case GL_UNSIGNED_SHORT:
          case GL_INT:
          case GL_UNSIGNED_INT:
          case GL_INT_2_10_10_10_REV:
          case GL_UNSIGNED_INT_2_10_10_10_REV:
            break;

          default:
            context->recordError(Error(GL_INVALID_ENUM));
            return;
        }

        if (stride < 0)
        {
            context->recordError(Error(GL_INVALID_VALUE));
            return;
        }

        if ((type == GL_INT_2_10_10_10_REV || type == GL_UNSIGNED_INT_2_10_10_10_REV) && size != 4)
        {
            context->recordError(Error(GL_INVALID_OPERATION));
            return;
        }

        // [OpenGL ES 3.0.2] Section 2.8 page 24:
        // An INVALID_OPERATION error is generated when a non-zero vertex array object
        // is bound, zero is bound to the ARRAY_BUFFER buffer object binding point,
        // and the pointer argument is not NULL.
        if (context->getState().getVertexArray()->id() != 0 && context->getState().getArrayBufferId() == 0 && pointer != NULL)
        {
            context->recordError(Error(GL_INVALID_OPERATION));
            return;
        }

        context->getState().setVertexAttribState(index, context->getState().getTargetBuffer(GL_ARRAY_BUFFER), size, type, false, true,
                                                 stride, pointer);
    }
}

void GetVertexAttribIiv(GLuint index, GLenum pname, GLint* params)
{
    EVENT("(GLuint index = %u, GLenum pname = 0x%X, GLint* params = 0x%0.8p)",
          index, pname, params);

    Context *context = getNonLostContext();
    if (context)
    {
        if (context->getClientVersion() < 3)
        {
            context->recordError(Error(GL_INVALID_OPERATION));
            return;
        }

        if (index >= MAX_VERTEX_ATTRIBS)
        {
            context->recordError(Error(GL_INVALID_VALUE));
            return;
        }

        const VertexAttribute &attribState = context->getState().getVertexAttribState(index);

        if (!ValidateGetVertexAttribParameters(context, pname))
        {
            return;
        }

        if (pname == GL_CURRENT_VERTEX_ATTRIB)
        {
            const VertexAttribCurrentValueData &currentValueData = context->getState().getVertexAttribCurrentValue(index);
            for (int i = 0; i < 4; ++i)
            {
                params[i] = currentValueData.IntValues[i];
            }
        }
        else
        {
            *params = QuerySingleVertexAttributeParameter<GLint>(attribState, pname);
        }
    }
}

void GetVertexAttribIuiv(GLuint index, GLenum pname, GLuint* params)
{
    EVENT("(GLuint index = %u, GLenum pname = 0x%X, GLuint* params = 0x%0.8p)",
          index, pname, params);

    Context *context = getNonLostContext();
    if (context)
    {
        if (context->getClientVersion() < 3)
        {
            context->recordError(Error(GL_INVALID_OPERATION));
            return;
        }

        if (index >= MAX_VERTEX_ATTRIBS)
        {
            context->recordError(Error(GL_INVALID_VALUE));
            return;
        }

        const VertexAttribute &attribState = context->getState().getVertexAttribState(index);

        if (!ValidateGetVertexAttribParameters(context, pname))
        {
            return;
        }

        if (pname == GL_CURRENT_VERTEX_ATTRIB)
        {
            const VertexAttribCurrentValueData &currentValueData = context->getState().getVertexAttribCurrentValue(index);
            for (int i = 0; i < 4; ++i)
            {
                params[i] = currentValueData.UnsignedIntValues[i];
            }
        }
        else
        {
            *params = QuerySingleVertexAttributeParameter<GLuint>(attribState, pname);
        }
    }
}

void VertexAttribI4i(GLuint index, GLint x, GLint y, GLint z, GLint w)
{
    EVENT("(GLuint index = %u, GLint x = %d, GLint y = %d, GLint z = %d, GLint w = %d)",
          index, x, y, z, w);

    Context *context = getNonLostContext();
    if (context)
    {
        if (context->getClientVersion() < 3)
        {
            context->recordError(Error(GL_INVALID_OPERATION));
            return;
        }

        if (index >= MAX_VERTEX_ATTRIBS)
        {
            context->recordError(Error(GL_INVALID_VALUE));
            return;
        }

        GLint vals[4] = { x, y, z, w };
        context->getState().setVertexAttribi(index, vals);
    }
}

void VertexAttribI4ui(GLuint index, GLuint x, GLuint y, GLuint z, GLuint w)
{
    EVENT("(GLuint index = %u, GLuint x = %u, GLuint y = %u, GLuint z = %u, GLuint w = %u)",
          index, x, y, z, w);

    Context *context = getNonLostContext();
    if (context)
    {
        if (context->getClientVersion() < 3)
        {
            context->recordError(Error(GL_INVALID_OPERATION));
            return;
        }

        if (index >= MAX_VERTEX_ATTRIBS)
        {
            context->recordError(Error(GL_INVALID_VALUE));
            return;
        }

        GLuint vals[4] = { x, y, z, w };
        context->getState().setVertexAttribu(index, vals);
    }
}

void VertexAttribI4iv(GLuint index, const GLint* v)
{
    EVENT("(GLuint index = %u, const GLint* v = 0x%0.8p)", index, v);

    Context *context = getNonLostContext();
    if (context)
    {
        if (context->getClientVersion() < 3)
        {
            context->recordError(Error(GL_INVALID_OPERATION));
            return;
        }

        if (index >= MAX_VERTEX_ATTRIBS)
        {
            context->recordError(Error(GL_INVALID_VALUE));
            return;
        }

        context->getState().setVertexAttribi(index, v);
    }
}

void VertexAttribI4uiv(GLuint index, const GLuint* v)
{
    EVENT("(GLuint index = %u, const GLuint* v = 0x%0.8p)", index, v);

    Context *context = getNonLostContext();
    if (context)
    {
        if (context->getClientVersion() < 3)
        {
            context->recordError(Error(GL_INVALID_OPERATION));
            return;
        }

        if (index >= MAX_VERTEX_ATTRIBS)
        {
            context->recordError(Error(GL_INVALID_VALUE));
            return;
        }

        context->getState().setVertexAttribu(index, v);
    }
}

void GetUniformuiv(GLuint program, GLint location, GLuint* params)
{
    EVENT("(GLuint program = %u, GLint location = %d, GLuint* params = 0x%0.8p)",
          program, location, params);

    Context *context = getNonLostContext();
    if (context)
    {
        if (!ValidateGetUniformuiv(context, program, location, params))
        {
            return;
        }

        Program *programObject = context->getProgram(program);
        ASSERT(programObject);

        programObject->getUniformuiv(location, params);
    }
}

GLint GetFragDataLocation(GLuint program, const GLchar *name)
{
    EVENT("(GLuint program = %u, const GLchar *name = 0x%0.8p)",
          program, name);

    Context *context = getNonLostContext();
    if (context)
    {
        if (context->getClientVersion() < 3)
        {
            context->recordError(Error(GL_INVALID_OPERATION));
            return -1;
        }

        if (program == 0)
        {
            context->recordError(Error(GL_INVALID_VALUE));
            return -1;
        }

        Program *programObject = context->getProgram(program);

        if (!programObject || !programObject->isLinked())
        {
            context->recordError(Error(GL_INVALID_OPERATION));
            return -1;
        }

        return programObject->getFragDataLocation(name);
    }

    return 0;
}

void Uniform1ui(GLint location, GLuint v0)
{
    Uniform1uiv(location, 1, &v0);
}

void Uniform2ui(GLint location, GLuint v0, GLuint v1)
{
    const GLuint xy[] = { v0, v1 };
    Uniform2uiv(location, 1, xy);
}

void Uniform3ui(GLint location, GLuint v0, GLuint v1, GLuint v2)
{
    const GLuint xyz[] = { v0, v1, v2 };
    Uniform3uiv(location, 1, xyz);
}

void Uniform4ui(GLint location, GLuint v0, GLuint v1, GLuint v2, GLuint v3)
{
    const GLuint xyzw[] = { v0, v1, v2, v3 };
    Uniform4uiv(location, 1, xyzw);
}

void Uniform1uiv(GLint location, GLsizei count, const GLuint* value)
{
    EVENT("(GLint location = %d, GLsizei count = %d, const GLuint* value = 0x%0.8p)",
          location, count, value);

    Context *context = getNonLostContext();
    if (context)
    {
        if (!ValidateUniform(context, GL_UNSIGNED_INT, location, count))
        {
            return;
        }

        Program *program = context->getState().getProgram();
        program->setUniform1uiv(location, count, value);
    }
}

void Uniform2uiv(GLint location, GLsizei count, const GLuint* value)
{
    EVENT("(GLint location = %d, GLsizei count = %d, const GLuint* value = 0x%0.8p)",
          location, count, value);

    Context *context = getNonLostContext();
    if (context)
    {
        if (!ValidateUniform(context, GL_UNSIGNED_INT_VEC2, location, count))
        {
            return;
        }

        Program *program = context->getState().getProgram();
        program->setUniform2uiv(location, count, value);
    }
}

void Uniform3uiv(GLint location, GLsizei count, const GLuint* value)
{
    EVENT("(GLint location = %d, GLsizei count = %d, const GLuint* value)",
          location, count, value);

    Context *context = getNonLostContext();
    if (context)
    {
        if (!ValidateUniform(context, GL_UNSIGNED_INT_VEC3, location, count))
        {
            return;
        }

        Program *program = context->getState().getProgram();
        program->setUniform3uiv(location, count, value);
    }
}

void Uniform4uiv(GLint location, GLsizei count, const GLuint* value)
{
    EVENT("(GLint location = %d, GLsizei count = %d, const GLuint* value = 0x%0.8p)",
          location, count, value);

    Context *context = getNonLostContext();
    if (context)
    {
        if (!ValidateUniform(context, GL_UNSIGNED_INT_VEC4, location, count))
        {
            return;
        }

        Program *program = context->getState().getProgram();
        program->setUniform4uiv(location, count, value);
    }
}

void ClearBufferiv(GLenum buffer, GLint drawbuffer, const GLint* value)
{
    EVENT("(GLenum buffer = 0x%X, GLint drawbuffer = %d, const GLint* value = 0x%0.8p)",
          buffer, drawbuffer, value);

    Context *context = getNonLostContext();
    if (context)
    {
        if (!ValidateClearBuffer(context))
        {
            return;
        }

        switch (buffer)
        {
          case GL_COLOR:
            if (drawbuffer < 0 || static_cast<GLuint>(drawbuffer) >= context->getCaps().maxDrawBuffers)
            {
                context->recordError(Error(GL_INVALID_VALUE));
                return;
            }
            break;

          case GL_STENCIL:
            if (drawbuffer != 0)
            {
                context->recordError(Error(GL_INVALID_VALUE));
                return;
            }
            break;

          default:
            context->recordError(Error(GL_INVALID_ENUM));
            return;
        }

        Error error = context->clearBufferiv(buffer, drawbuffer, value);
        if (error.isError())
        {
            context->recordError(error);
            return;
        }
    }
}

void ClearBufferuiv(GLenum buffer, GLint drawbuffer, const GLuint* value)
{
    EVENT("(GLenum buffer = 0x%X, GLint drawbuffer = %d, const GLuint* value = 0x%0.8p)",
          buffer, drawbuffer, value);

    Context *context = getNonLostContext();
    if (context)
    {
        if (!ValidateClearBuffer(context))
        {
            return;
        }

        switch (buffer)
        {
          case GL_COLOR:
            if (drawbuffer < 0 || static_cast<GLuint>(drawbuffer) >= context->getCaps().maxDrawBuffers)
            {
                context->recordError(Error(GL_INVALID_VALUE));
                return;
            }
            break;

          default:
            context->recordError(Error(GL_INVALID_ENUM));
            return;
        }

        Error error = context->clearBufferuiv(buffer, drawbuffer, value);
        if (error.isError())
        {
            context->recordError(error);
            return;
        }
    }
}

void ClearBufferfv(GLenum buffer, GLint drawbuffer, const GLfloat* value)
{
    EVENT("(GLenum buffer = 0x%X, GLint drawbuffer = %d, const GLfloat* value = 0x%0.8p)",
          buffer, drawbuffer, value);

    Context *context = getNonLostContext();
    if (context)
    {
        if (!ValidateClearBuffer(context))
        {
            return;
        }

        switch (buffer)
        {
          case GL_COLOR:
            if (drawbuffer < 0 || static_cast<GLuint>(drawbuffer) >= context->getCaps().maxDrawBuffers)
            {
                context->recordError(Error(GL_INVALID_VALUE));
                return;
            }
            break;

          case GL_DEPTH:
            if (drawbuffer != 0)
            {
                context->recordError(Error(GL_INVALID_VALUE));
                return;
            }
            break;

          default:
            context->recordError(Error(GL_INVALID_ENUM));
            return;
        }

        Error error = context->clearBufferfv(buffer, drawbuffer, value);
        if (error.isError())
        {
            context->recordError(error);
            return;
        }
    }
}

void ClearBufferfi(GLenum buffer, GLint drawbuffer, GLfloat depth, GLint stencil)
{
    EVENT("(GLenum buffer = 0x%X, GLint drawbuffer = %d, GLfloat depth, GLint stencil = %d)",
          buffer, drawbuffer, depth, stencil);

    Context *context = getNonLostContext();
    if (context)
    {
        if (!ValidateClearBuffer(context))
        {
            return;
        }

        switch (buffer)
        {
          case GL_DEPTH_STENCIL:
            if (drawbuffer != 0)
            {
                context->recordError(Error(GL_INVALID_VALUE));
                return;
            }
            break;

          default:
            context->recordError(Error(GL_INVALID_ENUM));
            return;
        }

        Error error = context->clearBufferfi(buffer, drawbuffer, depth, stencil);
        if (error.isError())
        {
            context->recordError(error);
            return;
        }
    }
}

const GLubyte* GetStringi(GLenum name, GLuint index)
{
    EVENT("(GLenum name = 0x%X, GLuint index = %u)", name, index);

    Context *context = getNonLostContext();
    if (context)
    {
        if (context->getClientVersion() < 3)
        {
            context->recordError(Error(GL_INVALID_OPERATION));
            return NULL;
        }

        if (name != GL_EXTENSIONS)
        {
            context->recordError(Error(GL_INVALID_ENUM));
            return NULL;
        }

        if (index >= context->getExtensionStringCount())
        {
            context->recordError(Error(GL_INVALID_VALUE));
            return NULL;
        }

        return reinterpret_cast<const GLubyte*>(context->getExtensionString(index).c_str());
    }

    return NULL;
}

void CopyBufferSubData(GLenum readTarget, GLenum writeTarget, GLintptr readOffset, GLintptr writeOffset, GLsizeiptr size)
{
    EVENT("(GLenum readTarget = 0x%X, GLenum writeTarget = 0x%X, GLintptr readOffset = %d, GLintptr writeOffset = %d, GLsizeiptr size = %d)",
          readTarget, writeTarget, readOffset, writeOffset, size);

    Context *context = getNonLostContext();
    if (context)
    {
        if (context->getClientVersion() < 3)
        {
            context->recordError(Error(GL_INVALID_OPERATION));
            return;
        }

        if (!ValidBufferTarget(context, readTarget) || !ValidBufferTarget(context, writeTarget))
        {
            context->recordError(Error(GL_INVALID_ENUM));
            return;
        }

        Buffer *readBuffer = context->getState().getTargetBuffer(readTarget);
        Buffer *writeBuffer = context->getState().getTargetBuffer(writeTarget);

        if (!readBuffer || !writeBuffer)
        {
            context->recordError(Error(GL_INVALID_OPERATION));
            return;
        }

        // Verify that readBuffer and writeBuffer are not currently mapped
        if (readBuffer->isMapped() || writeBuffer->isMapped())
        {
            context->recordError(Error(GL_INVALID_OPERATION));
            return;
        }

        if (readOffset < 0 || writeOffset < 0 || size < 0 ||
            static_cast<unsigned int>(readOffset + size) > readBuffer->getSize() ||
            static_cast<unsigned int>(writeOffset + size) > writeBuffer->getSize())
        {
            context->recordError(Error(GL_INVALID_VALUE));
            return;
        }

        if (readBuffer == writeBuffer && std::abs(readOffset - writeOffset) < size)
        {
            context->recordError(Error(GL_INVALID_VALUE));
            return;
        }

        // if size is zero, the copy is a successful no-op
        if (size > 0)
        {
            Error error = writeBuffer->copyBufferSubData(readBuffer, readOffset, writeOffset, size);
            if (error.isError())
            {
                context->recordError(error);
                return;
            }
        }
    }
}

void GetUniformIndices(GLuint program, GLsizei uniformCount, const GLchar* const* uniformNames, GLuint* uniformIndices)
{
    EVENT("(GLuint program = %u, GLsizei uniformCount = %d, const GLchar* const* uniformNames = 0x%0.8p, GLuint* uniformIndices = 0x%0.8p)",
          program, uniformCount, uniformNames, uniformIndices);

    Context *context = getNonLostContext();
    if (context)
    {
        if (context->getClientVersion() < 3)
        {
            context->recordError(Error(GL_INVALID_OPERATION));
            return;
        }

        if (uniformCount < 0)
        {
            context->recordError(Error(GL_INVALID_VALUE));
            return;
        }

        Program *programObject = context->getProgram(program);

        if (!programObject)
        {
            if (context->getShader(program))
            {
                context->recordError(Error(GL_INVALID_OPERATION));
                return;
            }
            else
            {
                context->recordError(Error(GL_INVALID_VALUE));
                return;
            }
        }

        if (!programObject->isLinked())
        {
            for (int uniformId = 0; uniformId < uniformCount; uniformId++)
            {
                uniformIndices[uniformId] = GL_INVALID_INDEX;
            }
        }
        else
        {
            for (int uniformId = 0; uniformId < uniformCount; uniformId++)
            {
                uniformIndices[uniformId] = programObject->getUniformIndex(uniformNames[uniformId]);
            }
        }
    }
}

void GetActiveUniformsiv(GLuint program, GLsizei uniformCount, const GLuint* uniformIndices, GLenum pname, GLint* params)
{
    EVENT("(GLuint program = %u, GLsizei uniformCount = %d, const GLuint* uniformIndices = 0x%0.8p, GLenum pname = 0x%X, GLint* params = 0x%0.8p)",
          program, uniformCount, uniformIndices, pname, params);

    Context *context = getNonLostContext();
    if (context)
    {
        if (context->getClientVersion() < 3)
        {
            context->recordError(Error(GL_INVALID_OPERATION));
            return;
        }

        if (uniformCount < 0)
        {
            context->recordError(Error(GL_INVALID_VALUE));
            return;
        }

        Program *programObject = context->getProgram(program);

        if (!programObject)
        {
            if (context->getShader(program))
            {
                context->recordError(Error(GL_INVALID_OPERATION));
                return;
            }
            else
            {
                context->recordError(Error(GL_INVALID_VALUE));
                return;
            }
        }

        switch (pname)
        {
          case GL_UNIFORM_TYPE:
          case GL_UNIFORM_SIZE:
          case GL_UNIFORM_NAME_LENGTH:
          case GL_UNIFORM_BLOCK_INDEX:
          case GL_UNIFORM_OFFSET:
          case GL_UNIFORM_ARRAY_STRIDE:
          case GL_UNIFORM_MATRIX_STRIDE:
          case GL_UNIFORM_IS_ROW_MAJOR:
            break;

          default:
            context->recordError(Error(GL_INVALID_ENUM));
            return;
        }

        if (uniformCount > 0)
        {
            context->recordError(Error(GL_INVALID_VALUE));
            return;
        }

        for (int uniformId = 0; uniformId < uniformCount; uniformId++)
        {
            const GLuint index = uniformIndices[uniformId];

            if (index >= static_cast<GLuint>(programObject->getActiveUniformCount()))
            {
                context->recordError(Error(GL_INVALID_VALUE));
                return;
            }
        }

        for (int uniformId = 0; uniformId < uniformCount; uniformId++)
        {
            const GLuint index = uniformIndices[uniformId];
            params[uniformId] = programObject->getActiveUniformi(index, pname);
        }
    }
}

GLuint GetUniformBlockIndex(GLuint program, const GLchar* uniformBlockName)
{
    EVENT("(GLuint program = %u, const GLchar* uniformBlockName = 0x%0.8p)", program, uniformBlockName);

    Context *context = getNonLostContext();
    if (context)
    {
        if (context->getClientVersion() < 3)
        {
            context->recordError(Error(GL_INVALID_OPERATION));
            return GL_INVALID_INDEX;
        }

        Program *programObject = context->getProgram(program);

        if (!programObject)
        {
            if (context->getShader(program))
            {
                context->recordError(Error(GL_INVALID_OPERATION));
                return GL_INVALID_INDEX;
            }
            else
            {
                context->recordError(Error(GL_INVALID_VALUE));
                return GL_INVALID_INDEX;
            }
        }

        return programObject->getUniformBlockIndex(uniformBlockName);
    }

    return 0;
}

void GetActiveUniformBlockiv(GLuint program, GLuint uniformBlockIndex, GLenum pname, GLint* params)
{
    EVENT("(GLuint program = %u, GLuint uniformBlockIndex = %u, GLenum pname = 0x%X, GLint* params = 0x%0.8p)",
          program, uniformBlockIndex, pname, params);

    Context *context = getNonLostContext();
    if (context)
    {
        if (context->getClientVersion() < 3)
        {
            context->recordError(Error(GL_INVALID_OPERATION));
            return;
        }
        Program *programObject = context->getProgram(program);

        if (!programObject)
        {
            if (context->getShader(program))
            {
                context->recordError(Error(GL_INVALID_OPERATION));
                return;
            }
            else
            {
                context->recordError(Error(GL_INVALID_VALUE));
                return;
            }
        }

        if (uniformBlockIndex >= programObject->getActiveUniformBlockCount())
        {
            context->recordError(Error(GL_INVALID_VALUE));
            return;
        }

        switch (pname)
        {
          case GL_UNIFORM_BLOCK_BINDING:
            *params = static_cast<GLint>(programObject->getUniformBlockBinding(uniformBlockIndex));
            break;

          case GL_UNIFORM_BLOCK_DATA_SIZE:
          case GL_UNIFORM_BLOCK_NAME_LENGTH:
          case GL_UNIFORM_BLOCK_ACTIVE_UNIFORMS:
          case GL_UNIFORM_BLOCK_ACTIVE_UNIFORM_INDICES:
          case GL_UNIFORM_BLOCK_REFERENCED_BY_VERTEX_SHADER:
          case GL_UNIFORM_BLOCK_REFERENCED_BY_FRAGMENT_SHADER:
            programObject->getActiveUniformBlockiv(uniformBlockIndex, pname, params);
            break;

          default:
            context->recordError(Error(GL_INVALID_ENUM));
            return;
        }
    }
}

void GetActiveUniformBlockName(GLuint program, GLuint uniformBlockIndex, GLsizei bufSize, GLsizei* length, GLchar* uniformBlockName)
{
    EVENT("(GLuint program = %u, GLuint uniformBlockIndex = %u, GLsizei bufSize = %d, GLsizei* length = 0x%0.8p, GLchar* uniformBlockName = 0x%0.8p)",
          program, uniformBlockIndex, bufSize, length, uniformBlockName);

    Context *context = getNonLostContext();
    if (context)
    {
        if (context->getClientVersion() < 3)
        {
            context->recordError(Error(GL_INVALID_OPERATION));
            return;
        }

        Program *programObject = context->getProgram(program);

        if (!programObject)
        {
            if (context->getShader(program))
            {
                context->recordError(Error(GL_INVALID_OPERATION));
                return;
            }
            else
            {
                context->recordError(Error(GL_INVALID_VALUE));
                return;
            }
        }

        if (uniformBlockIndex >= programObject->getActiveUniformBlockCount())
        {
            context->recordError(Error(GL_INVALID_VALUE));
            return;
        }

        programObject->getActiveUniformBlockName(uniformBlockIndex, bufSize, length, uniformBlockName);
    }
}

void UniformBlockBinding(GLuint program, GLuint uniformBlockIndex, GLuint uniformBlockBinding)
{
    EVENT("(GLuint program = %u, GLuint uniformBlockIndex = %u, GLuint uniformBlockBinding = %u)",
          program, uniformBlockIndex, uniformBlockBinding);

    Context *context = getNonLostContext();
    if (context)
    {
        if (context->getClientVersion() < 3)
        {
            context->recordError(Error(GL_INVALID_OPERATION));
            return;
        }

        if (uniformBlockBinding >= context->getCaps().maxUniformBufferBindings)
        {
            context->recordError(Error(GL_INVALID_VALUE));
            return;
        }

        Program *programObject = context->getProgram(program);

        if (!programObject)
        {
            if (context->getShader(program))
            {
                context->recordError(Error(GL_INVALID_OPERATION));
                return;
            }
            else
            {
                context->recordError(Error(GL_INVALID_VALUE));
                return;
            }
        }

        // if never linked, there won't be any uniform blocks
        if (uniformBlockIndex >= programObject->getActiveUniformBlockCount())
        {
            context->recordError(Error(GL_INVALID_VALUE));
            return;
        }

        programObject->bindUniformBlock(uniformBlockIndex, uniformBlockBinding);
    }
}

void DrawArraysInstanced(GLenum mode, GLint first, GLsizei count, GLsizei instanceCount)
{
    EVENT("(GLenum mode = 0x%X, GLint first = %d, GLsizei count = %d, GLsizei instanceCount = %d)",
          mode, first, count, instanceCount);

    Context *context = getNonLostContext();
    if (context)
    {
        if (context->getClientVersion() < 3)
        {
            context->recordError(Error(GL_INVALID_OPERATION));
            return;
        }

        // glDrawArraysInstanced
        UNIMPLEMENTED();
    }
}

void DrawElementsInstanced(GLenum mode, GLsizei count, GLenum type, const GLvoid* indices, GLsizei instanceCount)
{
    EVENT("(GLenum mode = 0x%X, GLsizei count = %d, GLenum type = 0x%X, const GLvoid* indices = 0x%0.8p, GLsizei instanceCount = %d)",
          mode, count, type, indices, instanceCount);

    Context *context = getNonLostContext();
    if (context)
    {
        if (context->getClientVersion() < 3)
        {
            context->recordError(Error(GL_INVALID_OPERATION));
            return;
        }

        // glDrawElementsInstanced
        UNIMPLEMENTED();
    }
}

GLsync FenceSync_(GLenum condition, GLbitfield flags)
{
    EVENT("(GLenum condition = 0x%X, GLbitfield flags = 0x%X)", condition, flags);

    Context *context = getNonLostContext();
    if (context)
    {
        if (context->getClientVersion() < 3)
        {
            context->recordError(Error(GL_INVALID_OPERATION));
            return 0;
        }

        if (condition != GL_SYNC_GPU_COMMANDS_COMPLETE)
        {
            context->recordError(Error(GL_INVALID_ENUM));
            return 0;
        }

        if (flags != 0)
        {
            context->recordError(Error(GL_INVALID_VALUE));
            return 0;
        }

        GLsync fenceSync = context->createFenceSync();

        FenceSync *fenceSyncObject = context->getFenceSync(fenceSync);
        Error error = fenceSyncObject->set(condition);
        if (error.isError())
        {
            context->deleteFenceSync(fenceSync);
            context->recordError(error);
            return NULL;
        }

        return fenceSync;
    }

    return NULL;
}

GLboolean IsSync(GLsync sync)
{
    EVENT("(GLsync sync = 0x%0.8p)", sync);

    Context *context = getNonLostContext();
    if (context)
    {
        if (context->getClientVersion() < 3)
        {
            context->recordError(Error(GL_INVALID_OPERATION));
            return GL_FALSE;
        }

        return (context->getFenceSync(sync) != NULL);
    }

    return GL_FALSE;
}

void DeleteSync(GLsync sync)
{
    EVENT("(GLsync sync = 0x%0.8p)", sync);

    Context *context = getNonLostContext();
    if (context)
    {
        if (context->getClientVersion() < 3)
        {
            context->recordError(Error(GL_INVALID_OPERATION));
            return;
        }

        if (sync != static_cast<GLsync>(0) && !context->getFenceSync(sync))
        {
            context->recordError(Error(GL_INVALID_VALUE));
            return;
        }

        context->deleteFenceSync(sync);
    }
}

GLenum ClientWaitSync(GLsync sync, GLbitfield flags, GLuint64 timeout)
{
    EVENT("(GLsync sync = 0x%0.8p, GLbitfield flags = 0x%X, GLuint64 timeout = %llu)",
          sync, flags, timeout);

    Context *context = getNonLostContext();
    if (context)
    {
        if (context->getClientVersion() < 3)
        {
            context->recordError(Error(GL_INVALID_OPERATION));
            return GL_WAIT_FAILED;
        }

        if ((flags & ~(GL_SYNC_FLUSH_COMMANDS_BIT)) != 0)
        {
            context->recordError(Error(GL_INVALID_VALUE));
            return GL_WAIT_FAILED;
        }

        FenceSync *fenceSync = context->getFenceSync(sync);

        if (!fenceSync)
        {
            context->recordError(Error(GL_INVALID_VALUE));
            return GL_WAIT_FAILED;
        }

        GLenum result = GL_WAIT_FAILED;
        Error error = fenceSync->clientWait(flags, timeout, &result);
        if (error.isError())
        {
            context->recordError(error);
            return GL_WAIT_FAILED;
        }

        return result;
    }

    return GL_FALSE;
}

void WaitSync(GLsync sync, GLbitfield flags, GLuint64 timeout)
{
    EVENT("(GLsync sync = 0x%0.8p, GLbitfield flags = 0x%X, GLuint64 timeout = %llu)",
          sync, flags, timeout);

    Context *context = getNonLostContext();
    if (context)
    {
        if (context->getClientVersion() < 3)
        {
            context->recordError(Error(GL_INVALID_OPERATION));
            return;
        }

        if (flags != 0)
        {
            context->recordError(Error(GL_INVALID_VALUE));
            return;
        }

        if (timeout != GL_TIMEOUT_IGNORED)
        {
            context->recordError(Error(GL_INVALID_VALUE));
            return;
        }

        FenceSync *fenceSync = context->getFenceSync(sync);

        if (!fenceSync)
        {
            context->recordError(Error(GL_INVALID_VALUE));
            return;
        }

        Error error = fenceSync->serverWait(flags, timeout);
        if (error.isError())
        {
            context->recordError(error);
        }
    }
}

void GetInteger64v(GLenum pname, GLint64* params)
{
    EVENT("(GLenum pname = 0x%X, GLint64* params = 0x%0.8p)",
          pname, params);

    Context *context = getNonLostContext();
    if (context)
    {
        if (context->getClientVersion() < 3)
        {
            context->recordError(Error(GL_INVALID_OPERATION));
            return;
        }

        GLenum nativeType;
        unsigned int numParams = 0;
        if (!ValidateStateQuery(context, pname, &nativeType, &numParams))
        {
            return;
        }

        if (nativeType == GL_INT_64_ANGLEX)
        {
            context->getInteger64v(pname, params);
        }
        else
        {
            CastStateValues(context, nativeType, pname, numParams, params);
        }
    }
}

void GetSynciv(GLsync sync, GLenum pname, GLsizei bufSize, GLsizei* length, GLint* values)
{
    EVENT("(GLsync sync = 0x%0.8p, GLenum pname = 0x%X, GLsizei bufSize = %d, GLsizei* length = 0x%0.8p, GLint* values = 0x%0.8p)",
          sync, pname, bufSize, length, values);

    Context *context = getNonLostContext();
    if (context)
    {
        if (context->getClientVersion() < 3)
        {
            context->recordError(Error(GL_INVALID_OPERATION));
            return;
        }

        if (bufSize < 0)
        {
            context->recordError(Error(GL_INVALID_VALUE));
            return;
        }

        FenceSync *fenceSync = context->getFenceSync(sync);

        if (!fenceSync)
        {
            context->recordError(Error(GL_INVALID_VALUE));
            return;
        }

        switch (pname)
        {
          case GL_OBJECT_TYPE:     values[0] = static_cast<GLint>(GL_SYNC_FENCE);              break;
          case GL_SYNC_CONDITION:  values[0] = static_cast<GLint>(fenceSync->getCondition());  break;
          case GL_SYNC_FLAGS:      values[0] = 0;                                              break;

          case GL_SYNC_STATUS:
            {
                Error error = fenceSync->getStatus(values);
                if (error.isError())
                {
                    context->recordError(error);
                    return;
                }
                break;
            }

          default:
            context->recordError(Error(GL_INVALID_ENUM));
            return;
        }
    }
}

void GetInteger64i_v(GLenum target, GLuint index, GLint64* data)
{
    EVENT("(GLenum target = 0x%X, GLuint index = %u, GLint64* data = 0x%0.8p)",
          target, index, data);

    Context *context = getNonLostContext();
    if (context)
    {
        if (context->getClientVersion() < 3)
        {
            context->recordError(Error(GL_INVALID_OPERATION));
            return;
        }

        const Caps &caps = context->getCaps();
        switch (target)
        {
          case GL_TRANSFORM_FEEDBACK_BUFFER_START:
          case GL_TRANSFORM_FEEDBACK_BUFFER_SIZE:
          case GL_TRANSFORM_FEEDBACK_BUFFER_BINDING:
            if (index >= caps.maxTransformFeedbackSeparateAttributes)
            {
                context->recordError(Error(GL_INVALID_VALUE));
                return;
            }
            break;

          case GL_UNIFORM_BUFFER_START:
          case GL_UNIFORM_BUFFER_SIZE:
          case GL_UNIFORM_BUFFER_BINDING:
            if (index >= caps.maxUniformBufferBindings)
            {
                context->recordError(Error(GL_INVALID_VALUE));
                return;
            }
            break;

          default:
            context->recordError(Error(GL_INVALID_ENUM));
            return;
        }

        if (!(context->getIndexedInteger64v(target, index, data)))
        {
            GLenum nativeType;
            unsigned int numParams = 0;
            if (!context->getIndexedQueryParameterInfo(target, &nativeType, &numParams))
            {
                context->recordError(Error(GL_INVALID_ENUM));
                return;
            }

            if (numParams == 0)
                return; // it is known that pname is valid, but there are no parameters to return

            if (nativeType == GL_INT)
            {
                GLint *intParams = new GLint[numParams];

                context->getIndexedIntegerv(target, index, intParams);

                for (unsigned int i = 0; i < numParams; ++i)
                {
                    data[i] = static_cast<GLint64>(intParams[i]);
                }

                delete [] intParams;
            }
            else
            {
                UNREACHABLE();
            }
        }
    }
}

void GetBufferParameteri64v(GLenum target, GLenum pname, GLint64* params)
{
    EVENT("(GLenum target = 0x%X, GLenum pname = 0x%X, GLint64* params = 0x%0.8p)",
          target, pname, params);

    Context *context = getNonLostContext();
    if (context)
    {
        if (context->getClientVersion() < 3)
        {
            context->recordError(Error(GL_INVALID_OPERATION));
            return;
        }

        if (!ValidBufferTarget(context, target))
        {
            context->recordError(Error(GL_INVALID_ENUM));
            return;
        }

        if (!ValidBufferParameter(context, pname))
        {
            context->recordError(Error(GL_INVALID_ENUM));
            return;
        }

        Buffer *buffer = context->getState().getTargetBuffer(target);

        if (!buffer)
        {
            // A null buffer means that "0" is bound to the requested buffer target
            context->recordError(Error(GL_INVALID_OPERATION));
            return;
        }

        switch (pname)
        {
          case GL_BUFFER_USAGE:
            *params = static_cast<GLint64>(buffer->getUsage());
            break;
          case GL_BUFFER_SIZE:
            *params = buffer->getSize();
            break;
          case GL_BUFFER_ACCESS_FLAGS:
            *params = static_cast<GLint64>(buffer->getAccessFlags());
            break;
          case GL_BUFFER_MAPPED:
            *params = static_cast<GLint64>(buffer->isMapped());
            break;
          case GL_BUFFER_MAP_OFFSET:
            *params = buffer->getMapOffset();
            break;
          case GL_BUFFER_MAP_LENGTH:
            *params = buffer->getMapLength();
            break;
          default: UNREACHABLE(); break;
        }
    }
}

void GenSamplers(GLsizei count, GLuint* samplers)
{
    EVENT("(GLsizei count = %d, GLuint* samplers = 0x%0.8p)", count, samplers);

    Context *context = getNonLostContext();
    if (context)
    {
        if (context->getClientVersion() < 3)
        {
            context->recordError(Error(GL_INVALID_OPERATION));
            return;
        }

        if (count < 0)
        {
            context->recordError(Error(GL_INVALID_VALUE));
            return;
        }

        for (int i = 0; i < count; i++)
        {
            samplers[i] = context->createSampler();
        }
    }
}

void DeleteSamplers(GLsizei count, const GLuint* samplers)
{
    EVENT("(GLsizei count = %d, const GLuint* samplers = 0x%0.8p)", count, samplers);

    Context *context = getNonLostContext();
    if (context)
    {
        if (context->getClientVersion() < 3)
        {
            context->recordError(Error(GL_INVALID_OPERATION));
            return;
        }

        if (count < 0)
        {
            context->recordError(Error(GL_INVALID_VALUE));
            return;
        }

        for (int i = 0; i < count; i++)
        {
            context->deleteSampler(samplers[i]);
        }
    }
}

GLboolean IsSampler(GLuint sampler)
{
    EVENT("(GLuint sampler = %u)", sampler);

    Context *context = getNonLostContext();
    if (context)
    {
        if (context->getClientVersion() < 3)
        {
            context->recordError(Error(GL_INVALID_OPERATION));
            return GL_FALSE;
        }

        return context->isSampler(sampler);
    }

    return GL_FALSE;
}

void BindSampler(GLuint unit, GLuint sampler)
{
    EVENT("(GLuint unit = %u, GLuint sampler = %u)", unit, sampler);

    Context *context = getNonLostContext();
    if (context)
    {
        if (context->getClientVersion() < 3)
        {
            context->recordError(Error(GL_INVALID_OPERATION));
            return;
        }

        if (sampler != 0 && !context->isSampler(sampler))
        {
            context->recordError(Error(GL_INVALID_OPERATION));
            return;
        }

        if (unit >= context->getCaps().maxCombinedTextureImageUnits)
        {
            context->recordError(Error(GL_INVALID_VALUE));
            return;
        }

        context->bindSampler(unit, sampler);
    }
}

void SamplerParameteri(GLuint sampler, GLenum pname, GLint param)
{
    EVENT("(GLuint sampler = %u, GLenum pname = 0x%X, GLint param = %d)", sampler, pname, param);

    Context *context = getNonLostContext();
    if (context)
    {
        if (context->getClientVersion() < 3)
        {
            context->recordError(Error(GL_INVALID_OPERATION));
            return;
        }

        if (!ValidateSamplerObjectParameter(context, pname))
        {
            return;
        }

        if (!ValidateTexParamParameters(context, pname, param))
        {
            return;
        }

        if (!context->isSampler(sampler))
        {
            context->recordError(Error(GL_INVALID_OPERATION));
            return;
        }

        context->samplerParameteri(sampler, pname, param);
    }
}

void SamplerParameteriv(GLuint sampler, GLenum pname, const GLint* param)
{
    SamplerParameteri(sampler, pname, *param);
}

void SamplerParameterf(GLuint sampler, GLenum pname, GLfloat param)
{
    EVENT("(GLuint sampler = %u, GLenum pname = 0x%X, GLfloat param = %g)", sampler, pname, param);

    Context *context = getNonLostContext();
    if (context)
    {
        if (context->getClientVersion() < 3)
        {
            context->recordError(Error(GL_INVALID_OPERATION));
            return;
        }

        if (!ValidateSamplerObjectParameter(context, pname))
        {
            return;
        }

        if (!ValidateTexParamParameters(context, pname, static_cast<GLint>(param)))
        {
            return;
        }

        if (!context->isSampler(sampler))
        {
            context->recordError(Error(GL_INVALID_OPERATION));
            return;
        }

        context->samplerParameterf(sampler, pname, param);
    }
}

void SamplerParameterfv(GLuint sampler, GLenum pname, const GLfloat* param)
{
    SamplerParameterf(sampler, pname, *param);
}

void GetSamplerParameteriv(GLuint sampler, GLenum pname, GLint* params)
{
    EVENT("(GLuint sampler = %u, GLenum pname = 0x%X, GLint* params = 0x%0.8p)", sampler, pname, params);

    Context *context = getNonLostContext();
    if (context)
    {
        if (context->getClientVersion() < 3)
        {
            context->recordError(Error(GL_INVALID_OPERATION));
            return;
        }

        if (!ValidateSamplerObjectParameter(context, pname))
        {
            return;
        }

        if (!context->isSampler(sampler))
        {
            context->recordError(Error(GL_INVALID_OPERATION));
            return;
        }

        *params = context->getSamplerParameteri(sampler, pname);
    }
}

void GetSamplerParameterfv(GLuint sampler, GLenum pname, GLfloat* params)
{
    EVENT("(GLuint sample = %ur, GLenum pname = 0x%X, GLfloat* params = 0x%0.8p)", sampler, pname, params);

    Context *context = getNonLostContext();
    if (context)
    {
        if (context->getClientVersion() < 3)
        {
            context->recordError(Error(GL_INVALID_OPERATION));
            return;
        }

        if (!ValidateSamplerObjectParameter(context, pname))
        {
            return;
        }

        if (!context->isSampler(sampler))
        {
            context->recordError(Error(GL_INVALID_OPERATION));
            return;
        }

        *params = context->getSamplerParameterf(sampler, pname);
    }
}

void VertexAttribDivisor(GLuint index, GLuint divisor)
{
    EVENT("(GLuint index = %u, GLuint divisor = %u)", index, divisor);

    Context *context = getNonLostContext();
    if (context)
    {
        if (context->getClientVersion() < 3)
        {
            context->recordError(Error(GL_INVALID_OPERATION));
            return;
        }

        if (index >= MAX_VERTEX_ATTRIBS)
        {
            context->recordError(Error(GL_INVALID_VALUE));
            return;
        }

        context->setVertexAttribDivisor(index, divisor);
    }
}

void BindTransformFeedback(GLenum target, GLuint id)
{
    EVENT("(GLenum target = 0x%X, GLuint id = %u)", target, id);

    Context *context = getNonLostContext();
    if (context)
    {
        if (context->getClientVersion() < 3)
        {
            context->recordError(Error(GL_INVALID_OPERATION));
            return;
        }

        switch (target)
        {
          case GL_TRANSFORM_FEEDBACK:
            {
                // Cannot bind a transform feedback object if the current one is started and not paused (3.0.2 pg 85 section 2.14.1)
                TransformFeedback *curTransformFeedback = context->getState().getCurrentTransformFeedback();
                if (curTransformFeedback && curTransformFeedback->isStarted() && !curTransformFeedback->isPaused())
                {
                    context->recordError(Error(GL_INVALID_OPERATION));
                    return;
                }

                // Cannot bind a transform feedback object that does not exist (3.0.2 pg 85 section 2.14.1)
                if (context->getTransformFeedback(id) == NULL)
                {
                    context->recordError(Error(GL_INVALID_OPERATION));
                    return;
                }

                context->bindTransformFeedback(id);
            }
            break;

          default:
            context->recordError(Error(GL_INVALID_ENUM));
            return;
        }
    }
}

void DeleteTransformFeedbacks(GLsizei n, const GLuint* ids)
{
    EVENT("(GLsizei n = %d, const GLuint* ids = 0x%0.8p)", n, ids);

    Context *context = getNonLostContext();
    if (context)
    {
        if (context->getClientVersion() < 3)
        {
            context->recordError(Error(GL_INVALID_OPERATION));
            return;
        }

        for (int i = 0; i < n; i++)
        {
            context->deleteTransformFeedback(ids[i]);
        }
    }
}

void GenTransformFeedbacks(GLsizei n, GLuint* ids)
{
    EVENT("(GLsizei n = %d, GLuint* ids = 0x%0.8p)", n, ids);

    Context *context = getNonLostContext();
    if (context)
    {
        if (context->getClientVersion() < 3)
        {
            context->recordError(Error(GL_INVALID_OPERATION));
            return;
        }

        for (int i = 0; i < n; i++)
        {
            ids[i] = context->createTransformFeedback();
        }
    }
}

GLboolean IsTransformFeedback(GLuint id)
{
    EVENT("(GLuint id = %u)", id);

    Context *context = getNonLostContext();
    if (context)
    {
        if (context->getClientVersion() < 3)
        {
            context->recordError(Error(GL_INVALID_OPERATION));
            return GL_FALSE;
        }

        return ((context->getTransformFeedback(id) != NULL) ? GL_TRUE : GL_FALSE);
    }

    return GL_FALSE;
}

void PauseTransformFeedback(void)
{
    EVENT("(void)");

    Context *context = getNonLostContext();
    if (context)
    {
        if (context->getClientVersion() < 3)
        {
            context->recordError(Error(GL_INVALID_OPERATION));
            return;
        }

        TransformFeedback *transformFeedback = context->getState().getCurrentTransformFeedback();
        ASSERT(transformFeedback != NULL);

        // Current transform feedback must be started and not paused in order to pause (3.0.2 pg 86)
        if (!transformFeedback->isStarted() || transformFeedback->isPaused())
        {
            context->recordError(Error(GL_INVALID_OPERATION));
            return;
        }

        transformFeedback->pause();
    }
}

void ResumeTransformFeedback(void)
{
    EVENT("(void)");

    Context *context = getNonLostContext();
    if (context)
    {
        if (context->getClientVersion() < 3)
        {
            context->recordError(Error(GL_INVALID_OPERATION));
            return;
        }

        TransformFeedback *transformFeedback = context->getState().getCurrentTransformFeedback();
        ASSERT(transformFeedback != NULL);

        // Current transform feedback must be started and paused in order to resume (3.0.2 pg 86)
        if (!transformFeedback->isStarted() || !transformFeedback->isPaused())
        {
            context->recordError(Error(GL_INVALID_OPERATION));
            return;
        }

        transformFeedback->resume();
    }
}

void GetProgramBinary(GLuint program, GLsizei bufSize, GLsizei* length, GLenum* binaryFormat, GLvoid* binary)
{
    EVENT("(GLuint program = %u, GLsizei bufSize = %d, GLsizei* length = 0x%0.8p, GLenum* binaryFormat = 0x%0.8p, GLvoid* binary = 0x%0.8p)",
          program, bufSize, length, binaryFormat, binary);

    Context *context = getNonLostContext();
    if (context)
    {
        if (context->getClientVersion() < 3)
        {
            context->recordError(Error(GL_INVALID_OPERATION));
            return;
        }

        // glGetProgramBinary
        UNIMPLEMENTED();
    }
}

void ProgramBinary(GLuint program, GLenum binaryFormat, const GLvoid* binary, GLsizei length)
{
    EVENT("(GLuint program = %u, GLenum binaryFormat = 0x%X, const GLvoid* binary = 0x%0.8p, GLsizei length = %d)",
          program, binaryFormat, binary, length);

    Context *context = getNonLostContext();
    if (context)
    {
        if (context->getClientVersion() < 3)
        {
            context->recordError(Error(GL_INVALID_OPERATION));
            return;
        }

        // glProgramBinary
        UNIMPLEMENTED();
    }
}

void ProgramParameteri(GLuint program, GLenum pname, GLint value)
{
    EVENT("(GLuint program = %u, GLenum pname = 0x%X, GLint value = %d)",
          program, pname, value);

    Context *context = getNonLostContext();
    if (context)
    {
        if (context->getClientVersion() < 3)
        {
            context->recordError(Error(GL_INVALID_OPERATION));
            return;
        }

        // glProgramParameteri
        UNIMPLEMENTED();
    }
}

void InvalidateFramebuffer(GLenum target, GLsizei numAttachments, const GLenum* attachments)
{
    EVENT("(GLenum target = 0x%X, GLsizei numAttachments = %d, const GLenum* attachments = 0x%0.8p)",
          target, numAttachments, attachments);

    Context *context = getNonLostContext();
    if (context)
    {
        if (context->getClientVersion() < 3)
        {
            context->recordError(Error(GL_INVALID_OPERATION));
            return;
        }

        if (!ValidateInvalidateFramebufferParameters(context, target, numAttachments, attachments))
        {
            return;
        }

        Framebuffer *framebuffer = context->getState().getTargetFramebuffer(target);
        ASSERT(framebuffer);

        if (framebuffer->completeness(context->getData()) == GL_FRAMEBUFFER_COMPLETE)
        {
            Error error = framebuffer->invalidate(context->getCaps(), numAttachments, attachments);
            if (error.isError())
            {
                context->recordError(error);
                return;
            }
        }
    }
}

void InvalidateSubFramebuffer(GLenum target, GLsizei numAttachments, const GLenum* attachments, GLint x, GLint y, GLsizei width, GLsizei height)
{
    EVENT("(GLenum target = 0x%X, GLsizei numAttachments = %d, const GLenum* attachments = 0x%0.8p, GLint x = %d, "
          "GLint y = %d, GLsizei width = %d, GLsizei height = %d)",
          target, numAttachments, attachments, x, y, width, height);

    Context *context = getNonLostContext();
    if (context)
    {
        if (context->getClientVersion() < 3)
        {
            context->recordError(Error(GL_INVALID_OPERATION));
            return;
        }

        if (!ValidateInvalidateFramebufferParameters(context, target, numAttachments, attachments))
        {
            return;
        }

        Framebuffer *framebuffer = context->getState().getTargetFramebuffer(target);
        ASSERT(framebuffer);

        if (framebuffer->completeness(context->getData()) == GL_FRAMEBUFFER_COMPLETE)
        {
            Error error = framebuffer->invalidateSub(numAttachments, attachments, x, y, width, height);
            if (error.isError())
            {
                context->recordError(error);
                return;
            }
        }
    }
}

void TexStorage2D(GLenum target, GLsizei levels, GLenum internalformat, GLsizei width, GLsizei height)
{
    EVENT("(GLenum target = 0x%X, GLsizei levels = %d, GLenum internalformat = 0x%X, GLsizei width = %d, GLsizei height = %d)",
          target, levels, internalformat, width, height);

    Context *context = getNonLostContext();
    if (context)
    {
        if (context->getClientVersion() < 3)
        {
            context->recordError(Error(GL_INVALID_OPERATION));
            return;
        }

        if (!ValidateES3TexStorageParameters(context, target, levels, internalformat, width, height, 1))
        {
            return;
        }

        switch (target)
        {
          case GL_TEXTURE_2D:
            {
                Texture2D *texture2d = context->getTexture2D();
                Error error = texture2d->storage(levels, internalformat, width, height);
                if (error.isError())
                {
                    context->recordError(error);
                    return;
                }
            }
            break;

          case GL_TEXTURE_CUBE_MAP:
            {
                TextureCubeMap *textureCube = context->getTextureCubeMap();
                Error error = textureCube->storage(levels, internalformat, width);
                if (error.isError())
                {
                    context->recordError(error);
                    return;
                }
            }
            break;

          default:
            context->recordError(Error(GL_INVALID_ENUM));
            return;
        }
    }
}

void TexStorage3D(GLenum target, GLsizei levels, GLenum internalformat, GLsizei width, GLsizei height, GLsizei depth)
{
    EVENT("(GLenum target = 0x%X, GLsizei levels = %d, GLenum internalformat = 0x%X, GLsizei width = %d, "
          "GLsizei height = %d, GLsizei depth = %d)",
          target, levels, internalformat, width, height, depth);

    Context *context = getNonLostContext();
    if (context)
    {
        if (context->getClientVersion() < 3)
        {
            context->recordError(Error(GL_INVALID_OPERATION));
            return;
        }

        if (!ValidateES3TexStorageParameters(context, target, levels, internalformat, width, height, depth))
        {
            return;
        }

        switch (target)
        {
          case GL_TEXTURE_3D:
            {
                Texture3D *texture3d = context->getTexture3D();
                Error error = texture3d->storage(levels, internalformat, width, height, depth);
                if (error.isError())
                {
                    context->recordError(error);
                    return;
                }
            }
            break;

          case GL_TEXTURE_2D_ARRAY:
            {
                Texture2DArray *texture2darray = context->getTexture2DArray();
                Error error = texture2darray->storage(levels, internalformat, width, height, depth);
                if (error.isError())
                {
                    context->recordError(error);
                    return;
                }
            }
            break;

          default:
            UNREACHABLE();
        }
    }
}

void GetInternalformativ(GLenum target, GLenum internalformat, GLenum pname, GLsizei bufSize, GLint* params)
{
    EVENT("(GLenum target = 0x%X, GLenum internalformat = 0x%X, GLenum pname = 0x%X, GLsizei bufSize = %d, "
          "GLint* params = 0x%0.8p)",
          target, internalformat, pname, bufSize, params);

    Context *context = getNonLostContext();
    if (context)
    {
        if (context->getClientVersion() < 3)
        {
            context->recordError(Error(GL_INVALID_OPERATION));
            return;
        }

        const TextureCaps &formatCaps = context->getTextureCaps().get(internalformat);
        if (!formatCaps.renderable)
        {
            context->recordError(Error(GL_INVALID_ENUM));
            return;
        }

        if (target != GL_RENDERBUFFER)
        {
            context->recordError(Error(GL_INVALID_ENUM));
            return;
        }

        if (bufSize < 0)
        {
            context->recordError(Error(GL_INVALID_VALUE));
            return;
        }

        switch (pname)
        {
          case GL_NUM_SAMPLE_COUNTS:
            if (bufSize != 0)
            {
                *params = formatCaps.sampleCounts.size();
            }
            break;

          case GL_SAMPLES:
            std::copy_n(formatCaps.sampleCounts.rbegin(), std::min<size_t>(bufSize, formatCaps.sampleCounts.size()), params);
            break;

          default:
            context->recordError(Error(GL_INVALID_ENUM));
            return;
        }
    }
}

// Extension functions

void BlitFramebufferANGLE(GLint srcX0, GLint srcY0, GLint srcX1, GLint srcY1, GLint dstX0, GLint dstY0, GLint dstX1, GLint dstY1,
                                      GLbitfield mask, GLenum filter)
{
    EVENT("(GLint srcX0 = %d, GLint srcY0 = %d, GLint srcX1 = %d, GLint srcY1 = %d, "
          "GLint dstX0 = %d, GLint dstY0 = %d, GLint dstX1 = %d, GLint dstY1 = %d, "
          "GLbitfield mask = 0x%X, GLenum filter = 0x%X)",
          srcX0, srcY0, srcX1, srcX1, dstX0, dstY0, dstX1, dstY1, mask, filter);

    Context *context = getNonLostContext();
    if (context)
    {
        if (!ValidateBlitFramebufferParameters(context, srcX0, srcY0, srcX1, srcY1,
                                               dstX0, dstY0, dstX1, dstY1, mask, filter,
                                               true))
        {
            return;
        }

        Error error = context->blitFramebuffer(srcX0, srcY0, srcX1, srcY1, dstX0, dstY0, dstX1, dstY1,
                                                   mask, filter);
        if (error.isError())
        {
            context->recordError(error);
            return;
        }
    }
}

void TexImage3DOES(GLenum target, GLint level, GLenum internalformat, GLsizei width, GLsizei height, GLsizei depth,
                               GLint border, GLenum format, GLenum type, const GLvoid* pixels)
{
    EVENT("(GLenum target = 0x%X, GLint level = %d, GLenum internalformat = 0x%X, "
          "GLsizei width = %d, GLsizei height = %d, GLsizei depth = %d, GLint border = %d, "
          "GLenum format = 0x%X, GLenum type = 0x%x, const GLvoid* pixels = 0x%0.8p)",
          target, level, internalformat, width, height, depth, border, format, type, pixels);

    UNIMPLEMENTED();   // FIXME
}

void GetProgramBinaryOES(GLuint program, GLsizei bufSize, GLsizei *length,
                                     GLenum *binaryFormat, void *binary)
{
    EVENT("(GLenum program = 0x%X, bufSize = %d, length = 0x%0.8p, binaryFormat = 0x%0.8p, binary = 0x%0.8p)",
          program, bufSize, length, binaryFormat, binary);

    Context *context = getNonLostContext();
    if (context)
    {
        Program *programObject = context->getProgram(program);

        if (!programObject || !programObject->isLinked())
        {
            context->recordError(Error(GL_INVALID_OPERATION));
            return;
        }

        Error error = programObject->saveBinary(binaryFormat, binary, bufSize, length);
        if (error.isError())
        {
            context->recordError(error);
            return;
        }
    }
}

void ProgramBinaryOES(GLuint program, GLenum binaryFormat,
                                  const void *binary, GLint length)
{
    EVENT("(GLenum program = 0x%X, binaryFormat = 0x%x, binary = 0x%0.8p, length = %d)",
          program, binaryFormat, binary, length);

    Context *context = getNonLostContext();
    if (context)
    {
        const std::vector<GLenum> &programBinaryFormats = context->getCaps().programBinaryFormats;
        if (std::find(programBinaryFormats.begin(), programBinaryFormats.end(), binaryFormat) == programBinaryFormats.end())
        {
            context->recordError(Error(GL_INVALID_ENUM));
            return;
        }

        Program *programObject = context->getProgram(program);
        if (!programObject)
        {
            context->recordError(Error(GL_INVALID_OPERATION));
            return;
        }

        Error error = programObject->loadBinary(binaryFormat, binary, length);
        if (error.isError())
        {
            context->recordError(error);
            return;
        }
    }
}

void DrawBuffersEXT(GLsizei n, const GLenum *bufs)
{
    EVENT("(GLenum n = %d, bufs = 0x%0.8p)", n, bufs);

    Context *context = getNonLostContext();
    if (context)
    {
        if (n < 0 || static_cast<GLuint>(n) > context->getCaps().maxDrawBuffers)
        {
            context->recordError(Error(GL_INVALID_VALUE));
            return;
        }

        ASSERT(context->getState().getDrawFramebuffer());

        if (context->getState().getDrawFramebuffer()->id() == 0)
        {
            if (n != 1)
            {
                context->recordError(Error(GL_INVALID_OPERATION));
                return;
            }

            if (bufs[0] != GL_NONE && bufs[0] != GL_BACK)
            {
                context->recordError(Error(GL_INVALID_OPERATION));
                return;
            }
        }
        else
        {
            for (int colorAttachment = 0; colorAttachment < n; colorAttachment++)
            {
                const GLenum attachment = GL_COLOR_ATTACHMENT0_EXT + colorAttachment;
                if (bufs[colorAttachment] != GL_NONE && bufs[colorAttachment] != attachment)
                {
                    context->recordError(Error(GL_INVALID_OPERATION));
                    return;
                }
            }
        }

        Framebuffer *framebuffer = context->getState().getDrawFramebuffer();
        ASSERT(framebuffer);

        for (unsigned int colorAttachment = 0; colorAttachment < static_cast<unsigned int>(n); colorAttachment++)
        {
            framebuffer->setDrawBufferState(colorAttachment, bufs[colorAttachment]);
        }

        for (unsigned int colorAttachment = n; colorAttachment < context->getCaps().maxDrawBuffers; colorAttachment++)
        {
            framebuffer->setDrawBufferState(colorAttachment, GL_NONE);
        }
    }
}

void GetBufferPointervOES(GLenum target, GLenum pname, void** params)
{
    EVENT("(GLenum target = 0x%X, GLenum pname = 0x%X, GLvoid** params = 0x%0.8p)", target, pname, params);

    Context *context = getNonLostContext();
    if (context)
    {
        if (!ValidBufferTarget(context, target))
        {
            context->recordError(Error(GL_INVALID_ENUM));
            return;
        }

        if (pname != GL_BUFFER_MAP_POINTER)
        {
            context->recordError(Error(GL_INVALID_ENUM));
            return;
        }

        Buffer *buffer = context->getState().getTargetBuffer(target);

        if (!buffer || !buffer->isMapped())
        {
            *params = NULL;
        }
        else
        {
            *params = buffer->getMapPointer();
        }
    }
}

void * MapBufferOES(GLenum target, GLenum access)
{
    EVENT("(GLenum target = 0x%X, GLbitfield access = 0x%X)", target, access);

    Context *context = getNonLostContext();
    if (context)
    {
        if (!ValidBufferTarget(context, target))
        {
            context->recordError(Error(GL_INVALID_ENUM));
            return NULL;
        }

        Buffer *buffer = context->getState().getTargetBuffer(target);

        if (buffer == NULL)
        {
            context->recordError(Error(GL_INVALID_OPERATION));
            return NULL;
        }

        if (access != GL_WRITE_ONLY_OES)
        {
            context->recordError(Error(GL_INVALID_ENUM));
            return NULL;
        }

        if (buffer->isMapped())
        {
            context->recordError(Error(GL_INVALID_OPERATION));
            return NULL;
        }

        Error error = buffer->mapRange(0, buffer->getSize(), GL_MAP_WRITE_BIT);
        if (error.isError())
        {
            context->recordError(error);
            return NULL;
        }

        return buffer->getMapPointer();
    }

    return NULL;
}

GLboolean UnmapBufferOES(GLenum target)
{
    EVENT("(GLenum target = 0x%X)", target);

    Context *context = getNonLostContext();
    if (context)
    {
        if (!ValidBufferTarget(context, target))
        {
            context->recordError(Error(GL_INVALID_ENUM));
            return GL_FALSE;
        }

        Buffer *buffer = context->getState().getTargetBuffer(target);

        if (buffer == NULL || !buffer->isMapped())
        {
            context->recordError(Error(GL_INVALID_OPERATION));
            return GL_FALSE;
        }

        // TODO: detect if we had corruption. if so, throw an error and return false.

        Error error = buffer->unmap();
        if (error.isError())
        {
            context->recordError(error);
            return GL_FALSE;
        }

        return GL_TRUE;
    }

    return GL_FALSE;
}

void* MapBufferRangeEXT (GLenum target, GLintptr offset, GLsizeiptr length, GLbitfield access)
{
    EVENT("(GLenum target = 0x%X, GLintptr offset = %d, GLsizeiptr length = %d, GLbitfield access = 0x%X)",
          target, offset, length, access);

    Context *context = getNonLostContext();
    if (context)
    {
        if (!ValidBufferTarget(context, target))
        {
            context->recordError(Error(GL_INVALID_ENUM));
            return NULL;
        }

        if (offset < 0 || length < 0)
        {
            context->recordError(Error(GL_INVALID_VALUE));
            return NULL;
        }

        Buffer *buffer = context->getState().getTargetBuffer(target);

        if (buffer == NULL)
        {
            context->recordError(Error(GL_INVALID_OPERATION));
            return NULL;
        }

        // Check for buffer overflow
        size_t offsetSize = static_cast<size_t>(offset);
        size_t lengthSize = static_cast<size_t>(length);

        if (!rx::IsUnsignedAdditionSafe(offsetSize, lengthSize) ||
            offsetSize + lengthSize > static_cast<size_t>(buffer->getSize()))
        {
            context->recordError(Error(GL_INVALID_VALUE));
            return NULL;
        }

        // Check for invalid bits in the mask
        GLbitfield allAccessBits = GL_MAP_READ_BIT |
                                   GL_MAP_WRITE_BIT |
                                   GL_MAP_INVALIDATE_RANGE_BIT |
                                   GL_MAP_INVALIDATE_BUFFER_BIT |
                                   GL_MAP_FLUSH_EXPLICIT_BIT |
                                   GL_MAP_UNSYNCHRONIZED_BIT;

        if (access & ~(allAccessBits))
        {
            context->recordError(Error(GL_INVALID_VALUE));
            return NULL;
        }

        if (length == 0 || buffer->isMapped())
        {
            context->recordError(Error(GL_INVALID_OPERATION));
            return NULL;
        }

        // Check for invalid bit combinations
        if ((access & (GL_MAP_READ_BIT | GL_MAP_WRITE_BIT)) == 0)
        {
            context->recordError(Error(GL_INVALID_OPERATION));
            return NULL;
        }

        GLbitfield writeOnlyBits = GL_MAP_INVALIDATE_RANGE_BIT |
                                   GL_MAP_INVALIDATE_BUFFER_BIT |
                                   GL_MAP_UNSYNCHRONIZED_BIT;

        if ((access & GL_MAP_READ_BIT) != 0 && (access & writeOnlyBits) != 0)
        {
            context->recordError(Error(GL_INVALID_OPERATION));
            return NULL;
        }

        if ((access & GL_MAP_WRITE_BIT) == 0 && (access & GL_MAP_FLUSH_EXPLICIT_BIT) != 0)
        {
            context->recordError(Error(GL_INVALID_OPERATION));
            return NULL;
        }

        Error error = buffer->mapRange(offset, length, access);
        if (error.isError())
        {
            context->recordError(error);
            return NULL;
        }

        return buffer->getMapPointer();
    }

    return NULL;
}

void FlushMappedBufferRangeEXT (GLenum target, GLintptr offset, GLsizeiptr length)
{
    EVENT("(GLenum target = 0x%X, GLintptr offset = %d, GLsizeiptr length = %d)", target, offset, length);

    Context *context = getNonLostContext();
    if (context)
    {
        if (offset < 0 || length < 0)
        {
            context->recordError(Error(GL_INVALID_VALUE));
            return;
        }

        if (!ValidBufferTarget(context, target))
        {
            context->recordError(Error(GL_INVALID_ENUM));
            return;
        }

        Buffer *buffer = context->getState().getTargetBuffer(target);

        if (buffer == NULL)
        {
            context->recordError(Error(GL_INVALID_OPERATION));
            return;
        }

        if (!buffer->isMapped() || (buffer->getAccessFlags() & GL_MAP_FLUSH_EXPLICIT_BIT) == 0)
        {
            context->recordError(Error(GL_INVALID_OPERATION));
            return;
        }

        // Check for buffer overflow
        size_t offsetSize = static_cast<size_t>(offset);
        size_t lengthSize = static_cast<size_t>(length);

        if (!rx::IsUnsignedAdditionSafe(offsetSize, lengthSize) ||
            offsetSize + lengthSize > static_cast<size_t>(buffer->getMapLength()))
        {
            context->recordError(Error(GL_INVALID_VALUE));
            return;
        }

        // We do not currently support a non-trivial implementation of FlushMappedBufferRange
    }
}

}
