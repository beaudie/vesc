//
// Copyright 2021 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//

// validationCL.h: Validation functions for generic CL entry point parameters

#ifndef LIBANGLE_VALIDATIONCL_H_
#define LIBANGLE_VALIDATIONCL_H_

#include "libANGLE/CLCommandQueue.h"
#include "libANGLE/CLContext.h"
#include "libANGLE/CLDevice.h"
#include "libANGLE/CLEvent.h"
#include "libANGLE/CLKernel.h"
#include "libANGLE/CLMemory.h"
#include "libANGLE/CLPlatform.h"
#include "libANGLE/CLProgram.h"
#include "libANGLE/CLSampler.h"
#include "libANGLE/entry_points_utils.h"

#include "common/PackedCLEnums_autogen.h"

namespace cl
{
// A template struct for determining the default value to return for each entry point.
template <angle::EntryPoint EP, typename ReturnType>
struct DefaultReturnValue
{
    static constexpr ReturnType kValue = static_cast<ReturnType>(0);
};

template <angle::EntryPoint EP, typename ReturnType>
ReturnType GetDefaultReturnValue();

template <angle::EntryPoint EP, typename ReturnType>
ReturnType GetDefaultReturnValue()
{
    return DefaultReturnValue<EP, ReturnType>::kValue;
}
}  // namespace cl

#define ANGLE_CL_VALIDATE(EP, RETURN_TYPE, ...)                                     \
    do                                                                              \
    {                                                                               \
        if (!Validate##EP(__VA_ARGS__))                                             \
        {                                                                           \
            return GetDefaultReturnValue<angle::EntryPoint::CL##EP, RETURN_TYPE>(); \
        }                                                                           \
    } while (0)

#define ANGLE_CL_VALIDATE_VOID(EP, ...) \
    do                                  \
    {                                   \
        if (!Validate##EP(__VA_ARGS__)) \
        {                               \
            return;                     \
        }                               \
    } while (0)

#endif  // LIBANGLE_VALIDATIONCL_H_
