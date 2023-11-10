//
// Copyright 2019 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//
// ScenicWindow.cpp:
//    Implements methods from ScenicWindow
//

#include "util/fuchsia/ScenicWindow.h"

#include <fuchsia/element/cpp/fidl.h>
#include <lib/async-loop/cpp/loop.h>
#include <lib/async-loop/default.h>
#include <lib/fidl/cpp/interface_ptr.h>
#include <lib/fidl/cpp/interface_request.h>
#include <lib/zx/channel.h>
#include <zircon/status.h>

namespace
{

async::Loop *GetDefaultLoop()
{
    static async::Loop *defaultLoop = new async::Loop(&kAsyncLoopConfigNeverAttachToThread);
    return defaultLoop;
}

zx::channel ConnectToServiceRoot()
{
    zx::channel clientChannel;
    zx::channel serverChannel;
    zx_status_t result = zx::channel::create(0, &clientChannel, &serverChannel);
    ASSERT(result == ZX_OK);
    result = fdio_service_connect("/svc/.", serverChannel.release());
    ASSERT(result == ZX_OK);
    return clientChannel;
}

template <typename Interface>
zx_status_t ConnectToService(zx_handle_t serviceRoot, fidl::InterfaceRequest<Interface> request)
{
    ASSERT(request.is_valid());
    return fdio_service_connect_at(serviceRoot, Interface::Name_, request.TakeChannel().release());
}

template <typename Interface>
fidl::InterfacePtr<Interface> ConnectToService(zx_handle_t serviceRoot,
                                               async_dispatcher_t *dispatcher)
{
    fidl::InterfacePtr<Interface> result;
    ConnectToService(serviceRoot, result.NewRequest(dispatcher));
    return result;
}

}  // namespace

ScenicWindow::ScenicWindow()
    : mLoop(GetDefaultLoop()),
      mServiceRoot(ConnectToServiceRoot()),
      mPresenter(ConnectToService<fuchsia::element::GraphicalPresenter>(mServiceRoot.get(),
                                                                        mLoop->dispatcher())),
{
    mScenicSession.set_error_handler(fit::bind_member(this, &ScenicWindow::onScenicError));
    mScenicSession.set_event_handler(fit::bind_member(this, &ScenicWindow::onScenicEvents));
    mScenicSession.set_on_frame_presented_handler(
        fit::bind_member(this, &ScenicWindow::onFramePresented));
}

ScenicWindow::~ScenicWindow()
{
    destroy();
}

bool ScenicWindow::initializeImpl(const std::string &name, int width, int height)
{
    return true;
}

void ScenicWindow::disableErrorMessageDialog() {}

void ScenicWindow::destroy()
{
    mFuchsiaEGLWindow.reset();
}

void ScenicWindow::resetNativeWindow()
{
    mFuchsiaEGLWindow.reset(fuchsia_egl_window_create(imagePipeHandle, mWidth, mHeight));
}

EGLNativeWindowType ScenicWindow::getNativeWindow() const
{
    return reinterpret_cast<EGLNativeWindowType>(mFuchsiaEGLWindow.get());
}

EGLNativeDisplayType ScenicWindow::getNativeDisplay() const
{
    return EGL_DEFAULT_DISPLAY;
}

void ScenicWindow::messageLoop()
{
    mLoop->ResetQuit();
    mLoop->RunUntilIdle();
}

void ScenicWindow::setMousePosition(int x, int y)
{
    UNIMPLEMENTED();
}

bool ScenicWindow::setOrientation(int width, int height)
{
    UNIMPLEMENTED();
    return false;
}

bool ScenicWindow::setPosition(int x, int y)
{
    UNIMPLEMENTED();
    return false;
}

bool ScenicWindow::resize(int width, int height)
{
    mWidth  = width;
    mHeight = height;

    fuchsia_egl_window_resize(mFuchsiaEGLWindow.get(), width, height);

    mViewSizeDirty = true;

    updateViewSize();

    return true;
}

void ScenicWindow::setVisible(bool isVisible) {}

void ScenicWindow::signalTestEvent() {}

void ScenicWindow::present()
{
    UNIMPLEMENTED();
}

void ScenicWindow::updateViewSize()
{
    if (!mViewSizeDirty || !mHasViewMetrics || !mHasViewProperties)
    {
        return;
    }

    mViewSizeDirty = false;

    // Surface size in pixels is
    //   (mWidth, mHeight)
    //
    // View size in pixels is
    //   (mDisplayWidthDips * mDisplayScaleX) x (mDisplayHeightDips * mDisplayScaleY)

    float widthDips  = mWidth / mDisplayScaleX;
    float heightDips = mHeight / mDisplayScaleY;

    present();
}

// static
OSWindow *OSWindow::New()
{
    return new ScenicWindow;
}
