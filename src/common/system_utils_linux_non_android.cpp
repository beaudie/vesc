//
// Copyright 2021 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//

// system_utils_android.cpp: Implementation of OS-specific functions for Linux (excluding Android)

#include "system_utils.h"

namespace angle
{
void AddTraceEvent(char phase,
                   const unsigned char *categoryEnabledFlag,
                   const char *name,
                   unsigned long long id,
                   double timestamp,
                   int numArgs,
                   const char **argNames,
                   const unsigned char *argTypes,
                   const unsigned long long *argValues,
                   unsigned char flags)
{
    // No platform integration.
}

}  // namespace angle
