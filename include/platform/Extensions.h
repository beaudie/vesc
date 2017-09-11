//
// Copyright 2017 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//
// Extensions.h:
//   OpenGL context extensions. Exposed here so they are accessible to the tests.

#ifndef ANGLE_PLATFORM_EXTENSIONS_H_
#define ANGLE_PLATFORM_EXTENSIONS_H_

namespace gl
{

struct Extensions
{
    // ES2 Extension support

    // GL_OES_element_index_uint
    bool elementIndexUint = false;

    // GL_OES_packed_depth_stencil
    bool packedDepthStencil = false;

    // GL_OES_get_program_binary
    bool getProgramBinary = false;

    // GL_OES_rgb8_rgba8
    // Implies that TextureCaps for GL_RGB8 and GL_RGBA8 exist
    bool rgb8rgba8 = false;

    // GL_EXT_texture_format_BGRA8888
    // Implies that TextureCaps for GL_BGRA8 exist
    bool textureFormatBGRA8888 = false;

    // GL_EXT_read_format_bgra
    bool readFormatBGRA = false;

    // GL_NV_pixel_buffer_object
    bool pixelBufferObject = false;

    // GL_OES_mapbuffer and GL_EXT_map_buffer_range
    bool mapBuffer      = false;
    bool mapBufferRange = false;

    // GL_EXT_color_buffer_half_float
    // Together with GL_OES_texture_half_float in a GLES 2.0 context, implies that half-float
    // textures are renderable.
    bool colorBufferHalfFloat = false;

    // GL_OES_texture_half_float and GL_OES_texture_half_float_linear
    // Implies that TextureCaps for GL_RGB16F, GL_RGBA16F, GL_ALPHA32F_EXT, GL_LUMINANCE32F_EXT and
    // GL_LUMINANCE_ALPHA32F_EXT exist
    bool textureHalfFloat       = false;
    bool textureHalfFloatLinear = false;

    // GL_OES_texture_float and GL_OES_texture_float_linear
    // Implies that TextureCaps for GL_RGB32F, GL_RGBA32F, GL_ALPHA16F_EXT, GL_LUMINANCE16F_EXT and
    // GL_LUMINANCE_ALPHA16F_EXT exist
    bool textureFloat       = false;
    bool textureFloatLinear = false;

    // GL_EXT_texture_rg
    // Implies that TextureCaps for GL_R8, GL_RG8 (and floating point R/RG texture formats if
    // floating point extensions are also present) exist
    bool textureRG = false;

    // GL_EXT_texture_compression_dxt1, GL_ANGLE_texture_compression_dxt3 and
    // GL_ANGLE_texture_compression_dxt5 Implies that TextureCaps exist for
    // GL_COMPRESSED_RGB_S3TC_DXT1_EXT, GL_COMPRESSED_RGBA_S3TC_DXT1_EXT
    // GL_COMPRESSED_RGBA_S3TC_DXT3_ANGLE and GL_COMPRESSED_RGBA_S3TC_DXT5_ANGLE
    bool textureCompressionDXT1 = false;
    bool textureCompressionDXT3 = false;
    bool textureCompressionDXT5 = false;

    // GL_EXT_texture_compression_s3tc_srgb
    // Implies that TextureCaps exist for GL_COMPRESSED_SRGB_S3TC_DXT1_EXT,
    // GL_COMPRESSED_SRGB_ALPHA_S3TC_DXT1_EXT, GL_COMPRESSED_SRGB_ALPHA_S3TC_DXT3_EXT, and
    // GL_COMPRESSED_SRGB_ALPHA_S3TC_DXT5_EXT
    bool textureCompressionS3TCsRGB = false;

    // GL_KHR_texture_compression_astc_hdr
    bool textureCompressionASTCHDR = false;

    // GL_KHR_texture_compression_astc_ldr
    bool textureCompressionASTCLDR = false;

    // GL_OES_compressed_ETC1_RGB8_texture
    // Implies that TextureCaps for GL_ETC1_RGB8_OES exist
    bool compressedETC1RGB8Texture = false;

    // GL_EXT_sRGB
    // Implies that TextureCaps for GL_SRGB8_ALPHA8 and GL_SRGB8 exist
    // TODO: Don't advertise this extension in ES3
    bool sRGB = false;

    // GL_ANGLE_depth_texture
    bool depthTextures = false;

    // GL_OES_depth32
    // Allows DEPTH_COMPONENT32_OES as a valid Renderbuffer format.
    bool depth32 = false;

    // GL_EXT_texture_storage
    bool textureStorage = false;

    // GL_OES_texture_npot
    bool textureNPOT = false;

    // GL_EXT_draw_buffers
    bool drawBuffers = false;

    // GL_EXT_texture_filter_anisotropic
    bool textureFilterAnisotropic = false;
    GLfloat maxTextureAnisotropy  = 0.0f;

    // GL_EXT_occlusion_query_boolean
    bool occlusionQueryBoolean = false;

    // GL_NV_fence
    bool fence = false;

    // GL_ANGLE_timer_query
    bool timerQuery = false;

    // GL_EXT_disjoint_timer_query
    bool disjointTimerQuery            = false;
    GLuint queryCounterBitsTimeElapsed = 0u;
    GLuint queryCounterBitsTimestamp   = 0u;

    // GL_EXT_robustness
    bool robustness = false;

    // GL_KHR_robust_buffer_access_behavior
    bool robustBufferAccessBehavior = false;

    // GL_EXT_blend_minmax
    bool blendMinMax = false;

    // GL_ANGLE_framebuffer_blit
    bool framebufferBlit = false;

    // GL_ANGLE_framebuffer_multisample
    bool framebufferMultisample = false;

    // GL_ANGLE_instanced_arrays
    bool instancedArrays = false;

    // GL_ANGLE_pack_reverse_row_order
    bool packReverseRowOrder = false;

    // GL_OES_standard_derivatives
    bool standardDerivatives = false;

    // GL_EXT_shader_texture_lod
    bool shaderTextureLOD = false;

    // GL_EXT_shader_framebuffer_fetch
    bool shaderFramebufferFetch = false;

    // GL_ARM_shader_framebuffer_fetch
    bool ARMshaderFramebufferFetch = false;

    // GL_NV_shader_framebuffer_fetch
    bool NVshaderFramebufferFetch = false;

    // GL_EXT_frag_depth
    bool fragDepth = false;

    // ANGLE_multiview
    bool multiview  = false;
    GLuint maxViews = 1u;

    // GL_ANGLE_texture_usage
    bool textureUsage = false;

    // GL_ANGLE_translated_shader_source
    bool translatedShaderSource = false;

    // GL_OES_fbo_render_mipmap
    bool fboRenderMipmap = false;

    // GL_EXT_discard_framebuffer
    bool discardFramebuffer = false;

    // EXT_debug_marker
    bool debugMarker = false;

    // GL_OES_EGL_image
    bool eglImage = false;

    // GL_OES_EGL_image_external
    bool eglImageExternal = false;

    // GL_OES_EGL_image_external_essl3
    bool eglImageExternalEssl3 = false;

    // NV_EGL_stream_consumer_external
    bool eglStreamConsumerExternal = false;

    // EXT_unpack_subimage
    bool unpackSubimage = false;

    // NV_pack_subimage
    bool packSubimage = false;

    // GL_OES_vertex_array_object
    bool vertexArrayObject = false;

    // GL_KHR_debug
    bool debug                     = false;
    GLuint maxDebugMessageLength   = 0u;
    GLuint maxDebugLoggedMessages  = 0u;
    GLuint maxDebugGroupStackDepth = 0u;
    GLuint maxLabelLength          = 0u;

    // KHR_no_error
    bool noError = false;

    // GL_ANGLE_lossy_etc_decode
    bool lossyETCDecode = false;

    // GL_CHROMIUM_bind_uniform_location
    bool bindUniformLocation = false;

    // GL_CHROMIUM_sync_query
    bool syncQuery = false;

    // GL_CHROMIUM_copy_texture
    bool copyTexture = false;

    // GL_CHROMIUM_copy_compressed_texture
    bool copyCompressedTexture = false;

    // GL_ANGLE_webgl_compatibility
    bool webglCompatibility = false;

    // GL_ANGLE_request_extension
    bool requestExtension = false;

    // GL_CHROMIUM_bind_generates_resource
    bool bindGeneratesResource = false;

    // GL_ANGLE_robust_client_memory
    bool robustClientMemory = false;

    // GL_EXT_texture_sRGB_decode
    bool textureSRGBDecode = false;

    // GL_EXT_sRGB_write_control
    bool sRGBWriteControl = false;

    // GL_CHROMIUM_color_buffer_float_rgb
    bool colorBufferFloatRGB = false;

    // GL_CHROMIUM_color_buffer_float_rgba
    bool colorBufferFloatRGBA = false;

    // ES3 Extension support

    // GL_EXT_color_buffer_float
    bool colorBufferFloat = false;

    // GL_EXT_multisample_compatibility.
    // written against ES 3.1 but can apply to earlier versions.
    bool multisampleCompatibility = false;

    // GL_CHROMIUM_framebuffer_mixed_samples
    bool framebufferMixedSamples = false;

    // GL_EXT_texture_norm16
    // written against ES 3.1 but can apply to ES 3.0 as well.
    bool textureNorm16 = false;

    // GL_CHROMIUM_path_rendering
    bool pathRendering = false;

    // GL_OES_surfaceless_context
    bool surfacelessContext = false;

    // GL_ANGLE_client_arrays
    bool clientArrays = false;

    // GL_ANGLE_robust_resource_initialization
    bool robustResourceInitialization = false;

    // GL_ANGLE_program_cache_control
    bool programCacheControl = false;

    // GL_ANGLE_texture_rectangle
    bool textureRectangle = false;
};

}  // namespace gl

#endif  // ANGLE_PLATFORM_EXTENSIONS_H_
