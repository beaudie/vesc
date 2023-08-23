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
}  // namespace tcu