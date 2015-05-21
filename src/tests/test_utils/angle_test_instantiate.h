//
// Copyright (c) 2014 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//

// angle_test_instantiate.h: Facilities to add parametrized tests filter
// by platform so that test not supported are not registered.

#ifndef ANGLE_TEST_INSTANTIATE_H_
#define ANGLE_TEST_INSTANTIATE_H_

#include "common/debug.h"

namespace angle
{

// This functions is used to filter which tests should be registered,
// internally it calls GetTestParamRenderer(T) that should be implemented
// or new test parameter types.
template<typename T>
inline std::vector<T> FilterTestParams(const T *params, size_t numParams)
{
    std::vector<T> filtered;

    for (size_t i = 0; i < numParams; i++)
    {
        switch (GetTestParamRenderer(params[i]))
        {
          case EGL_PLATFORM_ANGLE_TYPE_DEFAULT_ANGLE:
            filtered.push_back(params[i]);
            break;

          case EGL_PLATFORM_ANGLE_TYPE_D3D9_ANGLE:
#if defined(ANGLE_ENABLE_D3D9)
            filtered.push_back(params[i]);
#endif
            break;

          case EGL_PLATFORM_ANGLE_TYPE_D3D11_ANGLE:
#if defined(ANGLE_ENABLE_D3D11)
            filtered.push_back(params[i]);
#endif
            break;

          case EGL_PLATFORM_ANGLE_TYPE_OPENGL_ANGLE:
          case EGL_PLATFORM_ANGLE_TYPE_OPENGLES_ANGLE:
#if defined(ANGLE_ENABLE_OPENGL)
            filtered.push_back(params[i]);
#endif
            break;

          default:
            UNREACHABLE();
            break;
        }
    }

    return filtered;
}

namespace detail {

// Used with decltype to get the type of the first argument.
template<typename T, typename ... Args>
T ReturnFirst(T first, Args...)
{
    return first;
}

} // namespace detail

// Instantiate the test once for each extra argument. The types of all the
// arguments must match, and GetTestParamRenderer must be implemented for
// that type.
#define ANGLE_INSTANTIATE_TEST(testName, ...) \
    const decltype(::angle::detail::ReturnFirst(__VA_ARGS__)) testName##params[] = {__VA_ARGS__}; \
    INSTANTIATE_TEST_CASE_P(, testName, testing::ValuesIn(::angle::FilterTestParams(testName##params, ArraySize(testName##params))));

} // namespace angle

#endif // ANGLE_TEST_INSTANTIATE_H_
