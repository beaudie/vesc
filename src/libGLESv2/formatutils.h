//
// Copyright (c) 2013 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//

// formatutils.h: Queries for GL image formats.

#ifndef LIBGLESV2_FORMATUTILS_H_
#define LIBGLESV2_FORMATUTILS_H_

#include "angle_gl.h"

#include "libGLESv2/Caps.h"
#include "libGLESv2/angletypes.h"

typedef void (*MipGenerationFunction)(unsigned int sourceWidth, unsigned int sourceHeight, unsigned int sourceDepth,
                                      const unsigned char *sourceData, int sourceRowPitch, int sourceDepthPitch,
                                      unsigned char *destData, int destRowPitch, int destDepthPitch);

typedef void (*LoadImageFunction)(int width, int height, int depth,
                                  const void *input, unsigned int inputRowPitch, unsigned int inputDepthPitch,
                                  void *output, unsigned int outputRowPitch, unsigned int outputDepthPitch);

typedef void (*InitializeTextureDataFunction)(int width, int height, int depth,
                                              void *output, unsigned int outputRowPitch, unsigned int outputDepthPitch);

typedef void (*ColorReadFunction)(const void *source, void *dest);
typedef void (*ColorWriteFunction)(const void *source, void *dest);
typedef void (*ColorCopyFunction)(const void *source, void *dest);

typedef void (*VertexCopyFunction)(const void *input, size_t stride, size_t count, void *output);

namespace gl
{

struct FormatTypeInfo
{
    GLenum internalFormat;
    ColorWriteFunction colorWriteFunction;
};
const FormatTypeInfo &GetFormatTypeInfo(GLenum format, GLenum type);

struct TypeInfo
{
    GLuint bytes;
    bool specialInterpretation;
};
const TypeInfo &GetTypeInfo(GLenum type);

struct InternalFormatInfo
{
    GLuint redBits;
    GLuint greenBits;
    GLuint blueBits;

    GLuint luminanceBits;

    GLuint alphaBits;
    GLuint sharedBits;

    GLuint depthBits;
    GLuint stencilBits;

    GLuint pixelBytes;

    GLuint componentCount;

    bool compressed;
    GLuint compressedBlockWidth;
    GLuint compressedBlockHeight;

    GLenum format;
    GLenum type;

    GLenum componentType;
    GLenum colorEncoding;

    typedef bool (*SupportCheckFunction)(GLuint, const Extensions &);
    SupportCheckFunction textureSupport;
    SupportCheckFunction renderSupport;
    SupportCheckFunction filterSupport;

    GLuint computeRowPitch(GLenum type, GLsizei width, GLint alignment) const;
    GLuint computeDepthPitch(GLenum type, GLsizei width, GLsizei height, GLint alignment) const;
    GLuint computeBlockSize(GLenum type, GLsizei width, GLsizei height) const;
};
const InternalFormatInfo &GetInternalFormatInfo(GLenum internalFormat);

typedef std::set<GLenum> FormatSet;
const FormatSet &GetAllSizedInternalFormats();

}

#endif LIBGLESV2_FORMATUTILS_H_
