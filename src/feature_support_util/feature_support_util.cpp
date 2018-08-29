//
// Copyright (c) 2018 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//

// OptIn.cpp: Implementation of the code that helps the Android EGL loader
// determine whether to use ANGLE or a native GLES driver.

#include "feature_support_util.h"

#ifdef __cplusplus
extern "C" {
#endif

ANGLE_EXPORT bool ANGLEUseForApplication(const char *app_name,
                                         // TODO: Add app_version and app_intent
                                         const char *device_mfr,
                                         const char *device_model,
                                         ANGLEPreference dev_opt,
                                         ANGLEPreference app_pref)
{
    if (dev_opt != ANGLE_NO_PREFERENCE)
    {
        return (dev_opt == ANGLE_PREFER_ANGLE);
    }
    else if ((app_pref != ANGLE_NO_PREFERENCE) && false /*rules allow app to choose*/)
    {
        return (app_pref == ANGLE_PREFER_ANGLE);
    }
    else
    {
        return false /*whatever the rules come up with*/;
    }
}

#ifdef __cplusplus
}  // extern "C"
#endif
