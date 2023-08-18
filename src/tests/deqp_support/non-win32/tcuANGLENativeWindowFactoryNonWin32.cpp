#include "tcuANGLENativeWindowFactoryNonWin32.h"
#include "tcuTexture.hpp"

namespace tcu
{
enum
{
    DEFAULT_SURFACE_WIDTH  = 400,
    DEFAULT_SURFACE_HEIGHT = 300,
};

constexpr eglu::NativeWindow::Capability kNonWin32WindowCapabilities =
    static_cast<eglu::NativeWindow::Capability>(
        eglu::NativeWindow::CAPABILITY_CREATE_SURFACE_LEGACY |
        eglu::NativeWindow::CAPABILITY_GET_SURFACE_SIZE |
        eglu::NativeWindow::CAPABILITY_GET_SCREEN_SIZE |
        eglu::NativeWindow::CAPABILITY_SET_SURFACE_SIZE |
        eglu::NativeWindow::CAPABILITY_CHANGE_VISIBILITY |
        eglu::NativeWindow::CAPABILITY_CREATE_SURFACE_PLATFORM_EXTENSION);

NativeWindowFactory *NativeWindowFactory::New(EventState *eventState, uint32_t preRotation)
{
    return new NativeWindowFactoryNonWin32(eventState, preRotation, kNonWin32WindowCapabilities);
}

NativeWindowFactoryNonWin32::NativeWindowFactoryNonWin32(
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
    return new NativeWindowNonWin32(nativeDisplay, params, eventState, preRotation,
                                    kNonWin32WindowCapabilities);
}

NativeWindowNonWin32::NativeWindowNonWin32(ANGLENativeDisplay *nativeDisplay,
                                           const eglu::WindowParams &params,
                                           EventState *eventState,
                                           uint32_t preRotation,
                                           const eglu::NativeWindow::Capability windowCapability)
    : NativeWindow(nativeDisplay, params, eventState, preRotation, windowCapability)
{}
}  // namespace tcu