//
// Copyright 2021 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//

// DisplayEGLSurfaceless.cpp: Surfaceless implementation of egl::DisplayEGL

#include "libANGLE/renderer/gl/egl/DisplayEGLSurfaceless.h"

#include "libANGLE/renderer/gl/ContextGL.h"
#include "libANGLE/renderer/gl/egl/ContextEGL.h"
#include "libANGLE/renderer/gl/egl/DisplayEGL.h"
#include "libANGLE/renderer/gl/egl/FunctionsEGLDL.h"

namespace
{

rx::RobustnessVideoMemoryPurgeStatus GetRobustnessVideoMemoryPurge(const egl::AttributeMap &attribs)
{
    return static_cast<rx::RobustnessVideoMemoryPurgeStatus>(
        attribs.get(EGL_GENERATE_RESET_ON_VIDEO_MEMORY_PURGE_NV, GL_FALSE));
}

}  // anonymous namespace

namespace rx
{

DisplayEGLSurfaceless::DisplayEGLSurfaceless(const egl::DisplayState &state) : DisplayEGL(state) {}

DisplayEGLSurfaceless::~DisplayEGLSurfaceless() {}

egl::Error DisplayEGLSurfaceless::initialize(egl::Display *display)
{
    egl::Error result = DisplayEGL::initialize(display);

    const char *necessaryExtensions[] = {
        "EGL_KHR_image_base",
        "EGL_EXT_image_dma_buf_import",
        "EGL_KHR_surfaceless_context",
    };
    for (const char *ext : necessaryExtensions)
    {
        if (!mEGL->hasExtension(ext))
        {
            return egl::EglNotInitialized() << "need " << ext;
        }
    }

    return result;
}

SurfaceImpl *DisplayEGLSurfaceless::createWindowSurface(const egl::SurfaceState &state,
                                                        EGLNativeWindowType window,
                                                        const egl::AttributeMap &attribs)
{
    return nullptr;
}

SurfaceImpl *DisplayEGLSurfaceless::createPbufferSurface(const egl::SurfaceState &state,
                                                         const egl::AttributeMap &attribs)
{
    return nullptr;
}

ContextImpl *DisplayEGLSurfaceless::createContext(const gl::State &state,
                                                  gl::ErrorSet *errorSet,
                                                  const egl::Config *configuration,
                                                  const gl::Context *shareContext,
                                                  const egl::AttributeMap &attribs)
{
    RobustnessVideoMemoryPurgeStatus robustnessVideoMemoryPurgeStatus =
        GetRobustnessVideoMemoryPurge(attribs);
    return new ContextEGL(state, errorSet, mRenderer, robustnessVideoMemoryPurgeStatus);
}

bool DisplayEGLSurfaceless::validateEglConfig(const EGLint *configAttribs)
{

    EGLint numConfigs;
    if (!mEGL->chooseConfig(configAttribs, NULL, 0, &numConfigs))
    {
        ERR() << "eglChooseConfig failed with error " << egl::Error(mEGL->getError());
        return false;
    }
    if (numConfigs == 0)
    {
        return false;
    }
    return true;
}

egl::ConfigSet DisplayEGLSurfaceless::generateConfigs()
{
    // clang-format off
    std::vector<EGLint> configAttribs8888 =
    {
        EGL_RED_SIZE, 8,
        EGL_GREEN_SIZE, 8,
        EGL_BLUE_SIZE, 8,
        EGL_ALPHA_SIZE, 8,
        EGL_BUFFER_SIZE, 32,
        EGL_DEPTH_SIZE, 24,
        EGL_NONE
    };
    // clang-format on

    if (!validateEglConfig(configAttribs8888.data()))
    {
        ERR() << "No suitable EGL configs found.";
        return egl::ConfigSet();
    }
    mConfigAttribList = configAttribs8888;
    return DisplayEGL::generateConfigs();
}

EGLint DisplayEGLSurfaceless::fixSurfaceType(EGLint surfaceType) const
{
    EGLint type = DisplayEGL::fixSurfaceType(surfaceType);
    // Ozone native surfaces don't support EGL_WINDOW_BIT,
    // but ANGLE uses renderbuffers to emulate windows
    return type | EGL_WINDOW_BIT;
}

}  // namespace rx
