//
// Copyright 2002 The ANGLE Project Authors. All rights reserved.
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
class PixelLocalStoragePlane
{
  public:
    PixelLocalStoragePlane()                               = default;
    PixelLocalStoragePlane(const PixelLocalStoragePlane &) = delete;
    PixelLocalStoragePlane(PixelLocalStoragePlane &&);
    ~PixelLocalStoragePlane() { ASSERT(!mTextureRef); }  // Call deleteContextObjects first!

    PixelLocalStoragePlane &operator=(const PixelLocalStoragePlane &) = delete;

    void deinitialize(Context *);
    void setMemoryless(Context *, GLenum internalformat);
    void setTextureBacked(Context *, Texture *, int level, int layer);

    void setClearValuef(const GLfloat val[]) { memcpy(mClearValuef, val, sizeof(mClearValuef)); }
    void setClearValuei(const GLint val[]) { memcpy(mClearValuei, val, sizeof(mClearValuei)); }
    void setClearValueui(const GLuint val[]) { memcpy(mClearValueui, val, sizeof(mClearValueui)); }

    // [ANGLE_shader_pixel_local_storage] Section 4.4.2.X "Configuring Pixel Local Storage
    // on a Framebuffer": When a texture object is deleted, any pixel local storage plane to
    // which it was bound is automatically converted to a memoryless plane of matching
    // internalformat.
    void convertToMemorylessIfTextureIDDeleted(const Context *);

    bool isDeinitialized() const { return mInternalformat == GL_NONE; }

    bool isMemoryless() const
    {
        // isMemoryless() should be false if the plane is deinitialized.
        ASSERT(!(isDeinitialized() && mMemoryless));
        return mMemoryless;
    }

    // GL_PIXEL_LOCAL_FORMAT_ANGLE, GL_PIXEL_LOCAL_TEXTURE_NAME_ANGLE,
    // GL_PIXEL_LOCAL_TEXTURE_LEVEL_ANGLE, GL_PIXEL_LOCAL_TEXTURE_LAYER_ANGLE
    GLint getIntegeri(GLenum target, GLuint index);

    // If this plane is texture backed, stores the bound texture image's {width, height, 0} to
    // Extents and returns true. Otherwise returns false.
    //
    // The caller must call ensure convertToMemorylessIfTextureIDDeleted() has been called before
    // calling this method.
    bool getTextureImageExtents(const Context *, Extents *);

    // Attaches this plane to the specified color attachment point on the current draw framebuffer.
    void attachToDrawFramebuffer(Context *, Extents plsExtents, GLenum colorAttachment);

    // If loadop is GL_ZERO or GL_CLEAR_ANGLE, clears the draw buffer at 0-based index 'drawbuffer'
    // on the current framebuffer, using either zero or the plane's clear value for its format.
    //
    // The GL scissor test must be disabled.
    void performLoadOperationClear(Context *, GLint drawbuffer, GLenum loadop);

    // Specifies how the backend translator formats images that are used for pixel local storage.
    enum class ImageFormatting : bool
    {
        Native,    // PLS formats are mapped directly to native image formats.
        R32Packed  // PLS formats are packed manually into r32f, r32i, and r32ui image formats.
    };

    // Binds this PLS plane to a texture image unit for image load/store shader operations.
    void bindToImage(Context *, Extents plsExtents, GLuint unit, ImageFormatting);

  private:
    // Returns true if the texture handle associated with our texture ref has been deleted.
    bool isTextureIDDeleted(const Context *) const;

    // Ensures we have an internal backing texture for memoryless planes. In GL, we need a backing
    // texture even if the plane is memoryless; glInvalidateFramebuffer() will ideally prevent the
    // driver from writing out data where possible.
    void ensureBackingIfMemoryless(Context *, Extents plsSize);

    GLenum mInternalformat = GL_NONE;  // GL_NONE if this plane is in a deinitialized state.
    Texture *mTextureRef   = nullptr;
    ImageIndex mTextureImageIndex;
    bool mMemoryless = false;

    GLfloat mClearValuef[4] = {};
    GLint mClearValuei[4]   = {};
    GLuint mClearValueui[4] = {};
};

// Manages a collection of PixelLocalStoragePlanes and applies them to ANGLE's GL state.
//
// The main magic of ANGLE_shader_pixel_local_storage happens inside shaders, so we just emulate the
// client API on top of ANGLE's OpenGL ES API for simplicity.
class PixelLocalStorage
{
  public:
    // Creates a pixel local storage implementation that uses image load/store shader operations.
    using ImageFormatting = PixelLocalStoragePlane::ImageFormatting;
    static std::unique_ptr<PixelLocalStorage> MakeImageLoadStore(Context *, ImageFormatting);

    PixelLocalStorage();
    virtual ~PixelLocalStorage();

    // Deletes any GL objects that have been allocated for pixel local storage. These can't be
    // cleaned up in the destructor because they require a non-const Context object.
    virtual void deleteContextObjects(Context *);

    bool isPlaneDeinitialized(GLint plane);
    GLint getNumPlanes() const { return (GLint)mPlanes.size(); }
    PixelLocalStoragePlane &getPlane(GLint plane) { return mPlanes[plane]; }

    void deinitialize(Context *, GLint plane);
    void setMemoryless(Context *angle, GLint plane, GLenum internalformat)
    {
        growPlanesToFit(plane).setMemoryless(angle, internalformat);
    }
    void setTextureBacked(Context *angle, GLint plane, Texture *tex, int level, int layer)
    {
        growPlanesToFit(plane).setTextureBacked(angle, tex, level, layer);
    }
    void setClearValuef(GLint plane, const GLfloat value[])
    {
        growPlanesToFit(plane).setClearValuef(value);
    }
    void setClearValuei(GLint plane, const GLint value[])
    {
        growPlanesToFit(plane).setClearValuei(value);
    }
    void setClearValueui(GLint plane, const GLuint value[])
    {
        growPlanesToFit(plane).setClearValueui(value);
    }

    // Sets the PLS rendering dimensions. Validation ensures all active PLS planes have these same
    // dimensions, as well as the framebuffer's rendering area, if it has attachments.
    void setPLSExtents(Extents plsExtents) { mPLSExtents = plsExtents; }

    void begin(Context *, GLsizei n, const GLenum loadops[]);
    void end(Context *);
    virtual void barrier(Context *) = 0;

  protected:
    virtual void onDeleteContextObjects(Context *)                                      = 0;
    virtual void onBegin(Context *, GLsizei n, const GLenum loadops[], Extents plsSize) = 0;
    virtual void onEnd(Context *, GLint numActivePLSPlanes)                             = 0;

  private:
    PixelLocalStoragePlane &growPlanesToFit(GLint plane);

    std::vector<PixelLocalStoragePlane> mPlanes;
    Extents mPLSExtents;

    // "n" from the last call to begin(), or 0 if pixel local storage is not active.
    GLint mNumActivePLSPlanes = 0;
};

}  // namespace gl

#endif  // LIBANGLE_PIXEL_LOCAL_STORAGE_H_
