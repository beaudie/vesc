//
// Copyright (c) 2014 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//

#include "test_utils/angle_test_configs.h"

#include "common/platform.h"
#include "util/util_gl.h"

namespace angle
{
bool operator<(const PlatformParameters &a, const PlatformParameters &b)
{
    if (a.majorVersion != b.majorVersion)
    {
        return a.majorVersion < b.majorVersion;
    }

    if (a.minorVersion != b.minorVersion)
    {
        return a.minorVersion < b.minorVersion;
    }

    return a.eglParameters < b.eglParameters;
}

bool operator==(const PlatformParameters &a, const PlatformParameters &b)
{
    return (a.majorVersion == b.majorVersion) && (a.minorVersion == b.minorVersion) &&
           (a.eglParameters == b.eglParameters);
}

std::ostream &operator<<(std::ostream &stream, const PlatformParameters &pp)
{
    stream << "ES" << pp.majorVersion << "_";
    if (pp.minorVersion != 0)
    {
        stream << pp.minorVersion << "_";
    }

    switch (pp.driver)
    {
        case GLESDriverType::AngleEGL:
        {
            switch (pp.eglParameters.renderer)
            {
                case EGL_PLATFORM_ANGLE_TYPE_DEFAULT_ANGLE:
                    stream << "DEFAULT";
                    break;
                case EGL_PLATFORM_ANGLE_TYPE_D3D9_ANGLE:
                    stream << "D3D9";
                    break;
                case EGL_PLATFORM_ANGLE_TYPE_D3D11_ANGLE:
                    stream << "D3D11";
                    break;
                case EGL_PLATFORM_ANGLE_TYPE_NULL_ANGLE:
                    stream << "NULL";
                    break;
                case EGL_PLATFORM_ANGLE_TYPE_OPENGL_ANGLE:
                    stream << "OPENGL";
                    break;
                case EGL_PLATFORM_ANGLE_TYPE_OPENGLES_ANGLE:
                    stream << "OPENGLES";
                    break;
                case EGL_PLATFORM_ANGLE_TYPE_VULKAN_ANGLE:
                    stream << "VULKAN";
                    break;
                default:
                    stream << "UNDEFINED";
                    break;
            }
            break;
        }
        case GLESDriverType::SystemWGL:
            stream << "WGL";
            break;
        case GLESDriverType::SystemEGL:
            stream << "GLES";
            break;
        default:
            stream << "ERROR";
            break;
    }

    if (pp.eglParameters.majorVersion != EGL_DONT_CARE)
    {
        stream << "_" << pp.eglParameters.majorVersion;
    }

    if (pp.eglParameters.minorVersion != EGL_DONT_CARE)
    {
        stream << "_" << pp.eglParameters.minorVersion;
    }

    switch (pp.eglParameters.deviceType)
    {
        case EGL_DONT_CARE:
        case EGL_PLATFORM_ANGLE_DEVICE_TYPE_HARDWARE_ANGLE:
            // default
            break;

        case EGL_PLATFORM_ANGLE_DEVICE_TYPE_NULL_ANGLE:
            stream << "_NULL";
            break;

        case EGL_PLATFORM_ANGLE_DEVICE_TYPE_D3D_REFERENCE_ANGLE:
            stream << "_REFERENCE";
            break;

        case EGL_PLATFORM_ANGLE_DEVICE_TYPE_D3D_WARP_ANGLE:
            stream << "_WARP";
            break;

        default:
            stream << "_ERR";
            break;
    }

    switch (pp.eglParameters.presentPath)
    {
        case EGL_EXPERIMENTAL_PRESENT_PATH_COPY_ANGLE:
            stream << "_PRESENT_PATH_COPY";
            break;

        case EGL_EXPERIMENTAL_PRESENT_PATH_FAST_ANGLE:
            stream << "_PRESENT_PATH_FAST";
            break;

        case EGL_DONT_CARE:
            // default
            break;

        default:
            stream << "_ERR";
            break;
    }

    return stream;
}

// EGL platforms
namespace egl_platform
{
}  // namespace egl_platform
}  // namespace angle
