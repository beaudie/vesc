#include "tcuANGLENativeWindowFactoryAndroid.h"
#include "tcuTexture.hpp"

namespace tcu
{
enum
{
    DEFAULT_SURFACE_WIDTH  = 400,
    DEFAULT_SURFACE_HEIGHT = 300,
};

constexpr eglu::NativeWindow::Capability kAndroidWindowCapabilities =
    static_cast<eglu::NativeWindow::Capability>(
        eglu::NativeWindow::CAPABILITY_CREATE_SURFACE_LEGACY |
        eglu::NativeWindow::CAPABILITY_GET_SURFACE_SIZE |
        eglu::NativeWindow::CAPABILITY_GET_SCREEN_SIZE |
        eglu::NativeWindow::CAPABILITY_SET_SURFACE_SIZE |
        eglu::NativeWindow::CAPABILITY_CHANGE_VISIBILITY |
        eglu::NativeWindow::CAPABILITY_CREATE_SURFACE_PLATFORM_EXTENSION);

NativeWindowFactoryAndroid::NativeWindowFactoryAndroid(EventState *eventState, uint32_t preRotation)
    : eglu::NativeWindowFactory("window", "ANGLE Window", kAndroidWindowCapabilities),
      mEvents(eventState),
      mPreRotation(preRotation)
{}

eglu::NativeWindow *NativeWindowFactoryAndroid::createWindow(eglu::NativeDisplay *nativeDisplay,
                                                             const eglu::WindowParams &params) const
{
    DE_ASSERT(DE_FALSE);
    return nullptr;
}

eglu::NativeWindow *NativeWindowFactoryAndroid::createWindow(eglu::NativeDisplay *nativeDisplay,
                                                             eglw::EGLDisplay display,
                                                             eglw::EGLConfig config,
                                                             const eglw::EGLAttrib *attribList,
                                                             const eglu::WindowParams &params) const
{
    return new NativeWindowAndroid(dynamic_cast<ANGLENativeDisplay *>(nativeDisplay), params,
                                   mEvents, mPreRotation);
}

NativeWindowAndroid::NativeWindowAndroid(ANGLENativeDisplay *nativeDisplay,
                                         const eglu::WindowParams &params,
                                         EventState *eventState,
                                         uint32_t preRotation)
    : eglu::NativeWindow(kAndroidWindowCapabilities),
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

void NativeWindowAndroid::setVisibility(eglu::WindowParams::Visibility visibility)
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

NativeWindowAndroid::~NativeWindowAndroid()
{
    OSWindow::Delete(&mWindow);
}

eglw::EGLNativeWindowType NativeWindowAndroid::getLegacyNative()
{
    return reinterpret_cast<eglw::EGLNativeWindowType>(mWindow->getNativeWindow());
}

void *NativeWindowAndroid::getPlatformExtension()
{
    return mWindow->getPlatformExtension();
}

IVec2 NativeWindowAndroid::getSurfaceSize() const
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

void NativeWindowAndroid::processEvents()
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

void NativeWindowAndroid::setSurfaceSize(IVec2 size)
{
    int osWindowWidth  = size.x();
    int osWindowHeight = size.y();

    if (mPreRotation == 90 || mPreRotation == 270)
    {
        std::swap(osWindowWidth, osWindowHeight);
    }

    mWindow->resize(osWindowWidth, osWindowHeight);
}

IVec2 NativeWindowAndroid::getScreenSize() const
{
    return getSurfaceSize();
}

void NativeWindowAndroid::readScreenPixels(tcu::TextureLevel *dst) const
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