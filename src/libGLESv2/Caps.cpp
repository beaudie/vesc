//
// Copyright (c) 2014 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//

#include "libGLESv2/Caps.h"
#include "common/debug.h"
#include "common/angleutils.h"

#include <algorithm>
#include <sstream>

namespace gl
{

TextureCaps::TextureCaps()
    : mSupportsTexture2D(false),
      mSupportsTextureCubeMap(false),
      mSupportsTexture3D(false),
      mSupportsTexture2DArray(false),
      mSupportsTextureFiltering(false),
      mSupportsColorRendering(false),
      mSupportsDepthRendering(false),
      mSupportsStencilRendering(false),
      mSupportedSampleCounts()
{
}

bool TextureCaps::getTexture2DSupport() const
{
    return mSupportsTexture2D;
}

void TextureCaps::setTexture2DSupport(bool support)
{
    mSupportsTexture2D = support;
}

bool TextureCaps::getTextureCubeMapSupport() const
{
    return mSupportsTextureCubeMap;
}

void TextureCaps::setTextureCubeMapSupport(bool support)
{
    mSupportsTextureCubeMap = support;
}

bool TextureCaps::getTexture3DSupport() const
{
    return mSupportsTexture3D;
}

void TextureCaps::setTexture3DSupport(bool support)
{
    mSupportsTexture3D = support;
}

bool TextureCaps::getTexture2DArraySupport() const
{
    return mSupportsTexture2DArray;
}

void TextureCaps::setTexture2DArraySupport(bool support)
{
    mSupportsTexture2DArray = support;
}

bool TextureCaps::getTextureFilteringSupport() const
{
    return mSupportsTextureFiltering;
}

void TextureCaps::setTextureFilteringSupport(bool support)
{
    mSupportsTextureFiltering = support;
}

bool TextureCaps::getColorRenderingSupport() const
{
    return mSupportsColorRendering;
}

void TextureCaps::setColorRenderingSupport(bool support)
{
    mSupportsColorRendering = support;
}

bool TextureCaps::getDepthRenderingSupport() const
{
    return mSupportsDepthRendering;
}

void TextureCaps::setDepthRenderingSupport(bool support)
{
    mSupportsDepthRendering = support;
}

bool TextureCaps::getStencilRenderingSupport() const
{
    return mSupportsStencilRendering;
}

void TextureCaps::setStencilRenderingSupport(bool support)
{
    mSupportsStencilRendering = support;
}

bool TextureCaps::isSampleCountSupported(GLuint sampleCount) const
{
    return mSupportedSampleCounts.find(sampleCount) != mSupportedSampleCounts.end();
}

GLuint TextureCaps::getMaxSupportedSamples() const
{
    return mSupportedSampleCounts.empty() ? 0 : *mSupportedSampleCounts.crbegin();
}

void TextureCaps::addSampleSupport(GLuint sampleCount)
{
    mSupportedSampleCounts.insert(sampleCount);
}

static void UpdateExtensionVector(const std::string& extension, bool supported, bool *dirtyFlag, std::vector<std::string> *extensionVector)
{
    std::vector<std::string>::iterator iter = std::find(extensionVector->begin(), extensionVector->end(), extension);
    if ((iter == extensionVector->end()) && supported == GL_TRUE)
    {
        *dirtyFlag = true;
        extensionVector->push_back(extension);
    }
    else if ((iter != extensionVector->end()) && supported != GL_TRUE)
    {
        *dirtyFlag = true;
        extensionVector->erase(iter);
    }
}

Caps::Caps()
    : mSupports32BitIndices(false),
      mSupportsPackedDepthStencil(false),
      mSupportsProgramBinary(false),
      mSupportsRGB8AndRGBA8Textures(false),
      mSupportsBGRA8Textures(false),
      mSupportsBGRAReads(false),
      mSupportsPixelBufferObjects(false),
      mSupportsMapBuffer(false),
      mSupportsMapBufferRange(false),
      mSupportsHalfFloatTextures(false),
      mSupportsHalfFloatTextureFiltering(false),
      mSupportsFloatTextures(false),
      mSupportsFloatTextureFiltering(false),
      mSupportsRGTextures(false),
      mSupportsDXT1Textures(false),
      mSupportsDXT3Textures(false),
      mSupportsDXT5Textures(false),
      mSupportsDepthTextures(false),
      mSupportsTextureStorage(false),
      mSupportsNPOTTextures(false),
      mSupportsDrawBuffers(false),
      mSupportsAnisotropicFiltering(false),
      mMaxTextureAnisotropy(1.0f),
      mSupportsOcclusionQueries(false),
      mSupportsEventQueries(false),
      mSupportsANGLETimerQueries(false),
      mSupportsRobustness(false),
      mSupportsMinMaxBlending(false),
      mSupportsANGLEFramebufferBlit(false),
      mSupportsANGLEFramebufferMultisample(false),
      mSupportsANGLEInstancedArrays(false),
      mSupportsANGLEPackReverseRowOrder(false),
      mSupportsDerivitives(false),
      mSupportsTextureLOD(false),
      mSupportsFragDepth(false),
      mSupportsANGLETextureUsage(false),
      mSupportsANGLETranslatedSource(false),
      mSupportsColorBufferFloat(false),

      mExtensionStringsDirty(true),
      mCombinedExtensionString(""),
      mStaticCombinedExtensionString(NULL),
      mStaticExtensionStrings(),
      mSupportedExtensions()
{
}

const TextureCaps &Caps::getTextureFormatCaps(GLenum format) const
{
    static TextureCaps defaultUnsupportedTexture;
    TextureFormatCapsMap::const_iterator iter = mTextureCaps.find(format);
    return (iter != mTextureCaps.end()) ? iter->second : defaultUnsupportedTexture;
}

const TextureFormatCapsMap & Caps::getTextureFormatCapsMap() const
{
    return mTextureCaps;
}

void Caps::setTextureFormatCaps(GLenum format, const TextureCaps &support)
{
    mTextureCaps[format] = support;
}

bool Caps::get32BitIndexSupport() const
{
    return mSupports32BitIndices;
}

void Caps::set32BitIndexSupport(bool support)
{
    mSupports32BitIndices = support;
    UpdateExtensionVector("GL_OES_element_index_uint", support, &mExtensionStringsDirty, &mSupportedExtensions);
}

bool Caps::getPackedDepthStencilSupport() const
{
    return mSupportsPackedDepthStencil;
}

void Caps::setPackedDepthStencilSupport(bool support)
{
    mSupportsPackedDepthStencil = support;
    UpdateExtensionVector("GL_OES_packed_depth_stencil", support, &mExtensionStringsDirty, &mSupportedExtensions);
}

bool Caps::getProgramBinarySupport() const
{
    return mSupportsProgramBinary;
}

void Caps::setProgramBinarySupport(bool support)
{
    mSupportsProgramBinary = support;
    UpdateExtensionVector("GL_OES_get_program_binary", support, &mExtensionStringsDirty, &mSupportedExtensions);
}

bool Caps::getRGB8AndRGBA8TextureSupport() const
{
    return mSupportsRGB8AndRGBA8Textures;
}

void Caps::setRGB8AndRGBA8TextureSupport(bool support)
{
    mSupportsRGB8AndRGBA8Textures = support;
    UpdateExtensionVector("GL_OES_rgb8_rgba8", support, &mExtensionStringsDirty, &mSupportedExtensions);
}

bool Caps::getBGRA8TextureSupport() const
{
    return mSupportsBGRA8Textures;
}

void Caps::setBGRA8TextureSupport(bool support)
{
    mSupportsBGRA8Textures = support;
    UpdateExtensionVector("GL_EXT_texture_format_BGRA8888", support, &mExtensionStringsDirty, &mSupportedExtensions);
}

bool Caps::getBGRAReadSupport() const
{
    return mSupportsBGRAReads;
}

void Caps::setBGRAReadSupport(bool support)
{
    mSupportsBGRAReads = support;
    UpdateExtensionVector("GL_EXT_read_format_bgra", support, &mExtensionStringsDirty, &mSupportedExtensions);
}

bool Caps::getMapBufferSupport() const
{
    return mSupportsMapBuffer;
}

bool Caps::getPixelBufferObjectSupport() const
{
    return mSupportsPixelBufferObjects;
}

void Caps::setPixelBufferObjectSupport(bool support)
{
    mSupportsPixelBufferObjects = support;
    UpdateExtensionVector("GL_NV_pixel_buffer_object", support, &mExtensionStringsDirty, &mSupportedExtensions);
}

void Caps::setMapBufferSupport(bool support)
{
    mSupportsMapBuffer = support;
    UpdateExtensionVector("GL_OES_mapbuffer", support, &mExtensionStringsDirty, &mSupportedExtensions);
}

bool Caps::getMapBufferRangeSupport() const
{
    return mSupportsMapBufferRange;
}

void Caps::setMapBufferRangeSupport(bool support)
{
    mSupportsMapBufferRange = support;
    UpdateExtensionVector("GL_EXT_map_buffer_range", support, &mExtensionStringsDirty, &mSupportedExtensions);
}

bool Caps::getHalfFloatTextureSupport() const
{
    return mSupportsHalfFloatTextures;
}

void Caps::setHalfFloatTextureSupport(bool support)
{
    mSupportsHalfFloatTextures = support;
    UpdateExtensionVector("GL_OES_texture_half_float", support, &mExtensionStringsDirty, &mSupportedExtensions);
}

bool Caps::getHalfFloatTextureFilteringSupport() const
{
    return mSupportsHalfFloatTextureFiltering;
}

void Caps::setHalfFloatTextureFilteringSupport(bool support)
{
    mSupportsHalfFloatTextureFiltering = support;
    UpdateExtensionVector("GL_OES_texture_half_float_linear", support, &mExtensionStringsDirty, &mSupportedExtensions);
}

bool Caps::getFloatTextureSupport() const
{
    return mSupportsFloatTextures;
}

void Caps::setFloatTextureSupport(bool support)
{
    mSupportsFloatTextures = support;
    UpdateExtensionVector("GL_OES_texture_float", support, &mExtensionStringsDirty, &mSupportedExtensions);
}

bool Caps::getFloatTextureFilteringSupport() const
{
    return mSupportsFloatTextureFiltering;
}

void Caps::setFloatTextureFilteringSupport(bool support)
{
    mSupportsFloatTextureFiltering = support;
    UpdateExtensionVector("GL_OES_texture_float_linear", support, &mExtensionStringsDirty, &mSupportedExtensions);
}

bool Caps::getRGTextureSupport() const
{
    return mSupportsRGTextures;
}

void Caps::setRGTextureSupport(bool support)
{
    mSupportsRGTextures = support;
    UpdateExtensionVector("GL_EXT_texture_rg", support, &mExtensionStringsDirty, &mSupportedExtensions);
}

bool Caps::getDXT1TextureSupport() const
{
    return mSupportsDXT1Textures;
}

void Caps::setDXT1TextureSupport(bool support)
{
    mSupportsDXT1Textures = support;
    UpdateExtensionVector("GL_EXT_texture_compression_dxt1", support, &mExtensionStringsDirty, &mSupportedExtensions);
}

bool Caps::getDXT3TextureSupport() const
{
    return mSupportsDXT3Textures;
}

void Caps::setDXT3TextureSupport(bool support)
{

    mSupportsDXT3Textures = support;
    UpdateExtensionVector("GL_ANGLE_texture_compression_dxt3", support, &mExtensionStringsDirty, &mSupportedExtensions);
}

bool Caps::getDXT5TextureSupport() const
{
    return mSupportsDXT5Textures;
}

void Caps::setDXT5TextureSupport(bool support)
{
    mSupportsDXT5Textures = support;
    UpdateExtensionVector("GL_ANGLE_texture_compression_dxt5", support, &mExtensionStringsDirty, &mSupportedExtensions);
}

bool Caps::getDepthTextureSupport() const
{
    return mSupportsDepthTextures;
}

void Caps::setDepthTextureSupport(bool support)
{
    mSupportsDepthTextures = support;
    UpdateExtensionVector("GL_ANGLE_depth_texture", support, &mExtensionStringsDirty, &mSupportedExtensions);
}

void Caps::setNPOTTextureSupport(bool support)
{
    mSupportsNPOTTextures = support;
    UpdateExtensionVector("GL_OES_texture_npot", support, &mExtensionStringsDirty, &mSupportedExtensions);
}

bool Caps::getDrawBuffersSupport() const
{
    return mSupportsDrawBuffers;
}

void Caps::setDrawBuffersSupport(bool support)
{
    mSupportsDrawBuffers = support;
    UpdateExtensionVector("GL_EXT_draw_buffers", support, &mExtensionStringsDirty, &mSupportedExtensions);
}

bool Caps::getTextureStorageSupport() const
{
    return mSupportsTextureStorage;
}

void Caps::setTextureStorageSupport(bool support)
{
    mSupportsTextureStorage = support;
    UpdateExtensionVector("GL_EXT_texture_storage", support, &mExtensionStringsDirty, &mSupportedExtensions);
}

bool Caps::getAnisotropicFilteringSupport() const
{
    return mSupportsAnisotropicFiltering;
}

void Caps::setAnisotropicFilteringSupport(bool support)
{
    mSupportsAnisotropicFiltering = support;
    UpdateExtensionVector("GL_EXT_texture_filter_anisotropic", support, &mExtensionStringsDirty, &mSupportedExtensions);
}

GLfloat Caps::getMaxTextureAnisotropy() const
{
    return mMaxTextureAnisotropy;
}

void Caps::setMaxTextureAnisotropy(GLfloat value)
{
    mMaxTextureAnisotropy = value;
}

bool Caps::getOcclusionQuerySupport() const
{
    return mSupportsOcclusionQueries;
}

void Caps::setOcclusionQuerySupport(bool support)
{
    mSupportsOcclusionQueries = support;
    UpdateExtensionVector("GL_EXT_occlusion_query_boolean", support, &mExtensionStringsDirty, &mSupportedExtensions);
}

bool Caps::getFenceQuerySupport() const
{
    return mSupportsEventQueries;
}

void Caps::setFenceQuerySupport(bool support)
{
    mSupportsEventQueries = support;
    UpdateExtensionVector("GL_NV_fence", support, &mExtensionStringsDirty, &mSupportedExtensions);
}

bool Caps::getANGLETimerQuerySupport() const
{
    return mSupportsTextureStorage;
}

void Caps::setANGLETimerQuerySupport(bool support)
{
    mSupportsANGLETimerQueries = support;
    UpdateExtensionVector("GL_ANGLE_timer_query", support, &mExtensionStringsDirty, &mSupportedExtensions);
}

bool Caps::getRobustnessSupport() const
{
    return mSupportsRobustness;
}

void Caps::setRobustnessSupport(bool support)
{
    mSupportsRobustness = support;
    UpdateExtensionVector("GL_EXT_robustness", support, &mExtensionStringsDirty, &mSupportedExtensions);
}

bool Caps::getMinMaxBlendingSupport() const
{
    return mSupportsMinMaxBlending;
}

void Caps::setMinMaxBlendingSupport(bool support)
{
    mSupportsMinMaxBlending = support;
    UpdateExtensionVector("GL_EXT_blend_minmax", support, &mExtensionStringsDirty, &mSupportedExtensions);
}

bool Caps::getANGLEFramebufferBlitSupport() const
{
    return mSupportsANGLEFramebufferBlit;
}

void Caps::setANGLEFramebufferBlitSupport(bool support)
{
    mSupportsANGLEFramebufferBlit = support;
    UpdateExtensionVector("GL_ANGLE_framebuffer_blit", support, &mExtensionStringsDirty, &mSupportedExtensions);
}

bool Caps::getANGLEFramebufferMultisampleSupport() const
{
    return mSupportsANGLEFramebufferMultisample;
}

void Caps::setANGLEFramebufferMultisampleSupport(bool support)
{
    mSupportsANGLEFramebufferMultisample = support;
    UpdateExtensionVector("GL_ANGLE_framebuffer_multisample", support, &mExtensionStringsDirty, &mSupportedExtensions);
}

bool Caps::getANGLEInstancedArraysSupport() const
{
    return mSupportsANGLEInstancedArrays;
}

void Caps::setANGLEInstancedArraysSupport(bool support)
{
    mSupportsANGLEInstancedArrays = support;
    UpdateExtensionVector("GL_ANGLE_instanced_arrays", support, &mExtensionStringsDirty, &mSupportedExtensions);
}

bool Caps::getANGLEPackReverseRowOrderSupport() const
{
    return mSupportsANGLEPackReverseRowOrder;
}

void Caps::setANGLEPackReverseRowOrderSupport(bool support)
{
    mSupportsANGLEPackReverseRowOrder = support;
    UpdateExtensionVector("GL_ANGLE_pack_reverse_row_order", support, &mExtensionStringsDirty, &mSupportedExtensions);
}

bool Caps::getStandardDerivativeSupport() const
{
    return mSupportsDerivitives;
}

void Caps::setStandardDerivativeSupport(bool support)
{
    mSupportsDerivitives = support;
    UpdateExtensionVector("GL_OES_standard_derivatives", support, &mExtensionStringsDirty, &mSupportedExtensions);
}

bool Caps::getTextureLODSupport() const
{
    return mSupportsTextureLOD;
}

void Caps::setTextureLODSupport(bool support)
{
    mSupportsTextureLOD = support;
    UpdateExtensionVector("GL_EXT_shader_texture_lod", support, &mExtensionStringsDirty, &mSupportedExtensions);
}

bool Caps::getFragDepthSupport() const
{
    return mSupportsFragDepth;
}

void Caps::setFragDepthSupport(bool support)
{
    mSupportsFragDepth = support;
    UpdateExtensionVector("GL_EXT_frag_depth", support, &mExtensionStringsDirty, &mSupportedExtensions);
}

bool Caps::getANGLETextureUsageSupport() const
{
    return mSupportsANGLETextureUsage;
}

void Caps::setANGLETextureUsageSupport(bool support)
{
    mSupportsANGLETextureUsage = support;
    UpdateExtensionVector("GL_ANGLE_texture_usage", support, &mExtensionStringsDirty, &mSupportedExtensions);
}

bool Caps::getANGLETranslatedSourceSupport() const
{
    return mSupportsANGLETranslatedSource;
}

void Caps::setANGLETranslatedSourceSupport(bool support)
{
    mSupportsANGLETranslatedSource = support;
    UpdateExtensionVector("GL_ANGLE_translated_shader_source", support, &mExtensionStringsDirty, &mSupportedExtensions);
}

bool Caps::getColorBufferFloatSupport() const
{
    return mSupportsColorBufferFloat;
}

void Caps::setColorBufferFloatSupport(bool support)
{
    mSupportsColorBufferFloat = support;
    UpdateExtensionVector("GL_EXT_color_buffer_float", support, &mExtensionStringsDirty, &mSupportedExtensions);
}

const std::string &Caps::getExtensionString() const
{
    cleanExtensionStrings();
    return mCombinedExtensionString;
}

const char *Caps::getStaticExtensionString() const
{
    cleanExtensionStrings();
    return mStaticCombinedExtensionString;
}

const std::string &Caps::getExtensionString(size_t idx) const
{
    return mSupportedExtensions[idx];
}

const char *Caps::getStaticExtensionString(size_t idx) const
{
    cleanExtensionStrings();
    return mStaticExtensionStrings[idx];
}

size_t Caps::getExtensionStringCount() const
{
    return mSupportedExtensions.size();
}

void Caps::cleanExtensionStrings() const
{
    if (mExtensionStringsDirty)
    {
        mStaticExtensionStrings.resize(mSupportedExtensions.size());

        std::ostringstream stream;
        for (size_t i = 0; i < mSupportedExtensions.size(); i++)
        {
            stream << mSupportedExtensions[i];
            if (i + 1 < mSupportedExtensions.size())
            {
                stream << " ";
            }

            mStaticExtensionStrings[i] = MakeStaticString(mSupportedExtensions[i]);
        }
        mCombinedExtensionString = stream.str();
        mStaticCombinedExtensionString = MakeStaticString(mCombinedExtensionString);
        mExtensionStringsDirty = false;
    }
}

}
