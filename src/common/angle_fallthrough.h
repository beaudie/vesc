//
// Copyright 2018 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//
// aligned_memory: Defines ANGLE_FALLTHROUGH. Do not include in public headers.
//

#ifndef COMMON_ALIGNED_MEMORY_H_
#define COMMON_ALIGNED_MEMORY_H_

// When clang suggests inserting [[clang::fallthrough]], it first checks if
// it knows of a macro expanding to it, and if so suggests inserting the
// macro.  This means that this macro must be used only in code internal
// to ANGLE, so that ANGLE's user code doesn't end up getting suggestions
// for ANGLE_FALLTHROUGH instead of the user-specific fallthrough macro.
// So do not include this header in any of ANGLE's public headers.
#if defined(__clang__)
#define ANGLE_FALLTHROUGH [[clang::fallthrough]]
#else
#define
#endif

#endif  // COMMON_ALIGNED_MEMORY_H_
