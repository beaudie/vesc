//
// Copyright 2021 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//
// CLExtensions.h: Defines the rx::CLExtensions struct.

#ifndef LIBANGLE_RENDERER_CLEXTENSIONS_H_
#define LIBANGLE_RENDERER_CLEXTENSIONS_H_

#include "libANGLE/renderer/CLtypes.h"

namespace rx
{

struct CLExtensions
{
    CLExtensions();
    ~CLExtensions();

    CLExtensions(const CLExtensions &) = delete;
    CLExtensions &operator=(const CLExtensions &) = delete;

    CLExtensions(CLExtensions &&);
    CLExtensions &operator=(CLExtensions &&);

    void initializeExtensions(std::string &&extensionStr);

    std::string extensions;

    // cl_khr_icd
    bool khrICD = false;

    // cl_khr_fp64
    bool khrFP64 = false;
};

}  // namespace rx

#endif  // LIBANGLE_RENDERER_CLEXTENSIONS_H_
