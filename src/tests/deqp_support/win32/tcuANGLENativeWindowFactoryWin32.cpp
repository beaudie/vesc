//
// Created by yuxinhu on 8/21/23.
//

#include "tcuANGLENativeWindowFactoryWin32.h"

namespace tcu
{

constexpr eglu::NativeWindow::Capability kWin32WindowCapabilities =
    static_cast<eglu::NativeWindow::Capability>(
        eglu::NativeWindow::CAPABILITY_CREATE_SURFACE_LEGACY |
        eglu::NativeWindow::CAPABILITY_GET_SURFACE_SIZE |
        eglu::NativeWindow::CAPABILITY_GET_SCREEN_SIZE |
        eglu::NativeWindow::CAPABILITY_SET_SURFACE_SIZE |
        eglu::NativeWindow::CAPABILITY_CHANGE_VISIBILITY |
        eglu::NativeWindow::CAPABILITY_CREATE_SURFACE_PLATFORM_EXTENSION |
        eglu::NativeWindow::CAPABILITY_READ_SCREEN_PIXELS);

NativeWindowFactory *NativeWindowFactory::New(EventState *eventState, uint32_t preRotation)
{
    return new NativeWindowFactoryWin32(eventState, preRotation, kWin32WindowCapabilities);
}

NativeWindowFactoryWin32::NativeWindowFactoryWin32(
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
    return new NativeWindowFactoryNonWin32(nativeDisplay, params, eventState, preRotation,
                                           kWin32WindowCapabilities);
}

NativeWindowFactoryWin32::NativeWindowFactoryWin32(
    ANGLENativeDisplay *nativeDisplay,
    const eglu::WindowParams &params,
    EventState *eventState,
    uint32_t preRotation,
    const eglu::NativeWindow::Capability windowCapability)
    : NativeWindow(nativeDisplay, params, eventState, preRotation, windowCapability)
{}
}  // namespace tcu
