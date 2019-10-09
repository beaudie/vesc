//
// Copyright (c) 2019 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//
// Metal_platform.h:
//    This file includes required Apple framework headers for Metal backend to compile.
//    Also it defines conditional flags based on compiler settings, target platform
//    (iOS or macos) and so on.
//

#ifndef LIBANGLE_RENDERER_METAL_METAL_PLATFORM_H_
#define LIBANGLE_RENDERER_METAL_METAL_PLATFORM_H_

#import <Metal/Metal.h>

// Xcode SDK contains FixedToFloat macro, it conflicts with FixedToFloat() function
// defined in common/mathutil.h
#ifdef FixedToFloat
#    define FixedToFloat_Backup FixedToFloat
#    undef FixedToFloat
#endif

#ifdef FloatToFixed
#    define FloatToFixed_Backup FloatToFixed
#    undef FloatToFixed
#endif

#include "common/mathutil.h"

#endif /* LIBANGLE_RENDERER_METAL_METAL_PLATFORM_H_ */
