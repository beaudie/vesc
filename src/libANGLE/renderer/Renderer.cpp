//
// Copyright (c) 2012-2014 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//

// Renderer.cpp: Implements EGL dependencies for creating and destroying Renderer instances.

#include "common/utilities.h"
#include "libANGLE/AttributeMap.h"
#include "libANGLE/renderer/Renderer.h"

#include <EGL/eglext.h>

#if defined (ANGLE_ENABLE_D3D9)
#include "libANGLE/renderer/d3d/d3d9/Renderer9.h"
#endif // ANGLE_ENABLE_D3D9

#if defined (ANGLE_ENABLE_D3D11)
#include "libANGLE/renderer/d3d/d3d11/Renderer11.h"
#endif // ANGLE_ENABLE_D3D11

#if defined (ANGLE_TEST_CONFIG)
#define ANGLE_DEFAULT_D3D11 1
#endif

#if !defined(ANGLE_DEFAULT_D3D11)
// Enables use of the Direct3D 11 API for a default display, when available
#define ANGLE_DEFAULT_D3D11 0
#endif

namespace rx
{

Renderer::Renderer()
    : mCapsInitialized(false),
      mWorkaroundsInitialized(false)
{
}

Renderer::~Renderer()
{
}

const gl::Caps &Renderer::getRendererCaps() const
{
    if (!mCapsInitialized)
    {
        generateCaps(&mCaps, &mTextureCaps, &mExtensions);
        mCapsInitialized = true;
    }

    return mCaps;
}

const gl::TextureCapsMap &Renderer::getRendererTextureCaps() const
{
    if (!mCapsInitialized)
    {
        generateCaps(&mCaps, &mTextureCaps, &mExtensions);
        mCapsInitialized = true;
    }

    return mTextureCaps;
}

const gl::Extensions &Renderer::getRendererExtensions() const
{
    if (!mCapsInitialized)
    {
        generateCaps(&mCaps, &mTextureCaps, &mExtensions);
        mCapsInitialized = true;
    }

    return mExtensions;
}

const Workarounds &Renderer::getWorkarounds() const
{
    if (!mWorkaroundsInitialized)
    {
        mWorkarounds = generateWorkarounds();
        mWorkaroundsInitialized = true;
    }

    return mWorkarounds;
}

typedef Renderer *(*CreateRendererFunction)(egl::Display*, EGLNativeDisplayType, const egl::AttributeMap &);

template <typename RendererType>
static Renderer *CreateTypedRenderer(egl::Display *display, EGLNativeDisplayType nativeDisplay, const egl::AttributeMap &attributes)
{
    return new RendererType(display, nativeDisplay, attributes);
}

Renderer *CreateRenderer(egl::Display *display, EGLNativeDisplayType nativeDisplay, const egl::AttributeMap &attribMap)
{
    std::vector<CreateRendererFunction> rendererCreationFunctions;

    EGLint requestedDisplayType = attribMap.get(EGL_PLATFORM_ANGLE_TYPE_ANGLE, EGL_PLATFORM_ANGLE_TYPE_DEFAULT_ANGLE);

#   if defined(ANGLE_ENABLE_D3D11)
        if (nativeDisplay == EGL_D3D11_ELSE_D3D9_DISPLAY_ANGLE ||
            nativeDisplay == EGL_D3D11_ONLY_DISPLAY_ANGLE ||
            requestedDisplayType == EGL_PLATFORM_ANGLE_TYPE_D3D11_ANGLE)
        {
            rendererCreationFunctions.push_back(CreateTypedRenderer<Renderer11>);
        }
#   endif

#   if defined(ANGLE_ENABLE_D3D9)
        if (nativeDisplay == EGL_D3D11_ELSE_D3D9_DISPLAY_ANGLE ||
            requestedDisplayType == EGL_PLATFORM_ANGLE_TYPE_D3D9_ANGLE)
        {
            rendererCreationFunctions.push_back(CreateTypedRenderer<Renderer9>);
        }
#   endif

    if (nativeDisplay != EGL_D3D11_ELSE_D3D9_DISPLAY_ANGLE &&
        nativeDisplay != EGL_D3D11_ONLY_DISPLAY_ANGLE &&
        requestedDisplayType == EGL_PLATFORM_ANGLE_TYPE_DEFAULT_ANGLE)
    {
        // The default display is requested, try the D3D9 and D3D11 renderers, order them using
        // the definition of ANGLE_DEFAULT_D3D11
#       if ANGLE_DEFAULT_D3D11
#           if defined(ANGLE_ENABLE_D3D11)
                rendererCreationFunctions.push_back(CreateTypedRenderer<Renderer11>);
#           endif
#           if defined(ANGLE_ENABLE_D3D9)
                rendererCreationFunctions.push_back(CreateTypedRenderer<Renderer9>);
#           endif
#       else
#           if defined(ANGLE_ENABLE_D3D9)
                rendererCreationFunctions.push_back(CreateTypedRenderer<Renderer9>);
#           endif
#           if defined(ANGLE_ENABLE_D3D11)
                rendererCreationFunctions.push_back(CreateTypedRenderer<Renderer11>);
#           endif
#       endif
    }

    for (size_t i = 0; i < rendererCreationFunctions.size(); i++)
    {
        Renderer *renderer = rendererCreationFunctions[i](display, nativeDisplay, attribMap);
        if (renderer->initialize() == EGL_SUCCESS)
        {
            return renderer;
        }
        else
        {
            // Failed to create the renderer, try the next
            SafeDelete(renderer);
        }
    }

    return NULL;
}

}
