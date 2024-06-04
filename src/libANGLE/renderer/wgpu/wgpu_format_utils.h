//
// Copyright 2024 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//
// wgpu_format_utils:
//   Helper for WebGPU format code.

#ifndef LIBANGLE_RENDERER_WGPU_WGPU_FORMAT_UTILS_H_
#define LIBANGLE_RENDERER_WGPU_WGPU_FORMAT_UTILS_H_

#include <dawn/webgpu_cpp.h>

#include "libANGLE/formatutils.h"
#include "libANGLE/renderer/Format.h"
#include "libANGLE/renderer/copyvertex.h"

namespace rx
{
namespace webgpu
{

struct ImageFormatInitInfo final
{
    angle::FormatID format;
    InitializeTextureDataFunction initializer;
};

struct BufferFormatInitInfo final
{
    angle::FormatID format;
    VertexCopyFunction vertexLoadFunction;
    bool vertexLoadRequiresConversion;
};

wgpu::TextureFormat GetWgpuTextureFormatFromFormatID(angle::FormatID formatID);
angle::FormatID GetFormatIDFromWgpuTextureFormat(wgpu::TextureFormat wgpuFormat);
wgpu::VertexFormat GetWgpuVertexFormatFromFormatID(angle::FormatID formatID);
angle::FormatID GetFormatIDFromWgpuBufferFormat(wgpu::VertexFormat wgpuFormat);

// Describes a WebGPU format. WebGPU has separate formats for images and vertex buffers, this class
// describes both.
class Format final : private angle::NonCopyable
{
  public:
    Format();

    bool valid() const { return mIntendedGLFormat != 0; }

    // The intended format is the front-end format. For Textures this usually correponds to a
    // GLenum in the headers. Buffer formats don't always have a corresponding GLenum type.
    // Some Surface formats and unsized types also don't have a corresponding GLenum.
    angle::FormatID getIntendedFormatID() const { return mIntendedFormatID; }
    const angle::Format &getIntendedFormat() const { return angle::Format::Get(mIntendedFormatID); }

    // The actual Image format is used to implement the front-end format for Texture/Renderbuffers.
    const angle::Format &getActualImageFormat() const
    {
        return angle::Format::Get(getActualImageFormatID());
    }

    angle::FormatID getActualRenderableImageFormatID() const
    {
        return mActualRenderableImageFormatID;
    }
    const angle::Format &getActualRenderableImageFormat() const
    {
        return angle::Format::Get(mActualRenderableImageFormatID);
    }
    wgpu::TextureFormat getActualWgpuTextureFormat() const
    {
        return GetWgpuTextureFormatFromFormatID(mActualRenderableImageFormatID);
    }
    wgpu::VertexFormat getActualWgpuVertexFormat(bool compressed) const
    {
        return GetWgpuVertexFormatFromFormatID(compressed ? mActualCompressedBufferFormatID
                                                          : mActualBufferFormatID);
    }

    angle::FormatID getActualImageFormatID() const
    {
        return mIsRenderable ? mActualRenderableImageFormatID : mActualSampleOnlyImageFormatID;
    }

    LoadImageFunctionInfo getTextureLoadFunction(GLenum type) const
    {
        return mIsRenderable ? mRenderableTextureLoadFunctions(type) : mTextureLoadFunctions(type);
    }

    // The actual Buffer format is used to implement the front-end format for Buffers.  This format
    // is used by vertex buffers as well as texture buffers.
    const angle::Format &getActualBufferFormat(bool compressed) const
    {
        return angle::Format::Get(compressed ? mActualCompressedBufferFormatID
                                             : mActualBufferFormatID);
    }

    VertexCopyFunction getVertexLoadFunction(bool compressed) const
    {
        return compressed ? mCompressedVertexLoadFunction : mVertexLoadFunction;
    }

    bool getVertexLoadRequiresConversion(bool compressed) const
    {
        return compressed ? mCompressedVertexLoadRequiresConversion : mVertexLoadRequiresConversion;
    }

    // |intendedGLFormat| always correponds to a valid GLenum type. For types that don't have a
    // corresponding GLenum we do our best to specify a GLenum that is "close".
    const gl::InternalFormat &getInternalFormatInfo(GLenum type) const
    {
        return gl::GetInternalFormatInfo(mIntendedGLFormat, type);
    }

    bool canCompressBufferData() const
    {
        return mActualCompressedBufferFormatID != angle::FormatID::NONE &&
               mActualBufferFormatID != mActualCompressedBufferFormatID;
    }

  private:
    // This is an auto-generated method in vk_format_table_autogen.cpp.
    void initialize(const angle::Format &intendedAngleFormat);

    // These are used in the format table init.
    void initImageFallback(const ImageFormatInitInfo *info, int numInfo);

    void initBufferFallback(const BufferFormatInitInfo *fallbackInfo,
                            int numInfo,
                            int compressedStartIndex);

    angle::FormatID mIntendedFormatID;
    GLenum mIntendedGLFormat;
    angle::FormatID mActualSampleOnlyImageFormatID;
    angle::FormatID mActualRenderableImageFormatID;
    angle::FormatID mActualBufferFormatID;
    angle::FormatID mActualCompressedBufferFormatID;

    InitializeTextureDataFunction mImageInitializerFunction;
    LoadFunctionMap mTextureLoadFunctions;
    LoadFunctionMap mRenderableTextureLoadFunctions;
    VertexCopyFunction mVertexLoadFunction;
    VertexCopyFunction mCompressedVertexLoadFunction;

    bool mVertexLoadRequiresConversion;
    bool mCompressedVertexLoadRequiresConversion;
    bool mIsRenderable;
};
}  // namespace webgpu
}  // namespace rx

#endif  // LIBANGLE_RENDERER_WGPU_WGPU_FORMAT_UTILS_H_
