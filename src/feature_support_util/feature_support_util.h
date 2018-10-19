//
// Copyright 2018 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//

// feature_support_util.h: Internal-to-ANGLE header file for feature-support utilities.

#ifndef FEATURE_SUPPORT_UTIL_H_
#define FEATURE_SUPPORT_UTIL_H_

#include "export.h"

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
ANGLE_EXPORT bool ANGLEGetUtilityAPI(unsigned int *versionToUse);

// The Android EGL loader will call this function in order to determine whether
// to use ANGLE instead of a native OpenGL-ES (GLES) driver.
//
// Parameters:
// - rules_fd - File descriptor of the rules file to use
// - rules_offset - Offset into the fd before finding the contents of the rules file
// - rules_length - length of the rules file content
// - appName - Java name of the application (e.g. "com.google.android.apps.maps")
// - deviceMfr - Device manufacturer, from the "ro.product.manufacturer"com.google.android" property
// - deviceModel - Device model, from the "ro.product.model"com.google.android" property
//
ANGLE_EXPORT bool AndroidUseANGLEForApplication(int rules_fd,
                                                long rules_offset,
                                                long rules_length,
                                                const char *appName,
                                                const char *deviceMfr,
                                                const char *deviceModel);

// The following is the "version 2" external interface that the Android EGL loader used.

// Continue to use ANGLEGetUtilityAPI

// The Android EGL loader will call this function in order to parse a rules file
// and create a set of rules, for which a handle is returned.
//
// Parameters:
// - rules_fd      (IN) - File descriptor of the rules file to use
// - rules_offset  (IN) - Offset into the fd before finding the contents of the rules file
// - rules_length  (IN) - length of the rules file content
// - rules_handle (OUT) - handle to the rules data structure
//
// Return value:
// - bool - true if no errors, otherwise false
//
ANGLE_EXPORT bool AndroidParseRulesFile(int rules_fd,
                                        long rules_offset,
                                        long rules_length,
                                        void** rules_handle);

// The Android EGL loader will call this function in order to obtain a handle to
// the SystemInfo struct (one that also contains the device's manufacturer and
// model.
//
// Parameters:
// - deviceMfr   (IN) - Device manufacturer, from the "ro.product.manufacturer"com.google.android"
//                      property
// - deviceModel (IN) - Device model, from the "ro.product.model"com.google.android" property
// - system_info_handle (OUT) - handle to the SystemInfo structure
//
// Return value:
// - bool - true if no errors, otherwise false
//
ANGLE_EXPORT bool ANGLEGetSystemInfo(const char *deviceMfr,
                                     const char *deviceModel,
                                     void** system_info_handle);

// The Android EGL loader will call this function in order to determine whether
// to use ANGLE instead of a native OpenGL-ES (GLES) driver.
//
// Parameters:
// - rules_handle       (IN) - handle to the rules data structure
// - system_info_handle (IN) - handle to the SystemInfo structure
// - appName            (IN) - Java name of the application (e.g. "com.google.android.apps.maps")
//
// Return value:
// - bool - true if Android should use ANGLE for appName, otherwise false (i.e. use the native GLES
//          driver)
//
ANGLE_EXPORT bool ANGLEUseForAndroidApplication(void* rules_handle,
                                                void* system_info_handle,
                                                const char *appName);


// The following are internal versions supported by the current  feature-support-utility API.

constexpr unsigned int kFeatureVersion_LowestSupported  = 0;
constexpr unsigned int kFeatureVersion_HighestSupported = 1;

#ifdef __cplusplus
}  // extern "C"
#endif

#endif  // FEATURE_SUPPORT_UTIL_H_
