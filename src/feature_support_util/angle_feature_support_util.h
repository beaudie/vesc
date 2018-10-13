//
// Copyright 2018 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//

// angle_feature_support_util.h: External header file for feature-support utilities.
// feature_support_util.cpp: Helps Android EGL loader to determine whether to
// use ANGLE or a native GLES driver.  Can be extended in the future for
// more-general feature selection.

#ifndef ANGLE_FEATURE_SUPPORT_UTIL_H_
#define ANGLE_FEATURE_SUPPORT_UTIL_H_

#if defined(ANDROID)

// FIXME - NEED TO #define ANGLE_EXPORT in a way that works for Android builds

#else
#include "export.h"
#endif

#ifdef __cplusplus
extern "C" {
#endif

// The following is the "version 1" external interface that the Android EGL loader used.


// Callers of the ANGLE feature-support-utility API (e.g. the Android EGL loader) will call this
// function in order to determine what version of the API it can use (if any).
//
// The caller supplies the highest version of the API that it knows about.  If that version is
// supported by the feature-support-utility, true is returned and the version isn't changed.  If
// the supplied version is higher than supported by the feature-support-utility, true is returned
// and the version is changed to the highest supported by the feature-support-utility.  If the
// supplied version is lower than supported by the feature-support-utility, false is returned.
//
// Parameters:
//
// - versionToUse (IN/OUT) - The application supplies the highest version of the interface that it
//   knows about.  If successful, the output value is either unchanged or is the highest supported
//   by the interface.
//
ANGLE_EXPORT bool ANGLEGetUtilityAPI(uint32 *versionToUse);


// The Android EGL loader will call this function in order to determine whether
// to use ANGLE instead of a native OpenGL-ES (GLES) driver.
//
// Parameters:
// - fd - File descriptor of the rules file to use
// - appName - Java name of the application (e.g. "com.google.android.apps.maps")
// - deviceMfr - Device manufacturer, from the "ro.product.manufacturer"com.google.android" property
// - deviceModel - Device model, from the "ro.product.model"com.google.android" property
//
ANGLE_EXPORT bool AndroidUseANGLEForApplication(int fd,
                                                const char *appName,
                                                const char *deviceMfr,
                                                const char *deviceModel);

#ifdef __cplusplus
}  // extern "C"
#endif

#endif  // ANGLE_FEATURE_SUPPORT_UTIL_H_
