//
// Copyright (c) 2014 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//

#ifndef UTIL_TEST_FIXTURE_TYPES_H
#define UTIL_TEST_FIXTURE_TYPES_H

#include "EGLWindow.h"

#include <EGL/egl.h>
#include <EGL/eglext.h>

struct D3D11
{
    static EGLPlatformParameters GetPlatform()
    {
        return EGLPlatformParameters(EGL_PLATFORM_ANGLE_TYPE_D3D11_ANGLE, EGL_DONT_CARE, EGL_DONT_CARE, EGL_FALSE);
    };
};

struct D3D11_WARP
{
    static EGLPlatformParameters GetPlatform()
    {
        return EGLPlatformParameters(EGL_PLATFORM_ANGLE_TYPE_D3D11_ANGLE, EGL_DONT_CARE, EGL_DONT_CARE, EGL_TRUE);
    };
};

struct D3D9
{
    static EGLPlatformParameters GetPlatform()
    {
        return EGLPlatformParameters(EGL_PLATFORM_ANGLE_TYPE_D3D9_ANGLE, EGL_DONT_CARE, EGL_DONT_CARE, EGL_FALSE);
    };
};

// Test Fixture Type
template<GLint GLESMajorVersion, typename PlatformT>
struct TestFixture
{
    static EGLint GetGlesMajorVersion()
    {
        return GLESMajorVersion;
    }

    static EGLPlatformParameters GetPlatform()
    {
        return PlatformT::GetPlatform();
    }
};

// Typedefs of common fixture types
typedef TestFixture<2, D3D9>       ES2_D3D9;

typedef TestFixture<2, D3D11>      ES2_D3D11;
typedef TestFixture<2, D3D11_WARP> ES2_D3D11_WARP;
typedef TestFixture<3, D3D11>      ES3_D3D11;
typedef TestFixture<3, D3D11_WARP> ES3_D3D11_WARP;

#define ANGLE_TYPED_TEST_CASE(testName, ...) TYPED_TEST_CASE(testName, ::testing::Types<__VA_ARGS__>);

#endif // UTIL_TEST_FIXTURE_TYPES_H
