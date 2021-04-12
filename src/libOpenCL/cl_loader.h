//
// Copyright 2021 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//
// cl_loader.h:
//   Simple CL function loader.

#ifndef LIBCL_CL_LOADER_H_
#define LIBCL_CL_LOADER_H_

#include <export.h>

#ifndef CL_API_ENTRY
#    define CL_API_ENTRY ANGLE_EXPORT
#endif
#include "angle_cl.h"

#ifdef _WIN32

// 'cl_icd.h' includes D3D headers, which require to include 'windows.h' first
#    ifndef STRICT
#        define STRICT 1
#    endif
#    ifndef WIN32_LEAN_AND_MEAN
#        define WIN32_LEAN_AND_MEAN 1
#    endif
#    ifndef NOMINMAX
#        define NOMINMAX 1
#    endif
#    include <windows.h>

// Clearing macro FAR, because at this point 'minwindef.h' defines 'FAR' as 'far',
// which gets undefined somehow and causes compilation of D3D headers to fail.
#    undef FAR
#    define FAR

#endif

// 'angle_cl.h' has to be included before this to enable CL defines
#include "CL/cl_icd.h"

ANGLE_NO_EXPORT extern cl_icd_dispatch cl_loader;

namespace angle
{
using GenericProc = void (*)();
using LoadProc    = GenericProc(CL_API_CALL *)(const char *);
ANGLE_NO_EXPORT void LoadCL(LoadProc loadProc);
}  // namespace angle

#endif  // LIBCL_CL_LOADER_H_
