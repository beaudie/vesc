//
// Copyright 2015 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//

// platform_utils.h: Global helper methods for dealing with angle::Platform.

#ifndef LIBANGLE_PLATFORM_UTILS_H_
#define LIBANGLE_PLATFORM_UTILS_H_

namespace angle
{
class Platform;

void SetGlobalPlatformImpl(Platform *platformImpl);
void InitDefaultPlatformImpl();
void DeinitDefaultPlatformImpl();

}

#endif // LIBANGLE_PLATFORM_UTILS_H_
