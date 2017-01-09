//
// Copyright (c) 2017 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//

// SamplePosition11.h:
//   Get sample position by index of current render target.

#ifndef LIBANGLE_RENDERER_D3D_D3D11_SAMPLEPOSITION11_H_
#define LIBANGLE_RENDERER_D3D_D3D11_SAMPLEPOSITION11_H_

#include "libANGLE/Error.h"
#include "common/angleutils.h"
#include "libANGLE/angletypes.h"
#include "libANGLE/renderer/d3d/d3d11/renderer11_utils.h"
#include <map>

namespace rx
{
class Renderer11;
class RenderTragetD3D;

class SamplePosition11 : angle::NonCopyable
{
  public:
    explicit SamplePosition11(Renderer11 *renderer);
    ~SamplePosition11();

    gl::Error getSample(RenderTargetD3D *attachmentRenderTarget, size_t index, GLfloat *xy);

  private:
    gl::Error loadResources();
    gl::Error buildShaderMap();
    ID3D11PixelShader *findSamplePostionPS(GLenum internalFormat) const;

    Renderer11 *mRenderer;

    bool mResourcesLoaded;
    ID3D11Buffer *mParamsConstantBuffer;
    ID3D11Buffer *mVertexBuffer;
	ID3D11Buffer *mIndexBuffer;
    ID3D11VertexShader *mSamplePositionVS;
    std::map<GLenum, ID3D11PixelShader *> mSamplePositionPSMap;
    ID3D11ShaderResourceView *mShaderResourceView;

    ID3D11UnorderedAccessView *mBufResultUAV = nullptr;
	ID3D11InputLayout *mVertexLayout = nullptr;
	ID3D11Buffer *mReadBuffer = nullptr;
	D3D11_BUFFER_DESC readBufferDesc;

	struct BufType
	{
		float x, y;
	};
};
}

#endif  // LIBANGLE_RENDERER_D3D_D3D11_SAMPLEPOSITION11_H_
