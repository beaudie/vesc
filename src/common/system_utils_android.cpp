//
// Copyright 2021 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//

// system_utils_android.cpp: Implementation of OS-specific functions for Android

#include "debug.h"
#include "system_utils.h"

#include <android/trace.h>

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
    switch (phase)
    {
        case 'B':
        {
            ATrace_beginSection(name);
            break;
        }
        case 'E':
        {
            ATrace_endSection();
            break;
        }
        case 'I':
        {
            ATrace_beginSection(name);
            ATrace_endSection();
            break;
        }
        default:
            // Handle other event types if necessary.
            break;
    }
}

}  // namespace angle
