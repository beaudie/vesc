//
// Copyright 2022 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//

#ifndef ANGLE_MEM_STATS_H_
#define ANGLE_MEM_STATS_H_

#include <cstdlib>

#include "platform/PlatformMethods.h"

#ifdef __cplusplus
extern "C" {
#endif
typedef void (*ANGLECollectMemoryUsageStatsCallbackFunc)(void *context,
                                                         const char *category,
                                                         size_t totalBytes,
                                                         size_t numExtraProperties,
                                                         const char *const *extraPropertiesNames,
                                                         const size_t *extraPropertiesValues);
ANGLE_PLATFORM_EXPORT void ANGLECollectMemoryUsageStats(
    ANGLECollectMemoryUsageStatsCallbackFunc callback,
    void *context);
ANGLE_PLATFORM_EXPORT void ANGLECollectBackendMemoryUsageStats(
    ANGLECollectMemoryUsageStatsCallbackFunc callback,
    void *context);

typedef void (*ANGLECollectMemoryUsageStats_Fn)(ANGLECollectMemoryUsageStatsCallbackFunc callback,
                                                void *context);
#ifdef __cplusplus
}
#endif

#endif
