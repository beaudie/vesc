//
// Copyright 2020 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//

#ifndef CAPTURE_REPLAY_TEST_H
#define CAPTURE_REPLAY_TEST_H

#include <stdint.h>
#include <list>
#include <memory>
#include <string>

#include "common/system_utils.h"
#include "util/EGLPlatformParameters.h"
#include "util/OSWindow.h"
#include "util/egl_loader_autogen.h"

class EGLWindow;

namespace angle
{
class Library;
}  // namespace angle

// Based on CaptureReplaySample
class CaptureReplayTest
{
  public:
    CaptureReplayTest(int width, int height, EGLint glesMajorVersion, EGLint glesMinorVersion);
    ~CaptureReplayTest();
    bool initialize();
    void draw();
    void swap();
    int run();

  private:
    uint32_t mWidth;
    uint32_t mHeight;
    OSWindow *mOSWindow;
    EGLWindow *mEGLWindow;

    EGLPlatformParameters mPlatformParams;

    // Handle to the entry point binding library.
    std::unique_ptr<angle::Library> mEntryPointsLib;
};

#endif  // CAPTURE_REPLAY_TEST_H
