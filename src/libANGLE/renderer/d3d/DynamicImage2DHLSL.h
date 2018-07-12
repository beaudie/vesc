//
// Copyright (c) 2018 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//
// DynamicImage2DHLSL.h: Interface for link and run-time HLSL generation
//

#ifndef LIBANGLE_RENDERER_D3D_DYNAMICIMAGE2DHLSL_H_
#define LIBANGLE_RENDERER_D3D_DYNAMICIMAGE2DHLSL_H_

#include "common/angleutils.h"
#include "libANGLE/renderer/d3d/RendererD3D.h"

namespace rx
{

class DynamicImage2DHLSL : angle::NonCopyable
{
  public:
    static std::string generateComputeShaderForImage2DBoundSignature(
        const d3d::Context *context,
        ProgramD3D &programD3D,
        const gl::ProgramState &programData,
        std::vector<sh::Uniform> &image2DUniforms,
        const std::map<unsigned int, gl::TextureType> &image2DBoundLayout,
        std::map<unsigned int, unsigned int> &readonlyImage2DImageIndexCS,
        std::map<unsigned int, unsigned int> &image2DImageIndexCS);
};

}  // namespace rx

#endif  // LIBANGLE_RENDERER_D3D_DYNAMICHLSL_H_
