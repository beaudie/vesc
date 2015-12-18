//
// Copyright (c) 2015 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//

// DRMWindow.h: Definition of the implementation of OSWindow for DRM

#ifndef UTIL_DRM_WINDOW_H
#define UTIL_DRM_WINDOW_H

#include <string>

#include "OSWindow.h"

class DRMWindow : public OSWindow
{
  public:
    DRMWindow();
    ~DRMWindow();

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
    /*XXX
    void processEvent(const XEvent &event);

    Atom WM_DELETE_WINDOW;
    Atom WM_PROTOCOLS;
    Atom TEST_EVENT;

    Display *mDisplay;
    Window mWindow;
    */
    int size[2];
};

#endif // UTIL_DRM_WINDOW_H
