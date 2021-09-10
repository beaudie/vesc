//
// Copyright 2016 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//
// SamplerVk.cpp:
//    Implements the class methods for SamplerVk.
//

#include "libANGLE/renderer/vulkan/SamplerVk.h"

#include "common/debug.h"
#include "libANGLE/Context.h"
#include "libANGLE/renderer/vulkan/vk_utils.h"

namespace rx
{

SamplerVk::SamplerVk(const gl::SamplerState &state) : SamplerImpl(state) {}

SamplerVk::~SamplerVk() = default;

void SamplerVk::onDestroy(const gl::Context *context)
{
    mSampler.reset();
}

angle::Result SamplerVk::syncState(const gl::Context *context, const bool dirty)
{
    ContextVk *contextVk = vk::GetImpl(context);

#if !SVDT_ENABLE_VULKAN_CACHES_RACE_CONDITION_FIX
    RendererVk *renderer = contextVk->getRenderer();
#endif
    if (mSampler.valid())
    {
        if (!dirty)
        {
            return angle::Result::Continue;
        }
        mSampler.reset();
    }

    vk::SamplerDesc desc(contextVk, mState, false, 0, static_cast<angle::FormatID>(0));
#if SVDT_ENABLE_VULKAN_CACHES_RACE_CONDITION_FIX
    ANGLE_TRY(contextVk->getSamplerCache().getSampler(contextVk, desc, &mSampler));
#else
    ANGLE_TRY(renderer->getSamplerCache().getSampler(contextVk, desc, &mSampler));
#endif

    return angle::Result::Continue;
}

}  // namespace rx
