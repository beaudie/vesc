//
// Copyright 2015 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//

// TextureImpl_mock.h: Defines a mock of the TextureImpl class.

#ifndef LIBANGLE_RENDERER_TEXTUREIMPLMOCK_H_
#define LIBANGLE_RENDERER_TEXTUREIMPLMOCK_H_

#include "gmock/gmock.h"

#include "libANGLE/renderer/TextureImpl.h"

namespace rx
{

class MockTextureImpl : public TextureImpl
{
  public:
    MockTextureImpl() : TextureImpl(mMockState), mMockState(gl::TextureType::_2D) {}
    virtual ~MockTextureImpl() { destructor(); }
    MOCK_METHOD(angle::Result,
                setImage,
                (const gl::Context *,
                 const gl::ImageIndex &,
                 GLenum,
                 const gl::Extents &,
                 GLenum,
                 GLenum,
                 const gl::PixelUnpackState &,
                 gl::Buffer *,
                 const uint8_t *),
                (override));
    MOCK_METHOD(angle::Result,
                setSubImage,
                (const gl::Context *,
                 const gl::ImageIndex &,
                 const gl::Box &,
                 GLenum,
                 GLenum,
                 const gl::PixelUnpackState &,
                 gl::Buffer *,
                 const uint8_t *),
                (override));
    MOCK_METHOD(angle::Result,
                setCompressedImage,
                (const gl::Context *,
                 const gl::ImageIndex &,
                 GLenum,
                 const gl::Extents &,
                 const gl::PixelUnpackState &,
                 size_t,
                 const uint8_t *),
                (override));
    MOCK_METHOD(angle::Result,
                setCompressedSubImage,
                (const gl::Context *,
                 const gl::ImageIndex &,
                 const gl::Box &,
                 GLenum,
                 const gl::PixelUnpackState &,
                 size_t,
                 const uint8_t *),
                (override));
    MOCK_METHOD(angle::Result,
                copyImage,
                (const gl::Context *,
                 const gl::ImageIndex &,
                 const gl::Rectangle &,
                 GLenum,
                 gl::Framebuffer *),
                (override));
    MOCK_METHOD(angle::Result,
                copySubImage,
                (const gl::Context *,
                 const gl::ImageIndex &,
                 const gl::Offset &,
                 const gl::Rectangle &,
                 gl::Framebuffer *),
                (override));
    MOCK_METHOD(angle::Result,
                copyTexture,
                (const gl::Context *,
                 const gl::ImageIndex &,
                 GLenum,
                 GLenum,
                 size_t,
                 bool,
                 bool,
                 bool,
                 const gl::Texture *),
                (override));
    MOCK_METHOD(angle::Result,
                copySubTexture,
                (const gl::Context *,
                 const gl::ImageIndex &,
                 const gl::Offset &,
                 size_t,
                 const gl::Box &,
                 bool,
                 bool,
                 bool,
                 const gl::Texture *),
                (override));
    MOCK_METHOD(angle::Result,
                copyCompressedTexture,
                (const gl::Context *, const gl::Texture *source),
                (override));
    MOCK_METHOD(angle::Result,
                setStorage,
                (const gl::Context *, gl::TextureType, size_t, GLenum, const gl::Extents &),
                (override));
    MOCK_METHOD(angle::Result,
                setStorageExternalMemory,
                (const gl::Context *,
                 gl::TextureType,
                 size_t,
                 GLenum,
                 const gl::Extents &,
                 gl::MemoryObject *,
                 GLuint64),
                (override));
    MOCK_METHOD(angle::Result,
                setImageExternal,
                (const gl::Context *,
                 gl::TextureType,
                 egl::Stream *,
                 const egl::Stream::GLTextureDescription &),
                (override));
    MOCK_METHOD(angle::Result,
                setEGLImageTarget,
                (const gl::Context *, gl::TextureType, egl::Image *),
                (override));
    MOCK_METHOD(angle::Result, generateMipmap, (const gl::Context *), (override));
    MOCK_METHOD(angle::Result, bindTexImage, (const gl::Context *, egl::Surface *), (override));
    MOCK_METHOD(angle::Result, releaseTexImage, (const gl::Context *), (override));

    MOCK_METHOD(angle::Result,
                getAttachmentRenderTarget,
                (const gl::Context *,
                 GLenum,
                 const gl::ImageIndex &,
                 GLsizei,
                 FramebufferAttachmentRenderTarget **),
                (override));

    MOCK_METHOD(angle::Result,
                setStorageMultisample,
                (const gl::Context *, gl::TextureType, GLsizei, GLint, const gl::Extents &, bool),
                (override));

    MOCK_METHOD(angle::Result, setBaseLevel, (const gl::Context *, GLuint), (override));

    MOCK_METHOD(angle::Result,
                syncState,
                (const gl::Context *, const gl::Texture::DirtyBits &, gl::TextureCommand source),
                (override));

    MOCK_METHOD(void, destructor, ());

  protected:
    gl::TextureState mMockState;
};
}  // namespace rx

#endif  // LIBANGLE_RENDERER_TEXTUREIMPLMOCK_H_
