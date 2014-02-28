//
// Copyright (c) 2014 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//

#include "win32/Win32Window.h"

Key VirtualKeyCodeToKey(WPARAM key, LPARAM flags)
{
    switch (key)
    {
        // Check the scancode to distinguish between left and right shift
      case VK_SHIFT:
        {
            static unsigned int lShift = MapVirtualKey(VK_LSHIFT, MAPVK_VK_TO_VSC);
            unsigned int scancode = static_cast<unsigned int>((flags & (0xFF << 16)) >> 16);
            return scancode == lShift ? Key_LShift : Key_RShift;
        }

        // Check the "extended" flag to distinguish between left and right alt
      case VK_MENU:       return (HIWORD(flags) & KF_EXTENDED) ? Key_RAlt : Key_LAlt;

        // Check the "extended" flag to distinguish between left and right control
      case VK_CONTROL:    return (HIWORD(flags) & KF_EXTENDED) ? Key_RControl : Key_LControl;

        // Other keys are reported properly
      case VK_LWIN:       return Key_LSystem;
      case VK_RWIN:       return Key_RSystem;
      case VK_APPS:       return Key_Menu;
      case VK_OEM_1:      return Key_SemiColon;
      case VK_OEM_2:      return Key_Slash;
      case VK_OEM_PLUS:   return Key_Equal;
      case VK_OEM_MINUS:  return Key_Dash;
      case VK_OEM_4:      return Key_LBracket;
      case VK_OEM_6:      return Key_RBracket;
      case VK_OEM_COMMA:  return Key_Comma;
      case VK_OEM_PERIOD: return Key_Period;
      case VK_OEM_7:      return Key_Quote;
      case VK_OEM_5:      return Key_BackSlash;
      case VK_OEM_3:      return Key_Tilde;
      case VK_ESCAPE:     return Key_Escape;
      case VK_SPACE:      return Key_Space;
      case VK_RETURN:     return Key_Return;
      case VK_BACK:       return Key_Back;
      case VK_TAB:        return Key_Tab;
      case VK_PRIOR:      return Key_PageUp;
      case VK_NEXT:       return Key_PageDown;
      case VK_END:        return Key_End;
      case VK_HOME:       return Key_Home;
      case VK_INSERT:     return Key_Insert;
      case VK_DELETE:     return Key_Delete;
      case VK_ADD:        return Key_Add;
      case VK_SUBTRACT:   return Key_Subtract;
      case VK_MULTIPLY:   return Key_Multiply;
      case VK_DIVIDE:     return Key_Divide;
      case VK_PAUSE:      return Key_Pause;
      case VK_F1:         return Key_F1;
      case VK_F2:         return Key_F2;
      case VK_F3:         return Key_F3;
      case VK_F4:         return Key_F4;
      case VK_F5:         return Key_F5;
      case VK_F6:         return Key_F6;
      case VK_F7 :         return Key_F7;
      case VK_F8:         return Key_F8;
      case VK_F9:         return Key_F9;
      case VK_F10:        return Key_F10;
      case VK_F11:        return Key_F11;
      case VK_F12:        return Key_F12;
      case VK_F13:        return Key_F13;
      case VK_F14:        return Key_F14;
      case VK_F15:        return Key_F15;
      case VK_LEFT:       return Key_Left;
      case VK_RIGHT:      return Key_Right;
      case VK_UP:         return Key_Up;
      case VK_DOWN:       return Key_Down;
      case VK_NUMPAD0:    return Key_Numpad0;
      case VK_NUMPAD1:    return Key_Numpad1;
      case VK_NUMPAD2:    return Key_Numpad2;
      case VK_NUMPAD3:    return Key_Numpad3;
      case VK_NUMPAD4:    return Key_Numpad4;
      case VK_NUMPAD5:    return Key_Numpad5;
      case VK_NUMPAD6:    return Key_Numpad6;
      case VK_NUMPAD7:    return Key_Numpad7;
      case VK_NUMPAD8:    return Key_Numpad8;
      case VK_NUMPAD9:    return Key_Numpad9;
      case 'A':           return Key_A;
      case 'Z':           return Key_Z;
      case 'E':           return Key_E;
      case 'R':           return Key_R;
      case 'T':           return Key_T;
      case 'Y':           return Key_Y;
      case 'U':           return Key_U;
      case 'I':           return Key_I;
      case 'O':           return Key_O;
      case 'P':           return Key_P;
      case 'Q':           return Key_Q;
      case 'S':           return Key_S;
      case 'D':           return Key_D;
      case 'F':           return Key_F;
      case 'G':           return Key_G;
      case 'H':           return Key_H;
      case 'J':           return Key_J;
      case 'K':           return Key_K;
      case 'L':           return Key_L;
      case 'M':           return Key_M;
      case 'W':           return Key_W;
      case 'X':           return Key_X;
      case 'C':           return Key_C;
      case 'V':           return Key_V;
      case 'B':           return Key_B;
      case 'N':           return Key_N;
      case '0':           return Key_Num0;
      case '1':           return Key_Num1;
      case '2':           return Key_Num2;
      case '3':           return Key_Num3;
      case '4':           return Key_Num4;
      case '5':           return Key_Num5;
      case '6':           return Key_Num6;
      case '7':           return Key_Num7;
      case '8':           return Key_Num8;
      case '9':           return Key_Num9;
    }

    return Key(0);
}

LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
    switch(message)
    {
      case WM_NCCREATE:
        {
            LPCREATESTRUCT pCreateStruct = (LPCREATESTRUCT)lParam;
            SetWindowLongPtr(hWnd, GWLP_USERDATA, (LONG_PTR)pCreateStruct->lpCreateParams);
            return DefWindowProcA(hWnd, message, wParam, lParam);
        }
    }

    Window *window = (Window*)(LONG_PTR)GetWindowLongPtr(hWnd, GWLP_USERDATA);
    if (window)
    {
        switch (message)
        {
          case WM_DESTROY:
          case WM_CLOSE:
            {
                Event event;
                event.Type = Event::EventType_Closed;
                window->pushEvent(event);
                break;
            }

          case WM_MOVE:
            {
                RECT winRect;
                GetClientRect(hWnd, &winRect);

                POINT topLeft;
                topLeft.x = winRect.left;
                topLeft.y = winRect.top;
                ClientToScreen(hWnd, &topLeft);

                Event event;
                event.Type        = Event::EventType_Moved;
                event.Move.X      = topLeft.x;
                event.Move.Y      = topLeft.y;
                window->pushEvent(event);

                break;
            }

          case WM_SIZE:
            {
                RECT winRect;
                GetClientRect(hWnd, &winRect);

                POINT topLeft;
                topLeft.x = winRect.left;
                topLeft.y = winRect.top;
                ClientToScreen(hWnd, &topLeft);

                POINT botRight;
                botRight.x = winRect.right;
                botRight.y = winRect.bottom;
                ClientToScreen(hWnd, &botRight);

                Event event;
                event.Type        = Event::EventType_Resized;
                event.Size.Width  = botRight.x - topLeft.x;
                event.Size.Height = botRight.y - topLeft.y;
                window->pushEvent(event);

                break;
            }

          case WM_SETFOCUS:
            {
                Event event;
                event.Type = Event::EventType_GainedFocus;
                window->pushEvent(event);
                break;
            }

          case WM_KILLFOCUS:
            {
                Event event;
                event.Type = Event::EventType_LostFocus;
                window->pushEvent(event);
                break;
            }

          case WM_KEYDOWN:
          case WM_SYSKEYDOWN:
          case WM_KEYUP:
          case WM_SYSKEYUP:
            {
                bool down = (message == WM_KEYDOWN || message == WM_SYSKEYDOWN);

                Event event;
                event.Type        = down ? Event::EventType_KeyPressed : Event::EventType_KeyReleased;
                event.Key.Alt     = HIWORD(GetAsyncKeyState(VK_MENU))    != 0;
                event.Key.Control = HIWORD(GetAsyncKeyState(VK_CONTROL)) != 0;
                event.Key.Shift   = HIWORD(GetAsyncKeyState(VK_SHIFT))   != 0;
                event.Key.System  = HIWORD(GetAsyncKeyState(VK_LWIN)) || HIWORD(GetAsyncKeyState(VK_RWIN));
                event.Key.Code    = VirtualKeyCodeToKey(wParam, lParam);
                window->pushEvent(event);

                break;
            }

          case WM_MOUSEWHEEL:
            {
                Event event;
                event.Type = Event::EventType_MouseWheelMoved;
                event.MouseWheel.Delta = static_cast<short>(HIWORD(wParam)) / 120;
                window->pushEvent(event);
                break;
            }

          case WM_LBUTTONDOWN:
          case WM_LBUTTONDBLCLK:
            {
                Event event;
                event.Type               = Event::EventType_MouseButtonPressed;
                event.MouseButton.Button = MouseButton_Left;
                event.MouseButton.X      = static_cast<short>(LOWORD(lParam));
                event.MouseButton.Y      = static_cast<short>(HIWORD(lParam));
                window->pushEvent(event);
                break;
            }

          case WM_LBUTTONUP:
            {
                Event event;
                event.Type               = Event::EventType_MouseButtonReleased;
                event.MouseButton.Button = MouseButton_Left;
                event.MouseButton.X      = static_cast<short>(LOWORD(lParam));
                event.MouseButton.Y      = static_cast<short>(HIWORD(lParam));
                window->pushEvent(event);
                break;
            }

          case WM_RBUTTONDOWN:
          case WM_RBUTTONDBLCLK:
            {
                Event event;
                event.Type               = Event::EventType_MouseButtonPressed;
                event.MouseButton.Button = MouseButton_Right;
                event.MouseButton.X      = static_cast<short>(LOWORD(lParam));
                event.MouseButton.Y      = static_cast<short>(HIWORD(lParam));
                window->pushEvent(event);
                break;
            }

            // Mouse right button up event
          case WM_RBUTTONUP:
            {
                Event event;
                event.Type               = Event::EventType_MouseButtonReleased;
                event.MouseButton.Button = MouseButton_Right;
                event.MouseButton.X      = static_cast<short>(LOWORD(lParam));
                event.MouseButton.Y      = static_cast<short>(HIWORD(lParam));
                window->pushEvent(event);
                break;
            }

            // Mouse wheel button down event
          case WM_MBUTTONDOWN:
          case WM_MBUTTONDBLCLK:
            {
                Event event;
                event.Type               = Event::EventType_MouseButtonPressed;
                event.MouseButton.Button = MouseButton_Middle;
                event.MouseButton.X      = static_cast<short>(LOWORD(lParam));
                event.MouseButton.Y      = static_cast<short>(HIWORD(lParam));
                window->pushEvent(event);
                break;
            }

            // Mouse wheel button up event
          case WM_MBUTTONUP:
            {
                Event event;
                event.Type               = Event::EventType_MouseButtonReleased;
                event.MouseButton.Button = MouseButton_Middle;
                event.MouseButton.X      = static_cast<short>(LOWORD(lParam));
                event.MouseButton.Y      = static_cast<short>(HIWORD(lParam));
                window->pushEvent(event);
                break;
            }

            // Mouse X button down event
          case WM_XBUTTONDOWN:
          case WM_XBUTTONDBLCLK:
            {
                Event event;
                event.Type               = Event::EventType_MouseButtonPressed;
                event.MouseButton.Button = (HIWORD(wParam) == XBUTTON1) ? MouseButton_Button4 : MouseButton_Button5;
                event.MouseButton.X      = static_cast<short>(LOWORD(lParam));
                event.MouseButton.Y      = static_cast<short>(HIWORD(lParam));
                window->pushEvent(event);
                break;
            }

            // Mouse X button up event
          case WM_XBUTTONUP:
            {
                Event event;
                event.Type               = Event::EventType_MouseButtonReleased;
                event.MouseButton.Button = (HIWORD(wParam) == XBUTTON1) ? MouseButton_Button4 : MouseButton_Button5;
                event.MouseButton.X      = static_cast<short>(LOWORD(lParam));
                event.MouseButton.Y      = static_cast<short>(HIWORD(lParam));
                window->pushEvent(event);
                break;
            }

          case WM_MOUSEMOVE:
            {
                int mouseX = static_cast<short>(LOWORD(lParam));
                int mouseY = static_cast<short>(HIWORD(lParam));

                Event event;
                event.Type        = Event::EventType_MouseMoved;
                event.MouseMove.X = mouseX;
                event.MouseMove.Y = mouseY;
                window->pushEvent(event);
                break;
            }

          case WM_MOUSELEAVE:
            {
                Event event;
                event.Type = Event::EventType_MouseLeft;
                window->pushEvent(event);
                break;
            }
        }

    }
    return DefWindowProcA(hWnd, message, wParam, lParam);
}

Win32Window::Win32Window()
    : mClassName(),
      mDisplay(0),
      mNativeWindow(0),
      mNativeDisplay(0)
{
}

Win32Window::~Win32Window()
{
    destroy();
}

bool Win32Window::initialize(const std::string &name, size_t width, size_t height)
{
    destroy();

    mClassName = name;

    WNDCLASSEXA windowClass = { 0 };
    windowClass.cbSize = sizeof(WNDCLASSEXA);
    windowClass.style = CS_OWNDC;
    windowClass.lpfnWndProc = WndProc;
    windowClass.cbClsExtra = 0;
    windowClass.cbWndExtra = 0;
    windowClass.hInstance = GetModuleHandle(NULL);
    windowClass.hIcon = NULL;
    windowClass.hCursor = LoadCursorW(NULL, IDC_ARROW);
    windowClass.hbrBackground = 0;
    windowClass.lpszMenuName = NULL;
    windowClass.lpszClassName = mClassName.c_str();
    if (!RegisterClassExA(&windowClass))
    {
        return false;
    }

    DWORD style = WS_VISIBLE | WS_CAPTION | WS_MINIMIZEBOX | WS_THICKFRAME | WS_MAXIMIZEBOX | WS_SYSMENU;
    DWORD extendedStyle = WS_EX_APPWINDOW;

    RECT sizeRect = { 0, 0, width, height };
    AdjustWindowRectEx(&sizeRect, style, false, extendedStyle);

    mNativeWindow = CreateWindowExA(extendedStyle, mClassName.c_str(), name.c_str(), style, CW_USEDEFAULT, CW_USEDEFAULT,
                                    sizeRect.right - sizeRect.left, sizeRect.bottom - sizeRect.top, NULL, NULL,
                                    GetModuleHandle(NULL), this);

    SetWindowLongPtrA(mNativeWindow, GWLP_USERDATA, reinterpret_cast<LONG>(this));

    ShowWindow(mNativeWindow, SW_SHOW);

    mNativeDisplay = GetDC(mNativeWindow);
    if (!mNativeDisplay)
    {
        destroy();
        return false;
    }

    mDisplay = eglGetDisplay(mNativeDisplay);
    if (mDisplay == EGL_NO_DISPLAY)
    {
        mDisplay = eglGetDisplay((EGLNativeDisplayType)EGL_DEFAULT_DISPLAY);
    }

    EGLint majorVersion, minorVersion;
    if (!eglInitialize(mDisplay, &majorVersion, &minorVersion))
    {
        destroy();
        return false;
    }

    eglBindAPI(EGL_OPENGL_ES_API);
    if (eglGetError() != EGL_SUCCESS)
    {
        destroy();
        return false;
    }

    return true;
}

void Win32Window::destroy()
{
    eglMakeCurrent(mDisplay, EGL_NO_SURFACE, EGL_NO_SURFACE, EGL_NO_CONTEXT);
    eglTerminate(mDisplay);

    if (mNativeDisplay)
    {
        ReleaseDC(mNativeWindow, mNativeDisplay);
        mNativeDisplay = 0;
    }

    if (mNativeWindow)
    {
        DestroyWindow(mNativeWindow);
        mNativeWindow = 0;
    }

    UnregisterClassA(mClassName.c_str(), NULL);
}

EGLDisplay Win32Window::getDisplay() const
{
    return mDisplay;
}

EGLNativeWindowType Win32Window::getNativeWindow() const
{
    return mNativeWindow;
}

EGLNativeDisplayType Win32Window::getNativeDisplay() const
{
    return mNativeDisplay;
}

void Win32Window::messageLoop()
{
    MSG msg;
    while (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE))
    {
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }
}

void Win32Window::setMousePosition(int x, int y)
{
    RECT winRect;
    GetClientRect(mNativeWindow, &winRect);

    POINT topLeft;
    topLeft.x = winRect.left;
    topLeft.y = winRect.top;
    ClientToScreen(mNativeWindow, &topLeft);

    SetCursorPos(topLeft.x + x, topLeft.y + y);
}
