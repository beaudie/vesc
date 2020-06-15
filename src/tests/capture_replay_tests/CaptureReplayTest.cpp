//
// Copyright 2013 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//

#include "CaptureReplayTest.h"

#include "util/EGLWindow.h"
#include "util/gles_loader_autogen.h"

#include <string.h>
#include <functional>
#include <iostream>
#include <utility>
#include "util/frame_capture_utils.h"

#define ANGLE_MACRO_STRINGIZE_AUX(a) #a
#define ANGLE_MACRO_STRINGIZE(a) ANGLE_MACRO_STRINGIZE_AUX(a)
#define ANGLE_MACRO_CONCAT_AUX(a, b) a##b
#define ANGLE_MACRO_CONCAT(a, b) ANGLE_MACRO_CONCAT_AUX(a, b)

// Build the right context header based on replay ID
// This will expand to "angle_capture_context<#>.h"
#include ANGLE_MACRO_STRINGIZE(ANGLE_CAPTURE_REPLAY_TEST_HEADER)

// Assign the context numbered functions based on GN arg selecting replay ID
std::function<void()> SetupContextReplay = reinterpret_cast<void (*)()>(
    ANGLE_MACRO_CONCAT(SetupContext,
                       ANGLE_MACRO_CONCAT(ANGLE_CAPTURE_REPLAY_TEST_CONTEXT_ID, Replay)));
std::function<void(int)> ReplayContextFrame = reinterpret_cast<void (*)(int)>(
    ANGLE_MACRO_CONCAT(ReplayContext,
                       ANGLE_MACRO_CONCAT(ANGLE_CAPTURE_REPLAY_TEST_CONTEXT_ID, Frame)));
std::function<void()> ResetContextReplay = reinterpret_cast<void (*)()>(
    ANGLE_MACRO_CONCAT(ResetContext,
                       ANGLE_MACRO_CONCAT(ANGLE_CAPTURE_REPLAY_TEST_CONTEXT_ID, Replay)));

CaptureReplayTest::CaptureReplayTest(int width,
                                     int height,
                                     EGLint glesMajorVersion,
                                     EGLint glesMinorVersion)
    : mWidth(width), mHeight(height), mOSWindow(nullptr), mEGLWindow(nullptr)

{
    mPlatformParams.renderer   = EGL_PLATFORM_ANGLE_TYPE_DEFAULT_ANGLE;
    mPlatformParams.deviceType = EGL_PLATFORM_ANGLE_DEVICE_TYPE_HARDWARE_ANGLE;

    // Load EGL library so we can initialize the display.
    mEntryPointsLib.reset(
        angle::OpenSharedLibrary(ANGLE_EGL_LIBRARY_NAME, angle::SearchType::ApplicationDir));

    mEGLWindow = EGLWindow::New(glesMajorVersion, glesMinorVersion);
    mOSWindow  = OSWindow::New();
}

CaptureReplayTest::~CaptureReplayTest()
{
    EGLWindow::Delete(&mEGLWindow);
    OSWindow::Delete(&mOSWindow);
}

bool CaptureReplayTest::initialize()
{
    // Set CWD to executable directory.
    std::string exeDir = angle::GetExecutableDirectory();
    if (!angle::SetCWD(exeDir.c_str()))
        return false;
    if (kIsBinaryDataCompressed)
    {
        SetBinaryDataDecompressCallback(angle::DecompressBinaryData);
    }
    SetBinaryDataDir(ANGLE_CAPTURE_REPLAY_TEST_DATA_DIR);
    SetupContextReplay();

    eglSwapInterval(mEGLWindow->getDisplay(), 1);
    return true;
}

void CaptureReplayTest::draw()
{
    ReplayContextFrame(0);
}

void CaptureReplayTest::swap()
{
    mEGLWindow->swap();
}

int CaptureReplayTest::run()
{
    if (!mOSWindow->initialize("Capture Replay Test", mWidth, mHeight))
    {
        return -1;
    }

    mOSWindow->setVisible(true);

    ConfigParameters configParams;
    configParams.redBits     = 8;
    configParams.greenBits   = 8;
    configParams.blueBits    = 8;
    configParams.alphaBits   = 8;
    configParams.depthBits   = 24;
    configParams.stencilBits = 8;

    if (!mEGLWindow->initializeGL(mOSWindow, mEntryPointsLib.get(), angle::GLESDriverType::AngleEGL,
                                  mPlatformParams, configParams))
    {
        return -1;
    }

    // Disable vsync
    if (!mEGLWindow->setSwapInterval(0))
    {
        return -1;
    }

    angle::LoadGLES(eglGetProcAddress);

    int result = 0;

    if (!initialize())
    {
        result = -1;
    }

    draw();
    swap();

    mEGLWindow->destroyGL();
    mOSWindow->destroy();

    return result;
}

int main(int argc, char **argv)
{
    CaptureReplayTest app(128, 128, 2, 0);
    return app.run();
}
