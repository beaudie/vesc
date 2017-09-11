//
// Copyright (c) 2014 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//

#ifndef LIBANGLE_CAPS_H_
#define LIBANGLE_CAPS_H_

#include "angle_gl.h"
#include "libANGLE/Version.h"
#include "libANGLE/angletypes.h"
#include "platform/Extensions.h"

#include <map>
#include <set>
#include <string>
#include <vector>
#include <array>

namespace gl
{

struct Extensions;

typedef std::set<GLuint> SupportedSampleSet;

struct TextureCaps
{
    TextureCaps();

    // Supports for basic texturing: glTexImage, glTexSubImage, etc
    bool texturable;

    // Support for linear or anisotropic filtering
    bool filterable;

    // Support for being used as a framebuffer attachment or renderbuffer format
    bool renderable;

    // Set of supported sample counts, only guaranteed to be valid in ES3.
    SupportedSampleSet sampleCounts;

    // Get the maximum number of samples supported
    GLuint getMaxSamples() const;

    // Get the number of supported samples that is at least as many as requested.  Returns 0 if
    // there are no sample counts available
    GLuint getNearestSamples(GLuint requestedSamples) const;
};

TextureCaps GenerateMinimumTextureCaps(GLenum internalFormat,
                                       const Version &clientVersion,
                                       const Extensions &extensions);

class TextureCapsMap
{
  public:
    typedef std::map<GLenum, TextureCaps>::const_iterator const_iterator;

    void insert(GLenum internalFormat, const TextureCaps &caps);
    void remove(GLenum internalFormat);
    void clear();

    const TextureCaps &get(GLenum internalFormat) const;

    const_iterator begin() const;
    const_iterator end() const;

    size_t size() const;

  private:
    typedef std::map<GLenum, TextureCaps> InternalFormatToCapsMap;
    InternalFormatToCapsMap mCapsMap;
};

TextureCapsMap GenerateMinimumTextureCapsMap(const Version &clientVersion,
                                             const Extensions &extensions);

// Please see platform/Extensions.h for the Extensions struct, where it is accessible to tests.

// Set all texture related extension support based on the supported textures.
// Determines support for:
// GL_OES_packed_depth_stencil
// GL_OES_rgb8_rgba8
// GL_EXT_texture_format_BGRA8888
// GL_EXT_color_buffer_half_float,
// GL_OES_texture_half_float, GL_OES_texture_half_float_linear
// GL_OES_texture_float, GL_OES_texture_float_linear
// GL_EXT_texture_rg
// GL_EXT_texture_compression_dxt1, GL_ANGLE_texture_compression_dxt3,
// GL_ANGLE_texture_compression_dxt5
// GL_KHR_texture_compression_astc_hdr, GL_KHR_texture_compression_astc_ldr
// GL_OES_compressed_ETC1_RGB8_texture
// GL_EXT_sRGB
// GL_ANGLE_depth_texture, GL_OES_depth32
// GL_EXT_color_buffer_float
// GL_EXT_texture_norm16
void SetTextureExtensionSupport(Extensions *extensions, const TextureCapsMap &textureCaps);

// Generate a vector of supported extension strings
std::vector<std::string> GetExtensionsStrings(const Extensions &extensions);

struct ExtensionInfo
{
    // If this extension can be enabled with glRequestExtension (GL_ANGLE_request_extension)
    bool Requestable = false;

    // Pointer to a boolean member of the Extensions struct
    typedef bool(Extensions::*ExtensionBool);
    ExtensionBool ExtensionsMember = nullptr;
};

using ExtensionInfoMap = std::map<std::string, ExtensionInfo>;
const ExtensionInfoMap &GetExtensionInfoMap();

struct Limitations
{
    Limitations();

    // Renderer doesn't support gl_FrontFacing in fragment shaders
    bool noFrontFacingSupport;

    // Renderer doesn't support GL_SAMPLE_ALPHA_TO_COVERAGE
    bool noSampleAlphaToCoverageSupport;

    // In glVertexAttribDivisorANGLE, attribute zero must have a zero divisor
    bool attributeZeroRequiresZeroDivisorInEXT;

    // Unable to support different values for front and back faces for stencil refs and masks
    bool noSeparateStencilRefsAndMasks;

    // Renderer doesn't support non-constant indexing loops in fragment shader
    bool shadersRequireIndexedLoopValidation;

    // Renderer doesn't support Simultaneous use of GL_CONSTANT_ALPHA/GL_ONE_MINUS_CONSTANT_ALPHA
    // and GL_CONSTANT_COLOR/GL_ONE_MINUS_CONSTANT_COLOR blend functions.
    bool noSimultaneousConstantColorAndAlphaBlendFunc;
};

struct TypePrecision
{
    TypePrecision();

    void setIEEEFloat();
    void setTwosComplementInt(unsigned int bits);
    void setSimulatedFloat(unsigned int range, unsigned int precision);
    void setSimulatedInt(unsigned int range);

    void get(GLint *returnRange, GLint *returnPrecision) const;

    std::array<GLint, 2> range;
    GLint precision;
};

struct Caps
{
    Caps();

    // ES 3.1 (April 29, 2015) 20.39: implementation dependent values
    GLuint64 maxElementIndex;
    GLuint max3DTextureSize;
    GLuint max2DTextureSize;
    GLuint maxRectangleTextureSize;
    GLuint maxArrayTextureLayers;
    GLfloat maxLODBias;
    GLuint maxCubeMapTextureSize;
    GLuint maxRenderbufferSize;
    GLfloat minAliasedPointSize;
    GLfloat maxAliasedPointSize;
    GLfloat minAliasedLineWidth;
    GLfloat maxAliasedLineWidth;

    // ES 3.1 (April 29, 2015) 20.40: implementation dependent values (cont.)
    GLuint maxDrawBuffers;
    GLuint maxFramebufferWidth;
    GLuint maxFramebufferHeight;
    GLuint maxFramebufferSamples;
    GLuint maxColorAttachments;
    GLuint maxViewportWidth;
    GLuint maxViewportHeight;
    GLuint maxSampleMaskWords;
    GLuint maxColorTextureSamples;
    GLuint maxDepthTextureSamples;
    GLuint maxIntegerSamples;
    GLuint64 maxServerWaitTimeout;

    // ES 3.1 (April 29, 2015) Table 20.41: Implementation dependent values (cont.)
    GLint maxVertexAttribRelativeOffset;
    GLuint maxVertexAttribBindings;
    GLint maxVertexAttribStride;
    GLuint maxElementsIndices;
    GLuint maxElementsVertices;
    std::vector<GLenum> compressedTextureFormats;
    std::vector<GLenum> programBinaryFormats;
    std::vector<GLenum> shaderBinaryFormats;
    TypePrecision vertexHighpFloat;
    TypePrecision vertexMediumpFloat;
    TypePrecision vertexLowpFloat;
    TypePrecision vertexHighpInt;
    TypePrecision vertexMediumpInt;
    TypePrecision vertexLowpInt;
    TypePrecision fragmentHighpFloat;
    TypePrecision fragmentMediumpFloat;
    TypePrecision fragmentLowpFloat;
    TypePrecision fragmentHighpInt;
    TypePrecision fragmentMediumpInt;
    TypePrecision fragmentLowpInt;

    // ES 3.1 (April 29, 2015) Table 20.43: Implementation dependent Vertex shader limits
    GLuint maxVertexAttributes;
    GLuint maxVertexUniformComponents;
    GLuint maxVertexUniformVectors;
    GLuint maxVertexUniformBlocks;
    GLuint maxVertexOutputComponents;
    GLuint maxVertexTextureImageUnits;
    GLuint maxVertexAtomicCounterBuffers;
    GLuint maxVertexAtomicCounters;
    GLuint maxVertexImageUniforms;
    GLuint maxVertexShaderStorageBlocks;

    // ES 3.1 (April 29, 2015) Table 20.44: Implementation dependent Fragment shader limits
    GLuint maxFragmentUniformComponents;
    GLuint maxFragmentUniformVectors;
    GLuint maxFragmentUniformBlocks;
    GLuint maxFragmentInputComponents;
    GLuint maxTextureImageUnits;
    GLuint maxFragmentAtomicCounterBuffers;
    GLuint maxFragmentAtomicCounters;
    GLuint maxFragmentImageUniforms;
    GLuint maxFragmentShaderStorageBlocks;
    GLint minProgramTextureGatherOffset;
    GLuint maxProgramTextureGatherOffset;
    GLint minProgramTexelOffset;
    GLint maxProgramTexelOffset;

    // ES 3.1 (April 29, 2015) Table 20.45: implementation dependent compute shader limits
    std::array<GLuint, 3> maxComputeWorkGroupCount;
    std::array<GLuint, 3> maxComputeWorkGroupSize;
    GLuint maxComputeWorkGroupInvocations;
    GLuint maxComputeUniformBlocks;
    GLuint maxComputeTextureImageUnits;
    GLuint maxComputeSharedMemorySize;
    GLuint maxComputeUniformComponents;
    GLuint maxComputeAtomicCounterBuffers;
    GLuint maxComputeAtomicCounters;
    GLuint maxComputeImageUniforms;
    GLuint maxCombinedComputeUniformComponents;
    GLuint maxComputeShaderStorageBlocks;

    // ES 3.1 (April 29, 2015) Table 20.46: implementation dependent aggregate shader limits
    GLuint maxUniformBufferBindings;
    GLuint64 maxUniformBlockSize;
    GLuint uniformBufferOffsetAlignment;
    GLuint maxCombinedUniformBlocks;
    GLuint64 maxCombinedVertexUniformComponents;
    GLuint64 maxCombinedFragmentUniformComponents;
    GLuint maxVaryingComponents;
    GLuint maxVaryingVectors;
    GLuint maxCombinedTextureImageUnits;
    GLuint maxCombinedShaderOutputResources;

    // ES 3.1 (April 29, 2015) Table 20.47: implementation dependent aggregate shader limits (cont.)
    GLuint maxUniformLocations;
    GLuint maxAtomicCounterBufferBindings;
    GLuint maxAtomicCounterBufferSize;
    GLuint maxCombinedAtomicCounterBuffers;
    GLuint maxCombinedAtomicCounters;
    GLuint maxImageUnits;
    GLuint maxCombinedImageUniforms;
    GLuint maxShaderStorageBufferBindings;
    GLuint64 maxShaderStorageBlockSize;
    GLuint maxCombinedShaderStorageBlocks;
    GLuint shaderStorageBufferOffsetAlignment;

    // ES 3.1 (April 29, 2015) Table 20.48: implementation dependent transform feedback limits
    GLuint maxTransformFeedbackInterleavedComponents;
    GLuint maxTransformFeedbackSeparateAttributes;
    GLuint maxTransformFeedbackSeparateComponents;

    // ES 3.1 (April 29, 2015) Table 20.49: Framebuffer Dependent Values
    GLuint maxSamples;
};

Caps GenerateMinimumCaps(const Version &clientVersion);
}

namespace egl
{

struct Caps
{
    Caps();

    // Support for NPOT surfaces
    bool textureNPOT;
};

struct DisplayExtensions
{
    DisplayExtensions();

    // Generate a vector of supported extension strings
    std::vector<std::string> getStrings() const;

    // EGL_EXT_create_context_robustness
    bool createContextRobustness;

    // EGL_ANGLE_d3d_share_handle_client_buffer
    bool d3dShareHandleClientBuffer;

    // EGL_ANGLE_d3d_texture_client_buffer
    bool d3dTextureClientBuffer;

    // EGL_ANGLE_surface_d3d_texture_2d_share_handle
    bool surfaceD3DTexture2DShareHandle;

    // EGL_ANGLE_query_surface_pointer
    bool querySurfacePointer;

    // EGL_ANGLE_window_fixed_size
    bool windowFixedSize;

    // EGL_ANGLE_keyed_mutex
    bool keyedMutex;

    // EGL_ANGLE_surface_orientation
    bool surfaceOrientation;

    // EGL_NV_post_sub_buffer
    bool postSubBuffer;

    // EGL_KHR_create_context
    bool createContext;

    // EGL_EXT_device_query
    bool deviceQuery;

    // EGL_KHR_image
    bool image;

    // EGL_KHR_image_base
    bool imageBase;

    // EGL_KHR_image_pixmap
    bool imagePixmap;

    // EGL_KHR_gl_texture_2D_image
    bool glTexture2DImage;

    // EGL_KHR_gl_texture_cubemap_image
    bool glTextureCubemapImage;

    // EGL_KHR_gl_texture_3D_image
    bool glTexture3DImage;

    // EGL_KHR_gl_renderbuffer_image
    bool glRenderbufferImage;

    // EGL_KHR_get_all_proc_addresses
    bool getAllProcAddresses;

    // EGL_ANGLE_flexible_surface_compatibility
    bool flexibleSurfaceCompatibility;

    // EGL_ANGLE_direct_composition
    bool directComposition;

    // KHR_create_context_no_error
    bool createContextNoError;

    // EGL_KHR_stream
    bool stream;

    // EGL_KHR_stream_consumer_gltexture
    bool streamConsumerGLTexture;

    // EGL_NV_stream_consumer_gltexture_yuv
    bool streamConsumerGLTextureYUV;

    // EGL_ANGLE_stream_producer_d3d_texture_nv12
    bool streamProducerD3DTextureNV12;

    // EGL_ANGLE_create_context_webgl_compatibility
    bool createContextWebGLCompatibility;

    // EGL_CHROMIUM_create_context_bind_generates_resource
    bool createContextBindGeneratesResource;

    // EGL_CHROMIUM_get_sync_values
    bool getSyncValues;

    // EGL_EXT_swap_buffers_with_damage
    bool swapBuffersWithDamage;

    // EGL_EXT_pixel_format_float
    bool pixelFormatFloat;

    // EGL_KHR_surfaceless_context
    bool surfacelessContext;

    // EGL_ANGLE_display_texture_share_group
    bool displayTextureShareGroup;

    // EGL_ANGLE_create_context_client_arrays
    bool createContextClientArrays;

    // EGL_ANGLE_program_cache_control
    bool programCacheControl;
};

struct DeviceExtensions
{
    DeviceExtensions();

    // Generate a vector of supported extension strings
    std::vector<std::string> getStrings() const;

    // EGL_ANGLE_device_d3d
    bool deviceD3D;
};

struct ClientExtensions
{
    ClientExtensions();

    // Generate a vector of supported extension strings
    std::vector<std::string> getStrings() const;

    // EGL_EXT_client_extensions
    bool clientExtensions;

    // EGL_EXT_platform_base
    bool platformBase;

    // EGL_EXT_platform_device
    bool platformDevice;

    // EGL_ANGLE_platform_angle
    bool platformANGLE;

    // EGL_ANGLE_platform_angle_d3d
    bool platformANGLED3D;

    // EGL_ANGLE_platform_angle_opengl
    bool platformANGLEOpenGL;

    // EGL_ANGLE_platform_angle_null
    bool platformANGLENULL;

    // EGL_ANGLE_platform_angle_vulkan
    bool platformANGLEVulkan;

    // EGL_ANGLE_device_creation
    bool deviceCreation;

    // EGL_ANGLE_device_creation_d3d11
    bool deviceCreationD3D11;

    // EGL_ANGLE_x11_visual
    bool x11Visual;

    // EGL_ANGLE_experimental_present_path
    bool experimentalPresentPath;

    // EGL_KHR_client_get_all_proc_addresses
    bool clientGetAllProcAddresses;

    // EGL_ANGLE_display_robust_resource_initialization
    bool displayRobustResourceInitialization;
};

}  // namespace egl

#endif // LIBANGLE_CAPS_H_
