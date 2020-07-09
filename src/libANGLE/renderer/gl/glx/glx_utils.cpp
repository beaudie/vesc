//
// Copyright 2020 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//

// glx_utils.cpp: Utility routines specific to the G:X->EGL implementation.

#include "libANGLE/renderer/gl/glx/glx_utils.h"

#include "common/angleutils.h"

namespace rx
{

namespace x11
{

std::string XErrorToString(Display *display, int status)
{
    // Write nulls to the buffer so that if XGetErrorText fails, converting to an std::string will
    // be an empty string.
    char buffer[256] = {0};
    XGetErrorText(display, status, buffer, ArraySize(buffer));
    return std::string(buffer);
}

int GetPixmapDimensions(Display *display,
                        Pixmap pixmap,
                        unsigned int *outWidth,
                        unsigned int *outHeight,
                        unsigned int *outDepth)
{
    Window rootWindow;
    int x                    = 0;
    int y                    = 0;
    unsigned int width       = 0;
    unsigned int height      = 0;
    unsigned int borderWidth = 0;
    unsigned int depth       = 0;
    int status =
        XGetGeometry(display, pixmap, &rootWindow, &x, &y, &width, &height, &borderWidth, &depth);
    if (outWidth)
    {
        *outWidth = width;
    }
    if (outHeight)
    {
        *outHeight = height;
    }
    if (outDepth)
    {
        *outDepth = depth;
    }

    return status;
}

}  // namespace x11

}  // namespace rx
