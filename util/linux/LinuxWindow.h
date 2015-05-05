//
// Copyright (c) 2015 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//

#ifndef SAMPLE_UTIL_LINUX_WINDOW_H
#define SAMPLE_UTIL_LINUX_WINDOW_H

#include <string>
#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include <X11/Xresource.h>

#include "OSWindow.h"

class LinuxWindow : public OSWindow
{
  public:
    LinuxWindow();
    ~LinuxWindow();

    bool initialize(const std::string &name, size_t width, size_t height) override;
    void destroy() override;

    EGLNativeWindowType getNativeWindow() const override;
    EGLNativeDisplayType getNativeDisplay() const override;

    void messageLoop() override;

    void pushEvent(Event event) override;

    void setMousePosition(int x, int y) override;
    bool setPosition(int x, int y) override;
    bool resize(int width, int height) override;
    void setVisible(bool isVisible) override;

    void signalTestEvent() override;

  private:
    Display *mDisplay;
    Window mWindow;
};

#endif // SAMPLE_UTIL_LINUX_WINDOW_H
