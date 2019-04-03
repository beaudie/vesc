// Copyright 2019 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//
// MemoryObjectVkFd.cpp: Defines the class interface for MemoryObjectVkFd, implementing
// MemoryObjectImpl.

#include "libANGLE/renderer/vulkan/linux/MemoryObjectVkFd.h"

#include <unistd.h>
#include <vulkan/vulkan.h>

#include "common/debug.h"
#include "libANGLE/Context.h"
#include "libANGLE/renderer/vulkan/ContextVk.h"
#include "libANGLE/renderer/vulkan/RendererVk.h"

namespace rx
{

MemoryObjectVkFd::MemoryObjectVkFd() : mSize(0), mFd(-1) {}

MemoryObjectVkFd::~MemoryObjectVkFd() = default;

void MemoryObjectVkFd::onDestroy(const gl::Context *context)
{
    if (mFd != -1)
    {
        close(mFd);
        mFd = -1;
    }
}

angle::Result MemoryObjectVkFd::importMemoryFd(gl::Context *context,
                                               GLuint64 size,
                                               GLenum handleType,
                                               GLint fd)
{
    switch (handleType)
    {
        case GL_HANDLE_TYPE_OPAQUE_FD_EXT:
            return importMemoryOpaqueFd(context, size, fd);

        default:
            UNREACHABLE();
            return angle::Result::Stop;
    }
}

angle::Result MemoryObjectVkFd::importMemoryOpaqueFd(gl::Context *context, GLuint64 size, GLint fd)
{
    ASSERT(mFd == -1);
    mFd   = fd;
    mSize = size;
    return angle::Result::Continue;
}

}  // namespace rx
