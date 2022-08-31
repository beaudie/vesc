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

PixelLocalStoragePlane::~PixelLocalStoragePlane()
{
    // Call deinitialize or onContextTeardown first!
    ASSERT(!mTextureRef && !mMemorylessTextureID.value);
}

void PixelLocalStoragePlane::onContextTeardown(const Context *angle)
{
    // We normally call deleteTexture on memoryless plane texture IDs, since we own them, but in
    // this case we can just let go of them because the context is being torn down.
    mMemorylessTextureID = TextureID();
    if (mTextureRef)
    {
        mTextureRef->release(angle);
        mTextureRef = nullptr;
    }
}

void PixelLocalStoragePlane::deinitialize(Context *angle)
{
    mInternalformat = GL_NONE;
    mMemoryless     = false;
    if (mMemorylessTextureID.value)
    {
        // The app could have technically deleted mMemorylessTextureID by guessing its value and
        // calling glDeleteTextures, but it seems unnecessary to worry about that here. (Worst case
        // we delete one of their textures.) This also isn't a problem in WebGL.
        angle->deleteTexture(mMemorylessTextureID);
        mMemorylessTextureID = TextureID();
    }
    if (mTextureRef)
    {
        mTextureRef->release(angle);
        mTextureRef = nullptr;
    }
}

void PixelLocalStoragePlane::setMemoryless(Context *angle, GLenum internalformat)
{
    deinitialize(angle);
    mInternalformat    = internalformat;
    mMemoryless        = true;
    mTextureImageIndex = ImageIndex::MakeFromType(TextureType::_2D, 0, 0);
    // The backing texture will get allocated lazily, once we know what dimensions it should be.
    ASSERT(!mMemorylessTextureID.value);
    ASSERT(mTextureRef == nullptr);
}

void PixelLocalStoragePlane::setTextureBacked(Context *angle, Texture *tex, int level, int layer)
{
    deinitialize(angle);
    ASSERT(tex->getImmutableFormat());
    mInternalformat    = tex->getState().getBaseLevelDesc().format.info->internalFormat;
    mMemoryless        = false;
    mTextureImageIndex = ImageIndex::MakeFromType(tex->getType(), level, layer);
    mTextureRef        = tex;
    mTextureRef->addRef();
}

bool PixelLocalStoragePlane::isTextureIDDeleted(const Context *angle) const
{
    // We can tell if the texture has been deleted by looking up mTextureRef's ID on the context. If
    // they don't match, it's been deleted.
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
        ASSERT(mTextureRef);
        mMemoryless = true;
        // mMemorylessTextureID can be nonzero here if the app deleted it by guessing the ID and
        // calling glDeleteTextures! In that case, we can just drop it.
        mMemorylessTextureID = TextureID();
        mTextureImageIndex   = ImageIndex::MakeFromType(TextureType::_2D, 0, 0);
        mTextureRef->release(angle);
        // The backing texture will get allocated lazily, once we know what dimensions it should be.
        mTextureRef = nullptr;
    }
}

GLint PixelLocalStoragePlane::getIntegeri(const Context *angle, GLenum target, GLuint index)
{
    ASSERT(!isDeinitialized());
    ASSERT(!isTextureIDDeleted(angle));  // Call convertToMemorylessIfTextureIDDeleted() first!
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
        ASSERT(mTextureRef);
        return;
    }

    // Internal textures backing memoryless planes are always 2D and not mipmapped.
    ASSERT(mTextureImageIndex.getType() == TextureType::_2D);
    ASSERT(mTextureImageIndex.getLevelIndex() == 0);
    ASSERT(mTextureImageIndex.getLayerIndex() == 0);
    ASSERT(bool(mMemorylessTextureID.value) == bool(mTextureRef));

    // Do we need to allocate a new backing texture?
    if (!mTextureRef || (GLsizei)mTextureRef->getWidth(TextureTarget::_2D, 0) != plsExtents.width ||
        (GLsizei)mTextureRef->getHeight(TextureTarget::_2D, 0) != plsExtents.height)
    {
        // Call setMemoryless() to release our current data.
        setMemoryless(angle, mInternalformat);
        ASSERT(!mTextureRef);
        ASSERT(!mMemorylessTextureID.value);

        // Create a new texture that backs the memoryless plane.
        angle->genTextures(1, &mMemorylessTextureID);
        {
            ScopedBindTexture2D scopedBindTexture2D(angle, mMemorylessTextureID);
            angle->bindTexture(TextureType::_2D, mMemorylessTextureID);
            angle->texStorage2D(TextureType::_2D, 1, mInternalformat, plsExtents.width,
                                plsExtents.height);
        }

        mTextureRef = angle->getTexture(mMemorylessTextureID);
        ASSERT(mTextureRef);
        ASSERT(mTextureRef->id() == mMemorylessTextureID);
        mTextureRef->addRef();
    }
}

void PixelLocalStoragePlane::attachToDrawFramebuffer(Context *angle,
                                                     Extents plsExtents,
                                                     GLenum colorAttachment)
{
    ASSERT(!isDeinitialized());
    ASSERT(!isTextureIDDeleted(angle));  // Call convertToMemorylessIfTextureIDDeleted() first!
    ensureBackingIfMemoryless(angle, plsExtents);
    ASSERT(mTextureRef);
    if (mTextureImageIndex.usesTex3D())  // GL_TEXTURE_3D or GL_TEXTURE_2D_ARRAY.
    {
        angle->framebufferTextureLayer(GL_DRAW_FRAMEBUFFER, colorAttachment, mTextureRef->id(),
                                       mTextureImageIndex.getLevelIndex(),
                                       mTextureImageIndex.getLayerIndex());
    }
    else
    {
        angle->framebufferTexture2D(GL_DRAW_FRAMEBUFFER, colorAttachment,
                                    mTextureImageIndex.getTarget(), mTextureRef->id(),
                                    mTextureImageIndex.getLevelIndex());
    }
}

void PixelLocalStoragePlane::performLoadOperationClear(Context *angle,
                                                       GLint drawBuffer,
                                                       GLenum loadop,
                                                       const void *data)
{
    ASSERT(loadop == GL_ZERO || loadop == GL_CLEAR_ANGLE);
    // The GL scissor test must be disabled, since the intention is to clear the entire surface.
    ASSERT(!angle->getState().isScissorTestEnabled());
    switch (mInternalformat)
    {
        case GL_RGBA8:
        case GL_R32F:
        {
            GLfloat clearValue[4]{};
            if (loadop == GL_CLEAR_ANGLE)
            {
                memcpy(clearValue, data, sizeof(clearValue));
            }
            angle->clearBufferfv(GL_COLOR, drawBuffer, clearValue);
            break;
        }
        case GL_RGBA8I:
        {
            GLint clearValue[4]{};
            if (loadop == GL_CLEAR_ANGLE)
            {
                memcpy(clearValue, data, sizeof(clearValue));
            }
            angle->clearBufferiv(GL_COLOR, drawBuffer, clearValue);
            break;
        }
        case GL_RGBA8UI:
        case GL_R32UI:
        {
            GLuint clearValue[4]{};
            if (loadop == GL_CLEAR_ANGLE)
            {
                memcpy(clearValue, data, sizeof(clearValue));
            }
            angle->clearBufferuiv(GL_COLOR, drawBuffer, clearValue);
            break;
        }
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
    if (mTextureRef->getType() != TextureType::_2D)
    {
        // TODO(anglebug.com/7279): Texture types other than GL_TEXTURE_2D will take a lot of
        // consideration to support on all backends. Hold of on fully implementing them until the
        // other backends are in place.
        UNIMPLEMENTED();
    }
    angle->bindImageTexture(unit, mTextureRef->id(), mTextureImageIndex.getLevelIndex(), GL_FALSE,
                            mTextureImageIndex.getLayerIndex(), GL_READ_WRITE, imageBindingFormat);
}

PixelLocalStorage::PixelLocalStorage() {}
PixelLocalStorage::~PixelLocalStorage() {}

void PixelLocalStorage::onContextTeardown(const Context *angle)
{
    ASSERT(angle->getRefCount() == 0);  // Is the context actually being torn down?
    for (PixelLocalStoragePlane &plane : mPlanes)
    {
        plane.onContextTeardown(angle);
    }
}

void PixelLocalStorage::deleteContextObjects(Context *angle)
{
    for (PixelLocalStoragePlane &plane : mPlanes)
    {
        plane.deinitialize(angle);
    }
}

void PixelLocalStorage::begin(Context *angle,
                              GLsizei n,
                              const GLenum loadops[],
                              const void *cleardata)
{
    onBegin(angle, n, loadops, reinterpret_cast<const char *>(cleardata), mPLSExtents);
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

    // Call deleteContextObjects or onContextTeardown first!
    ~PixelLocalStorageImageLoadStore() override { ASSERT(!mScratchFramebufferForClearing.value); }

    void onContextTeardown(const Context *angle) override
    {
        PixelLocalStorage::onContextTeardown(angle);
        mScratchFramebufferForClearing = FramebufferID();  // Let go of GL objects.
    }

    void deleteContextObjects(Context *angle) override
    {
        // Delete the scratch framebuffer first, to avoid any trickyness with orphaning.
        angle->deleteFramebuffer(mScratchFramebufferForClearing);
        mScratchFramebufferForClearing = FramebufferID();
        PixelLocalStorage::deleteContextObjects(angle);
    }

    void onBegin(Context *angle,
                 GLsizei n,
                 const GLenum loadops[],
                 const char *cleardata,
                 Extents plsExtents) override
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
        ScopedBindDrawFramebuffer scopedBindDrawFramebuffer(angle, mScratchFramebufferForClearing);
        ScopedDisableScissor scopedDisableScissor(angle);

        // Bind and clear the PLS planes.
        bool didClear = false;
        for (int i = 0; i < n; ++i)
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
                plane.performLoadOperationClear(angle, 0, loadop, cleardata + i * 4 * 4);
                didClear = true;
            }
            plane.bindToImage(angle, plsExtents, i, mImageFormatting);
        }

        if (didClear)
        {
            // Detach the last PLS texture from the scratch framebuffer.
            angle->framebufferTexture2D(GL_DRAW_FRAMEBUFFER, GL_COLOR_ATTACHMENT0,
                                        TextureTarget::_2D, TextureID(), 0);
        }
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
