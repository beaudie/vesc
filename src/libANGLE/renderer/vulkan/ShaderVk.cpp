//
// Copyright 2016 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//
// ShaderVk.cpp:
//    Implements the class methods for ShaderVk.
//

#include "libANGLE/renderer/vulkan/ShaderVk.h"

#include "common/debug.h"
#include "libANGLE/Context.h"
#include "libANGLE/renderer/vulkan/ContextVk.h"
#include "platform/FeaturesVk.h"

namespace rx
{

ShaderVk::ShaderVk(const gl::ShaderState &data) : ShaderImpl(data) {}

ShaderVk::~ShaderVk() {}

std::shared_ptr<angle::WaitableEvent> ShaderVk::compile(
    gl::TranslateTaskConstructor &&translateTaskConstructor,
    gl::PostTranslateFunctor &&postTranslateFunctor,
    const gl::Context *context)
{
    std::stringstream shaderSourceStream;
    shaderSourceStream << mData.getSource();

    ShCompileOptions compileOptions = SH_INITIALIZE_UNINITIALIZED_LOCALS;

    ContextVk *contextVk = vk::GetImpl(context);

    if (contextVk->getFeatures().clampPointSize)
    {
        compileOptions |= SH_CLAMP_POINT_SIZE;
    }

    return compileImpl(std::move(translateTaskConstructor), std::move(postTranslateFunctor),
                       context, std::move(shaderSourceStream), compileOptions);
}

std::string ShaderVk::getDebugInfo() const
{
    return mData.getTranslatedSource();
}

}  // namespace rx
