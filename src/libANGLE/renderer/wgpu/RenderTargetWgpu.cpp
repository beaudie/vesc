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

RenderTargetWgpu::RenderTargetWgpu(RenderTargetWgpu &&other)
    : mImageHelper(other.mImageHelper), mTextureView(std::move(other.mTextureView))
{}

void RenderTargetWgpu::set(webgpu::ImageHelper *imageHelper, const wgpu::TextureView &textureView)
{
    mImageHelper = imageHelper;
    mTextureView = textureView;
}

void RenderTargetWgpu::reset()
{
    mImageHelper = nullptr;
    mTextureView = nullptr;
}
}  // namespace rx
