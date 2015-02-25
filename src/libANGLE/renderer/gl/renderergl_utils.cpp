//
// Copyright (c) 2012-2014 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//

// renderer11_utils.cpp: Conversion functions and other utility routines
// specific to the OpenGL renderer.

#include "libANGLE/renderer/gl/renderergl_utils.h"

#include "libANGLE/Caps.h"
#include "libANGLE/renderer/gl/FunctionsGL.h"

namespace rx
{

namespace gl_gl
{

static GLint QuerySingleGLInt(const FunctionsGL *functions, GLenum name)
{
    GLint result;
    functions->getIntegerv(name, &result);
    return result;
}

void GenerateCaps(const FunctionsGL *functions, gl::Caps *caps, gl::TextureCapsMap *textureCapsMap, gl::Extensions *extensions)
{
    // Set some minimum GLES2 caps, TODO: query for real GL caps

    // Table 6.28, implementation dependent values
    caps->maxElementIndex = static_cast<GLint64>(std::numeric_limits<unsigned int>::max());
    caps->max3DTextureSize = 0;
    caps->max2DTextureSize = 1024;
    caps->maxCubeMapTextureSize = caps->max2DTextureSize;
    caps->maxArrayTextureLayers = 1;
    caps->maxLODBias = 0.0f;
    caps->maxRenderbufferSize = caps->max2DTextureSize;
    caps->maxDrawBuffers = 1;
    caps->maxColorAttachments = 1;
    caps->maxViewportWidth = caps->max2DTextureSize;
    caps->maxViewportHeight = caps->maxViewportWidth;
    caps->minAliasedPointSize = 1.0f;
    caps->maxAliasedPointSize = 1.0f;
    caps->minAliasedLineWidth = 1.0f;
    caps->maxAliasedLineWidth = 1.0f;

    // Table 6.29, implementation dependent values (cont.)
    caps->maxElementsIndices = 0;
    caps->maxElementsVertices = 0;
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
    caps->maxServerWaitTimeout = 0;

    // Table 6.31, implementation dependent vertex shader limits
    caps->maxVertexAttributes = 16;
    caps->maxVertexUniformVectors = 256;
    caps->maxVertexUniformVectors = caps->maxVertexUniformVectors * 4;
    caps->maxVertexUniformBlocks = 0;
    caps->maxVertexOutputComponents = 16;
    caps->maxVertexTextureImageUnits = caps->maxTextureImageUnits;

    // Table 6.32, implementation dependent fragment shader limits
    caps->maxFragmentUniformVectors = 256;
    caps->maxFragmentUniformComponents = caps->maxFragmentUniformVectors * 4;
    caps->maxFragmentUniformBlocks = 0;
    caps->maxFragmentInputComponents = caps->maxVertexOutputComponents;
    caps->maxTextureImageUnits = 16;
    caps->minProgramTexelOffset = 0;
    caps->maxProgramTexelOffset = 0;

    // Table 6.33, implementation dependent aggregate shader limits
    caps->maxUniformBufferBindings = 0;
    caps->maxUniformBlockSize = 0;
    caps->uniformBufferOffsetAlignment = 0;
    caps->maxCombinedUniformBlocks = 0;
    caps->maxCombinedVertexUniformComponents = 0;
    caps->maxCombinedFragmentUniformComponents = 0;
    caps->maxVaryingComponents = 0;
    caps->maxVaryingVectors = caps->maxVertexOutputComponents / 4;
    caps->maxCombinedTextureImageUnits = caps->maxVertexTextureImageUnits + caps->maxTextureImageUnits;

    // Table 6.34, implementation dependent transform feedback limits
    caps->maxTransformFeedbackInterleavedComponents = 0;
    caps->maxTransformFeedbackSeparateAttributes = 0;
    caps->maxTransformFeedbackSeparateComponents = 0;

    // Texture Caps
    gl::TextureCaps supportedTextureFormat;
    supportedTextureFormat.texturable = true;
    supportedTextureFormat.filterable = true;
    supportedTextureFormat.renderable = true;

    textureCapsMap->insert(GL_RGB565, supportedTextureFormat);
    textureCapsMap->insert(GL_RGBA4, supportedTextureFormat);
    textureCapsMap->insert(GL_RGB5_A1, supportedTextureFormat);
    textureCapsMap->insert(GL_RGB8_OES, supportedTextureFormat);
    textureCapsMap->insert(GL_RGBA8_OES, supportedTextureFormat);

    textureCapsMap->insert(GL_DEPTH_COMPONENT16, supportedTextureFormat);
    textureCapsMap->insert(GL_STENCIL_INDEX8, supportedTextureFormat);

    // Extension support
    extensions->setTextureExtensionSupport(*textureCapsMap);
    extensions->textureNPOT = true;
    extensions->textureStorage = true;
}

}

}
