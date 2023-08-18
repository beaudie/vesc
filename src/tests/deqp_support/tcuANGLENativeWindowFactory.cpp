//
// Created by yuxinhu on 8/21/23.
//

#include "tcuANGLENativeWindowFactory.h"
#include "tcuTexture.hpp"

namespace tcu
{

constexpr eglu::NativeWindow::Capability kAndroidWindowCapabilities =
    static_cast<eglu::NativeWindow::Capability>(
        eglu::NativeWindow::CAPABILITY_CREATE_SURFACE_LEGACY |
        eglu::NativeWindow::CAPABILITY_GET_SURFACE_SIZE |
        eglu::NativeWindow::CAPABILITY_GET_SCREEN_SIZE |
        eglu::NativeWindow::CAPABILITY_SET_SURFACE_SIZE |
        eglu::NativeWindow::CAPABILITY_CHANGE_VISIBILITY |
        eglu::NativeWindow::CAPABILITY_CREATE_SURFACE_PLATFORM_EXTENSION);

NativeWindowFactory::NativeWindowFactory(EventState *eventState,
                                         uint32_t preRotation,
                                         const eglu::NativeWindow::Capability windowCapability)
    : eglu::NativeWindowFactory("window", "ANGLE Window", windowCapability),
      mEvents(eventState),
      mPreRotation(preRotation)
{}

eglu::NativeWindow *NativeWindowFactory::createWindow(eglu::NativeDisplay *nativeDisplay,
                                                      const eglu::WindowParams &params) const
{
    DE_ASSERT(DE_FALSE);
    return nullptr;
}

eglu::NativeWindow *NativeWindowFactory::createWindow(eglu::NativeDisplay *nativeDisplay,
                                                      eglw::EGLDisplay display,
                                                      eglw::EGLConfig config,
                                                      const eglw::EGLAttrib *attribList,
                                                      const eglu::WindowParams &params) const
{
    //    return new NativeWindow(dynamic_cast<ANGLENativeDisplay *>(nativeDisplay), params,
    //                                   mEvents, mPreRotation);
    return NativeWindow::New(dynamic_cast<ANGLENativeDisplay *>(nativeDisplay), params, mEvents,
                             mPreRotation);
}

NativeWindow::NativeWindow(ANGLENativeDisplay *nativeDisplay,
                           const eglu::WindowParams &params,
                           EventState *eventState,
                           uint32_t preRotation,
                           const eglu::NativeWindow::Capability windowCapability)
    : eglu::NativeWindow(windowCapability),
      mWindow(OSWindow::New()),
      mEvents(eventState),
      mPreRotation(preRotation)
{
    int osWindowWidth =
        params.width == eglu::WindowParams::SIZE_DONT_CARE ? DEFAULT_SURFACE_WIDTH : params.width;
    int osWindowHeight = params.height == eglu::WindowParams::SIZE_DONT_CARE
                             ? DEFAULT_SURFACE_HEIGHT
                             : params.height;

    if (mPreRotation == 90 || mPreRotation == 270)
    {
        std::swap(osWindowWidth, osWindowHeight);
    }

    mWindow->setNativeDisplay(nativeDisplay->getDeviceContext());
    bool initialized = mWindow->initialize("dEQP ANGLE Tests", osWindowWidth, osWindowHeight);
    TCU_CHECK(initialized);

    if (params.visibility != eglu::WindowParams::VISIBILITY_DONT_CARE)
        NativeWindow::setVisibility(params.visibility);
}

void NativeWindow::setVisibility(eglu::WindowParams::Visibility visibility)
{
    switch (visibility)
    {
        case eglu::WindowParams::VISIBILITY_HIDDEN:
            mWindow->setVisible(false);
            break;

        case eglu::WindowParams::VISIBILITY_VISIBLE:
        case eglu::WindowParams::VISIBILITY_FULLSCREEN:
            mWindow->setVisible(true);
            break;

        default:
            DE_ASSERT(DE_FALSE);
    }
}

NativeWindow::~NativeWindow()
{
    OSWindow::Delete(&mWindow);
}

eglw::EGLNativeWindowType NativeWindow::getLegacyNative()
{
    return reinterpret_cast<eglw::EGLNativeWindowType>(mWindow->getNativeWindow());
}

void *NativeWindow::getPlatformExtension()
{
    return mWindow->getPlatformExtension();
}

IVec2 NativeWindow::getSurfaceSize() const
{
    int width  = mWindow->getWidth();
    int height = mWindow->getHeight();

    if (mPreRotation == 90 || mPreRotation == 270)
    {
        // Return the original dimensions dEQP asked for.  This ensures that the dEQP code is never
        // aware of the window actually being rotated.
        std::swap(width, height);
    }

    return IVec2(width, height);
}

void NativeWindow::processEvents()
{
    mWindow->messageLoop();

    // Look for a quit event to forward to the EventState
    Event event = {};
    while (mWindow->popEvent(&event))
    {
        if (event.Type == Event::EVENT_CLOSED)
        {
            mEvents->signalQuitEvent();
        }
    }
}

void NativeWindow::setSurfaceSize(IVec2 size)
{
    int osWindowWidth  = size.x();
    int osWindowHeight = size.y();

    if (mPreRotation == 90 || mPreRotation == 270)
    {
        std::swap(osWindowWidth, osWindowHeight);
    }

    mWindow->resize(osWindowWidth, osWindowHeight);
}

IVec2 NativeWindow::getScreenSize() const
{
    return getSurfaceSize();
}

void NativeWindow::readScreenPixels(tcu::TextureLevel *dst) const
{
    dst->setStorage(TextureFormat(TextureFormat::BGRA, TextureFormat::UNORM_INT8),
                    mWindow->getWidth(), mWindow->getHeight());
    if (!mWindow->takeScreenshot(reinterpret_cast<uint8_t *>(dst->getAccess().getDataPtr())))
    {
        throw InternalError("Failed to read screen pixels", DE_NULL, __FILE__, __LINE__);
    }

    if (mPreRotation != 0)
    {
        throw InternalError("Read screen pixels with prerotation is not supported", DE_NULL,
                            __FILE__, __LINE__);
    }
}
}  // namespace tcu
