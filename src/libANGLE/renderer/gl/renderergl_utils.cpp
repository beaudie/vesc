//
// Copyright (c) 2012-2014 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//

// renderergl_utils.cpp: Conversion functions and other utility routines
// specific to the OpenGL renderer.

#include "libANGLE/renderer/gl/renderergl_utils.h"

#include <limits>

#include "libANGLE/Caps.h"
#include "libANGLE/formatutils.h"
#include "libANGLE/renderer/gl/FunctionsGL.h"
#include "libANGLE/renderer/gl/formatutilsgl.h"

#include <algorithm>
#include <sstream>

namespace rx
{

namespace nativegl_gl
{

static bool MeetsRequirements(const FunctionsGL *functions, const nativegl::SupportRequirement &requirements)
{
    for (const std::string &extension : requirements.requiredExtensions)
    {
        if (!functions->hasExtension(extension))
        {
            return false;
        }
    }

    if (functions->version >= requirements.version)
    {
        return true;
    }
    else if (!requirements.versionExtensions.empty())
    {
        for (const std::string &extension : requirements.versionExtensions)
        {
            if (!functions->hasExtension(extension))
            {
                return false;
            }
        }
        return true;
    }
    else
    {
        return false;
    }
}

static gl::TextureCaps GenerateTextureFormatCaps(const FunctionsGL *functions, GLenum internalFormat)
{
    gl::TextureCaps textureCaps;

    const nativegl::InternalFormat &formatInfo = nativegl::GetInternalFormatInfo(internalFormat, functions->standard);
    textureCaps.texturable = MeetsRequirements(functions, formatInfo.texture);
    textureCaps.filterable = textureCaps.texturable && MeetsRequirements(functions, formatInfo.filter);
    textureCaps.renderable = MeetsRequirements(functions, formatInfo.framebufferAttachment);

    // glGetInternalformativ is not available until version 4.2 but may be available through the 3.0
    // extension GL_ARB_internalformat_query
    if (textureCaps.renderable && functions->getInternalformativ)
    {
        GLint numSamples = 0;
        functions->getInternalformativ(GL_RENDERBUFFER, internalFormat, GL_NUM_SAMPLE_COUNTS, 1, &numSamples);

        if (numSamples > 0)
        {
            std::vector<GLint> samples(numSamples);
            functions->getInternalformativ(GL_RENDERBUFFER, internalFormat, GL_SAMPLES, samples.size(), &samples[0]);
            for (size_t sampleIndex = 0; sampleIndex < samples.size(); sampleIndex++)
            {
                textureCaps.sampleCounts.insert(samples[sampleIndex]);
            }
        }
    }

    return textureCaps;
}

static GLint QuerySingleGLInt(const FunctionsGL *functions, GLenum name)
{
    GLint result;
    functions->getIntegerv(name, &result);
    return result;
}

static GLint QueryGLIntRange(const FunctionsGL *functions, GLenum name, size_t index)
{
    GLint result[2];
    functions->getIntegerv(name, result);
    return result[index];
}

static GLint64 QuerySingleGLInt64(const FunctionsGL *functions, GLenum name)
{
    GLint64 result;
    functions->getInteger64v(name, &result);
    return result;
}

static GLfloat QuerySingleGLFloat(const FunctionsGL *functions, GLenum name)
{
    GLfloat result;
    functions->getFloatv(name, &result);
    return result;
}

static GLfloat QueryGLFloatRange(const FunctionsGL *functions, GLenum name, size_t index)
{
    GLfloat result[2];
    functions->getFloatv(name, result);
    return result[index];
}

static gl::TypePrecision QueryTypePrecision(const FunctionsGL *functions, GLenum shaderType, GLenum precisionType)
{
    gl::TypePrecision precision;
    functions->getShaderPrecisionFormat(shaderType, precisionType, precision.range, &precision.precision);
    return precision;
}

void GenerateCaps(const FunctionsGL *functions, gl::Caps *caps, gl::TextureCapsMap *textureCapsMap,
                  gl::Extensions *extensions)
{
    // Texture format support checks
    const gl::FormatSet &allFormats = gl::GetAllSizedInternalFormats();
    for (GLenum internalFormat : allFormats)
    {
        gl::TextureCaps textureCaps = GenerateTextureFormatCaps(functions, internalFormat);
        textureCapsMap->insert(internalFormat, textureCaps);

        if (gl::GetInternalFormatInfo(internalFormat).compressed)
        {
            caps->compressedTextureFormats.push_back(internalFormat);
        }
    }

    // Table 6.28, implementation dependent values
    caps->maxElementIndex = QuerySingleGLInt64(functions, GL_MAX_ELEMENT_INDEX); // 4.3/GL_ARB_ES3_compatibility
    caps->max3DTextureSize = QuerySingleGLInt(functions, GL_MAX_3D_TEXTURE_SIZE); // 1.2
    caps->max2DTextureSize = QuerySingleGLInt(functions, GL_MAX_TEXTURE_SIZE); // 1.0
    caps->maxCubeMapTextureSize = QuerySingleGLInt(functions, GL_MAX_CUBE_MAP_TEXTURE_SIZE); // 1.3
    caps->maxArrayTextureLayers = QuerySingleGLInt(functions, GL_MAX_ARRAY_TEXTURE_LAYERS); // 3.0/GL_EXT_texture_array
    caps->maxLODBias = QuerySingleGLFloat(functions, GL_MAX_TEXTURE_LOD_BIAS); // 1.5/GL_EXT_texture_lod_bias
    caps->maxRenderbufferSize = QuerySingleGLInt(functions, GL_MAX_RENDERBUFFER_SIZE); // 3.0/GL_EXT_framebuffer_object
    caps->maxDrawBuffers = QuerySingleGLInt(functions, GL_MAX_DRAW_BUFFERS); // 2.0/GL_EXT_draw_buffers2
    caps->maxColorAttachments = QuerySingleGLInt(functions, GL_MAX_COLOR_ATTACHMENTS); // 3.0/GL_EXT_framebuffer_object
    caps->maxViewportWidth = QueryGLIntRange(functions, GL_MAX_VIEWPORT_DIMS, 0); // 1.0
    caps->maxViewportHeight = QueryGLIntRange(functions, GL_MAX_VIEWPORT_DIMS, 1); // 1.0
    caps->minAliasedPointSize = QueryGLFloatRange(functions, GL_ALIASED_POINT_SIZE_RANGE, 0);
    caps->maxAliasedPointSize = QueryGLFloatRange(functions, GL_ALIASED_POINT_SIZE_RANGE, 1);
    caps->minAliasedLineWidth = QueryGLFloatRange(functions, GL_ALIASED_LINE_WIDTH_RANGE, 0); // 1.2
    caps->maxAliasedLineWidth = QueryGLFloatRange(functions, GL_ALIASED_LINE_WIDTH_RANGE, 1); // 1.2

    // Table 6.29, implementation dependent values (cont.)
    caps->maxElementsIndices = QuerySingleGLInt(functions, GL_MAX_ELEMENTS_INDICES); // 1.2
    caps->maxElementsVertices = QuerySingleGLInt(functions, GL_MAX_ELEMENTS_VERTICES); // 1.2

    // glGetShaderPrecisionFormat is not available until desktop GL version 4.1 or GL_ARB_ES2_compatibility exists
    if ((functions->standard == Standard_OpenGL   && (functions->version >= gl::Version(4, 1) || functions->hasExtension("GL_ARB_ES2_compatibility"))) ||
        (functions->standard == Standard_OpenGLES && functions->version >= gl::Version(2, 0)))
    {
        caps->vertexHighpFloat = QueryTypePrecision(functions, GL_VERTEX_SHADER, GL_HIGH_FLOAT);
        caps->vertexMediumpFloat = QueryTypePrecision(functions, GL_VERTEX_SHADER, GL_MEDIUM_FLOAT);
        caps->vertexLowpFloat = QueryTypePrecision(functions, GL_VERTEX_SHADER, GL_LOW_FLOAT);
        caps->fragmentHighpFloat = QueryTypePrecision(functions, GL_FRAGMENT_SHADER, GL_HIGH_FLOAT);
        caps->fragmentMediumpFloat = QueryTypePrecision(functions, GL_FRAGMENT_SHADER, GL_MEDIUM_FLOAT);
        caps->fragmentLowpFloat = QueryTypePrecision(functions, GL_FRAGMENT_SHADER, GL_LOW_FLOAT);
        caps->vertexHighpInt = QueryTypePrecision(functions, GL_VERTEX_SHADER, GL_HIGH_INT);
        caps->vertexMediumpInt = QueryTypePrecision(functions, GL_VERTEX_SHADER, GL_MEDIUM_INT);
        caps->vertexLowpInt = QueryTypePrecision(functions, GL_VERTEX_SHADER, GL_LOW_INT);
        caps->fragmentHighpInt = QueryTypePrecision(functions, GL_FRAGMENT_SHADER, GL_HIGH_INT);
        caps->fragmentMediumpInt = QueryTypePrecision(functions, GL_FRAGMENT_SHADER, GL_MEDIUM_INT);
        caps->fragmentLowpInt = QueryTypePrecision(functions, GL_FRAGMENT_SHADER, GL_LOW_INT);
    }
    else
    {
        // Set some default values
        caps->vertexHighpFloat.setIEEEFloat();
        caps->vertexMediumpFloat.setIEEEFloat();
        caps->vertexLowpFloat.setIEEEFloat();
        caps->fragmentHighpFloat.setIEEEFloat();
        caps->fragmentMediumpFloat.setIEEEFloat();
        caps->fragmentLowpFloat.setIEEEFloat();
        caps->vertexHighpInt.setTwosComplementInt(32);
        caps->vertexMediumpInt.setTwosComplementInt(32);
        caps->vertexLowpInt.setTwosComplementInt(32);
        caps->fragmentHighpInt.setTwosComplementInt(32);
        caps->fragmentMediumpInt.setTwosComplementInt(32);
        caps->fragmentLowpInt.setTwosComplementInt(32);
    }
    caps->maxServerWaitTimeout = QuerySingleGLInt64(functions, GL_MAX_SERVER_WAIT_TIMEOUT); // 3.2/GL_ARB_sync

    // Table 6.31, implementation dependent vertex shader limits
    caps->maxVertexAttributes = QuerySingleGLInt(functions, GL_MAX_VERTEX_ATTRIBS); // 2.0
    caps->maxVertexUniformComponents = QuerySingleGLInt(functions, GL_MAX_VERTEX_UNIFORM_COMPONENTS); // 2.0
    caps->maxVertexUniformVectors = QuerySingleGLInt(functions, GL_MAX_VERTEX_UNIFORM_VECTORS); // 4.1/GL_ARB_ES2_compatibility
    caps->maxVertexUniformBlocks = QuerySingleGLInt(functions, GL_MAX_VERTEX_UNIFORM_BLOCKS); // 3.1/GL_ARB_uniform_buffer_object
    caps->maxVertexOutputComponents = QuerySingleGLInt(functions, GL_MAX_VERTEX_OUTPUT_COMPONENTS); // 3.2
    caps->maxVertexTextureImageUnits = QuerySingleGLInt(functions, GL_MAX_VERTEX_TEXTURE_IMAGE_UNITS); // 2.0

    // Table 6.32, implementation dependent fragment shader limits
    caps->maxFragmentUniformComponents = QuerySingleGLInt(functions, GL_MAX_FRAGMENT_UNIFORM_COMPONENTS); // 2.0
    caps->maxFragmentUniformVectors = QuerySingleGLInt(functions, GL_MAX_FRAGMENT_UNIFORM_VECTORS); // 4.1/GL_ARB_ES2_compatibility
    caps->maxFragmentUniformBlocks = QuerySingleGLInt(functions, GL_MAX_FRAGMENT_UNIFORM_BLOCKS); // 3.1/GL_ARB_uniform_buffer_object
    caps->maxFragmentInputComponents = QuerySingleGLInt(functions, GL_MAX_FRAGMENT_INPUT_COMPONENTS); // 3.2
    caps->maxTextureImageUnits = QuerySingleGLInt(functions, GL_MAX_TEXTURE_IMAGE_UNITS); // 2.0
    caps->minProgramTexelOffset = QuerySingleGLInt(functions, GL_MIN_PROGRAM_TEXEL_OFFSET); // 3.0
    caps->maxProgramTexelOffset = QuerySingleGLInt(functions, GL_MAX_PROGRAM_TEXEL_OFFSET); // 3.0

    // Table 6.33, implementation dependent aggregate shader limits
    caps->maxUniformBufferBindings = QuerySingleGLInt(functions, GL_MAX_UNIFORM_BUFFER_BINDINGS); // 3.1/GL_ARB_uniform_buffer_object
    caps->maxUniformBlockSize = QuerySingleGLInt64(functions, GL_MAX_UNIFORM_BLOCK_SIZE); // 3.1/GL_ARB_uniform_buffer_object
    caps->uniformBufferOffsetAlignment = QuerySingleGLInt(functions, GL_UNIFORM_BUFFER_OFFSET_ALIGNMENT); // 3.1/GL_ARB_uniform_buffer_object
    caps->maxCombinedUniformBlocks = caps->maxVertexUniformBlocks + caps->maxFragmentInputComponents;
    caps->maxCombinedVertexUniformComponents = QuerySingleGLInt64(functions, GL_MAX_COMBINED_VERTEX_UNIFORM_COMPONENTS); // 3.1/GL_ARB_uniform_buffer_object
    caps->maxCombinedFragmentUniformComponents = QuerySingleGLInt64(functions, GL_MAX_COMBINED_FRAGMENT_UNIFORM_COMPONENTS); // 3.1/GL_ARB_uniform_buffer_object
    caps->maxVaryingComponents = QuerySingleGLInt(functions, GL_MAX_VARYING_COMPONENTS); // 3.0/GL_ARB_ES2_compatibility
    caps->maxVaryingVectors = QuerySingleGLInt(functions, GL_MAX_VARYING_VECTORS); // 4.1/GL_ARB_ES2_compatibility
    caps->maxCombinedTextureImageUnits = caps->maxVertexTextureImageUnits + caps->maxTextureImageUnits;

    // Table 6.34, implementation dependent transform feedback limits
    caps->maxTransformFeedbackInterleavedComponents = QuerySingleGLInt(functions, GL_MAX_TRANSFORM_FEEDBACK_INTERLEAVED_COMPONENTS); // 3.0/GL_EXT_transform_feedback
    caps->maxTransformFeedbackSeparateAttributes = QuerySingleGLInt(functions, GL_MAX_TRANSFORM_FEEDBACK_SEPARATE_ATTRIBS); // 3.0/GL_EXT_transform_feedback
    caps->maxTransformFeedbackSeparateComponents = QuerySingleGLInt(functions, GL_MAX_TRANSFORM_FEEDBACK_SEPARATE_COMPONENTS); // 3.0/GL_EXT_transform_feedback

    // Table 6.35, Framebuffer Dependent Values
    caps->maxSamples = QuerySingleGLInt(functions, GL_MAX_SAMPLES); // 3.0/GL_EXT_framebuffer_object

    // Extension support
    extensions->setTextureExtensionSupport(*textureCapsMap);
    extensions->textureNPOT = true;
    extensions->textureStorage = true;
    extensions->fboRenderMipmap = true;
    extensions->framebufferBlit = (functions->blitFramebuffer != nullptr);
    extensions->framebufferMultisample = caps->maxSamples > 0;
    extensions->fence = functions->hasExtension("GL_NV_fence");
}

}

}
