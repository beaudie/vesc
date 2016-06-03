//
// Copyright (c) 2016 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//

// DisplayEGL.cpp: Common across EGL parts of platform specific egl::Display implementations

#include "libANGLE/renderer/gl/egl/DisplayEGL.h"

namespace rx
{

#define EGL_NO_CONFIG ((EGLConfig)0)

DisplayEGL::DisplayEGL()
    : DisplayGL(),
      mEGL(nullptr),
      mConfig(EGL_NO_CONFIG),
      mContext(EGL_NO_CONTEXT),
      mFunctionsGL(nullptr)
{
}

DisplayEGL::~DisplayEGL()
{
}

std::string DisplayEGL::getVendorString() const
{
    const char *vendor = mEGL->queryString(EGL_VENDOR);
    if (!vendor)
    {
        return "";
    }
    return vendor;
}

egl::Error DisplayEGL::initializeContext(const egl::AttributeMap &eglAttributes)
{
    gl::Version eglVersion(mEGL->majorVersion, mEGL->minorVersion);

    gl::Version requestedVersion(
        eglAttributes.getAsInt(EGL_PLATFORM_ANGLE_MAX_VERSION_MAJOR_ANGLE, EGL_DONT_CARE),
        eglAttributes.getAsInt(EGL_PLATFORM_ANGLE_MAX_VERSION_MINOR_ANGLE, EGL_DONT_CARE));
    bool initializeRequested = static_cast<EGLint>(requestedVersion.major) != EGL_DONT_CARE &&
                               static_cast<EGLint>(requestedVersion.minor) != EGL_DONT_CARE;

    if (eglVersion >= gl::Version(1, 5) || mEGL->hasExtension("EGL_KHR_create_context"))
    {
        EGLint majorAttr = (eglVersion >= gl::Version(1, 5)) ? EGL_CONTEXT_MAJOR_VERSION
                                                             : EGL_CONTEXT_MAJOR_VERSION_KHR;
        EGLint minorAttr = (eglVersion >= gl::Version(1, 5)) ? EGL_CONTEXT_MINOR_VERSION
                                                             : EGL_CONTEXT_MINOR_VERSION_KHR;

        if (initializeRequested)
        {
            // clang-format off
            EGLint contextAttribList[] =
            {
                majorAttr, static_cast<EGLint>(requestedVersion.major),
                minorAttr, static_cast<EGLint>(requestedVersion.minor),
                EGL_NONE
            };
            // clang-format on
            mContext = mEGL->createContext(mConfig, EGL_NO_CONTEXT, contextAttribList);
        }
        else
        {
            // clang-format off
            const gl::Version esVersionsFrom2_0[] = {
                gl::Version(3, 2),
                gl::Version(3, 1),
                gl::Version(3, 0),
                gl::Version(2, 0),
            };
            // clang-format on

            for (auto &version : esVersionsFrom2_0)
            {
                // clang-format off
                EGLint contextAttribList[] =
                {
                    majorAttr, static_cast<EGLint>(version.major),
                    minorAttr, static_cast<EGLint>(version.minor),
                    EGL_NONE
                };
                // clang-format on

                mContext = mEGL->createContext(mConfig, EGL_NO_CONTEXT, contextAttribList);
                if (mContext != EGL_NO_CONTEXT)
                {
                    break;
                }
            }
        }
    }
    else
    {
        // clang-format off
        EGLint contextAttribList[] =
        {
            EGL_CONTEXT_CLIENT_VERSION, 2,
            EGL_NONE
        };
        // clang-format on
        if (initializeRequested)
        {
            if (requestedVersion.major != 2 || requestedVersion.minor != 0)
            {
                return egl::Error(EGL_BAD_ATTRIBUTE, "Unsupported requested context version");
            }
        }
        mContext = mEGL->createContext(mConfig, EGL_NO_CONTEXT, contextAttribList);
    }

    if (mContext == EGL_NO_CONTEXT)
    {
        return egl::Error(mEGL->getError(), "eglCreateContext failed");
    }

    return egl::Error(EGL_SUCCESS);
}

void DisplayEGL::generateExtensions(egl::DisplayExtensions *outExtensions) const
{
    outExtensions->createContextRobustness =
        mEGL->hasExtension("EGL_EXT_create_context_robustness");
    outExtensions->d3dShareHandleClientBuffer =
        mEGL->hasExtension("EGL_ANGLE_d3d_share_handle_client_buffer");
    outExtensions->surfaceD3DTexture2DShareHandle =
        mEGL->hasExtension("EGL_ANGLE_surface_d3d_texture_2d_share_handle");
    outExtensions->querySurfacePointer = mEGL->hasExtension("EGL_ANGLE_query_surface_pointer");
    outExtensions->windowFixedSize = mEGL->hasExtension("EGL_ANGLE_window_fixed_size");
    outExtensions->keyedMutex = mEGL->hasExtension("EGL_ANGLE_keyed_mutex");
    outExtensions->surfaceOrientation = mEGL->hasExtension("EGL_ANGLE_surface_orientation");
    outExtensions->postSubBuffer = mEGL->hasExtension("EGL_NV_post_sub_buffer");
    outExtensions->createContext = mEGL->hasExtension("EGL_KHR_create_context");
    outExtensions->deviceQuery = mEGL->hasExtension("EGL_EXT_device_query");
    outExtensions->image = mEGL->hasExtension("EGL_KHR_image");
    outExtensions->imageBase = mEGL->hasExtension("EGL_KHR_image_base");
    outExtensions->imagePixmap = mEGL->hasExtension("EGL_KHR_image_pixmap");
    outExtensions->glTexture2DImage = mEGL->hasExtension("EGL_KHR_gl_texture_2D_image");
    outExtensions->glTextureCubemapImage = mEGL->hasExtension("EGL_KHR_gl_texture_cubemap_image");
    outExtensions->glTexture3DImage = mEGL->hasExtension("EGL_KHR_gl_texture_3D_image");
    outExtensions->glRenderbufferImage = mEGL->hasExtension("EGL_KHR_gl_renderbuffer_image");
    outExtensions->getAllProcAddresses = mEGL->hasExtension("EGL_KHR_get_all_proc_addresses");
    outExtensions->flexibleSurfaceCompatibility =
        mEGL->hasExtension("EGL_ANGLE_flexible_surface_compatibility");
    outExtensions->directComposition = mEGL->hasExtension("EGL_ANGLE_direct_composition");
    outExtensions->createContextNoError = mEGL->hasExtension("KHR_create_context_no_error");
}

void DisplayEGL::generateCaps(egl::Caps *outCaps) const
{
    outCaps->textureNPOT = true;  // Since we request GLES >= 2
}

const FunctionsGL *DisplayEGL::getFunctionsGL() const
{
    return mFunctionsGL;
}
}  // namespace rx
