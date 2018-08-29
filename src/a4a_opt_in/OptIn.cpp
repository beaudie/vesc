//
// Copyright (c) 2018 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//

// OptIn.cpp: Implementation of the code that helps the Android EGL loader
// determine whether to use ANGLE or a native GLES driver.

#include "OptIn.h"

namespace angle
{

ANGLE_EXPORT bool CanAppChooseANGLE(const char *app_name,
                                    const char *device_mfr,
                                    const char *device_model)
{
    return false;
}

ANGLE_EXPORT bool ShouldAppUseANGLE(const char *app_name,
                                    const char *device_mfr,
                                    const char *device_model)
{
    return false;
}

}  // namespace angle
