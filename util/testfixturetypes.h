//
// Copyright (c) 2014 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//

#ifndef UTIL_TEST_FIXTURE_TYPES_H
#define UTIL_TEST_FIXTURE_TYPES_H

#include <EGL/egl.h>
#include <EGL/eglext.h>

struct GlesType
{
    EGLint glesMajorVersion;
};

struct RendererType
{
    EGLint requestedRenderer;
};

namespace Gles
{

struct Two : GlesType
{
    Two() 
    { 
        glesMajorVersion = 2;
    };
};

struct Three : GlesType
{
    Three() 
    { 
        glesMajorVersion = 3;
    };
};

}

namespace Rend
{

struct D3D11 : RendererType
{
    D3D11() 
    { 
        requestedRenderer = EGL_PLATFORM_ANGLE_TYPE_D3D11_ANGLE;
    };
};

struct D3D9 : RendererType
{
    D3D9() 
    { 
        requestedRenderer = EGL_PLATFORM_ANGLE_TYPE_D3D9_ANGLE;
    };
};

struct WARP : RendererType
{
    WARP() 
    { 
        requestedRenderer = EGL_PLATFORM_ANGLE_TYPE_D3D11_WARP_ANGLE;
    };
};

}

// Test Fixture Type
template<typename G, typename R>
struct TFT
{
    TFT() 
    { 
        G gles;
        R rend;
        glesMajorVersion = gles.glesMajorVersion;
        requestedRenderer = rend.requestedRenderer;
    };

    EGLint glesMajorVersion;
    EGLint requestedRenderer;
};

template<typename T>
static EGLint GetGlesMajorVersion()
{
    T testRunConfig;
    return testRunConfig.glesMajorVersion;
}

template<typename T>
static EGLint GetRequestedRenderer()
{
    T testRunConfig;
    return testRunConfig.requestedRenderer;
}

#endif // UTIL_TEST_FIXTURE_TYPES_H