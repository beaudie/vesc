//
// Copyright 2021 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//
// CLMemoryVk.cpp: Implements the class methods for CLMemoryVk.

#include "libANGLE/renderer/vulkan/CLMemoryVk.h"

namespace rx
{

CLMemoryVk::CLMemoryVk(const cl::Memory &memory) : CLMemoryImpl(memory) {}

CLMemoryVk::~CLMemoryVk() = default;

size_t CLMemoryVk::getSize(cl_int &errorCode) const
{
    UNIMPLEMENTED();
    // TODO(annestrand) Placeholder error for now
    errorCode = CL_OUT_OF_RESOURCES;
    return 0;
}

CLMemoryImpl::Ptr CLMemoryVk::createSubBuffer(const cl::Buffer &buffer,
                                              cl::MemFlags flags,
                                              size_t size,
                                              cl_int &errorCode)
{
    UNIMPLEMENTED();
    // TODO(annestrand) Placeholder error for now
    errorCode = CL_OUT_OF_RESOURCES;
    return CLMemoryImpl::Ptr{};
}

}  // namespace rx
