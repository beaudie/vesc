//
// Copyright (c) 2002-2014 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//

// Texture.cpp: Implements the gl::Texture class. [OpenGL ES 2.0.24] section 3.7 page 63.

#include "libANGLE/Texture.h"

#include "common/mathutil.h"
#include "common/utilities.h"
#include "libANGLE/Config.h"
#include "libANGLE/Context.h"
#include "libANGLE/ContextState.h"
#include "libANGLE/Image.h"
#include "libANGLE/Surface.h"
#include "libANGLE/formatutils.h"
#include "libANGLE/renderer/GLImplFactory.h"
#include "libANGLE/renderer/TextureImpl.h"

namespace gl
{

namespace
{
bool IsPointSampled(const gl::SamplerState &samplerState)
{
    return (samplerState.magFilter == GL_NEAREST &&
            (samplerState.minFilter == GL_NEAREST ||
             samplerState.minFilter == GL_NEAREST_MIPMAP_NEAREST));
}

size_t getImageDescIndex(GLenum target, size_t level)
{
    return IsCubeMapTextureTarget(target) ? ((level * 6) + CubeMapTextureTargetToLayerIndex(target))
                                          : level;
}
}  // namespace

bool IsMipmapFiltered(const gl::SamplerState &samplerState)
{
    switch (samplerState.minFilter)
    {
      case GL_NEAREST:
      case GL_LINEAR:
        return false;
      case GL_NEAREST_MIPMAP_NEAREST:
      case GL_LINEAR_MIPMAP_NEAREST:
      case GL_NEAREST_MIPMAP_LINEAR:
      case GL_LINEAR_MIPMAP_LINEAR:
        return true;
      default: UNREACHABLE();
        return false;
    }
}

TextureState::TextureState(GLenum target)
    : mTarget(target),
      mSwizzleRed(GL_RED),
      mSwizzleGreen(GL_GREEN),
      mSwizzleBlue(GL_BLUE),
      mSwizzleAlpha(GL_ALPHA),
      mSamplerState(),
      mBaseLevel(0),
      mMaxLevel(1000),
      mImmutableFormat(false),
      mImmutableLevels(0),
      mUsage(GL_NONE),
      mImageDescs((IMPLEMENTATION_MAX_TEXTURE_LEVELS + 1) *
                  (target == GL_TEXTURE_CUBE_MAP ? 6 : 1)),
      mCompletenessCache()
{
}

bool TextureState::swizzleRequired() const
{
    return mSwizzleRed != GL_RED || mSwizzleGreen != GL_GREEN || mSwizzleBlue != GL_BLUE ||
           mSwizzleAlpha != GL_ALPHA;
}

GLuint TextureState::getEffectiveBaseLevel() const
{
    if (mImmutableFormat)
    {
        // GLES 3.0.4 section 3.8.10
        return std::min(mBaseLevel, mImmutableLevels - 1);
    }
    // Some classes use the effective base level to index arrays with level data. By clamping the
    // effective base level to max levels these arrays need just one extra item to store properties
    // that should be returned for all out-of-range base level values, instead of needing special
    // handling for out-of-range base levels.
    return std::min(mBaseLevel, static_cast<GLuint>(gl::IMPLEMENTATION_MAX_TEXTURE_LEVELS));
}

GLuint TextureState::getEffectiveMaxLevel() const
{
    if (mImmutableFormat)
    {
        // GLES 3.0.4 section 3.8.10
        GLuint clampedMaxLevel = std::max(mMaxLevel, getEffectiveBaseLevel());
        clampedMaxLevel        = std::min(clampedMaxLevel, mImmutableLevels - 1);
        return clampedMaxLevel;
    }
    return mMaxLevel;
}

size_t TextureState::getMipmapMaxLevel() const
{
    const TextureState::ImageDesc &baseImageDesc =
        getImageDesc(getBaseImageTarget(), getEffectiveBaseLevel());
    size_t expectedMipLevels = 0;
    if (mTarget == GL_TEXTURE_3D)
    {
        const int maxDim = std::max(std::max(baseImageDesc.size.width, baseImageDesc.size.height),
                                    baseImageDesc.size.depth);
        expectedMipLevels = log2(maxDim);
    }
    else
    {
        expectedMipLevels = log2(std::max(baseImageDesc.size.width, baseImageDesc.size.height));
    }

    return std::min<size_t>(getEffectiveBaseLevel() + expectedMipLevels, getEffectiveMaxLevel());
}

bool TextureState::setBaseLevel(GLuint baseLevel)
{
    if (mBaseLevel != baseLevel)
    {
        mBaseLevel                    = baseLevel;
        mCompletenessCache.cacheValid = false;
        return true;
    }
    return false;
}

void TextureState::setMaxLevel(GLuint maxLevel)
{
    if (mMaxLevel != maxLevel)
    {
        mMaxLevel                     = maxLevel;
        mCompletenessCache.cacheValid = false;
    }
}

// Tests for cube texture completeness. [OpenGL ES 2.0.24] section 3.7.10 page 81.
bool TextureState::isCubeComplete() const
{
    ASSERT(mTarget == GL_TEXTURE_CUBE_MAP);

    const TextureState::ImageDesc &baseImageDesc = getImageDesc(FirstCubeMapTextureTarget, 0);
    if (baseImageDesc.size.width == 0 || baseImageDesc.size.width != baseImageDesc.size.height)
    {
        return false;
    }

    for (GLenum face = FirstCubeMapTextureTarget + 1; face <= LastCubeMapTextureTarget; face++)
    {
        const TextureState::ImageDesc &faceImageDesc = getImageDesc(face, 0);
        if (faceImageDesc.size.width != baseImageDesc.size.width ||
            faceImageDesc.size.height != baseImageDesc.size.height ||
            faceImageDesc.internalFormat != baseImageDesc.internalFormat)
        {
            return false;
        }
    }

    return true;
}

bool TextureState::isSamplerComplete(const SamplerState &samplerState,
                                     const ContextState &data) const
{
    const TextureState::ImageDesc &baseImageDesc =
        getImageDesc(getBaseImageTarget(), getEffectiveBaseLevel());
    const TextureCaps &textureCaps = data.textureCaps->get(baseImageDesc.internalFormat);
    if (!mCompletenessCache.cacheValid || mCompletenessCache.samplerState != samplerState ||
        mCompletenessCache.filterable != textureCaps.filterable ||
        mCompletenessCache.clientVersion != data.clientVersion ||
        mCompletenessCache.supportsNPOT != data.extensions->textureNPOT)
    {
        mCompletenessCache.cacheValid      = true;
        mCompletenessCache.samplerState    = samplerState;
        mCompletenessCache.filterable      = textureCaps.filterable;
        mCompletenessCache.clientVersion   = data.clientVersion;
        mCompletenessCache.supportsNPOT    = data.extensions->textureNPOT;
        mCompletenessCache.samplerComplete = computeSamplerCompleteness(samplerState, data);
    }
    return mCompletenessCache.samplerComplete;
}

bool TextureState::computeSamplerCompleteness(const SamplerState &samplerState,
                                              const ContextState &data) const
{
    if (mBaseLevel > mMaxLevel)
    {
        return false;
    }
    const TextureState::ImageDesc &baseImageDesc =
        getImageDesc(getBaseImageTarget(), getEffectiveBaseLevel());
    if (baseImageDesc.size.width == 0 || baseImageDesc.size.height == 0 ||
        baseImageDesc.size.depth == 0)
    {
        return false;
    }
    // The cases where the texture is incomplete because base level is out of range should be
    // handled by the above condition.
    ASSERT(mBaseLevel < gl::IMPLEMENTATION_MAX_TEXTURE_LEVELS || mImmutableFormat);

    if (mTarget == GL_TEXTURE_CUBE_MAP && baseImageDesc.size.width != baseImageDesc.size.height)
    {
        return false;
    }

    const TextureCaps &textureCaps = data.textureCaps->get(baseImageDesc.internalFormat);
    if (!textureCaps.filterable && !IsPointSampled(samplerState))
    {
        return false;
    }

    bool npotSupport = data.extensions->textureNPOT || data.clientVersion >= 3;
    if (!npotSupport)
    {
        if ((samplerState.wrapS != GL_CLAMP_TO_EDGE && !gl::isPow2(baseImageDesc.size.width)) ||
            (samplerState.wrapT != GL_CLAMP_TO_EDGE && !gl::isPow2(baseImageDesc.size.height)))
        {
            return false;
        }
    }

    if (IsMipmapFiltered(samplerState))
    {
        if (!npotSupport)
        {
            if (!gl::isPow2(baseImageDesc.size.width) || !gl::isPow2(baseImageDesc.size.height))
            {
                return false;
            }
        }

        if (!computeMipmapCompleteness())
        {
            return false;
        }
    }
    else
    {
        if (mTarget == GL_TEXTURE_CUBE_MAP && !isCubeComplete())
        {
            return false;
        }
    }

    // OpenGLES 3.0.2 spec section 3.8.13 states that a texture is not mipmap complete if:
    // The internalformat specified for the texture arrays is a sized internal depth or
    // depth and stencil format (see table 3.13), the value of TEXTURE_COMPARE_-
    // MODE is NONE, and either the magnification filter is not NEAREST or the mini-
    // fication filter is neither NEAREST nor NEAREST_MIPMAP_NEAREST.
    const gl::InternalFormat &formatInfo = gl::GetInternalFormatInfo(baseImageDesc.internalFormat);
    if (formatInfo.depthBits > 0 && data.clientVersion > 2)
    {
        if (samplerState.compareMode == GL_NONE)
        {
            if ((samplerState.minFilter != GL_NEAREST &&
                 samplerState.minFilter != GL_NEAREST_MIPMAP_NEAREST) ||
                samplerState.magFilter != GL_NEAREST)
            {
                return false;
            }
        }
    }

    return true;
}

bool TextureState::computeMipmapCompleteness() const
{
    const size_t maxLevel = getMipmapMaxLevel();

    for (size_t level = getEffectiveBaseLevel(); level <= maxLevel; level++)
    {
        if (mTarget == GL_TEXTURE_CUBE_MAP)
        {
            for (GLenum face = FirstCubeMapTextureTarget; face <= LastCubeMapTextureTarget; face++)
            {
                if (!computeLevelCompleteness(face, level))
                {
                    return false;
                }
            }
        }
        else
        {
            if (!computeLevelCompleteness(mTarget, level))
            {
                return false;
            }
        }
    }

    return true;
}

bool TextureState::computeLevelCompleteness(GLenum target, size_t level) const
{
    ASSERT(level < IMPLEMENTATION_MAX_TEXTURE_LEVELS);

    if (mImmutableFormat)
    {
        return true;
    }

    const TextureState::ImageDesc &baseImageDesc =
        getImageDesc(getBaseImageTarget(), getEffectiveBaseLevel());
    if (baseImageDesc.size.width == 0 || baseImageDesc.size.height == 0 ||
        baseImageDesc.size.depth == 0)
    {
        return false;
    }

    const TextureState::ImageDesc &levelImageDesc = getImageDesc(target, level);
    if (levelImageDesc.size.width == 0 || levelImageDesc.size.height == 0 ||
        levelImageDesc.size.depth == 0)
    {
        return false;
    }

    if (levelImageDesc.internalFormat != baseImageDesc.internalFormat)
    {
        return false;
    }

    ASSERT(level >= getEffectiveBaseLevel());
    const size_t relativeLevel = level - getEffectiveBaseLevel();
    if (levelImageDesc.size.width != std::max(1, baseImageDesc.size.width >> relativeLevel))
    {
        return false;
    }

    if (levelImageDesc.size.height != std::max(1, baseImageDesc.size.height >> relativeLevel))
    {
        return false;
    }

    if (mTarget == GL_TEXTURE_3D)
    {
        if (levelImageDesc.size.depth != std::max(1, baseImageDesc.size.depth >> relativeLevel))
        {
            return false;
        }
    }
    else if (mTarget == GL_TEXTURE_2D_ARRAY)
    {
        if (levelImageDesc.size.depth != baseImageDesc.size.depth)
        {
            return false;
        }
    }

    return true;
}

GLenum TextureState::getBaseImageTarget() const
{
    return mTarget == GL_TEXTURE_CUBE_MAP ? FirstCubeMapTextureTarget : mTarget;
}

TextureState::ImageDesc::ImageDesc() : ImageDesc(Extents(0, 0, 0), GL_NONE)
{
}

TextureState::ImageDesc::ImageDesc(const Extents &size, GLenum internalFormat)
    : size(size), internalFormat(internalFormat)
{
}

const TextureState::ImageDesc &TextureState::getImageDesc(GLenum target, size_t level) const
{
    size_t descIndex = getImageDescIndex(target, level);
    ASSERT(descIndex < mImageDescs.size());
    return mImageDescs[descIndex];
}

void TextureState::setImageDesc(GLenum target, size_t level, const TextureState::ImageDesc &desc)
{
    size_t descIndex = getImageDescIndex(target, level);
    ASSERT(descIndex < mImageDescs.size());
    mImageDescs[descIndex]        = desc;
    mCompletenessCache.cacheValid = false;
}

void TextureState::setImageDescChain(int baseLevel,
                                     int maxLevel,
                                     Extents baseSize,
                                     GLenum sizedInternalFormat)
{
    for (int level = baseLevel; level <= maxLevel; level++)
    {
        int relativeLevel = (level - baseLevel);
        Extents levelSize(std::max<int>(baseSize.width >> relativeLevel, 1),
                          std::max<int>(baseSize.height >> relativeLevel, 1),
                          (mTarget == GL_TEXTURE_2D_ARRAY)
                              ? baseSize.depth
                              : std::max<int>(baseSize.depth >> relativeLevel, 1));
        ImageDesc levelInfo(levelSize, sizedInternalFormat);

        if (mTarget == GL_TEXTURE_CUBE_MAP)
        {
            for (GLenum face = FirstCubeMapTextureTarget; face <= LastCubeMapTextureTarget; face++)
            {
                setImageDesc(face, level, levelInfo);
            }
        }
        else
        {
            setImageDesc(mTarget, level, levelInfo);
        }
    }
}

void TextureState::clearImageDesc(GLenum target, size_t level)
{
    setImageDesc(target, level, TextureState::ImageDesc());
}

void TextureState::clearImageDescs()
{
    for (size_t descIndex = 0; descIndex < mImageDescs.size(); descIndex++)
    {
        mImageDescs[descIndex] = TextureState::ImageDesc();
    }
    mCompletenessCache.cacheValid = false;
}

TextureState::SamplerCompletenessCache::SamplerCompletenessCache()
    : cacheValid(false),
      samplerState(),
      filterable(false),
      clientVersion(0),
      supportsNPOT(false),
      samplerComplete(false)
{
}

Texture::Texture(rx::GLImplFactory *factory, GLuint id, GLenum target)
    : egl::ImageSibling(id),
      mState(target),
      mTexture(factory->createTexture(mState)),
      mLabel(),
      mBoundSurface(nullptr),
      mBoundStream(nullptr)
{
}

Texture::~Texture()
{
    if (mBoundSurface)
    {
        mBoundSurface->releaseTexImage(EGL_BACK_BUFFER);
        mBoundSurface = nullptr;
    }
    if (mBoundStream)
    {
        mBoundStream->releaseTextures();
        mBoundStream = nullptr;
    }
    SafeDelete(mTexture);
}

void Texture::setLabel(const std::string &label)
{
    mLabel = label;
}

const std::string &Texture::getLabel() const
{
    return mLabel;
}

GLenum Texture::getTarget() const
{
    return mState.mTarget;
}

void Texture::setSwizzleRed(GLenum swizzleRed)
{
    mState.mSwizzleRed = swizzleRed;
}

GLenum Texture::getSwizzleRed() const
{
    return mState.mSwizzleRed;
}

void Texture::setSwizzleGreen(GLenum swizzleGreen)
{
    mState.mSwizzleGreen = swizzleGreen;
}

GLenum Texture::getSwizzleGreen() const
{
    return mState.mSwizzleGreen;
}

void Texture::setSwizzleBlue(GLenum swizzleBlue)
{
    mState.mSwizzleBlue = swizzleBlue;
}

GLenum Texture::getSwizzleBlue() const
{
    return mState.mSwizzleBlue;
}

void Texture::setSwizzleAlpha(GLenum swizzleAlpha)
{
    mState.mSwizzleAlpha = swizzleAlpha;
}

GLenum Texture::getSwizzleAlpha() const
{
    return mState.mSwizzleAlpha;
}

void Texture::setMinFilter(GLenum minFilter)
{
    mState.mSamplerState.minFilter = minFilter;
}

GLenum Texture::getMinFilter() const
{
    return mState.mSamplerState.minFilter;
}

void Texture::setMagFilter(GLenum magFilter)
{
    mState.mSamplerState.magFilter = magFilter;
}

GLenum Texture::getMagFilter() const
{
    return mState.mSamplerState.magFilter;
}

void Texture::setWrapS(GLenum wrapS)
{
    mState.mSamplerState.wrapS = wrapS;
}

GLenum Texture::getWrapS() const
{
    return mState.mSamplerState.wrapS;
}

void Texture::setWrapT(GLenum wrapT)
{
    mState.mSamplerState.wrapT = wrapT;
}

GLenum Texture::getWrapT() const
{
    return mState.mSamplerState.wrapT;
}

void Texture::setWrapR(GLenum wrapR)
{
    mState.mSamplerState.wrapR = wrapR;
}

GLenum Texture::getWrapR() const
{
    return mState.mSamplerState.wrapR;
}

void Texture::setMaxAnisotropy(float maxAnisotropy)
{
    mState.mSamplerState.maxAnisotropy = maxAnisotropy;
}

float Texture::getMaxAnisotropy() const
{
    return mState.mSamplerState.maxAnisotropy;
}

void Texture::setMinLod(GLfloat minLod)
{
    mState.mSamplerState.minLod = minLod;
}

GLfloat Texture::getMinLod() const
{
    return mState.mSamplerState.minLod;
}

void Texture::setMaxLod(GLfloat maxLod)
{
    mState.mSamplerState.maxLod = maxLod;
}

GLfloat Texture::getMaxLod() const
{
    return mState.mSamplerState.maxLod;
}

void Texture::setCompareMode(GLenum compareMode)
{
    mState.mSamplerState.compareMode = compareMode;
}

GLenum Texture::getCompareMode() const
{
    return mState.mSamplerState.compareMode;
}

void Texture::setCompareFunc(GLenum compareFunc)
{
    mState.mSamplerState.compareFunc = compareFunc;
}

GLenum Texture::getCompareFunc() const
{
    return mState.mSamplerState.compareFunc;
}

const SamplerState &Texture::getSamplerState() const
{
    return mState.mSamplerState;
}

void Texture::setBaseLevel(GLuint baseLevel)
{
    if (mState.setBaseLevel(baseLevel))
    {
        mTexture->setBaseLevel(mState.getEffectiveBaseLevel());
    }
}

GLuint Texture::getBaseLevel() const
{
    return mState.mBaseLevel;
}

void Texture::setMaxLevel(GLuint maxLevel)
{
    mState.setMaxLevel(maxLevel);
}

GLuint Texture::getMaxLevel() const
{
    return mState.mMaxLevel;
}

bool Texture::getImmutableFormat() const
{
    return mState.mImmutableFormat;
}

GLuint Texture::getImmutableLevels() const
{
    return mState.mImmutableLevels;
}

void Texture::setUsage(GLenum usage)
{
    mState.mUsage = usage;
}

GLenum Texture::getUsage() const
{
    return mState.mUsage;
}

const TextureState &Texture::getTextureState() const
{
    return mState;
}

size_t Texture::getWidth(GLenum target, size_t level) const
{
    ASSERT(target == mState.mTarget ||
           (mState.mTarget == GL_TEXTURE_CUBE_MAP && IsCubeMapTextureTarget(target)));
    return mState.getImageDesc(target, level).size.width;
}

size_t Texture::getHeight(GLenum target, size_t level) const
{
    ASSERT(target == mState.mTarget ||
           (mState.mTarget == GL_TEXTURE_CUBE_MAP && IsCubeMapTextureTarget(target)));
    return mState.getImageDesc(target, level).size.height;
}

size_t Texture::getDepth(GLenum target, size_t level) const
{
    ASSERT(target == mState.mTarget ||
           (mState.mTarget == GL_TEXTURE_CUBE_MAP && IsCubeMapTextureTarget(target)));
    return mState.getImageDesc(target, level).size.depth;
}

GLenum Texture::getInternalFormat(GLenum target, size_t level) const
{
    ASSERT(target == mState.mTarget ||
           (mState.mTarget == GL_TEXTURE_CUBE_MAP && IsCubeMapTextureTarget(target)));
    return mState.getImageDesc(target, level).internalFormat;
}

bool Texture::isMipmapComplete() const
{
    return mState.computeMipmapCompleteness();
}

egl::Surface *Texture::getBoundSurface() const
{
    return mBoundSurface;
}

egl::Stream *Texture::getBoundStream() const
{
    return mBoundStream;
}

Error Texture::setImage(const PixelUnpackState &unpackState,
                        GLenum target,
                        size_t level,
                        GLenum internalFormat,
                        const Extents &size,
                        GLenum format,
                        GLenum type,
                        const uint8_t *pixels)
{
    ASSERT(target == mState.mTarget ||
           (mState.mTarget == GL_TEXTURE_CUBE_MAP && IsCubeMapTextureTarget(target)));

    // Release from previous calls to eglBindTexImage, to avoid calling the Impl after
    releaseTexImageInternal();
    orphanImages();

    Error error =
        mTexture->setImage(target, level, internalFormat, size, format, type, unpackState, pixels);
    if (error.isError())
    {
        return error;
    }

    mState.setImageDesc(
        target, level, TextureState::ImageDesc(size, GetSizedInternalFormat(internalFormat, type)));

    return Error(GL_NO_ERROR);
}

Error Texture::setSubImage(const PixelUnpackState &unpackState,
                           GLenum target,
                           size_t level,
                           const Box &area,
                           GLenum format,
                           GLenum type,
                           const uint8_t *pixels)
{
    ASSERT(target == mState.mTarget ||
           (mState.mTarget == GL_TEXTURE_CUBE_MAP && IsCubeMapTextureTarget(target)));
    return mTexture->setSubImage(target, level, area, format, type, unpackState, pixels);
}

Error Texture::setCompressedImage(const PixelUnpackState &unpackState,
                                  GLenum target,
                                  size_t level,
                                  GLenum internalFormat,
                                  const Extents &size,
                                  size_t imageSize,
                                  const uint8_t *pixels)
{
    ASSERT(target == mState.mTarget ||
           (mState.mTarget == GL_TEXTURE_CUBE_MAP && IsCubeMapTextureTarget(target)));

    // Release from previous calls to eglBindTexImage, to avoid calling the Impl after
    releaseTexImageInternal();
    orphanImages();

    Error error = mTexture->setCompressedImage(target, level, internalFormat, size, unpackState,
                                               imageSize, pixels);
    if (error.isError())
    {
        return error;
    }

    mState.setImageDesc(
        target, level,
        TextureState::ImageDesc(size, GetSizedInternalFormat(internalFormat, GL_UNSIGNED_BYTE)));

    return Error(GL_NO_ERROR);
}

Error Texture::setCompressedSubImage(const PixelUnpackState &unpackState,
                                     GLenum target,
                                     size_t level,
                                     const Box &area,
                                     GLenum format,
                                     size_t imageSize,
                                     const uint8_t *pixels)
{
    ASSERT(target == mState.mTarget ||
           (mState.mTarget == GL_TEXTURE_CUBE_MAP && IsCubeMapTextureTarget(target)));

    return mTexture->setCompressedSubImage(target, level, area, format, unpackState, imageSize,
                                           pixels);
}

Error Texture::copyImage(GLenum target, size_t level, const Rectangle &sourceArea, GLenum internalFormat,
                         const Framebuffer *source)
{
    ASSERT(target == mState.mTarget ||
           (mState.mTarget == GL_TEXTURE_CUBE_MAP && IsCubeMapTextureTarget(target)));

    // Release from previous calls to eglBindTexImage, to avoid calling the Impl after
    releaseTexImageInternal();
    orphanImages();

    Error error = mTexture->copyImage(target, level, sourceArea, internalFormat, source);
    if (error.isError())
    {
        return error;
    }

    mState.setImageDesc(
        target, level,
        TextureState::ImageDesc(Extents(sourceArea.width, sourceArea.height, 1),
                                GetSizedInternalFormat(internalFormat, GL_UNSIGNED_BYTE)));

    return Error(GL_NO_ERROR);
}

Error Texture::copySubImage(GLenum target, size_t level, const Offset &destOffset, const Rectangle &sourceArea,
                            const Framebuffer *source)
{
    ASSERT(target == mState.mTarget ||
           (mState.mTarget == GL_TEXTURE_CUBE_MAP && IsCubeMapTextureTarget(target)));

    return mTexture->copySubImage(target, level, destOffset, sourceArea, source);
}

Error Texture::setStorage(GLenum target, size_t levels, GLenum internalFormat, const Extents &size)
{
    ASSERT(target == mState.mTarget);

    // Release from previous calls to eglBindTexImage, to avoid calling the Impl after
    releaseTexImageInternal();
    orphanImages();

    Error error = mTexture->setStorage(target, levels, internalFormat, size);
    if (error.isError())
    {
        return error;
    }

    mState.mImmutableFormat = true;
    mState.mImmutableLevels = static_cast<GLuint>(levels);
    mState.clearImageDescs();
    mState.setImageDescChain(0, levels - 1, size, internalFormat);
    return Error(GL_NO_ERROR);
}

Error Texture::generateMipmaps()
{
    // Release from previous calls to eglBindTexImage, to avoid calling the Impl after
    releaseTexImageInternal();

    // EGL_KHR_gl_image states that images are only orphaned when generating mipmaps if the texture
    // is not mip complete.
    if (!isMipmapComplete())
    {
        orphanImages();
    }

    const int baseLevel = mState.getEffectiveBaseLevel();
    const int maxLevel  = mState.getMipmapMaxLevel();

    if (maxLevel > baseLevel)
    {
        ANGLE_TRY(mTexture->generateMipmap());

        const TextureState::ImageDesc &baseImageInfo =
            mState.getImageDesc(mState.getBaseImageTarget(), baseLevel);
        mState.setImageDescChain(baseLevel, maxLevel, baseImageInfo.size,
                                 baseImageInfo.internalFormat);
    }

    return NoError();
}

void Texture::bindTexImageFromSurface(egl::Surface *surface)
{
    ASSERT(surface);

    if (mBoundSurface)
    {
        releaseTexImageFromSurface();
    }

    mTexture->bindTexImage(surface);
    mBoundSurface = surface;

    // Set the image info to the size and format of the surface
    ASSERT(mState.mTarget == GL_TEXTURE_2D);
    Extents size(surface->getWidth(), surface->getHeight(), 1);
    TextureState::ImageDesc desc(size, surface->getConfig()->renderTargetFormat);
    mState.setImageDesc(mState.mTarget, 0, desc);
}

void Texture::releaseTexImageFromSurface()
{
    ASSERT(mBoundSurface);
    mBoundSurface = nullptr;
    mTexture->releaseTexImage();

    // Erase the image info for level 0
    ASSERT(mState.mTarget == GL_TEXTURE_2D);
    mState.clearImageDesc(mState.mTarget, 0);
}

void Texture::bindStream(egl::Stream *stream)
{
    ASSERT(stream);

    // It should not be possible to bind a texture already bound to another stream
    ASSERT(mBoundStream == nullptr);

    mBoundStream = stream;

    ASSERT(mState.mTarget == GL_TEXTURE_EXTERNAL_OES);
}

void Texture::releaseStream()
{
    ASSERT(mBoundStream);
    mBoundStream = nullptr;
}

void Texture::acquireImageFromStream(const egl::Stream::GLTextureDescription &desc)
{
    ASSERT(mBoundStream != nullptr);
    mTexture->setImageExternal(mState.mTarget, mBoundStream, desc);

    Extents size(desc.width, desc.height, 1);
    mState.setImageDesc(mState.mTarget, 0, TextureState::ImageDesc(size, desc.internalFormat));
}

void Texture::releaseImageFromStream()
{
    ASSERT(mBoundStream != nullptr);
    mTexture->setImageExternal(mState.mTarget, nullptr, egl::Stream::GLTextureDescription());

    // Set to incomplete
    mState.clearImageDesc(mState.mTarget, 0);
}

void Texture::releaseTexImageInternal()
{
    if (mBoundSurface)
    {
        // Notify the surface
        mBoundSurface->releaseTexImageFromTexture();

        // Then, call the same method as from the surface
        releaseTexImageFromSurface();
    }
}

Error Texture::setEGLImageTarget(GLenum target, egl::Image *imageTarget)
{
    ASSERT(target == mState.mTarget);
    ASSERT(target == GL_TEXTURE_2D);

    // Release from previous calls to eglBindTexImage, to avoid calling the Impl after
    releaseTexImageInternal();
    orphanImages();

    Error error = mTexture->setEGLImageTarget(target, imageTarget);
    if (error.isError())
    {
        return error;
    }

    setTargetImage(imageTarget);

    Extents size(static_cast<int>(imageTarget->getWidth()),
                 static_cast<int>(imageTarget->getHeight()), 1);
    GLenum internalFormat = imageTarget->getInternalFormat();
    GLenum type           = GetInternalFormatInfo(internalFormat).type;

    mState.clearImageDescs();
    mState.setImageDesc(
        target, 0, TextureState::ImageDesc(size, GetSizedInternalFormat(internalFormat, type)));

    return Error(GL_NO_ERROR);
}

Extents Texture::getAttachmentSize(const gl::FramebufferAttachment::Target &target) const
{
    return mState.getImageDesc(target.textureIndex().type, target.textureIndex().mipIndex).size;
}

GLenum Texture::getAttachmentInternalFormat(const gl::FramebufferAttachment::Target &target) const
{
    return getInternalFormat(target.textureIndex().type, target.textureIndex().mipIndex);
}

GLsizei Texture::getAttachmentSamples(const gl::FramebufferAttachment::Target &/*target*/) const
{
    // Multisample textures not currently supported
    return 0;
}

void Texture::onAttach()
{
    addRef();
}

void Texture::onDetach()
{
    release();
}

GLuint Texture::getId() const
{
    return id();
}

rx::FramebufferAttachmentObjectImpl *Texture::getAttachmentImpl() const
{
    return mTexture;
}
}
