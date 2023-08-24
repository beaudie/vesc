/*-------------------------------------------------------------------------
 * drawElements Quality Program Tester Core
 * ----------------------------------------
 *
 * Copyright 2023 The Android Open Source Project
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 *
 */
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
    return new NativeWindowWin32(nativeDisplay, params, eventState, preRotation,
                                 kWin32WindowCapabilities);
}
NativeWindowWin32::NativeWindowWin32(ANGLENativeDisplay *nativeDisplay,
                                     const eglu::WindowParams &params,
                                     EventState *eventState,
                                     uint32_t preRotation,
                                     const eglu::NativeWindow::Capability windowCapability)
    : NativeWindow(nativeDisplay, params, eventState, preRotation, windowCapability)
{}
}  // namespace tcu
