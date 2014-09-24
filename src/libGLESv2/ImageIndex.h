//
// Copyright 2014 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//

// ImageIndex.h: A helper struct for indexing into an Image array

#ifndef LIBGLESV2_IMAGE_INDEX_H_
#define LIBGLESV2_IMAGE_INDEX_H_

#include "angle_gl.h"

namespace gl
{

struct ImageIndex
{
    GLenum type;
    GLint mipIndex;
    GLint layerIndex;

    ImageIndex(GLenum typeIn, GLint mipIndexIn, GLint layerIndexIn);
    ImageIndex(const ImageIndex &other);
    ImageIndex &operator=(const ImageIndex &other);

    bool hasLayer() const { return layerIndex != ENTIRE_LEVEL; }

    static ImageIndex Make2D(GLint mipIndex);
    static ImageIndex MakeCube(GLenum target, GLint mipIndex);
    static ImageIndex Make2DArray(GLint mipIndex, GLint layerIndex);
    static ImageIndex Make3D(GLint mipIndex, GLint layerIndex = ENTIRE_LEVEL);

    static const GLint ENTIRE_LEVEL = static_cast<GLint>(-1);
};

class ImageIndexIterator
{
  public:

    static ImageIndexIterator Make2D(GLint minMip, GLint maxMip);
    static ImageIndexIterator MakeCube(GLint minMip, GLint maxMip);
    static ImageIndexIterator Make3D(GLint minMip, GLint maxMip, GLint minLayer, GLint maxLayer);
    static ImageIndexIterator Make2DArray(GLint minMip, GLint maxMip, const GLsizei *layerCounts);

    ImageIndex next();
    bool hasNext() const;
    void setMipMajorIteration(bool enabled) { mMipMajorIteration = enabled; }

  private:

    ImageIndexIterator(GLenum type, GLint minMip, GLint maxMip, GLint minLayer,
                       GLint maxLayer, const GLsizei *layerCounts);

    GLint maxLayer() const;

    GLenum mType;
    GLint mMinMip;
    GLint mMaxMip;
    GLint mMinLayer;
    GLint mMaxLayer;
    const GLsizei *mLayerCounts;
    GLint mCurrentMip;
    GLint mCurrentLayer;
    bool mMipMajorIteration;
};

}

#endif // LIBGLESV2_IMAGE_INDEX_H_
