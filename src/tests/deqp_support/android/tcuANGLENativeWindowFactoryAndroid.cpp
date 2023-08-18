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

NativeWindowFactory *NativeWindowFactory::New(EventState *eventState, uint32_t preRotation)
{
    return new NativeWindowFactoryAndroid(eventState, preRotation, kAndroidWindowCapabilities);
}

NativeWindowFactoryAndroid::NativeWindowFactoryAndroid(
    EventState *eventState,
    uint32_t preRotation,
    const eglu::NativeWindow::Capability windowCapability)
    : NativeWindowFactory(eventState, preRotation, windowCapability)
{}

NativeWindow *NativeWindow::New(ANGLENativeDisplay *nativeDisplay,
                                const eglu::WindowParams &params,
                                EventState *eventState,
                                uint32_t preRotation)
{
    return new NativeWindowAndroid(nativeDisplay, params, eventState, preRotation,
                                   kAndroidWindowCapabilities);
}

NativeWindowAndroid::NativeWindowAndroid(ANGLENativeDisplay *nativeDisplay,
                                         const eglu::WindowParams &params,
                                         EventState *eventState,
                                         uint32_t preRotation,
                                         const eglu::NativeWindow::Capability windowCapability)
    : NativeWindow(nativeDisplay, params, eventState, preRotation, windowCapability)
{}
}  // namespace tcu