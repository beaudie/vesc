//
// Copyright 2019 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//

#include "libANGLE/renderer/metal/mtl_common.h"

#include <dispatch/dispatch.h>

#include <cstring>

#include "libANGLE/angletypes.h"
#include "libANGLE/renderer/metal/RendererMtl.h"

namespace rx
{
namespace mtl
{

Context::Context(RendererMtl *rendererMtl) : mRendererMtl(rendererMtl) {}

id<MTLDevice> Context::getMetalDevice() const
{
    return mRendererMtl->getMetalDevice();
}

mtl::CommandQueue &Context::cmdQueue()
{
    return mRendererMtl->cmdQueue();
}

}  // namespace mtl
}  // namespace rx