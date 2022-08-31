//
// Copyright 2022 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//

// PixelLocalStorage.cpp: Defines the renderer-agnostic container classes
// gl::PixelLocalStorage and gl::PixelLocalStoragePlane for
// ANGLE_shader_pixel_local_storage.

#include "libANGLE/PixelLocalStorage.h"

#include "libANGLE/Context.h"
#include "libANGLE/Framebuffer.h"
#include "libANGLE/Texture.h"

namespace gl
{

// RAII utilities for working with GL state.
namespace
{
class ScopedBindTexture2D
{
  public:
    ScopedBindTexture2D(Context *angle, TextureID texture)
        : mANGLE(angle),
          mSavedTexBinding2D(
              mANGLE->getState().getSamplerTextureId(mANGLE->getState().getActiveSampler(),
                                                     TextureType::_2D))
    {
        mANGLE->bindTexture(TextureType::_2D, texture);
    }

    ~ScopedBindTexture2D() { mANGLE->bindTexture(TextureType::_2D, mSavedTexBinding2D); }

  private:
    Context *const mANGLE;
    TextureID mSavedTexBinding2D;
};

class ScopedBindDrawFramebuffer
{
  public:
    ScopedBindDrawFramebuffer(Context *angle, FramebufferID id)
        : mANGLE(angle), mSavedFramebuffer(mANGLE->getState().getDrawFramebuffer())
    {
        ASSERT(mSavedFramebuffer);
        mANGLE->bindDrawFramebuffer(id);
    }

    ~ScopedBindDrawFramebuffer() { mANGLE->bindDrawFramebuffer(mSavedFramebuffer->id()); }

  private:
    Context *const mANGLE;
    Framebuffer *const mSavedFramebuffer;
};

class ScopedDisableScissor
{
  public:
    ScopedDisableScissor(Context *angle)
        : mANGLE(angle), mScissorTestEnabled(mANGLE->getState().isScissorTestEnabled())
    {
        if (mScissorTestEnabled)
        {
            mANGLE->disable(GL_SCISSOR_TEST);
        }
    }

    ~ScopedDisableScissor()
    {
        if (mScissorTestEnabled)
        {
            mANGLE->enable(GL_SCISSOR_TEST);
        }
    }

  private:
    Context *const mANGLE;
    const GLint mScissorTestEnabled;
};
}  // namespace

PixelLocalStoragePlane::PixelLocalStoragePlane(PixelLocalStoragePlane &&plane)
{
    memcpy(this, &plane, sizeof(PixelLocalStoragePlane));
    plane.mTextureRef = nullptr;
}

void PixelLocalStoragePlane::deinitialize(Context *angle)
{
    mInternalformat = GL_NONE;
    if (mTextureRef)
    {
        if (mMemoryless)
        {
            // We own the texture handles of memoryless planes.
            ASSERT(angle->getTexture(mTextureRef->id()) == mTextureRef);
            angle->deleteTexture(mTextureRef->id());
        }
        mTextureRef->release(angle);
        mTextureRef = nullptr;
    }
    mMemoryless = false;
}

void PixelLocalStoragePlane::setMemoryless(Context *angle, GLenum internalformat)
{
    deinitialize(angle);
    mInternalformat    = internalformat;
    mTextureImageIndex = ImageIndex::MakeFromType(TextureType::_2D, 0, 0);
    mMemoryless        = true;
}

void PixelLocalStoragePlane::setTextureBacked(Context *angle,
                                              const ImageIndex &textureIndex,
                                              Texture *textureObject)
{
    const Format &textureFormat =
        textureObject->getFormat(textureIndex.getTarget(), textureIndex.getLevelIndex());
    textureObject->addRef();
    deinitialize(angle);
    mInternalformat    = textureFormat.info->internalFormat;
    mTextureRef        = textureObject;
    mTextureImageIndex = textureIndex;
    mMemoryless        = false;
}

bool PixelLocalStoragePlane::isTextureIDDeleted(const Context *angle) const
{
    ASSERT(!isDeinitialized());
    return mTextureRef && angle->getTexture(mTextureRef->id()) != mTextureRef;
}

// [ANGLE_shader_pixel_local_storage] Section 4.4.2.X "Configuring Pixel Local Storage on a
// Framebuffer": When a texture object is deleted, any pixel local storage plane to which it was
// bound is automatically converted to a memoryless plane of matching internalformat.
void PixelLocalStoragePlane::convertToMemorylessIfTextureIDDeleted(const Context *angle)
{
    if (!isDeinitialized() && isTextureIDDeleted(angle))
    {
        mTextureRef->release(angle);
        mTextureRef        = nullptr;
        mTextureImageIndex = ImageIndex::MakeFromType(TextureType::_2D, 0, 0);
        mMemoryless        = true;
    }
}

GLint PixelLocalStoragePlane::getIntegeri(GLenum target, GLuint index)
{
    ASSERT(!isDeinitialized());
    switch (target)
    {
        case GL_PIXEL_LOCAL_FORMAT_ANGLE:
            return mInternalformat;
        case GL_PIXEL_LOCAL_TEXTURE_NAME_ANGLE:
            return isMemoryless() ? 0 : mTextureRef->id().value;
        case GL_PIXEL_LOCAL_TEXTURE_LEVEL_ANGLE:
            return mTextureImageIndex.getLevelIndex();
        case GL_PIXEL_LOCAL_TEXTURE_LAYER_ANGLE:
            return mTextureImageIndex.getLayerIndex();
    }
    return 0;
}

bool PixelLocalStoragePlane::getTextureImageExtents(const Context *angle, Extents *extents)
{
    ASSERT(!isDeinitialized());
    ASSERT(!isTextureIDDeleted(angle));  // Call convertToMemorylessIfTextureIDDeleted() first!
    if (isMemoryless())
    {
        return false;
    }
    ASSERT(mTextureRef);
    extents->width  = (int)mTextureRef->getWidth(mTextureImageIndex.getTarget(),
                                                 mTextureImageIndex.getLevelIndex());
    extents->height = (int)mTextureRef->getHeight(mTextureImageIndex.getTarget(),
                                                  mTextureImageIndex.getLevelIndex());
    extents->depth  = 0;
    return true;
}

void PixelLocalStoragePlane::ensureBackingIfMemoryless(Context *angle, Extents plsExtents)
{
    ASSERT(!isDeinitialized());
    ASSERT(!isTextureIDDeleted(angle));  // Call convertToMemorylessIfTextureIDDeleted() first!
    if (!isMemoryless())
    {
        return;
    }
    // Internal textures backing memoryless planes are always 2D and not mipmapped.
    ASSERT(mTextureImageIndex.getType() == TextureType::_2D);
    ASSERT(mTextureImageIndex.getLevelIndex() == 0);
    ASSERT(mTextureImageIndex.getLayerIndex() == 0);
    if (!mTextureRef || (GLsizei)mTextureRef->getWidth(TextureTarget::_2D, 0) != plsExtents.width ||
        (GLsizei)mTextureRef->getHeight(TextureTarget::_2D, 0) != plsExtents.height)
    {
        // We need to create a new texture that backs the memoryless plane.
        if (mTextureRef)
        {
            mTextureRef->release(angle);
        }
        TextureID tex;
        angle->genTextures(1, &tex);
        {
            ScopedBindTexture2D scopedBindTexture2D(angle, tex);
            angle->bindTexture(TextureType::_2D, tex);
            angle->texStorage2D(TextureType::_2D, 1, mInternalformat, plsExtents.width,
                                plsExtents.height);
        }
        mTextureRef = angle->getTexture(tex);
        mTextureRef->addRef();
        ASSERT(mTextureRef->id() == tex);  // 'tex' will get deleted during cleanup.
    }
}

void PixelLocalStoragePlane::attachToDrawFramebuffer(Context *angle,
                                                     Extents plsExtents,
                                                     GLenum colorAttachment)
{
    ASSERT(!isDeinitialized());
    ASSERT(!isTextureIDDeleted(angle));  // Call convertToMemorylessIfTextureIDDeleted() first!
    ensureBackingIfMemoryless(angle, plsExtents);
    switch (mTextureImageIndex.getTarget())
    {
        case TextureTarget::_2D:
        case TextureTarget::CubeMapPositiveX:
        case TextureTarget::CubeMapNegativeX:
        case TextureTarget::CubeMapPositiveY:
        case TextureTarget::CubeMapNegativeY:
        case TextureTarget::CubeMapPositiveZ:
        case TextureTarget::CubeMapNegativeZ:
            angle->framebufferTexture2D(GL_DRAW_FRAMEBUFFER, colorAttachment,
                                        mTextureImageIndex.getTarget(), mTextureRef->id(),
                                        mTextureImageIndex.getLevelIndex());
            break;
        case TextureTarget::_2DArray:
        case TextureTarget::_3D:
            angle->framebufferTextureLayer(GL_DRAW_FRAMEBUFFER, colorAttachment, mTextureRef->id(),
                                           mTextureImageIndex.getLevelIndex(),
                                           mTextureImageIndex.getLayerIndex());
            break;
        default:
            UNREACHABLE();
    }
}

void PixelLocalStoragePlane::performLoadOperationClear(Context *angle,
                                                       GLint drawBuffer,
                                                       GLenum loadop)
{
    bool needsClear = loadop == GL_ZERO || loadop == GL_CLEAR_ANGLE;
    if (!needsClear)
    {
        return;
    }
    ASSERT(!angle->getState().isScissorTestEnabled());
    constexpr static char zero[4][4]{};
    switch (mInternalformat)
    {
        case GL_RGBA8:
        case GL_R32F:
            angle->clearBufferfv(
                GL_COLOR, 0,
                loadop == GL_CLEAR_ANGLE ? mClearValuef : reinterpret_cast<const float *>(zero));
            break;
        case GL_RGBA8I:
            angle->clearBufferiv(
                GL_COLOR, 0,
                loadop == GL_CLEAR_ANGLE ? mClearValuei : reinterpret_cast<const int32_t *>(zero));
            break;
        case GL_RGBA8UI:
        case GL_R32UI:
            angle->clearBufferuiv(GL_COLOR, 0,
                                  loadop == GL_CLEAR_ANGLE
                                      ? mClearValueui
                                      : reinterpret_cast<const uint32_t *>(zero));
            break;
        default:
            // Invalid PLS internalformats should not have made it this far.
            UNREACHABLE();
    }
}

void PixelLocalStoragePlane::bindToImage(Context *angle,
                                         Extents plsExtents,
                                         GLuint unit,
                                         ImageFormatting imageFormatting)
{
    ASSERT(!isDeinitialized());
    ASSERT(!isTextureIDDeleted(angle));  // Call convertToMemorylessIfTextureIDDeleted() first!
    ensureBackingIfMemoryless(angle, plsExtents);
    ASSERT(mTextureRef);
    GLenum imageBindingFormat = mInternalformat;
    if (imageFormatting == ImageFormatting::R32Packed)
    {
        // D3D and ES require us to pack all PLS formats into r32f, r32i, or r32ui images.
        switch (imageBindingFormat)
        {
            case GL_RGBA8:
            case GL_RGBA8UI:
                imageBindingFormat = GL_R32UI;
                break;
            case GL_RGBA8I:
                imageBindingFormat = GL_R32I;
                break;
        }
    }
    angle->bindImageTexture(unit, mTextureRef->id(), mTextureImageIndex.getLevelIndex(), GL_FALSE,
                            mTextureImageIndex.getLayerIndex(), GL_READ_WRITE, imageBindingFormat);
}

PixelLocalStorage::PixelLocalStorage() {}
PixelLocalStorage::~PixelLocalStorage() {}

void PixelLocalStorage::deleteContextObjects(Context *angle)
{
    for (PixelLocalStoragePlane &plane : mPlanes)
    {
        plane.deinitialize(angle);
    }
    onDeleteContextObjects(angle);
}

bool PixelLocalStorage::isPlaneDeinitialized(GLint plane)
{
    ASSERT(plane >= 0);
    return (size_t)plane < mPlanes.size() ? mPlanes[plane].isDeinitialized() : true;
}

void PixelLocalStorage::deinitialize(Context *angle, GLint plane)
{
    ASSERT(plane >= 0);
    if ((size_t)plane < mPlanes.size())
    {
        mPlanes[plane].deinitialize(angle);
    }
}

PixelLocalStoragePlane &PixelLocalStorage::growPlanesToFit(GLint plane)
{
    ASSERT(plane >= 0);
    if ((size_t)plane >= mPlanes.size())
    {
        mPlanes.resize(plane + 1);
    }
    return mPlanes[plane];
}

void PixelLocalStorage::begin(Context *angle, GLsizei n, const GLenum loadops[])
{
    onBegin(angle, n, loadops, mPLSExtents);
    mNumActivePLSPlanes = n;
}

void PixelLocalStorage::end(Context *angle)
{
    onEnd(angle, mNumActivePLSPlanes);
    mNumActivePLSPlanes = 0;
}

namespace
{
// Implements pixel local storage with image load/store shader operations.
class PixelLocalStorageImageLoadStore : public PixelLocalStorage
{
  public:
    PixelLocalStorageImageLoadStore(Context *angle, ImageFormatting imageFormatting)
        : mImageFormatting(imageFormatting)
    {
        angle->genFramebuffers(1, &mScratchFramebufferForClearing);
    }

    // Call deleteContextObjects first!
    ~PixelLocalStorageImageLoadStore() override { ASSERT(!mScratchFramebufferForClearing.value); }

    void onDeleteContextObjects(Context *angle) override
    {
        angle->deleteFramebuffer(mScratchFramebufferForClearing);
        mScratchFramebufferForClearing = FramebufferID();
    }

    void onBegin(Context *angle, GLsizei n, const GLenum loadops[], Extents plsExtents) override
    {
        // Save the image bindings so we can restore them during onEnd().
        const State &state = angle->getState();
        ASSERT((size_t)n <= state.getImageUnits().size());
        ASSERT(mSavedImageBindings.empty());
        mSavedImageBindings.reserve(n);
        for (int i = 0; i < n; ++i)
        {
            mSavedImageBindings.emplace_back(state.getImageUnit(i));
        }

        // Save the default framebuffer width/height so we can resture it during onEnd().
        Framebuffer *framebuffer       = state.getDrawFramebuffer();
        mSavedFramebufferDefaultWidth  = framebuffer->getDefaultWidth();
        mSavedFramebufferDefaultHeight = framebuffer->getDefaultHeight();

        // Specify the framebuffer width/height explicitly in case we end up rendering exclusively
        // to shader images.
        angle->framebufferParameteri(GL_DRAW_FRAMEBUFFER, GL_FRAMEBUFFER_DEFAULT_WIDTH,
                                     plsExtents.width);
        angle->framebufferParameteri(GL_DRAW_FRAMEBUFFER, GL_FRAMEBUFFER_DEFAULT_HEIGHT,
                                     plsExtents.height);

        // Guard GL state and bind a scratch framebuffer in case we need to reallocate or clear any
        // PLS planes.
        ScopedBindDrawFramebuffer autoBindDrawFramebuffer(angle, mScratchFramebufferForClearing);
        ScopedDisableScissor autoDisableScissor(angle);

        // Bind and clear the PLS planes. Any plane indexed beyond mPlanes.size() can be assumed to
        // be deinitialized.
        GLuint maxPlanes = std::min(n, getNumPlanes());
        for (GLuint i = 0; i < maxPlanes; ++i)
        {
            GLenum loadop = loadops[i];
            if (loadop == GL_DISABLE_ANGLE)
            {
                continue;
            }
            PixelLocalStoragePlane &plane = getPlane(i);
            ASSERT(!plane.isDeinitialized());
            if (loadop == GL_ZERO || loadop == GL_CLEAR_ANGLE)
            {
                plane.attachToDrawFramebuffer(angle, plsExtents, GL_COLOR_ATTACHMENT0);
                plane.performLoadOperationClear(angle, 0, loadop);
            }
            plane.bindToImage(angle, plsExtents, i, mImageFormatting);
        }

        // Detach any PLS textures from the scratch framebuffer.
        angle->framebufferTexture2D(GL_DRAW_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, TextureTarget::_2D,
                                    TextureID(), 0);
    }

    void onEnd(Context *angle, GLint numActivePLSPlanes) override
    {
        // Restore the image bindings. Since glBindImageTexture and any commands that modify
        // textures are banned while PLS is active, these will all still be alive and valid.
        ASSERT(mSavedImageBindings.size() == (size_t)numActivePLSPlanes);
        for (GLuint unit = 0; unit < mSavedImageBindings.size(); ++unit)
        {
            const ImageUnit &binding = mSavedImageBindings[unit];
            angle->bindImageTexture(unit, binding.texture.id(), binding.level, binding.layered,
                                    binding.layer, binding.access, binding.format);
        }
        mSavedImageBindings.clear();

        // Restore the default framebuffer width/height.
        angle->framebufferParameteri(GL_DRAW_FRAMEBUFFER, GL_FRAMEBUFFER_DEFAULT_WIDTH,
                                     mSavedFramebufferDefaultWidth);
        angle->framebufferParameteri(GL_DRAW_FRAMEBUFFER, GL_FRAMEBUFFER_DEFAULT_HEIGHT,
                                     mSavedFramebufferDefaultHeight);
    }

    void barrier(Context *angle) override
    {
        ASSERT(!angle->getExtensions().shaderPixelLocalStorageCoherentANGLE);
        angle->memoryBarrier(GL_SHADER_IMAGE_ACCESS_BARRIER_BIT);
    }

  private:
    // D3D and ES require us to pack all PLS formats into r32f, r32i, or r32ui images.
    const ImageFormatting mImageFormatting;
    FramebufferID mScratchFramebufferForClearing{};

    // Saved values to restore during onEnd().
    GLint mSavedFramebufferDefaultWidth;
    GLint mSavedFramebufferDefaultHeight;
    std::vector<ImageUnit> mSavedImageBindings;
};
}  // namespace

std::unique_ptr<PixelLocalStorage> PixelLocalStorage::MakeImageLoadStore(
    Context *angle,
    ImageFormatting imageFormatting)
{
    return std::make_unique<PixelLocalStorageImageLoadStore>(angle, imageFormatting);
}

}  // namespace gl
