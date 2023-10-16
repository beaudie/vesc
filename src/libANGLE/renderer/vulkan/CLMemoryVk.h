//
// Copyright 2021 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//
// CLMemoryVk.h: Defines the class interface for CLMemoryVk, implementing CLMemoryImpl.

#ifndef LIBANGLE_RENDERER_VULKAN_CLMEMORYVK_H_
#define LIBANGLE_RENDERER_VULKAN_CLMEMORYVK_H_

#include "libANGLE/renderer/vulkan/cl_types.h"

#include "libANGLE/renderer/CLMemoryImpl.h"

namespace rx
{

class CLMemoryVk : public CLMemoryImpl
{
  public:
    CLMemoryVk(const cl::Memory &memory);
    ~CLMemoryVk() override;

    size_t getSize(cl_int &errorCode) const override;

    CLMemoryImpl::Ptr createSubBuffer(const cl::Buffer &buffer,
                                      cl::MemFlags flags,
                                      size_t size,
                                      cl_int &errorCode) override;
};

}  // namespace rx

#endif  // LIBANGLE_RENDERER_VULKAN_CLMEMORYVK_H_
