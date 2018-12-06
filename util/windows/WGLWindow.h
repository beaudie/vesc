//
// Copyright 2018 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//
// WGLWindow:
//   Implements initializing a WGL rendering context.
//

#ifndef UTIL_WGLWINDOW_H_
#define UTIL_WGLWINDOW_H_

#include "common/angleutils.h"
#include "export.h"

class OSWindow;

namespace angle
{
class Library;
}  // namespace angle

class ANGLE_EXPORT WGLWindow : angle::NonCopyable
{
  public:
    WGLWindow(int glesMajorVersion, int glesMinorVersion);
    ~WGLWindow();

    // Internally initializes GL resources.
    bool initializeGL(OSWindow *osWindow, angle::Library *wglLibrary);
    void destroyGL();
    bool isGLInitialized() const;

    void makeCurrent();

    // Set config propeties.
    // It should also be possible to set multisample and floating point framebuffers.
    void setConfigRedBits(int bits) { mRedBits = bits; }
    void setConfigGreenBits(int bits) { mGreenBits = bits; }
    void setConfigBlueBits(int bits) { mBlueBits = bits; }
    void setConfigAlphaBits(int bits) { mAlphaBits = bits; }
    void setConfigDepthBits(int bits) { mDepthBits = bits; }
    void setConfigStencilBits(int bits) { mStencilBits = bits; }

  private:
    int mRedBits;
    int mGreenBits;
    int mBlueBits;
    int mAlphaBits;
    int mDepthBits;
    int mStencilBits;
};

#endif  // UTIL_WGLWINDOW_H_
