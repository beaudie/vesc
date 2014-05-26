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
    : texture2D(false),
      textureCubeMap(false),
      texture3D(false),
      texture2DArray(false),
      filtering(false),
      colorRendering(false),
      depthRendering(false),
      stencilRendering(false),
      sampleCounts()
{
}

Extensions::Extensions()
    : elementIndexUint(false),
      packedDepthStencil(false),
      getProgramBinary(false),
      rgb8rgba8(false),
      textureFormatBGRA8888(false),
      readFormatBGRA(false),
      pixelBufferObject(false),
      mapBuffer(false),
      mapBufferRange(false),
      textureHalfFloat(false),
      textureHalfFloatLinear(false),
      textureFloat(false),
      textureFloatLinear(false),
      textureRG(false),
      textureCompressionDXT1(false),
      textureCompressionDXT3(false),
      textureCompressionDXT5(false),
      depthTextures(false),
      textureNPOT(false),
      drawBuffers(false),
      textureStorage(false),
      textureFilterAnisotropic(false),
      maxTextureAnisotropy(false),
      occlusionQueryBoolean(false),
      fence(false),
      timerQuery(false),
      robustness(false),
      blendMinMax(false),
      framebufferBlit(false),
      framebufferMultisample(false),
      instancedArrays(false),
      packReverseRowOrder(false),
      standardDerivatives(false),
      shaderTextureLOD(false),
      fragDepth(false),
      textureUsage(false),
      translatedShaderSource(false),
      colorBufferFloat(false)
{
}

Caps::Caps()
    : maxElementIndex(0),
      max3DTextureSize(0),
      max2DTextureSize(0),
      maxArrayTextureLayers(0),
      maxLODBias(0),
      maxCubeMapTextureSize(0),
      maxRenderbufferSize(0),
      maxDrawBuffers(0),
      maxColorAttachments(0),
      maxViewportWidth(0),
      maxViewportHeight(0),
      minAliasedPointSize(0),
      maxAliasedPointSize(0),
      minAliasedLineWidth(0),
      maxAliasedLineWidth(0),
      textureCaps(),
      extensions()
{
}

}
