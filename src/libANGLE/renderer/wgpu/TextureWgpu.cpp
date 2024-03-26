//
// Copyright 2024 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//
// TextureWgpu.cpp:
//    Implements the class methods for TextureWgpu.
//

#include "libANGLE/renderer/wgpu/TextureWgpu.h"

#include "common/debug.h"
#include "libANGLE/angletypes.h"
#include "libANGLE/renderer/wgpu/ContextWgpu.h"
#include "libANGLE/renderer/wgpu/DisplayWgpu.h"

namespace rx
{

TextureWgpu::TextureWgpu(const gl::TextureState &state) : TextureImpl(state) {}

TextureWgpu::~TextureWgpu() {}

angle::Result TextureWgpu::setImage(const gl::Context *context,
                                    const gl::ImageIndex &index,
                                    GLenum internalFormat,
                                    const gl::Extents &size,
                                    GLenum format,
                                    GLenum type,
                                    const gl::PixelUnpackState &unpack,
                                    gl::Buffer *unpackBuffer,
                                    const uint8_t *pixels)
{
    return setImageImpl(context, index, size);
}

angle::Result TextureWgpu::setSubImage(const gl::Context *context,
                                       const gl::ImageIndex &index,
                                       const gl::Box &area,
                                       GLenum format,
                                       GLenum type,
                                       const gl::PixelUnpackState &unpack,
                                       gl::Buffer *unpackBuffer,
                                       const uint8_t *pixels)
{
    if (mImage == nullptr)
    {
        ANGLE_TRY(initializeImage(context, index, gl::Extents(area.x, area.y, area.z)));
    }
    return setSubImageImpl(context, index, area);
}

angle::Result TextureWgpu::setCompressedImage(const gl::Context *context,
                                              const gl::ImageIndex &index,
                                              GLenum internalFormat,
                                              const gl::Extents &size,
                                              const gl::PixelUnpackState &unpack,
                                              size_t imageSize,
                                              const uint8_t *pixels)
{
    return angle::Result::Continue;
}

angle::Result TextureWgpu::setCompressedSubImage(const gl::Context *context,
                                                 const gl::ImageIndex &index,
                                                 const gl::Box &area,
                                                 GLenum format,
                                                 const gl::PixelUnpackState &unpack,
                                                 size_t imageSize,
                                                 const uint8_t *pixels)
{
    return angle::Result::Continue;
}

angle::Result TextureWgpu::copyImage(const gl::Context *context,
                                     const gl::ImageIndex &index,
                                     const gl::Rectangle &sourceArea,
                                     GLenum internalFormat,
                                     gl::Framebuffer *source)
{
    return angle::Result::Continue;
}

angle::Result TextureWgpu::copySubImage(const gl::Context *context,
                                        const gl::ImageIndex &index,
                                        const gl::Offset &destOffset,
                                        const gl::Rectangle &sourceArea,
                                        gl::Framebuffer *source)
{
    return angle::Result::Continue;
}

angle::Result TextureWgpu::copyTexture(const gl::Context *context,
                                       const gl::ImageIndex &index,
                                       GLenum internalFormat,
                                       GLenum type,
                                       GLint sourceLevel,
                                       bool unpackFlipY,
                                       bool unpackPremultiplyAlpha,
                                       bool unpackUnmultiplyAlpha,
                                       const gl::Texture *source)
{
    return angle::Result::Continue;
}

angle::Result TextureWgpu::copySubTexture(const gl::Context *context,
                                          const gl::ImageIndex &index,
                                          const gl::Offset &destOffset,
                                          GLint sourceLevel,
                                          const gl::Box &sourceBox,
                                          bool unpackFlipY,
                                          bool unpackPremultiplyAlpha,
                                          bool unpackUnmultiplyAlpha,
                                          const gl::Texture *source)
{
    return angle::Result::Continue;
}

angle::Result TextureWgpu::copyRenderbufferSubData(const gl::Context *context,
                                                   const gl::Renderbuffer *srcBuffer,
                                                   GLint srcLevel,
                                                   GLint srcX,
                                                   GLint srcY,
                                                   GLint srcZ,
                                                   GLint dstLevel,
                                                   GLint dstX,
                                                   GLint dstY,
                                                   GLint dstZ,
                                                   GLsizei srcWidth,
                                                   GLsizei srcHeight,
                                                   GLsizei srcDepth)
{
    return angle::Result::Continue;
}

angle::Result TextureWgpu::copyTextureSubData(const gl::Context *context,
                                              const gl::Texture *srcTexture,
                                              GLint srcLevel,
                                              GLint srcX,
                                              GLint srcY,
                                              GLint srcZ,
                                              GLint dstLevel,
                                              GLint dstX,
                                              GLint dstY,
                                              GLint dstZ,
                                              GLsizei srcWidth,
                                              GLsizei srcHeight,
                                              GLsizei srcDepth)
{
    return angle::Result::Continue;
}

angle::Result TextureWgpu::copyCompressedTexture(const gl::Context *context,
                                                 const gl::Texture *source)
{
    return angle::Result::Continue;
}

angle::Result TextureWgpu::setStorage(const gl::Context *context,
                                      gl::TextureType type,
                                      size_t levels,
                                      GLenum internalFormat,
                                      const gl::Extents &size)
{
    return angle::Result::Continue;
}

angle::Result TextureWgpu::setStorageExternalMemory(const gl::Context *context,
                                                    gl::TextureType type,
                                                    size_t levels,
                                                    GLenum internalFormat,
                                                    const gl::Extents &size,
                                                    gl::MemoryObject *memoryObject,
                                                    GLuint64 offset,
                                                    GLbitfield createFlags,
                                                    GLbitfield usageFlags,
                                                    const void *imageCreateInfoPNext)
{
    return angle::Result::Continue;
}

angle::Result TextureWgpu::setEGLImageTarget(const gl::Context *context,
                                             gl::TextureType type,
                                             egl::Image *image)
{
    return angle::Result::Continue;
}

angle::Result TextureWgpu::setImageExternal(const gl::Context *context,
                                            gl::TextureType type,
                                            egl::Stream *stream,
                                            const egl::Stream::GLTextureDescription &desc)
{
    return angle::Result::Continue;
}

angle::Result TextureWgpu::generateMipmap(const gl::Context *context)
{
    return angle::Result::Continue;
}

angle::Result TextureWgpu::setBaseLevel(const gl::Context *context, GLuint baseLevel)
{
    return angle::Result::Continue;
}

angle::Result TextureWgpu::bindTexImage(const gl::Context *context, egl::Surface *surface)
{
    return angle::Result::Continue;
}

angle::Result TextureWgpu::releaseTexImage(const gl::Context *context)
{
    return angle::Result::Continue;
}

angle::Result TextureWgpu::syncState(const gl::Context *context,
                                     const gl::Texture::DirtyBits &dirtyBits,
                                     gl::Command source)
{
    return angle::Result::Continue;
}

angle::Result TextureWgpu::setStorageMultisample(const gl::Context *context,
                                                 gl::TextureType type,
                                                 GLsizei samples,
                                                 GLint internalformat,
                                                 const gl::Extents &size,
                                                 bool fixedSampleLocations)
{
    return angle::Result::Continue;
}

angle::Result TextureWgpu::initializeContents(const gl::Context *context,
                                              GLenum binding,
                                              const gl::ImageIndex &imageIndex)
{
    return angle::Result::Continue;
}

angle::Result TextureWgpu::setImageImpl(const gl::Context *context,
                                        const gl::ImageIndex &index,
                                        const gl::Extents &size)
{
    ANGLE_TRY(redefineLevel(context, index, size));
    return setSubImageImpl(context, index, gl::Box(gl::kOffsetZero, size));
}

angle::Result TextureWgpu::setSubImageImpl(const gl::Context *context,
                                           const gl::ImageIndex &index,
                                           const gl::Box &area)
{

    ContextWgpu *contextWgpu = GetImplAs<ContextWgpu>(context);
    DisplayWgpu *displayWgpu = contextWgpu->getDisplay();

    webgpu::TextureInfo textureInfo           = mImage->getWgpuTextureInfo(index);
    wgpu::TextureDescriptor textureDescriptor = mImage->createTextureDescriptor(
        textureInfo.usage, textureInfo.dimension,
        gl_wgpu::getExtent3D(gl::Extents(area.x, area.y, area.z)), wgpu::TextureFormat::RGBA8Sint,
        textureInfo.mipLevelCount, 1, 0);
    ANGLE_TRY(mImage->stageTextureUpload(displayWgpu->getDevice(), index,
                                         gl::Offset(area.x, area.y, area.z), textureDescriptor));
    mImage->flushStagedUpdates(displayWgpu->getDevice());
    return angle::Result::Continue;
}

angle::Result TextureWgpu::initializeImage(const gl::Context *context,
                                           const gl::ImageIndex &index,
                                           const gl::Extents &size)
{
    ContextWgpu *contextWgpu        = GetImplAs<ContextWgpu>(context);
    DisplayWgpu *displayWgpu        = contextWgpu->getDisplay();
    mImage                          = new webgpu::ImageHelper();
    webgpu::TextureInfo textureInfo = mImage->getWgpuTextureInfo(index);
    return mImage->initImage(
        displayWgpu->getDevice(), gl::LevelIndex(index.getLevelIndex()),
        mImage->createTextureDescriptor(textureInfo.usage, textureInfo.dimension,
                                        gl_wgpu::getExtent3D(size), wgpu::TextureFormat::RGBA8Sint,
                                        textureInfo.mipLevelCount, 1, 0));
}

angle::Result TextureWgpu::redefineLevel(const gl::Context *context,
                                         const gl::ImageIndex &index,
                                         const gl::Extents &size)
{
    gl::LevelIndex levelIndexGL(index.getLevelIndex());
    if (mImage && levelIndexGL >= mImage->getFirstAllocatedLevel() &&
        levelIndexGL <
            (mImage->getFirstAllocatedLevel() + mImage->getTextureDescriptor().mipLevelCount))
    {
        bool dimensionChanged = mImage->getTextureDescriptor().dimension !=
                                gl_wgpu::getWgpuTextureDimension(index.getType());
        if (dimensionChanged || size != wgpu_gl::getExtents(mImage->getTextureDescriptor().size))
        {
            mImage = nullptr;
        }
    }

    if (size.empty())
    {
        return angle::Result::Continue;
    }
    return initializeImage(context, index, size);
}

}  // namespace rx
