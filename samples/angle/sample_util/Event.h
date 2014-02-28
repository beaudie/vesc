//
// Copyright (c) 2014 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//

#ifndef SAMPLE_UTIL_EVENT_H
#define SAMPLE_UTIL_EVENT_H

#include "keyboard.h"
#include "mouse.h"

class Event
{
  public:
    struct MoveEvent
    {
        int X;
        int Y;
    };

    struct SizeEvent
    {
        int Width;
        int Height;
    };

    struct KeyEvent
    {
        Key Code;
        bool Alt;
        bool Control;
        bool Shift;
        bool System;
    };

    struct MouseMoveEvent
    {
        int X;
        int Y;
    };

    struct MouseButtonEvent
    {
        MouseButton Button;
        int X;
        int Y;
    };

    struct MouseWheelEvent
    {
        int Delta;
    };

    enum EventType
    {
        EVENTTYPE_CLOSED,                 // The window requested to be closed
        EVENTTYPE_MOVED,                  // The window has moved
        EVENTTYPE_RESIZED,                // The window was resized
        EVENTTYPE_LOSTFOCUS,              // The window lost the focus
        EVENTTYPE_GAINEDFOCUS,            // The window gained the focus
        EVENTTYPE_TEXTENTERED,            // A character was entered
        EVENTTYPE_KEYPRESSED,             // A key was pressed
        EVENTTYPE_KEYRELEASED,            // A key was released
        EVENTTYPE_MOUSEWHEELMOVED,        // The mouse wheel was scrolled
        EVENTTYPE_MOUSEBUTTONPRESSED,     // A mouse button was pressed
        EVENTTYPE_MOUSEBUTTONRELEASED,    // A mouse button was released
        EVENTTYPE_MOUSEMOVED,             // The mouse cursor moved
        EVENTTYPE_MOUSEENTERED,           // The mouse cursor entered the area of the window
        EVENTTYPE_MOUSELEFT,              // The mouse cursor left the area of the window
    };

    EventType Type;

    union
    {
        MoveEvent               Move;               // Move event parameters
        SizeEvent               Size;               // Size event parameters
        KeyEvent                Key;                // Key event parameters
        MouseMoveEvent          MouseMove;          // Mouse move event parameters
        MouseButtonEvent        MouseButton;        // Mouse button event parameters
        MouseWheelEvent         MouseWheel;         // Mouse wheel event parameters
    };
};

#endif SAMPLE_UTIL_EVENT_H
