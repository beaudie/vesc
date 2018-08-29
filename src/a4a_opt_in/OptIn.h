//
// Copyright (c) 2018 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//

// OptIn.cpp: Helps Android EGL loader to determine whether to use ANGLE or a native GLES driver.

#ifndef OPT_IN_H_
#define OPT_IN_H_

#include "export.h"

#define LIBANGLE_UTIL_IMPLEMENTATION

#ifdef __cplusplus
extern "C" {
#endif

typedef enum ANGLEPreference {
    ANGLE_NO_PREFERENCE = 0,
    ANGLE_PREFER_NATIVE = 1,
    ANGLE_PREFER_ANGLE  = 2,
} ANGLEPreference;

ANGLE_EXPORT bool ANGLEUseForApplication(const char *app_name,
                                         // TODO: Add app_version and app_intent
                                         const char *device_mfr,
                                         const char *device_model,
                                         ANGLEPreference dev_opt,
                                         ANGLEPreference app_pref);

#ifdef __cplusplus
}  // extern "C"
#endif

#endif  // OPT_IN_H_
