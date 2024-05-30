//
// Copyright The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//

// PixmapImageSiblingEGL.h: Defines the PixmapImageSiblingEGL to wrap EGL images
// created from pixmaps

#ifndef LIBANGLE_RENDERER_GL_EGL_PIXMAPIMAGESIBLINGEGL_H_
#define LIBANGLE_RENDERER_GL_EGL_PIXMAPIMAGESIBLINGEGL_H_

#include "libANGLE/renderer/gl/egl/ExternalImageSiblingEGL.h"

namespace rx
{

class PixmapImageSiblingEGL : public ExternalImageSiblingEGL
{
  public:
    PixmapImageSiblingEGL(EGLClientBuffer buffer, const egl::AttributeMap &attribs);
    ~PixmapImageSiblingEGL() override;

    egl::Error initialize(const egl::Display *display) override;

    // ExternalImageSiblingImpl interface
    gl::Format getFormat() const override;
    bool isRenderable(const gl::Context *context) const override;
    bool isTexturable(const gl::Context *context) const override;
    bool isYUV() const override;
    bool hasProtectedContent() const override;
    gl::Extents getSize() const override;
    size_t getSamples() const override;

    // ExternalImageSiblingEGL interface
    EGLClientBuffer getBuffer() const override;
    void getImageCreationAttributes(std::vector<EGLint> *outAttributes) const override;

  private:
    EGLClientBuffer mBuffer;
    egl::AttributeMap mAttribs;
    gl::Extents mSize;
    gl::Format mFormat;
};

}  // namespace rx

#endif  // LIBANGLE_RENDERER_GL_EGL_PIXMAPIMAGESIBLINGEGL_H_
