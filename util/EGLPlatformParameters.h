//
// Copyright 2018 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//
// EGLPlatformParameters: Basic description of an EGL device.

#ifndef UTIL_EGLPLATFORMPARAMETERS_H_
#define UTIL_EGLPLATFORMPARAMETERS_H_

#include "util/egl_loader_autogen.h"
#include "util/util_export.h"

struct EGLPlatformParameters
{
    constexpr EGLPlatformParameters();
    constexpr explicit EGLPlatformParameters(EGLint renderer);
    constexpr EGLPlatformParameters(EGLint renderer,
                                    EGLint majorVersion,
                                    EGLint minorVersion,
                                    EGLint deviceType);
    constexpr EGLPlatformParameters(EGLint renderer,
                                    EGLint majorVersion,
                                    EGLint minorVersion,
                                    EGLint deviceType,
                                    EGLint presentPath);

    EGLint renderer;
    EGLint majorVersion;
    EGLint minorVersion;
    EGLint deviceType;
    EGLint presentPath;
};

constexpr EGLPlatformParameters::EGLPlatformParameters()
    : renderer(EGL_PLATFORM_ANGLE_TYPE_DEFAULT_ANGLE),
      majorVersion(EGL_DONT_CARE),
      minorVersion(EGL_DONT_CARE),
      deviceType(EGL_PLATFORM_ANGLE_DEVICE_TYPE_HARDWARE_ANGLE),
      presentPath(EGL_DONT_CARE)
{}

constexpr EGLPlatformParameters::EGLPlatformParameters(EGLint renderer)
    : renderer(renderer),
      majorVersion(EGL_DONT_CARE),
      minorVersion(EGL_DONT_CARE),
      deviceType(EGL_PLATFORM_ANGLE_DEVICE_TYPE_HARDWARE_ANGLE),
      presentPath(EGL_DONT_CARE)
{}

constexpr EGLPlatformParameters::EGLPlatformParameters(EGLint renderer,
                                                       EGLint majorVersion,
                                                       EGLint minorVersion,
                                                       EGLint deviceType)
    : renderer(renderer),
      majorVersion(majorVersion),
      minorVersion(minorVersion),
      deviceType(deviceType),
      presentPath(EGL_DONT_CARE)
{}

constexpr EGLPlatformParameters::EGLPlatformParameters(EGLint renderer,
                                                       EGLint majorVersion,
                                                       EGLint minorVersion,
                                                       EGLint deviceType,
                                                       EGLint presentPath)
    : renderer(renderer),
      majorVersion(majorVersion),
      minorVersion(minorVersion),
      deviceType(deviceType),
      presentPath(presentPath)
{}

ANGLE_UTIL_EXPORT bool operator<(const EGLPlatformParameters &a, const EGLPlatformParameters &b);
ANGLE_UTIL_EXPORT bool operator==(const EGLPlatformParameters &a, const EGLPlatformParameters &b);

#endif  // UTIL_EGLPLATFORMPARAMETERS_H_
