//
// Copyright 2014 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//

// TextureD3D.h: Implementations of the Texture interfaces shared betweeen the D3D backends.

#ifndef LIBGLESV2_RENDERER_TEXTURED3D_H_
#define LIBGLESV2_RENDERER_TEXTURED3D_H_

#include "libGLESv2/renderer/TextureImpl.h"
#include "libGLESv2/angletypes.h"
#include "libGLESv2/constants.h"

namespace gl
{
class Framebuffer;
}

namespace rx
{

class Image;
class ImageD3D;
class Renderer;
class TextureStorageInterface;
class TextureStorageInterface2D;

class TextureD3D
{
  public:
    TextureD3D(rx::Renderer *renderer);
    virtual ~TextureD3D();

    GLint getBaseLevelWidth() const;
    GLint getBaseLevelHeight() const;
    GLint getBaseLevelDepth() const;
    GLenum getBaseLevelInternalFormat() const;

    bool isImmutable() const { return mImmutable; }

  protected:
    void setImage(const gl::PixelUnpackState &unpack, GLenum type, const void *pixels, rx::Image *image);
    bool subImage(GLint xoffset, GLint yoffset, GLint zoffset, GLsizei width, GLsizei height, GLsizei depth,
                  GLenum format, GLenum type, const gl::PixelUnpackState &unpack, const void *pixels, rx::Image *image);
    void setCompressedImage(GLsizei imageSize, const void *pixels, rx::Image *image);
    bool subImageCompressed(GLint xoffset, GLint yoffset, GLint zoffset, GLsizei width, GLsizei height, GLsizei depth,
                            GLenum format, GLsizei imageSize, const void *pixels, rx::Image *image);
    bool isFastUnpackable(const gl::PixelUnpackState &unpack, GLenum sizedInternalFormat);
    bool fastUnpackPixels(const gl::PixelUnpackState &unpack, const void *pixels, const gl::Box &destArea,
                                  GLenum sizedInternalFormat, GLenum type, rx::RenderTarget *destRenderTarget);

    GLint creationLevels(GLsizei width, GLsizei height, GLsizei depth) const;
    int mipLevels() const;

    Renderer *mRenderer;

    GLenum mUsage;

    bool mDirtyImages;

    bool mImmutable;

  private:
    DISALLOW_COPY_AND_ASSIGN(TextureD3D);

    virtual TextureStorageInterface *getBaseLevelStorage() = 0;
    virtual const ImageD3D *getBaseLevelImage() const = 0;
};

class TextureD3D_2D : public Texture2DImpl, public TextureD3D
{
  public:
    TextureD3D_2D(rx::Renderer *renderer);
    virtual ~TextureD3D_2D();

    static TextureD3D_2D *makeTextureD3D_2D(Texture2DImpl *texture);

    virtual TextureStorageInterface *getNativeTexture();

    virtual Image *getImage(int level) const;

    virtual void setUsage(GLenum usage);
    virtual bool hasDirtyImages() const { return mDirtyImages; }
    virtual void resetDirty();

    GLsizei getWidth(GLint level) const;
    GLsizei getHeight(GLint level) const;
    GLenum getInternalFormat(GLint level) const;
    GLenum getActualFormat(GLint level) const;
    bool isDepth(GLint level) const;

    virtual void setImage(GLint level, GLsizei width, GLsizei height, GLenum internalFormat, GLenum format, GLenum type, const gl::PixelUnpackState &unpack, const void *pixels);
    virtual void setCompressedImage(GLint level, GLenum format, GLsizei width, GLsizei height, GLsizei imageSize, const void *pixels);
    virtual void subImage(GLint level, GLint xoffset, GLint yoffset, GLsizei width, GLsizei height, GLenum format, GLenum type, const gl::PixelUnpackState &unpack, const void *pixels);
    virtual void subImageCompressed(GLint level, GLint xoffset, GLint yoffset, GLsizei width, GLsizei height, GLenum format, GLsizei imageSize, const void *pixels);
    virtual void copyImage(GLint level, GLenum format, GLint x, GLint y, GLsizei width, GLsizei height, gl::Framebuffer *source);
    virtual void copySubImage(GLenum target, GLint level, GLint xoffset, GLint yoffset, GLint zoffset, GLint x, GLint y, GLsizei width, GLsizei height, gl::Framebuffer *source);
    virtual void storage(GLsizei levels, GLenum internalformat, GLsizei width, GLsizei height);

    virtual bool isSamplerComplete(const gl::SamplerState &samplerState) const;
    virtual void bindTexImage(egl::Surface *surface);
    virtual void releaseTexImage();

    virtual void generateMipmaps();

    virtual unsigned int getRenderTargetSerial(GLint level);

    virtual RenderTarget *getRenderTarget(GLint level);
    virtual RenderTarget *getDepthSencil(GLint level);

  private:
    DISALLOW_COPY_AND_ASSIGN(TextureD3D_2D);

    void initializeStorage(bool renderTarget);
    TextureStorageInterface2D *createCompleteStorage(bool renderTarget) const;
    void setCompleteTexStorage(TextureStorageInterface2D *newCompleteTexStorage);

    void updateStorage();
    bool ensureRenderTarget();
    virtual TextureStorageInterface *getBaseLevelStorage();
    virtual const ImageD3D *getBaseLevelImage() const;

    bool isMipmapComplete() const;
    bool isValidLevel(int level) const;
    bool isLevelComplete(int level) const;

    void updateStorageLevel(int level);

    virtual void redefineImage(GLint level, GLenum internalformat, GLsizei width, GLsizei height);
    void commitRect(GLint level, GLint xoffset, GLint yoffset, GLsizei width, GLsizei height);

    TextureStorageInterface2D *mTexStorage;
    ImageD3D *mImageArray[gl::IMPLEMENTATION_MAX_TEXTURE_LEVELS];
};

}

#endif // LIBGLESV2_RENDERER_TEXTURED3D_H_
