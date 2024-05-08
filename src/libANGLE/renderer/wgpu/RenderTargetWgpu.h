//
// Copyright 2024 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//
// RenderTargetWgpu.h:
//    Defines the class interface for RenderTargetWgpu.
//

#ifndef LIBANGLE_RENDERER_WGPU_RENDERTARGETWGPU_H_
#define LIBANGLE_RENDERER_WGPU_RENDERTARGETWGPU_H_

#include <dawn/webgpu_cpp.h>
#include <stdint.h>

#include "libANGLE/FramebufferAttachment.h"
#include "libANGLE/renderer/wgpu/wgpu_helpers.h"
#include "libANGLE/renderer/wgpu/wgpu_utils.h"

namespace rx
{
class RenderTargetWgpu final : public FramebufferAttachmentRenderTarget
{
  public:
    RenderTargetWgpu();
    ~RenderTargetWgpu() override;

    // Used in std::vector initialization.
    RenderTargetWgpu(RenderTargetWgpu &&other);

    void setTextureView(const wgpu::TextureView &textureView);
    void reset();

    wgpu::TextureView getTexture() { return mTexture; }

  private:
    wgpu::TextureView mTexture;
};
}  // namespace rx

#endif  // LIBANGLE_RENDERER_WGPU_RENDERTARGETWGPU_H_
