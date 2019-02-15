//
// Copyright 2016 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//
// ShaderVk.h:
//    Defines the class interface for ShaderVk, implementing ShaderImpl.
//

#ifndef LIBANGLE_RENDERER_VULKAN_SHADERVK_H_
#define LIBANGLE_RENDERER_VULKAN_SHADERVK_H_

#include "libANGLE/renderer/ShaderImpl.h"

namespace rx
{

class ShaderVk : public ShaderImpl
{
  public:
    ShaderVk(const gl::ShaderState &data);
    ~ShaderVk() override;

    std::shared_ptr<angle::WaitableEvent> compile(
        gl::TranslateTaskConstructor &&translateTaskConstructor,
        gl::PostTranslateFunctor &&postTranslateFunctor,
        const gl::Context *context) override;

    std::string getDebugInfo() const override;
};

}  // namespace rx

#endif  // LIBANGLE_RENDERER_VULKAN_SHADERVK_H_
