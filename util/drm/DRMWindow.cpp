//
// Copyright (c) 2015 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//

// DRMWindow.cpp: Implementation of OSWindow for DRM

#include <fcntl.h>

#include <xf86drm.h>
#include <xf86drmMode.h>
#include <gbm.h>

#include "drm/DRMWindow.h"

#include "common/debug.h"
#include "system_utils.h"
#include "Timer.h"

#ifdef XXX
namespace {

Bool WaitForMapNotify(Display *dpy, XEvent *event, XPointer window)
{
    return event->type == MapNotify && event->xmap.window == reinterpret_cast<Window>(window);
}

static Key DRMCodeToKey(Display *display, unsigned int scancode)
{
    int temp;
    KeySym *keySymbols;
    keySymbols = XGetKeyboardMapping(display, scancode, 1, &temp);

    unsigned int keySymbol = keySymbols[0];
    XFree(keySymbols);

    switch (keySymbol)
    {
      case XK_Shift_L:     return KEY_LSHIFT;
      case XK_Shift_R:     return KEY_RSHIFT;
      case XK_Alt_L:       return KEY_LALT;
      case XK_Alt_R:       return KEY_RALT;
      case XK_Control_L:   return KEY_LCONTROL;
      case XK_Control_R:   return KEY_RCONTROL;
      case XK_Super_L:     return KEY_LSYSTEM;
      case XK_Super_R:     return KEY_RSYSTEM;
      case XK_Menu:        return KEY_MENU;

      case XK_semicolon:   return KEY_SEMICOLON;
      case XK_slash:       return KEY_SLASH;
      case XK_equal:       return KEY_EQUAL;
      case XK_minus:       return KEY_DASH;
      case XK_bracketleft: return KEY_LBRACKET;
      case XK_bracketright:return KEY_RBRACKET;
      case XK_comma:       return KEY_COMMA;
      case XK_period:      return KEY_PERIOD;
      case XK_backslash:   return KEY_BACKSLASH;
      case XK_asciitilde:  return KEY_TILDE;
      case XK_Escape:      return KEY_ESCAPE;
      case XK_space:       return KEY_SPACE;
      case XK_Return:      return KEY_RETURN;
      case XK_BackSpace:   return KEY_BACK;
      case XK_Tab:         return KEY_TAB;
      case XK_Page_Up:     return KEY_PAGEUP;
      case XK_Page_Down:   return KEY_PAGEDOWN;
      case XK_End:         return KEY_END;
      case XK_Home:        return KEY_HOME;
      case XK_Insert:      return KEY_INSERT;
      case XK_Delete:      return KEY_DELETE;
      case XK_KP_Add:      return KEY_ADD;
      case XK_KP_Subtract: return KEY_SUBTRACT;
      case XK_KP_Multiply: return KEY_MULTIPLY;
      case XK_KP_Divide:   return KEY_DIVIDE;
      case XK_Pause:       return KEY_PAUSE;

      case XK_F1:          return KEY_F1;
      case XK_F2:          return KEY_F2;
      case XK_F3:          return KEY_F3;
      case XK_F4:          return KEY_F4;
      case XK_F5:          return KEY_F5;
      case XK_F6:          return KEY_F6;
      case XK_F7:          return KEY_F7;
      case XK_F8:          return KEY_F8;
      case XK_F9:          return KEY_F9;
      case XK_F10:         return KEY_F10;
      case XK_F11:         return KEY_F11;
      case XK_F12:         return KEY_F12;
      case XK_F13:         return KEY_F13;
      case XK_F14:         return KEY_F14;
      case XK_F15:         return KEY_F15;

      case XK_Left:        return KEY_LEFT;
      case XK_Right:       return KEY_RIGHT;
      case XK_Down:        return KEY_DOWN;
      case XK_Up:          return KEY_UP;

      case XK_KP_Insert:   return KEY_NUMPAD0;
      case XK_KP_End:      return KEY_NUMPAD1;
      case XK_KP_Down:     return KEY_NUMPAD2;
      case XK_KP_Page_Down:return KEY_NUMPAD3;
      case XK_KP_Left:     return KEY_NUMPAD4;
      case XK_KP_5:        return KEY_NUMPAD5;
      case XK_KP_Right:    return KEY_NUMPAD6;
      case XK_KP_Home:     return KEY_NUMPAD7;
      case XK_KP_Up:       return KEY_NUMPAD8;
      case XK_KP_Page_Up:  return KEY_NUMPAD9;

      case XK_a:           return KEY_A;
      case XK_b:           return KEY_B;
      case XK_c:           return KEY_C;
      case XK_d:           return KEY_D;
      case XK_e:           return KEY_E;
      case XK_f:           return KEY_F;
      case XK_g:           return KEY_G;
      case XK_h:           return KEY_H;
      case XK_i:           return KEY_I;
      case XK_j:           return KEY_J;
      case XK_k:           return KEY_K;
      case XK_l:           return KEY_L;
      case XK_m:           return KEY_M;
      case XK_n:           return KEY_N;
      case XK_o:           return KEY_O;
      case XK_p:           return KEY_P;
      case XK_q:           return KEY_Q;
      case XK_r:           return KEY_R;
      case XK_s:           return KEY_S;
      case XK_t:           return KEY_T;
      case XK_u:           return KEY_U;
      case XK_v:           return KEY_V;
      case XK_w:           return KEY_W;
      case XK_x:           return KEY_X;
      case XK_y:           return KEY_Y;
      case XK_z:           return KEY_Z;

      case XK_1:           return KEY_NUM1;
      case XK_2:           return KEY_NUM2;
      case XK_3:           return KEY_NUM3;
      case XK_4:           return KEY_NUM4;
      case XK_5:           return KEY_NUM5;
      case XK_6:           return KEY_NUM6;
      case XK_7:           return KEY_NUM7;
      case XK_8:           return KEY_NUM8;
      case XK_9:           return KEY_NUM9;
      case XK_0:           return KEY_NUM0;
    }

    return Key(0);
}

static void AddDRMKeyStateToEvent(Event *event, unsigned int state)
{
    event->Key.Shift = state & ShiftMask;
    event->Key.Control = state & ControlMask;
    event->Key.Alt = state & Mod1Mask;
    event->Key.System = state & Mod4Mask;
}

}
#endif

DRMWindow::DRMWindow()
/*XXX
    : WM_DELETE_WINDOW(None),
      WM_PROTOCOLS(None),
      TEST_EVENT(None),
      mDisplay(nullptr),
      mWindow(0),
      mRequestedVisualId(-1)
*/
{
}

DRMWindow::DRMWindow(int visualId)
/*XXX
    : WM_DELETE_WINDOW(None),
      WM_PROTOCOLS(None),
      TEST_EVENT(None),
      mDisplay(nullptr),
      mWindow(0),
      mRequestedVisualId(visualId)
*/
{
}

DRMWindow::~DRMWindow()
{
    destroy();
}

struct drm_display {
    struct gbm_device *gbm_device;
    drmModeConnectorPtr conn;
    drmModeModeInfoPtr mode;
    drmModeCrtcPtr crtc;
    int32_t width;
    int32_t height;
    bool setcrtc_done;

    // scanout buffers
/*XXX
    struct slbuf_param param;
    struct slbuf_func func;
    struct slbuf *scanout[2]; // front & back

    struct slbuf *screen_buffer; // on screen
    struct slbuf *pending_buffer; // scheduled flip to this
*/
    bool flip_pending;
};

static drm_display drm;

static drmModeModeInfoPtr
choose_mode(drmModeConnectorPtr conn)
{
    drmModeModeInfoPtr mode = NULL;
    assert(conn);
    assert(conn->connection == DRM_MODE_CONNECTED);
    // use first preferred mode if any, else end up with last mode in list
    for (int i = 0; i < conn->count_modes; ++i) {
        mode = conn->modes + i;
        if (mode->type & DRM_MODE_TYPE_PREFERRED)
            break;
    }
    return mode;
}

static int
choose_crtc(int fd, unsigned count_crtcs, drmModeConnectorPtr conn)
{
    for (int i = 0; i < conn->count_encoders; ++i) {
        drmModeEncoderPtr enc = drmModeGetEncoder(fd, conn->encoders[i]);
        unsigned b = enc->possible_crtcs;
        drmModeFreeEncoder(enc);
        for (int j = 0; b && j < count_crtcs; b >>= 1, ++j) {
            if (b & 1)
                return j;
        }
    }
    return -1;
}

static void init_display(void)
{
    if (drm.width)
        return;

    //XXX be smarter about choosing device
    int fd = open("/dev/dri/card0", O_RDWR | O_CLOEXEC);
    struct gbm_device *gbm_device = gbm_create_device(fd);
    ASSERT(gbm_device);

    drm.conn = NULL;
    drmModeResPtr mr = drmModeGetResources(fd);
    bool monitor_connected = false;
    for (int i = 0; !drm.crtc && i < mr->count_connectors; ++i) {
        drmModeFreeConnector(drm.conn);
        drm.conn = drmModeGetConnector(fd, mr->connectors[i]);
        if (!drm.conn || drm.conn->connection != DRM_MODE_CONNECTED)
            continue;
        monitor_connected = true;
        drm.mode = choose_mode(drm.conn);
        if (!drm.mode)
            continue;
        int n = choose_crtc(fd, mr->count_crtcs, drm.conn);
        if (n < 0)
            continue;
        drm.crtc = drmModeGetCrtc(fd, mr->crtcs[n]);
    }
    drmModeFreeResources(mr);

    if (drm.crtc) {
        drm.width = drm.mode->hdisplay;
        drm.height = drm.mode->vdisplay;
        return;
    }

    if (!monitor_connected) {
        // arbitrary size, so programs that request fullscreen windows can work
        drm.width = 1280;
        drm.height = 1024;
        return;
    }

    ASSERT(0);
}

bool DRMWindow::initialize(const std::string &name, size_t width, size_t height)
{
    destroy();

    init_display();

#ifdef XXX
    mDisplay = XOpenDisplay(NULL);
    if (!mDisplay)
    {
        return false;
    }

    {
        int screen = DefaultScreen(mDisplay);
        Window root = RootWindow(mDisplay, screen);

        Visual *visual;
        if (mRequestedVisualId == -1)
        {
            visual = DefaultVisual(mDisplay, screen);
        }
        else
        {
            XVisualInfo visualTemplate;
            visualTemplate.visualid = mRequestedVisualId;

            int numVisuals       = 0;
            XVisualInfo *visuals = XGetVisualInfo(mDisplay, VisualIDMask, &visualTemplate, &numVisuals);
            if (numVisuals <= 0)
            {
                return false;
            }
            ASSERT(numVisuals == 1);

            visual = visuals[0].visual;
            XFree(visuals);
        }

        int depth = DefaultDepth(mDisplay, screen);
        Colormap colormap = XCreateColormap(mDisplay, root, visual, AllocNone);

        XSetWindowAttributes attributes;
        unsigned long attributeMask = CWBorderPixel | CWColormap | CWEventMask;

        attributes.event_mask = StructureNotifyMask | PointerMotionMask | ButtonPressMask |
                                ButtonReleaseMask | FocusChangeMask | EnterWindowMask |
                                LeaveWindowMask | KeyPressMask | KeyReleaseMask;
        attributes.border_pixel = 0;
        attributes.colormap = colormap;

        mWindow = XCreateWindow(mDisplay, root, 0, 0, width, height, 0, depth, InputOutput,
                                visual, attributeMask, &attributes);
        XFreeColormap(mDisplay, colormap);
    }

    if (!mWindow)
    {
        destroy();
        return false;
    }

    // Tell the window manager to notify us when the user wants to close the
    // window so we can do it ourselves.
    WM_DELETE_WINDOW = XInternAtom(mDisplay, "WM_DELETE_WINDOW", False);
    WM_PROTOCOLS = XInternAtom(mDisplay, "WM_PROTOCOLS", False);
    if (WM_DELETE_WINDOW == None || WM_PROTOCOLS == None)
    {
        destroy();
        return false;
    }

    if(XSetWMProtocols(mDisplay, mWindow, &WM_DELETE_WINDOW, 1) == 0)
    {
        destroy();
        return false;
    }

    // Create an atom to identify our test event
    TEST_EVENT = XInternAtom(mDisplay, "ANGLE_TEST_EVENT", False);
    if (TEST_EVENT == None)
    {
        destroy();
        return false;
    }

    XFlush(mDisplay);
#endif

    mX = 0;
    mY = 0;
    mWidth = width;
    mHeight = height;
    return true;
}

void DRMWindow::destroy()
{
#ifdef XXX
    if (mWindow)
    {
        XDestroyWindow(mDisplay, mWindow);
        mWindow = 0;
    }
    if (mDisplay)
    {
        XCloseDisplay(mDisplay);
        mDisplay = nullptr;
    }
    WM_DELETE_WINDOW = None;
    WM_PROTOCOLS = None;
#endif
}

EGLNativeWindowType DRMWindow::getNativeWindow() const
{
    return 0;//XXX mWindow;
}

EGLNativeDisplayType DRMWindow::getNativeDisplay() const
{
    return EGL_DEFAULT_DISPLAY;//XXX mDisplay;
}

void DRMWindow::messageLoop()
{
#ifdef XXX
    int eventCount = XPending(mDisplay);
    while (eventCount--)
    {
        XEvent event;
        XNextEvent(mDisplay, &event);
        processEvent(event);
    }
#endif
}

void DRMWindow::setMousePosition(int x, int y)
{
    //XXX XWarpPointer(mDisplay, None, mWindow, 0, 0, 0, 0, x, y);
}

OSWindow *CreateOSWindow()
{
    return new DRMWindow();
}

bool DRMWindow::setPosition(int x, int y)
{
    /*XXX
    XMoveWindow(mDisplay, mWindow, x, y);
    XFlush(mDisplay);
    */
    return true;
}

bool DRMWindow::resize(int width, int height)
{
    /*XXX
    XResizeWindow(mDisplay, mWindow, width, height);
    XFlush(mDisplay);
    */

    Timer* timer = CreateTimer();
    timer->start();

    // Wait until the window as actually been resized so that the code calling resize
    // can assume the window has been resized.
    const double kResizeWaitDelay = 0.2;
    while (mHeight != height && mWidth != width && timer->getElapsedTime() < kResizeWaitDelay)
    {
        messageLoop();
        angle::Sleep(10);
    }

    delete timer;

    return true;
}

void DRMWindow::setVisible(bool isVisible)
{
#ifdef XXX
    if (isVisible)
    {
        XMapWindow(mDisplay, mWindow);

        // Wait until we get an event saying this window is mapped so that the
        // code calling setVisible can assume the window is visible.
        // This is important when creating a framebuffer as the framebuffer content
        // is undefined when the window is not visible.
        XEvent dummyEvent;
        XIfEvent(mDisplay, &dummyEvent, WaitForMapNotify, reinterpret_cast<XPointer>(mWindow));
    }
    else
    {
        XUnmapWindow(mDisplay, mWindow);
        XFlush(mDisplay);
    }
#endif
}

void DRMWindow::signalTestEvent()
{
    /*XXX
    XEvent event;
    event.type = ClientMessage;
    event.xclient.message_type = TEST_EVENT;
    // Format needs to be valid or a BadValue is generated
    event.xclient.format = 32;

    // Hijack StructureNotifyMask as we know we will be listening for it.
    XSendEvent(mDisplay, mWindow, False, StructureNotifyMask, &event);
    */
}

#ifdef XXX
void DRMWindow::processEvent(const XEvent &xEvent)
{
    // TODO(cwallez) text events
    switch (xEvent.type)
    {
      case ButtonPress:
        {
            Event event;
            MouseButton button = MOUSEBUTTON_UNKNOWN;
            int wheelX = 0;
            int wheelY = 0;

            // The mouse wheel updates are sent via button events.
            switch (xEvent.xbutton.button)
            {
              case Button4:
                wheelY = 1;
                break;
              case Button5:
                wheelY = -1;
                break;
              case 6:
                wheelX = 1;
                break;
              case 7:
                wheelX = -1;
                break;

              case Button1:
                button = MOUSEBUTTON_LEFT;
                break;
              case Button2:
                button = MOUSEBUTTON_MIDDLE;
                break;
              case Button3:
                button = MOUSEBUTTON_RIGHT;
                break;
              case 8:
                button = MOUSEBUTTON_BUTTON4;
                break;
              case 9:
                button = MOUSEBUTTON_BUTTON5;
                break;

              default:
                break;
            }

            if (wheelY != 0)
            {
                event.Type = Event::EVENT_MOUSE_WHEEL_MOVED;
                event.MouseWheel.Delta = wheelY;
                pushEvent(event);
            }

            if (button != MOUSEBUTTON_UNKNOWN)
            {
                event.Type = Event::EVENT_MOUSE_BUTTON_RELEASED;
                event.MouseButton.Button = button;
                event.MouseButton.X = xEvent.xbutton.x;
                event.MouseButton.Y = xEvent.xbutton.y;
                pushEvent(event);
            }
        }
        break;

      case ButtonRelease:
        {
            Event event;
            MouseButton button = MOUSEBUTTON_UNKNOWN;

            switch (xEvent.xbutton.button)
            {
              case Button1:
                button = MOUSEBUTTON_LEFT;
                break;
              case Button2:
                button = MOUSEBUTTON_MIDDLE;
                break;
              case Button3:
                button = MOUSEBUTTON_RIGHT;
                break;
              case 8:
                button = MOUSEBUTTON_BUTTON4;
                break;
              case 9:
                button = MOUSEBUTTON_BUTTON5;
                break;

              default:
                break;
            }

            if (button != MOUSEBUTTON_UNKNOWN)
            {
                event.Type = Event::EVENT_MOUSE_BUTTON_RELEASED;
                event.MouseButton.Button = button;
                event.MouseButton.X = xEvent.xbutton.x;
                event.MouseButton.Y = xEvent.xbutton.y;
                pushEvent(event);
            }
        }
        break;

      case KeyPress:
        {
            Event event;
            event.Type = Event::EVENT_KEY_PRESSED;
            event.Key.Code = DRMCodeToKey(mDisplay, xEvent.xkey.keycode);
            AddDRMKeyStateToEvent(&event, xEvent.xkey.state);
            pushEvent(event);
        }
        break;

      case KeyRelease:
        {
            Event event;
            event.Type = Event::EVENT_KEY_RELEASED;
            event.Key.Code = DRMCodeToKey(mDisplay, xEvent.xkey.keycode);
            AddDRMKeyStateToEvent(&event, xEvent.xkey.state);
            pushEvent(event);
        }
        break;

      case EnterNotify:
        {
            Event event;
            event.Type = Event::EVENT_MOUSE_ENTERED;
            pushEvent(event);
        }
        break;

      case LeaveNotify:
        {
            Event event;
            event.Type = Event::EVENT_MOUSE_LEFT;
            pushEvent(event);
        }
        break;

      case MotionNotify:
        {
            Event event;
            event.Type = Event::EVENT_MOUSE_MOVED;
            event.MouseMove.X = xEvent.xmotion.x;
            event.MouseMove.Y = xEvent.xmotion.y;
            pushEvent(event);
        }
        break;

      case ConfigureNotify:
        {
            if (xEvent.xconfigure.width != mWidth || xEvent.xconfigure.height != mHeight)
            {
                Event event;
                event.Type = Event::EVENT_RESIZED;
                event.Size.Width = xEvent.xconfigure.width;
                event.Size.Height = xEvent.xconfigure.height;
                pushEvent(event);
            }
            if (xEvent.xconfigure.x != mX || xEvent.xconfigure.y != mY)
            {
                // Sometimes, the window manager reparents our window (for example
                // when resizing) then the X and Y coordinates will be with respect to
                // the new parent and not what the user wants to know. Use
                // XTranslateCoordinates to get the coordinates on the screen.
                int screen = DefaultScreen(mDisplay);
                Window root = RootWindow(mDisplay, screen);

                int x, y;
                Window child;
                XTranslateCoordinates(mDisplay, mWindow, root, 0, 0, &x, &y, &child);

                if (x != mX || y != mY)
                {
                    Event event;
                    event.Type = Event::EVENT_MOVED;
                    event.Move.X = x;
                    event.Move.Y = y;
                    pushEvent(event);
                }
            }
        }
        break;

      case FocusIn:
        if (xEvent.xfocus.mode == NotifyNormal || xEvent.xfocus.mode == NotifyWhileGrabbed)
        {
            Event event;
            event.Type = Event::EVENT_GAINED_FOCUS;
            pushEvent(event);
        }
        break;

      case FocusOut:
        if (xEvent.xfocus.mode == NotifyNormal || xEvent.xfocus.mode == NotifyWhileGrabbed)
        {
            Event event;
            event.Type = Event::EVENT_LOST_FOCUS;
            pushEvent(event);
        }
        break;

      case DestroyNotify:
        // We already received WM_DELETE_WINDOW
        break;

      case ClientMessage:
        if (xEvent.xclient.message_type == WM_PROTOCOLS &&
            static_cast<Atom>(xEvent.xclient.data.l[0]) == WM_DELETE_WINDOW)
        {
            Event event;
            event.Type = Event::EVENT_CLOSED;
            pushEvent(event);
        }
        else if (xEvent.xclient.message_type == TEST_EVENT)
        {
            Event event;
            event.Type = Event::EVENT_TEST;
            pushEvent(event);
        }
        break;
    }
}
#endif
