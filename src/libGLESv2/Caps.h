#ifndef LIBGLESV2_CAPS_H
#define LIBGLESV2_CAPS_H

//
// Copyright (c) 2014 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//

#include <GLES2/gl2.h>
#include <GLES3/gl3.h>

#include <set>
#include <map>
#include <vector>

namespace gl
{

class TextureCaps
{
  public:
    TextureCaps();

    bool getTexture2DSupport() const;
    void setTexture2DSupport(bool support);

    bool getTextureCubeMapSupport() const;
    void setTextureCubeMapSupport(bool support);

    bool getTexture3DSupport() const;
    void setTexture3DSupport(bool support);

    bool getTexture2DArraySupport() const;
    void setTexture2DArraySupport(bool support);

    bool getTextureFilteringSupport() const;
    void setTextureFilteringSupport(bool support);

    bool getColorRenderingSupport() const;
    void setColorRenderingSupport(bool support);

    bool getDepthRenderingSupport() const;
    void setDepthRenderingSupport(bool support);

    bool getStencilRenderingSupport() const;
    void setStencilRenderingSupport(bool support);

    bool isSampleCountSupported(GLuint sampleCount) const;
    GLuint getMaxSupportedSamples() const;
    void addSampleSupport(GLuint sampleCount);

  private:
    bool mSupportsTexture2D;
    bool mSupportsTextureCubeMap;
    bool mSupportsTexture3D;
    bool mSupportsTexture2DArray;
    bool mSupportsTextureFiltering;
    bool mSupportsColorRendering;
    bool mSupportsDepthRendering;
    bool mSupportsStencilRendering;
    std::set<GLuint> mSupportedSampleCounts;
};

typedef std::map<GLenum, TextureCaps> TextureFormatCapsMap;

class Caps
{
  public:
    Caps();

    const TextureCaps &getTextureFormatCaps(GLenum format) const;
    const TextureFormatCapsMap &getTextureFormatCapsMap() const;
    void setTextureFormatCaps(GLenum format, const TextureCaps &support);

    bool get32BitIndexSupport() const;
    void set32BitIndexSupport(bool support);

    bool getPackedDepthStencilSupport() const;
    void setPackedDepthStencilSupport(bool support);

    bool getProgramBinarySupport() const;
    void setProgramBinarySupport(bool support);

    bool getRGB8AndRGBA8TextureSupport() const;
    void setRGB8AndRGBA8TextureSupport(bool support);

    bool getBGRA8TextureSupport() const;
    void setBGRA8TextureSupport(bool support);

    bool getBGRAReadSupport() const;
    void setBGRAReadSupport(bool support);

    bool getPixelBufferObjectSupport() const;
    void setPixelBufferObjectSupport(bool support);

    bool getMapBufferSupport() const;
    void setMapBufferSupport(bool support);

    bool getMapBufferRangeSupport() const;
    void setMapBufferRangeSupport(bool support);

    bool getHalfFloatTextureSupport() const;
    void setHalfFloatTextureSupport(bool support);

    bool getHalfFloatTextureFilteringSupport() const;
    void setHalfFloatTextureFilteringSupport(bool support);

    bool getFloatTextureSupport() const;
    void setFloatTextureSupport(bool support);

    bool getFloatTextureFilteringSupport() const;
    void setFloatTextureFilteringSupport(bool support);

    bool getRGTextureSupport() const;
    void setRGTextureSupport(bool support);

    bool getDXT1TextureSupport() const;
    void setDXT1TextureSupport(bool support);

    bool getDXT3TextureSupport() const;
    void setDXT3TextureSupport(bool support);

    bool getDXT5TextureSupport() const;
    void setDXT5TextureSupport(bool support);

    bool getDepthTextureSupport() const;
    void setDepthTextureSupport(bool support);

    // HACK: inlined since it is used by egl::Display
    bool getNPOTTextureSupport() const { return mSupportsNPOTTextures; }
    void setNPOTTextureSupport(bool support);

    bool getDrawBuffersSupport() const;
    void setDrawBuffersSupport(bool support);

    bool getTextureStorageSupport() const;
    void setTextureStorageSupport(bool support);

    bool getAnisotropicFilteringSupport() const;
    void setAnisotropicFilteringSupport(bool support);

    GLfloat getMaxTextureAnisotropy() const;
    void setMaxTextureAnisotropy(GLfloat value);

    bool getOcclusionQuerySupport() const;
    void setOcclusionQuerySupport(bool support);

    bool getFenceQuerySupport() const;
    void setFenceQuerySupport(bool support);

    bool getANGLETimerQuerySupport() const;
    void setANGLETimerQuerySupport(bool support);

    bool getRobustnessSupport() const;
    void setRobustnessSupport(bool support);

    bool getMinMaxBlendingSupport() const;
    void setMinMaxBlendingSupport(bool support);

    bool getANGLEFramebufferBlitSupport() const;
    void setANGLEFramebufferBlitSupport(bool support);

    bool getANGLEFramebufferMultisampleSupport() const;
    void setANGLEFramebufferMultisampleSupport(bool support);

    bool getANGLEInstancedArraysSupport() const;
    void setANGLEInstancedArraysSupport(bool support);

    bool getANGLEPackReverseRowOrderSupport() const;
    void setANGLEPackReverseRowOrderSupport(bool support);

    bool getStandardDerivativeSupport() const;
    void setStandardDerivativeSupport(bool support);

    bool getTextureLODSupport() const;
    void setTextureLODSupport(bool support);

    bool getFragDepthSupport() const;
    void setFragDepthSupport(bool support);

    bool getANGLETextureUsageSupport() const;
    void setANGLETextureUsageSupport(bool support);

    bool getANGLETranslatedSourceSupport() const;
    void setANGLETranslatedSourceSupport(bool support);

    bool getColorBufferFloatSupport() const;
    void setColorBufferFloatSupport(bool support);

    const std::string &getExtensionString() const;
    const char *getStaticExtensionString() const;
    const std::string &getExtensionString(size_t idx) const;
    const char *getStaticExtensionString(size_t idx) const;
    size_t getExtensionStringCount() const;

  private:
    // Texture format support
    TextureFormatCapsMap mTextureCaps;

    // ES2 Extension support

    // GL_OES_element_index_uint
    bool mSupports32BitIndices;

    // GL_OES_packed_depth_stencil
    bool mSupportsPackedDepthStencil;

    // GL_OES_get_program_binary
    bool mSupportsProgramBinary;

    // GL_OES_rgb8_rgba8
    // Implies that TextureCaps for GL_RGB8 and GL_RGBA8 exist
    bool mSupportsRGB8AndRGBA8Textures;

    // GL_EXT_texture_format_BGRA8888
    // Implies that TextureCaps for GL_BGRA8 exist
    bool mSupportsBGRA8Textures;

    // GL_EXT_read_format_bgra
    bool mSupportsBGRAReads;

    // GL_NV_pixel_buffer_object
    bool mSupportsPixelBufferObjects;

    // GL_OES_mapbuffer and GL_EXT_map_buffer_range
    bool mSupportsMapBuffer;
    bool mSupportsMapBufferRange;

    // GL_OES_texture_half_float and GL_OES_texture_half_float_linear
    // Implies that TextureCaps for GL_RGB16F, GL_RGBA16F, GL_ALPHA32F_EXT, GL_LUMINANCE32F_EXT and
    // GL_LUMINANCE_ALPHA32F_EXT exist
    bool mSupportsHalfFloatTextures;
    bool mSupportsHalfFloatTextureFiltering;

    // GL_OES_texture_float and GL_OES_texture_float_linear
    // Implies that TextureCaps for GL_RGB32F, GL_RGBA32F, GL_ALPHA16F_EXT, GL_LUMINANCE16F_EXT and
    // GL_LUMINANCE_ALPHA16F_EXT exist
    bool mSupportsFloatTextures;
    bool mSupportsFloatTextureFiltering;

    // GL_EXT_texture_rg
    // Implies that TextureCaps for GL_R8, GL_RG8 (and floating point R/RG texture formats if floating point extensions
    // are also present) exist
    bool mSupportsRGTextures;

    // GL_EXT_texture_compression_dxt1, GL_ANGLE_texture_compression_dxt3 and GL_ANGLE_texture_compression_dxt5
    // Implies that TextureCaps for GL_COMPRESSED_RGB_S3TC_DXT1_EXT, GL_COMPRESSED_RGBA_S3TC_DXT1_EXT
    // GL_COMPRESSED_RGBA_S3TC_DXT3_ANGLE and GL_COMPRESSED_RGBA_S3TC_DXT5_ANGLE
    bool mSupportsDXT1Textures;
    bool mSupportsDXT3Textures;
    bool mSupportsDXT5Textures;

    // GL_ANGLE_depth_texture
    bool mSupportsDepthTextures;

    // GL_EXT_texture_storage
    bool mSupportsTextureStorage;

    // GL_OES_texture_npot
    bool mSupportsNPOTTextures;

    // GL_EXT_draw_buffers
    bool mSupportsDrawBuffers;

    // GL_EXT_texture_filter_anisotropic
    bool mSupportsAnisotropicFiltering;
    GLfloat mMaxTextureAnisotropy;

    // GL_EXT_occlusion_query_boolean
    bool mSupportsOcclusionQueries;

    // GL_NV_fence
    bool mSupportsEventQueries;

    // GL_ANGLE_timer_query
    bool mSupportsANGLETimerQueries;

    // GL_EXT_robustness
    bool mSupportsRobustness;

    // GL_EXT_blend_minmax
    bool mSupportsMinMaxBlending;

    // GL_ANGLE_framebuffer_blit
    bool mSupportsANGLEFramebufferBlit;

    // GL_ANGLE_framebuffer_multisample
    bool mSupportsANGLEFramebufferMultisample;

    // GL_ANGLE_instanced_arrays
    bool mSupportsANGLEInstancedArrays;

    // GL_ANGLE_pack_reverse_row_order
    bool mSupportsANGLEPackReverseRowOrder;

    // GL_OES_standard_derivatives
    bool mSupportsDerivitives;

    // GL_EXT_shader_texture_lod
    bool mSupportsTextureLOD;

    // GL_EXT_frag_depth
    bool mSupportsFragDepth;

    // GL_ANGLE_texture_usage
    bool mSupportsANGLETextureUsage;

    // GL_ANGLE_translated_shader_source
    bool mSupportsANGLETranslatedSource;

    // ES3 Extension support

    // GL_EXT_color_buffer_float
    bool mSupportsColorBufferFloat;

    // Extension strings
    void cleanExtensionStrings() const;
    mutable bool mExtensionStringsDirty;
    mutable std::string mCombinedExtensionString;
    mutable const char *mStaticCombinedExtensionString;
    mutable std::vector<const char*> mStaticExtensionStrings;

    std::vector<std::string> mSupportedExtensions;
};

}

#endif // LIBGLESV2_CAPS_H
