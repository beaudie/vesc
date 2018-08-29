//
// Copyright (c) 2018 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//

// OptIn.cpp: Helps Android EGL loader to determine whether to use ANGLE or a native GLES driver.

#ifndef OPT_IN_H_
#define OPT_IN_H_

#include <export.h>

namespace angle
{

ANGLE_EXPORT bool CanAppChooseANGLE(const char *app_name,
                                    const char *device_mfr,
                                    const char *device_model);

ANGLE_EXPORT bool ShouldAppUseANGLE(const char *app_name,
                                    const char *device_mfr,
                                    const char *device_model);

}  // namespace angle

#endif  // OPT_IN_H_
