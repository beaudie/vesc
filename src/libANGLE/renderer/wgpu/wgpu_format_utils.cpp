//
// Copyright 2024 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "libANGLE/renderer/wgpu/wgpu_format_utils.h"

namespace rx
{
namespace webgpu
{
Format::Format()
    : mIntendedFormatID(angle::FormatID::NONE),
      mIntendedGLFormat(GL_NONE),
      mActualSampleOnlyImageFormatID(angle::FormatID::NONE),
      mActualRenderableImageFormatID(angle::FormatID::NONE),
      mActualBufferFormatID(angle::FormatID::NONE),
      mActualCompressedBufferFormatID(angle::FormatID::NONE),
      mImageInitializerFunction(nullptr),
      mTextureLoadFunctions(),
      mRenderableTextureLoadFunctions(),
      mVertexLoadFunction(nullptr),
      mCompressedVertexLoadFunction(nullptr),
      mVertexLoadRequiresConversion(false),
      mCompressedVertexLoadRequiresConversion(false),
      mIsRenderable(false)
{}
void Format::initImageFallback(const ImageFormatInitInfo *info, int numInfo)
{
    UNIMPLEMENTED();
}

void Format::initBufferFallback(const BufferFormatInitInfo *fallbackInfo,
                                int numInfo,
                                int compressedStartIndex)
{
    UNIMPLEMENTED();
}
}  // namespace webgpu
}  // namespace rx
