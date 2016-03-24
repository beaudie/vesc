//
// Copyright (c) 2016 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//

// CrosWindow.h: Definition of the implementation of OSWindow for Chrome OS

#ifndef UTIL_CROS_WINDOW_H
#define UTIL_CROS_WINDOW_H

#include <string>

#include "OSWindow.h"

class CrosWindow : public OSWindow
{
  public:
    CrosWindow();
    ~CrosWindow();

    bool initialize(const std::string &name, size_t width, size_t height) override;
    void destroy() override;

    EGLNativeWindowType getNativeWindow() const override;
    EGLNativeDisplayType getNativeDisplay() const override;

    void messageLoop() override;

    void setMousePosition(int x, int y) override;
    bool setPosition(int x, int y) override;
    bool resize(int width, int height) override;
    void setVisible(bool isVisible) override;

    void signalTestEvent() override;

  private:
    struct
    {
        int x;
        int y;
        int width;
        int height;
        int borderWidth;
        int borderHeight;
        int visible;
        int depth;
    } mNative;

    static int sLastDepth;
};

#endif  // UTIL_CROS_WINDOW_H
