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

#include "tcuANGLENativeDisplay.h"
#include <EGL/egl.h>
#include <EGL/eglext.h>

// clang-format off
#if (DE_OS == DE_OS_WIN32)
    #define ANGLE_EGL_LIBRARY_FULL_NAME ANGLE_EGL_LIBRARY_NAME ".dll"
#elif (DE_OS == DE_OS_UNIX) || (DE_OS == DE_OS_ANDROID)
    #define ANGLE_EGL_LIBRARY_FULL_NAME ANGLE_EGL_LIBRARY_NAME ".so"
#elif (DE_OS == DE_OS_OSX)
    #define ANGLE_EGL_LIBRARY_FULL_NAME ANGLE_EGL_LIBRARY_NAME ".dylib"
#else
    #error "Unsupported platform"
#endif
// clang-format on

namespace tcu
{

constexpr eglu::NativeDisplay::Capability kDisplayCapabilities =
    static_cast<eglu::NativeDisplay::Capability>(
        eglu::NativeDisplay::CAPABILITY_GET_DISPLAY_PLATFORM |
        eglu::NativeDisplay::CAPABILITY_GET_DISPLAY_PLATFORM_EXT);

ANGLENativeDisplay::ANGLENativeDisplay(EGLNativeDisplayType display, std::vector<EGLAttrib> attribs)
    : eglu::NativeDisplay(kDisplayCapabilities, EGL_PLATFORM_ANGLE_ANGLE, "EGL_EXT_platform_base"),
      mDeviceContext(display),
      mLibrary(ANGLE_EGL_LIBRARY_FULL_NAME),
      mPlatformAttributes(std::move(attribs))
{}

void *ANGLENativeDisplay::getPlatformNative()
{
    // On OSX 64bits mDeviceContext is a 32 bit integer, so we can't simply
    // use reinterpret_cast<void*>.
    return bitCast<void *>(mDeviceContext);
}
}  // namespace tcu
