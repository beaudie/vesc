//
// Copyright 2019 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//
// ScenicWindow.cpp:
//    Implements methods from ScenicWindow
//

#include "util/fuchsia/ScenicWindow.h"

#include <fuchsia/images/cpp/fidl.h>
#include <fuchsia/ui/views/cpp/fidl.h>
#include <lib/async-loop/cpp/loop.h>
#include <lib/fdio/directory.h>
#include <lib/fidl/cpp/interface_ptr.h>
#include <lib/fidl/cpp/interface_request.h>
#include <lib/ui/scenic/cpp/view_token_pair.h>
#include <lib/zx/channel.h>
#include <zircon/status.h>

#include "common/debug.h"

namespace
{

async::Loop *GetDefaultLoop()
{
    static async::Loop *defaultLoop = new async::Loop(&kAsyncLoopConfigAttachToThread);
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
fidl::InterfacePtr<Interface> ConnectToService(zx_handle_t serviceRoot)
{
    fidl::InterfacePtr<Interface> result;
    ConnectToService(serviceRoot, result.NewRequest());
    return result;
}

}  // namespace

ScenicWindow::ScenicWindow()
    : mLoop(GetDefaultLoop()),
      mServiceRoot(ConnectToServiceRoot()),
      mScenic(ConnectToService<fuchsia::ui::scenic::Scenic>(mServiceRoot.get())),
      mPresenter(ConnectToService<fuchsia::ui::policy::Presenter>(mServiceRoot.get())),
      mScenicSession(mScenic.get()),
      mShape(&mScenicSession),
      mMaterial(&mScenicSession)
{}

ScenicWindow::~ScenicWindow()
{
    destroy();
}

bool ScenicWindow::initialize(const std::string &name, size_t width, size_t height)
{
    mScenicSession.set_error_handler(fit::bind_member(this, &ScenicWindow::OnScenicError));
    mScenicSession.set_event_handler(fit::bind_member(this, &ScenicWindow::OnScenicEvents));

    // Set up scenic resources.
    mShape.SetEventMask(fuchsia::ui::gfx::kMetricsEventMask);
    mShape.SetMaterial(mMaterial);

    fuchsia::ui::views::ViewToken viewToken;
    fuchsia::ui::views::ViewHolderToken viewHolderToken;
    std::tie(viewToken, viewHolderToken) = scenic::NewViewTokenPair();

    // Create view.
    mView = std::make_unique<scenic::View>(&mScenicSession, std::move(viewToken), name);
    mView->AddChild(mShape);
    mScenicSession.Present(0, [](fuchsia::images::PresentationInfo info) {});

    // Present view.
    mPresenter->PresentView(std::move(viewHolderToken), nullptr);

    mWidth  = width;
    mHeight = height;

    // Block until initial view dimensions are known.
    while (!mDisplayWidthDips || !mDisplayHeightDips || !mDisplayScaleX || !mDisplayScaleY)
    {
        mLoop->Run(zx::time::infinite(), true /* once */);
    }

    resetNativeWindow();

    return true;
}

void ScenicWindow::destroy()
{
    mFuchsiaEGLWindow.reset();
}

void ScenicWindow::resetNativeWindow()
{
    fuchsia::images::ImagePipePtr imagePipe;
    uint32_t imagePipeId = mScenicSession.AllocResourceId();
    mScenicSession.Enqueue(scenic::NewCreateImagePipeCmd(imagePipeId, imagePipe.NewRequest()));
    mMaterial.SetTexture(imagePipeId);
    mScenicSession.ReleaseResource(imagePipeId);
    mScenicSession.Present(0, [](fuchsia::images::PresentationInfo info) {});

    mFuchsiaEGLWindow.reset(
        fuchsia_egl_window_create(imagePipe.Unbind().TakeChannel().release(), mWidth, mHeight));
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
    mLoop->Run(zx::deadline_after({}), false /* once */);
}

void ScenicWindow::setMousePosition(int x, int y)
{
    UNIMPLEMENTED();
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

    UpdateViewSize();

    return true;
}

void ScenicWindow::setVisible(bool isVisible) {}

void ScenicWindow::signalTestEvent() {}

void ScenicWindow::OnScenicEvents(std::vector<fuchsia::ui::scenic::Event> events)
{
    for (const auto &event : events)
    {
        if (event.is_gfx())
        {
            if (event.gfx().is_metrics())
            {
                if (event.gfx().metrics().node_id != mShape.id())
                    continue;
                OnViewMetrics(event.gfx().metrics().metrics);
            }
            else if (event.gfx().is_view_properties_changed())
            {
                if (event.gfx().view_properties_changed().view_id != mView->id())
                    continue;
                OnViewProperties(event.gfx().view_properties_changed().properties);
            }
        }
    }
}

void ScenicWindow::OnViewMetrics(const fuchsia::ui::gfx::Metrics &metrics)
{
    mDisplayScaleX = metrics.scale_x;
    mDisplayScaleY = metrics.scale_y;

    UpdateViewSize();
}

void ScenicWindow::OnViewProperties(const fuchsia::ui::gfx::ViewProperties &properties)
{
    float width = properties.bounding_box.max.x - properties.bounding_box.min.x -
                  properties.inset_from_min.x - properties.inset_from_max.x;
    float height = properties.bounding_box.max.y - properties.bounding_box.min.y -
                   properties.inset_from_min.y - properties.inset_from_max.y;

    mDisplayWidthDips  = width;
    mDisplayHeightDips = height;

    UpdateViewSize();
}

void ScenicWindow::UpdateViewSize()
{
    if (!mDisplayWidthDips || !mDisplayHeightDips || !mDisplayScaleX || !mDisplayScaleY)
    {
        return;
    }

    // Surface size in pixels is
    //   (mWidth, mHeight)
    //
    // View size in pixels is
    //   (mDisplayWidthDips * mDisplayScaleX) x (mDisplayHeightDips * mDisplayScaleY)

    mShape.SetShape(
        scenic::Rectangle(&mScenicSession, mWidth / mDisplayScaleX, mHeight / mDisplayScaleY));
    mShape.SetTranslation(0.5f * mWidth / mDisplayScaleX, 0.5f * mHeight / mDisplayScaleY, 0.f);
    mScenicSession.Present(0, [](fuchsia::images::PresentationInfo info) {});
}

void ScenicWindow::OnScenicError(zx_status_t status)
{
    WARN() << "OnScenicError: " << zx_status_get_string(status);
}

// static
OSWindow *OSWindow::New()
{
    return new ScenicWindow;
}
