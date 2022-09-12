//
// Copyright 2022 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//

// PixelLocalStorage.h: Defines the renderer-agnostic container classes
// gl::PixelLocalStorage and gl::PixelLocalStoragePlane for
// ANGLE_shader_pixel_local_storage.

#ifndef LIBANGLE_PIXEL_LOCAL_STORAGE_H_
#define LIBANGLE_PIXEL_LOCAL_STORAGE_H_

#include "angle_gl.h"
#include "libANGLE/ImageIndex.h"
#include "libANGLE/angletypes.h"

namespace gl
{

class Context;
class Texture;

// Holds the configuration of an ANGLE_shader_pixel_local_storage plane.
//
// Unlike normal framebuffer attachments, pixel local storage planes don't take effect until the
// application calls glBeginPixelLocalStorageANGLE, and the manner in which they take effect is
// highly dependent on the backend implementation. A PixelLocalStoragePlane is just a plain data
// description what to set up later once PLS is enabled.
class PixelLocalStoragePlane : angle::NonCopyable
{
  public:
    ~PixelLocalStoragePlane();

    // Called when the context is lost or destroyed. Causes this class to clear its GL object
    // handles.
    void onContextObjectsLost();

    // Called when the owning framebuffer is being destroyed. Causes this class to release its
    // texture object reference.
    void onFramebufferDestroyed(const Context *);

    void deinitialize(Context *);
    void setMemoryless(Context *, GLenum internalformat);
    void setTextureBacked(Context *, Texture *, int level, int layer);

    bool isDeinitialized() const { return mInternalformat == GL_NONE; }

    bool isMemoryless() const
    {
        // isMemoryless() should be false if the plane is deinitialized.
        ASSERT(!(isDeinitialized() && mMemoryless));
        return mMemoryless;
    }

    // [ANGLE_shader_pixel_local_storage] Section 4.4.2.X "Configuring Pixel Local Storage
    // on a Framebuffer": When a texture object is deleted, any pixel local storage plane to
    // which it was bound is automatically converted to a memoryless plane of matching
    // internalformat.
    void convertToMemorylessIfTextureIDDeleted(const Context *);

    // GL_PIXEL_LOCAL_FORMAT_ANGLE, GL_PIXEL_LOCAL_TEXTURE_NAME_ANGLE,
    // GL_PIXEL_LOCAL_TEXTURE_LEVEL_ANGLE, GL_PIXEL_LOCAL_TEXTURE_LAYER_ANGLE
    //
    // The caller must ensure convertToMemorylessIfTextureIDDeleted() has been called before calling
    // this method. Generally this happens during validation.
    GLint getIntegeri(const Context *, GLenum target, GLuint index);

    // If this plane is texture backed, stores the bound texture image's {width, height, 0} to
    // Extents and returns true. Otherwise returns false.
    //
    // The caller must ensure convertToMemorylessIfTextureIDDeleted() has been called before calling
    // this method. Generally this happens during validation.
    bool getTextureImageExtents(const Context *, Extents *);

    // Attaches this plane to the specified color attachment point on the current draw framebuffer.
    void attachToDrawFramebuffer(Context *, Extents plsExtents, GLenum colorAttachment);

    // Clears the draw buffer at 0-based index 'drawbuffer' on the current framebuffer. Reads the
    // clear value from 'data' if 'loadop' is GL_CLEAR_ANGLE, otherwise clears to zero.
    //
    // 'data' is interpereted as either 4 GLfloats, 4 GLints, or 4 GLuints, depending on
    // mInternalFormat.
    //
    // The GL scissor test must be disabled, since the intention is to clear the entire surface.
    //
    // loadop must be either GL_ZERO or GL_CLEAR_ANGLE.
    void performLoadOperationClear(Context *, GLint drawbuffer, GLenum loadop, const void *data);

    // Binds this PLS plane to a texture image unit for image load/store shader operations.
    void bindToImage(Context *, Extents plsExtents, GLuint unit, bool needsR32Packing);

  private:
    // Returns true if the texture ID bound to this plane has been deleted.
    bool isTextureIDDeleted(const Context *) const;

    // Ensures we have an internal backing texture for memoryless planes. In GL, we need a backing
    // texture even if the plane is memoryless; glInvalidateFramebuffer() will ideally prevent the
    // driver from writing out data where possible.
    void ensureBackingIfMemoryless(Context *, Extents plsSize);

    GLenum mInternalformat = GL_NONE;  // GL_NONE if this plane is in a deinitialized state.
    bool mMemoryless       = false;
    TextureID mMemorylessTextureID{};  // We own memoryless backing textures and must delete them.
    ImageIndex mTextureImageIndex;
    Texture *mTextureRef = nullptr;
};

// Manages a collection of PixelLocalStoragePlanes and applies them to ANGLE's GL state.
//
// The main magic of ANGLE_shader_pixel_local_storage happens inside shaders, so we just emulate the
// client API on top of ANGLE's OpenGL ES API for simplicity.
class PixelLocalStorage
{
  public:
    static std::unique_ptr<PixelLocalStorage> Make(Context *);

    PixelLocalStorage();
    virtual ~PixelLocalStorage();

    // Called when the owning framebuffer is being destroyed.
    void onFramebufferDestroyed(const Context *);

    // Deletes any GL objects that have been allocated for pixel local storage. These can't be
    // cleaned up in the destructor because they require a non-const Context object.
    virtual void deleteContextObjects(Context *);

    PixelLocalStoragePlane &getPlane(GLint plane)
    {
        ASSERT(0 <= plane && plane < IMPLEMENTATION_MAX_PIXEL_LOCAL_STORAGE_PLANES);
        return mPlanes[plane];
    }

    void deinitialize(Context *angle, GLint plane) { mPlanes[plane].deinitialize(angle); }
    void setMemoryless(Context *angle, GLint plane, GLenum internalformat)
    {
        mPlanes[plane].setMemoryless(angle, internalformat);
    }
    void setTextureBacked(Context *angle, GLint plane, Texture *tex, int level, int layer)
    {
        mPlanes[plane].setTextureBacked(angle, tex, level, layer);
    }

    // Called by validation after ensuring all active PLS planes have the same dimensions. Stores
    // the PLS rendering dimensions for future reference.
    void setPLSExtents(Extents plsExtents) { mPLSExtents = plsExtents; }

    void begin(Context *, GLsizei n, const GLenum loadops[], const void *cleardata);
    void end(Context *);
    virtual void barrier(Context *) = 0;

  protected:
    // Called when the context is lost or destroyed. Causes the subclass to clear its GL object
    // handles.
    virtual void onContextObjectsLost() = 0;

    virtual void onBegin(Context *,
                         GLsizei n,
                         const GLenum loadops[],
                         const char *cleardata,
                         Extents plsSize) = 0;

    virtual void onEnd(Context *, GLsizei numActivePLSPlanes) = 0;

  private:
    std::array<PixelLocalStoragePlane, IMPLEMENTATION_MAX_PIXEL_LOCAL_STORAGE_PLANES> mPlanes;
    Extents mPLSExtents;

    // "n" from the last call to begin(), or 0 if pixel local storage is not active.
    GLsizei mNumActivePLSPlanes = 0;
};

}  // namespace gl

#endif  // LIBANGLE_PIXEL_LOCAL_STORAGE_H_
