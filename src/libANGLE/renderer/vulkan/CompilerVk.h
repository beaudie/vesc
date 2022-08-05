//
// Copyright 2016 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//
// CompilerVk.h:
//    Defines the class interface for CompilerVk, implementing CompilerImpl.
//

#ifndef LIBANGLE_RENDERER_VULKAN_COMPILERVK_H_
#define LIBANGLE_RENDERER_VULKAN_COMPILERVK_H_

#include "libANGLE/renderer/CompilerImpl.h"

namespace rx
{
class ContextVk;

class CompilerVk : public CompilerImpl
{
  public:
    CompilerVk(const ContextVk *);
    ~CompilerVk() override;

    // TODO(jmadill): Expose translator built-in resources init method.
    ShShaderOutput getTranslatorOutputType() const override;
    CompilerBackendFeatures getBackendFeatures() const override;

  private:
    CompilerBackendFeatures mBackendFeatures;
};

}  // namespace rx

#endif  // LIBANGLE_RENDERER_VULKAN_COMPILERVK_H_
