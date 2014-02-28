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
        EventType_Closed,                 // The window requested to be closed
        EventType_Moved,                  // The window has moved
        EventType_Resized,                // The window was resized
        EventType_LostFocus,              // The window lost the focus
        EventType_GainedFocus,            // The window gained the focus
        EventType_TextEntered,            // A character was entered
        EventType_KeyPressed,             // A key was pressed
        EventType_KeyReleased,            // A key was released
        EventType_MouseWheelMoved,        // The mouse wheel was scrolled
        EventType_MouseButtonPressed,     // A mouse button was pressed
        EventType_MouseButtonReleased,    // A mouse button was released
        EventType_MouseMoved,             // The mouse cursor moved
        EventType_MouseEntered,           // The mouse cursor entered the area of the window
        EventType_MouseLeft,              // The mouse cursor left the area of the window
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
