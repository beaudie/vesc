//
// Copyright 2021 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//
// entry_points_cl_icd.cpp:
//   Defines the CL entry points required for extension cl_khr_icd.

#include "entry_points_cl_autogen.h"

extern "C" {

cl_int CL_API_CALL clIcdGetPlatformIDsKHR(cl_uint num_entries,
                                          cl_platform_id *platforms,
                                          cl_uint *num_platforms)
{
    return CL_GetPlatformIDs(num_entries, platforms, num_platforms);
}

cl_int CL_API_CALL clGetPlatformInfo(cl_platform_id platform,
                                     cl_platform_info param_name,
                                     size_t param_value_size,
                                     void *param_value,
                                     size_t *param_value_size_ret)
{
    return CL_GetPlatformInfo(platform, param_name, param_value_size, param_value,
                              param_value_size_ret);
}

void *CL_API_CALL clGetExtensionFunctionAddress(const char *func_name)
{
    return CL_GetExtensionFunctionAddress(func_name);
}

}  // extern "C"
