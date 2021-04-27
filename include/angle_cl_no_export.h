//
// Copyright 2021 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//
// angle_cl_no_export.h:
//   Includes all necessary CL headers and definitions for ANGLE without exporting the entry points.

#ifndef ANGLE_CL_NO_EXPORT_H_
#define ANGLE_CL_NO_EXPORT_H_

#include "export.h"

#ifndef CL_API_ENTRY
#    define CL_API_ENTRY ANGLE_NO_EXPORT
#endif
#include "angle_cl.h"

#endif  // ANGLE_CL_NO_EXPORT_H_
