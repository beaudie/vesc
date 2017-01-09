//
// Copyright (c) 2017 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//

// SamplePosition11.h:
//   Get sample position by index of current render target.

#ifndef LIBANGLE_RENDERER_D3D_D3D11_SAMPLEPOSITION11_H_
#define LIBANGLE_RENDERER_D3D_D3D11_SAMPLEPOSITION11_H_

#include <map>
#include "common/angleutils.h"
#include "libANGLE/Error.h"
#include "libANGLE/angletypes.h"
#include "libANGLE/renderer/d3d/d3d11/ResourceManager11.h"
#include "libANGLE/renderer/d3d/d3d11/renderer11_utils.h"

namespace rx
{
class Renderer11;
class RenderTragetD3D;

class SamplePosition11 : angle::NonCopyable
{
  public:
    explicit SamplePosition11(Renderer11 *renderer);
    ~SamplePosition11();

    gl::Error getSample(const gl::Context *context,
                        RenderTargetD3D *attachmentRenderTarget,
                        size_t index,
                        GLfloat *xy);

  private:
    gl::Error loadResources();
    gl::Error buildShaderMap();
    const d3d11::PixelShader *findSamplePostionPS(GLenum internalFormat) const;

    Renderer11 *mRenderer;

    bool mResourcesLoaded;
    d3d11::VertexShader mSamplePositionVS;
    d3d11::Buffer mParamsConstantBuffer;
    d3d11::Buffer mVertexBuffer;
    std::map<GLenum, d3d11::PixelShader> mSamplePositionPSMap;
    d3d11::ShaderResourceView mShaderResourceView;

    d3d11::UnorderedAccessView mSamplePositionUAV;
    d3d11::Buffer mSamplePositionsBuffer;
    d3d11::LazyInputLayout mInputLayout;

    RenderTargetD3D *mAttachmentRenderTarget = nullptr;
    struct SamplePosition
    {
        float x, y;
    };
    std::unordered_map<int, std::unique_ptr<SamplePosition>> samplePositionsMap;
};
}

#endif  // LIBANGLE_RENDERER_D3D_D3D11_SAMPLEPOSITION11_H_
