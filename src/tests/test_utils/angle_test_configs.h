//
// Copyright (c) 2014 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//

#ifndef ANGLE_TEST_CONFIGS_H_
#define ANGLE_TEST_CONFIGS_H_

// On Linux EGL/egl.h includes X.h which does defines for some very common
// names that are used by gtest (like None and Bool) and causes a lot of
// compilation errors. To work around this, even if this file doesn't use it,
// we include gtest before EGL so that it compiles fine in other files that
// want to use gtest.
#include <gtest/gtest.h>

#include <EGL/egl.h>
#include <EGL/eglext.h>

#include "GLSLANG/ShaderLang.h"
#include "angle_test_instantiate.h"
#include "util/EGLPlatformParameters.h"

namespace angle
{

// The GLES driver type determines what shared object we use to load the GLES entry points.
// AngleEGL loads from ANGLE's version of libEGL, libGLESv2, and libGLESv1_CM.
// SystemEGL uses the system copies of libEGL, libGLESv2, and libGLESv1_CM.
// SystemWGL loads Windows GL with the GLES compatiblity extensions. See util/WGLWindow.h.
enum class GLESDriverType
{
    AngleEGL,
    SystemEGL,
    SystemWGL,
};

struct PlatformParameters
{
    constexpr PlatformParameters();
    constexpr PlatformParameters(EGLint majorVersion,
                       EGLint minorVersion,
                       const EGLPlatformParameters &eglPlatformParameters);
    constexpr PlatformParameters(EGLint majorVersion, EGLint minorVersion, GLESDriverType driver);

    constexpr EGLint getRenderer() const;

    EGLint majorVersion;
    EGLint minorVersion;
    EGLPlatformParameters eglParameters;
    GLESDriverType driver;
};

constexpr PlatformParameters::PlatformParameters() : PlatformParameters(2, 0, EGLPlatformParameters()) {}

constexpr PlatformParameters::PlatformParameters(EGLint majorVersion,
    EGLint minorVersion,
    const EGLPlatformParameters &eglPlatformParameters)
    : majorVersion(majorVersion),
    minorVersion(minorVersion),
    eglParameters(eglPlatformParameters),
    driver(GLESDriverType::AngleEGL)
{}

constexpr PlatformParameters::PlatformParameters(EGLint majorVersion,
    EGLint minorVersion,
    GLESDriverType driver)
    : majorVersion(majorVersion), minorVersion(minorVersion), driver(driver)
{}

constexpr EGLint PlatformParameters::getRenderer() const
{
    return eglParameters.renderer;
}

bool operator<(const PlatformParameters &a, const PlatformParameters &b);
bool operator==(const PlatformParameters &a, const PlatformParameters &b);
std::ostream &operator<<(std::ostream &stream, const PlatformParameters &pp);

// EGL platforms
namespace egl_platform
{
constexpr EGLPlatformParameters DEFAULT()
{
    return EGLPlatformParameters(EGL_PLATFORM_ANGLE_TYPE_DEFAULT_ANGLE);
}

constexpr EGLPlatformParameters DEFAULT_NULL()
{
    return EGLPlatformParameters(EGL_PLATFORM_ANGLE_TYPE_DEFAULT_ANGLE, EGL_DONT_CARE,
        EGL_DONT_CARE, EGL_PLATFORM_ANGLE_DEVICE_TYPE_NULL_ANGLE);
}

constexpr EGLPlatformParameters D3D9()
{
    return EGLPlatformParameters(EGL_PLATFORM_ANGLE_TYPE_D3D9_ANGLE, EGL_DONT_CARE, EGL_DONT_CARE,
        EGL_PLATFORM_ANGLE_DEVICE_TYPE_HARDWARE_ANGLE);
}

constexpr EGLPlatformParameters D3D9_NULL()
{
    return EGLPlatformParameters(EGL_PLATFORM_ANGLE_TYPE_D3D9_ANGLE, EGL_DONT_CARE, EGL_DONT_CARE,
        EGL_PLATFORM_ANGLE_DEVICE_TYPE_NULL_ANGLE);
}

constexpr EGLPlatformParameters D3D9_REFERENCE()
{
    return EGLPlatformParameters(EGL_PLATFORM_ANGLE_TYPE_D3D9_ANGLE, EGL_DONT_CARE, EGL_DONT_CARE,
        EGL_PLATFORM_ANGLE_DEVICE_TYPE_D3D_REFERENCE_ANGLE);
}

constexpr EGLPlatformParameters D3D11()
{
    return EGLPlatformParameters(EGL_PLATFORM_ANGLE_TYPE_D3D11_ANGLE, EGL_DONT_CARE, EGL_DONT_CARE,
        EGL_PLATFORM_ANGLE_DEVICE_TYPE_HARDWARE_ANGLE);
}

constexpr EGLPlatformParameters D3D11(EGLenum presentPath)
{
    return EGLPlatformParameters(EGL_PLATFORM_ANGLE_TYPE_D3D11_ANGLE, EGL_DONT_CARE, EGL_DONT_CARE,
        EGL_PLATFORM_ANGLE_DEVICE_TYPE_HARDWARE_ANGLE, presentPath);
}

constexpr EGLPlatformParameters D3D11_FL11_1()
{
    return EGLPlatformParameters(EGL_PLATFORM_ANGLE_TYPE_D3D11_ANGLE, 11, 1,
        EGL_PLATFORM_ANGLE_DEVICE_TYPE_HARDWARE_ANGLE);
}

constexpr EGLPlatformParameters D3D11_FL11_0()
{
    return EGLPlatformParameters(EGL_PLATFORM_ANGLE_TYPE_D3D11_ANGLE, 11, 0,
        EGL_PLATFORM_ANGLE_DEVICE_TYPE_HARDWARE_ANGLE);
}

constexpr EGLPlatformParameters D3D11_FL10_1()
{
    return EGLPlatformParameters(EGL_PLATFORM_ANGLE_TYPE_D3D11_ANGLE, 10, 1,
        EGL_PLATFORM_ANGLE_DEVICE_TYPE_HARDWARE_ANGLE);
}

constexpr EGLPlatformParameters D3D11_FL10_0()
{
    return EGLPlatformParameters(EGL_PLATFORM_ANGLE_TYPE_D3D11_ANGLE, 10, 0,
        EGL_PLATFORM_ANGLE_DEVICE_TYPE_HARDWARE_ANGLE);
}

constexpr EGLPlatformParameters D3D11_FL9_3()
{
    return EGLPlatformParameters(EGL_PLATFORM_ANGLE_TYPE_D3D11_ANGLE, 9, 3,
        EGL_PLATFORM_ANGLE_DEVICE_TYPE_HARDWARE_ANGLE);
}

constexpr EGLPlatformParameters D3D11_NULL()
{
    return EGLPlatformParameters(EGL_PLATFORM_ANGLE_TYPE_D3D11_ANGLE, EGL_DONT_CARE, EGL_DONT_CARE,
        EGL_PLATFORM_ANGLE_DEVICE_TYPE_NULL_ANGLE);
}

constexpr EGLPlatformParameters D3D11_WARP()
{
    return EGLPlatformParameters(EGL_PLATFORM_ANGLE_TYPE_D3D11_ANGLE, EGL_DONT_CARE, EGL_DONT_CARE,
        EGL_PLATFORM_ANGLE_DEVICE_TYPE_D3D_WARP_ANGLE);
}

constexpr EGLPlatformParameters D3D11_FL11_1_WARP()
{
    return EGLPlatformParameters(EGL_PLATFORM_ANGLE_TYPE_D3D11_ANGLE, 11, 1,
        EGL_PLATFORM_ANGLE_DEVICE_TYPE_D3D_WARP_ANGLE);
}

constexpr EGLPlatformParameters D3D11_FL11_0_WARP()
{
    return EGLPlatformParameters(EGL_PLATFORM_ANGLE_TYPE_D3D11_ANGLE, 11, 0,
        EGL_PLATFORM_ANGLE_DEVICE_TYPE_D3D_WARP_ANGLE);
}

constexpr EGLPlatformParameters D3D11_FL10_1_WARP()
{
    return EGLPlatformParameters(EGL_PLATFORM_ANGLE_TYPE_D3D11_ANGLE, 10, 1,
        EGL_PLATFORM_ANGLE_DEVICE_TYPE_D3D_WARP_ANGLE);
}

constexpr EGLPlatformParameters D3D11_FL10_0_WARP()
{
    return EGLPlatformParameters(EGL_PLATFORM_ANGLE_TYPE_D3D11_ANGLE, 10, 0,
        EGL_PLATFORM_ANGLE_DEVICE_TYPE_D3D_WARP_ANGLE);
}

constexpr EGLPlatformParameters D3D11_FL9_3_WARP()
{
    return EGLPlatformParameters(EGL_PLATFORM_ANGLE_TYPE_D3D11_ANGLE, 9, 3,
        EGL_PLATFORM_ANGLE_DEVICE_TYPE_D3D_WARP_ANGLE);
}

constexpr EGLPlatformParameters D3D11_REFERENCE()
{
    return EGLPlatformParameters(EGL_PLATFORM_ANGLE_TYPE_D3D11_ANGLE, EGL_DONT_CARE, EGL_DONT_CARE,
        EGL_PLATFORM_ANGLE_DEVICE_TYPE_D3D_REFERENCE_ANGLE);
}

constexpr EGLPlatformParameters D3D11_FL11_1_REFERENCE()
{
    return EGLPlatformParameters(EGL_PLATFORM_ANGLE_TYPE_D3D11_ANGLE, 11, 1,
        EGL_PLATFORM_ANGLE_DEVICE_TYPE_D3D_REFERENCE_ANGLE);
}

constexpr EGLPlatformParameters D3D11_FL11_0_REFERENCE()
{
    return EGLPlatformParameters(EGL_PLATFORM_ANGLE_TYPE_D3D11_ANGLE, 11, 0,
        EGL_PLATFORM_ANGLE_DEVICE_TYPE_D3D_REFERENCE_ANGLE);
}

constexpr EGLPlatformParameters D3D11_FL10_1_REFERENCE()
{
    return EGLPlatformParameters(EGL_PLATFORM_ANGLE_TYPE_D3D11_ANGLE, 10, 1,
        EGL_PLATFORM_ANGLE_DEVICE_TYPE_D3D_REFERENCE_ANGLE);
}

constexpr EGLPlatformParameters D3D11_FL10_0_REFERENCE()
{
    return EGLPlatformParameters(EGL_PLATFORM_ANGLE_TYPE_D3D11_ANGLE, 10, 0,
        EGL_PLATFORM_ANGLE_DEVICE_TYPE_D3D_REFERENCE_ANGLE);
}

constexpr EGLPlatformParameters D3D11_FL9_3_REFERENCE()
{
    return EGLPlatformParameters(EGL_PLATFORM_ANGLE_TYPE_D3D11_ANGLE, 9, 3,
        EGL_PLATFORM_ANGLE_DEVICE_TYPE_D3D_REFERENCE_ANGLE);
}

constexpr EGLPlatformParameters OPENGL()
{
    return EGLPlatformParameters(EGL_PLATFORM_ANGLE_TYPE_OPENGL_ANGLE);
}

constexpr EGLPlatformParameters OPENGL(EGLint major, EGLint minor)
{
    return EGLPlatformParameters(EGL_PLATFORM_ANGLE_TYPE_OPENGL_ANGLE, major, minor, EGL_DONT_CARE);
}

constexpr EGLPlatformParameters OPENGL_NULL()
{
    return EGLPlatformParameters(EGL_PLATFORM_ANGLE_TYPE_OPENGL_ANGLE, EGL_DONT_CARE, EGL_DONT_CARE,
        EGL_PLATFORM_ANGLE_DEVICE_TYPE_NULL_ANGLE);
}

constexpr EGLPlatformParameters OPENGLES()
{
    return EGLPlatformParameters(EGL_PLATFORM_ANGLE_TYPE_OPENGLES_ANGLE);
}

constexpr EGLPlatformParameters OPENGLES(EGLint major, EGLint minor)
{
    return EGLPlatformParameters(EGL_PLATFORM_ANGLE_TYPE_OPENGLES_ANGLE, major, minor,
        EGL_DONT_CARE);
}

constexpr EGLPlatformParameters OPENGLES_NULL()
{
    return EGLPlatformParameters(EGL_PLATFORM_ANGLE_TYPE_OPENGLES_ANGLE, EGL_DONT_CARE,
        EGL_DONT_CARE, EGL_PLATFORM_ANGLE_DEVICE_TYPE_NULL_ANGLE);
}

constexpr EGLPlatformParameters OPENGL_OR_GLES(bool useNullDevice)
{
#if defined(ANGLE_PLATFORM_ANDROID)
    return useNullDevice ? OPENGLES_NULL() : OPENGLES();
#else
    return useNullDevice ? OPENGL_NULL() : OPENGL();
#endif
}

constexpr EGLPlatformParameters VULKAN()
{
    return EGLPlatformParameters(EGL_PLATFORM_ANGLE_TYPE_VULKAN_ANGLE);
}

constexpr EGLPlatformParameters VULKAN_NULL()
{
    return EGLPlatformParameters(EGL_PLATFORM_ANGLE_TYPE_VULKAN_ANGLE, EGL_DONT_CARE, EGL_DONT_CARE,
        EGL_PLATFORM_ANGLE_DEVICE_TYPE_NULL_ANGLE);
}
}  // namespace egl_platform

   // ANGLE tests platforms
constexpr PlatformParameters ES1_D3D9()
{
    return PlatformParameters(1, 0, egl_platform::D3D9());
}

constexpr PlatformParameters ES2_D3D9()
{
    return PlatformParameters(2, 0, egl_platform::D3D9());
}

constexpr PlatformParameters ES1_D3D11()
{
    return PlatformParameters(1, 0, egl_platform::D3D11());
}

constexpr PlatformParameters ES2_D3D11()
{
    return PlatformParameters(2, 0, egl_platform::D3D11());
}

constexpr PlatformParameters ES2_D3D11(EGLenum presentPath)
{
    return PlatformParameters(2, 0, egl_platform::D3D11(presentPath));
}

constexpr PlatformParameters ES2_D3D11_FL11_0()
{
    return PlatformParameters(2, 0, egl_platform::D3D11_FL11_0());
}

constexpr PlatformParameters ES2_D3D11_FL10_1()
{
    return PlatformParameters(2, 0, egl_platform::D3D11_FL10_1());
}

constexpr PlatformParameters ES2_D3D11_FL10_0()
{
    return PlatformParameters(2, 0, egl_platform::D3D11_FL10_0());
}

constexpr PlatformParameters ES2_D3D11_FL9_3()
{
    return PlatformParameters(2, 0, egl_platform::D3D11_FL9_3());
}

constexpr PlatformParameters ES2_D3D11_WARP()
{
    return PlatformParameters(2, 0, egl_platform::D3D11_WARP());
}

constexpr PlatformParameters ES2_D3D11_FL11_0_WARP()
{
    return PlatformParameters(2, 0, egl_platform::D3D11_FL11_0_WARP());
}

constexpr PlatformParameters ES2_D3D11_FL10_1_WARP()
{
    return PlatformParameters(2, 0, egl_platform::D3D11_FL10_1_WARP());
}

constexpr PlatformParameters ES2_D3D11_FL10_0_WARP()
{
    return PlatformParameters(2, 0, egl_platform::D3D11_FL10_0_WARP());
}

constexpr PlatformParameters ES2_D3D11_FL9_3_WARP()
{
    return PlatformParameters(2, 0, egl_platform::D3D11_FL9_3_WARP());
}

constexpr PlatformParameters ES2_D3D11_REFERENCE()
{
    return PlatformParameters(2, 0, egl_platform::D3D11_REFERENCE());
}

constexpr PlatformParameters ES2_D3D11_FL11_0_REFERENCE()
{
    return PlatformParameters(2, 0, egl_platform::D3D11_FL11_0_REFERENCE());
}

constexpr PlatformParameters ES2_D3D11_FL10_1_REFERENCE()
{
    return PlatformParameters(2, 0, egl_platform::D3D11_FL10_1_REFERENCE());
}

constexpr PlatformParameters ES2_D3D11_FL10_0_REFERENCE()
{
    return PlatformParameters(2, 0, egl_platform::D3D11_FL10_0_REFERENCE());
}

constexpr PlatformParameters ES2_D3D11_FL9_3_REFERENCE()
{
    return PlatformParameters(2, 0, egl_platform::D3D11_FL9_3_REFERENCE());
}

constexpr PlatformParameters ES3_D3D11()
{
    return PlatformParameters(3, 0, egl_platform::D3D11());
}

constexpr PlatformParameters ES3_D3D11_FL11_1()
{
    return PlatformParameters(3, 0, egl_platform::D3D11_FL11_1());
}

constexpr PlatformParameters ES3_D3D11_FL11_0()
{
    return PlatformParameters(3, 0, egl_platform::D3D11_FL11_0());
}

constexpr PlatformParameters ES3_D3D11_FL10_1()
{
    return PlatformParameters(3, 0, egl_platform::D3D11_FL10_1());
}

constexpr PlatformParameters ES31_D3D11()
{
    return PlatformParameters(3, 1, egl_platform::D3D11());
}

constexpr PlatformParameters ES31_D3D11_FL11_1()
{
    return PlatformParameters(3, 1, egl_platform::D3D11_FL11_1());
}

constexpr PlatformParameters ES31_D3D11_FL11_0()
{
    return PlatformParameters(3, 1, egl_platform::D3D11_FL11_0());
}

constexpr PlatformParameters ES3_D3D11_WARP()
{
    return PlatformParameters(3, 0, egl_platform::D3D11_WARP());
}

constexpr PlatformParameters ES3_D3D11_FL11_1_WARP()
{
    return PlatformParameters(3, 0, egl_platform::D3D11_FL11_1_WARP());
}

constexpr PlatformParameters ES3_D3D11_FL11_0_WARP()
{
    return PlatformParameters(3, 0, egl_platform::D3D11_FL11_0_WARP());
}

constexpr PlatformParameters ES3_D3D11_FL10_1_WARP()
{
    return PlatformParameters(3, 0, egl_platform::D3D11_FL10_1_WARP());
}

constexpr PlatformParameters ES1_OPENGLES()
{
    return PlatformParameters(1, 0, egl_platform::OPENGLES());
}

constexpr PlatformParameters ES2_OPENGLES()
{
    return PlatformParameters(2, 0, egl_platform::OPENGLES());
}

constexpr PlatformParameters ES2_OPENGLES(EGLint major, EGLint minor)
{
    return PlatformParameters(2, 0, egl_platform::OPENGLES(major, minor));
}

constexpr PlatformParameters ES3_OPENGLES()
{
    return PlatformParameters(3, 0, egl_platform::OPENGLES());
}

constexpr PlatformParameters ES3_OPENGLES(EGLint major, EGLint minor)
{
    return PlatformParameters(3, 0, egl_platform::OPENGLES(major, minor));
}

constexpr PlatformParameters ES31_OPENGLES()
{
    return PlatformParameters(3, 1, egl_platform::OPENGLES());
}

constexpr PlatformParameters ES31_OPENGLES(EGLint major, EGLint minor)
{
    return PlatformParameters(3, 1, egl_platform::OPENGLES(major, minor));
}

constexpr PlatformParameters ES1_OPENGL()
{
    return PlatformParameters(1, 0, egl_platform::OPENGL());
}

constexpr PlatformParameters ES2_OPENGL()
{
    return PlatformParameters(2, 0, egl_platform::OPENGL());
}

constexpr PlatformParameters ES2_OPENGL(EGLint major, EGLint minor)
{
    return PlatformParameters(2, 0, egl_platform::OPENGL(major, minor));
}

constexpr PlatformParameters ES3_OPENGL()
{
    return PlatformParameters(3, 0, egl_platform::OPENGL());
}

constexpr PlatformParameters ES3_OPENGL(EGLint major, EGLint minor)
{
    return PlatformParameters(3, 0, egl_platform::OPENGL(major, minor));
}

constexpr PlatformParameters ES31_OPENGL()
{
    return PlatformParameters(3, 1, egl_platform::OPENGL());
}

constexpr PlatformParameters ES31_OPENGL(EGLint major, EGLint minor)
{
    return PlatformParameters(3, 1, egl_platform::OPENGL(major, minor));
}

constexpr PlatformParameters ES1_NULL()
{
    return PlatformParameters(1, 0, EGLPlatformParameters(EGL_PLATFORM_ANGLE_TYPE_NULL_ANGLE));
}

constexpr PlatformParameters ES2_NULL()
{
    return PlatformParameters(2, 0, EGLPlatformParameters(EGL_PLATFORM_ANGLE_TYPE_NULL_ANGLE));
}

constexpr PlatformParameters ES3_NULL()
{
    return PlatformParameters(3, 0, EGLPlatformParameters(EGL_PLATFORM_ANGLE_TYPE_NULL_ANGLE));
}

constexpr PlatformParameters ES31_NULL()
{
    return PlatformParameters(3, 1, EGLPlatformParameters(EGL_PLATFORM_ANGLE_TYPE_NULL_ANGLE));
}

constexpr PlatformParameters ES1_VULKAN()
{
    return PlatformParameters(1, 0, egl_platform::VULKAN());
}

constexpr PlatformParameters ES1_VULKAN_NULL()
{
    return PlatformParameters(1, 0, egl_platform::VULKAN_NULL());
}

constexpr PlatformParameters ES2_VULKAN()
{
    return PlatformParameters(2, 0, egl_platform::VULKAN());
}

constexpr PlatformParameters ES2_VULKAN_NULL()
{
    return PlatformParameters(2, 0, egl_platform::VULKAN_NULL());
}

constexpr PlatformParameters ES3_VULKAN()
{
    return PlatformParameters(3, 0, egl_platform::VULKAN());
}

constexpr PlatformParameters ES3_VULKAN_NULL()
{
    return PlatformParameters(3, 0, egl_platform::VULKAN_NULL());
}

constexpr PlatformParameters ES2_WGL()
{
    return PlatformParameters(2, 0, GLESDriverType::SystemWGL);
}

constexpr PlatformParameters ES3_WGL()
{
    return PlatformParameters(3, 0, GLESDriverType::SystemWGL);
}
}  // namespace angle

// Sets of configs
#define ANGLE_ALL_TEST_CONFIGS_X(OP, PARAM0, PARAM1) \
 OP(ES1_D3D9, PARAM0, PARAM1) \
 OP(ES2_VULKAN, PARAM0, PARAM1) \
 OP(ES2_VULKAN_NULL, PARAM0, PARAM1) \
 OP(ES3_VULKAN, PARAM0, PARAM1) \
 OP(ES3_VULKAN_NULL, PARAM0, PARAM1) \
 OP(ES2_WGL, PARAM0, PARAM1) \
 OP(ES3_WGL, PARAM0, PARAM1)

#endif  // ANGLE_TEST_CONFIGS_H_
