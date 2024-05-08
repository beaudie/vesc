//
// Copyright 2024 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//
// RenderTargetWgpu.cpp:
//    Implements the class methods for RenderTargetWgpu.
//

#include "libANGLE/renderer/wgpu/RenderTargetWgpu.h"

namespace rx
{
RenderTargetWgpu::RenderTargetWgpu() {}

RenderTargetWgpu::~RenderTargetWgpu()
{
    reset();
}

RenderTargetWgpu::RenderTargetWgpu(RenderTargetWgpu &&other) : mTexture(std::move(other.mTexture))
{}

void RenderTargetWgpu::setTextureView(const wgpu::TextureView &textureView)
{
    mTexture = textureView;
}

void RenderTargetWgpu::reset()
{
    mTexture = nullptr;
}
}  // namespace rx
