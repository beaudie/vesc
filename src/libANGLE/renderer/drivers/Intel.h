//
// Copyright (c) 2016 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//

// Intel.h : provides platform information on Intel GPU

#ifndef LIBANGLE_RENDERER_DRIVERS_INTEL_H_
#define LIBANGLE_RENDERER_DRIVERS_INTEL_H_

#include "libANGLE/angletypes.h"

namespace rx
{

bool IsHaswell(uint32_t DeviceId);
bool IsBroadwell(uint32_t DeviceId);
bool IsCherryView(uint32_t DeviceId);
bool IsSkylake(uint32_t DeviceId);
bool IsBroxton(uint32_t DeviceId);
bool IsKabylake(uint32_t DeviceId);

}  // namespace rx
#endif  // LIBANGLE_RENDERER_DRIVERS_INTEL_H_