//
// Copyright (c) 2017 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//

// SamplePosition11.h:
//   Get sample position by index of current render target.

#include "libANGLE/renderer/d3d/d3d11/SamplePosition11.h"

#include "libANGLE/renderer/d3d/RenderTargetD3D.h"
#include "libANGLE/formatutils.h"
#include "libANGLE/renderer/d3d/d3d11/Renderer11.h"
#include "libANGLE/renderer/d3d/d3d11/RenderTarget11.h"
#include "libANGLE/renderer/d3d/d3d11/renderer11_utils.h"
#include "libANGLE/renderer/d3d/d3d11/formatutils11.h"
#include "libANGLE/renderer/d3d/d3d11/texture_format_table.h"
#include "third_party/trace_event/trace_event.h"

namespace rx
{

namespace
{
// Precompiled shaders
#include "libANGLE/renderer/d3d/d3d11/shaders/compiled/sampleposition_vs.h"
#include "libANGLE/renderer/d3d/d3d11/shaders/compiled/sampleposition_2f_ps.h"
#include "libANGLE/renderer/d3d/d3d11/shaders/compiled/sampleposition_2ui_ps.h"
#include "libANGLE/renderer/d3d/d3d11/shaders/compiled/sampleposition_2i_ps.h"
}

SamplePosition11::SamplePosition11(Renderer11 *renderer)
    : mRenderer(renderer),
      mResourcesLoaded(false),
      mSamplePositionVS(NULL),
      mParamsConstantBuffer(NULL)
{
}

SamplePosition11::~SamplePosition11()
{
    for (auto shaderMapIt = mSamplePositionPSMap.begin(); shaderMapIt != mSamplePositionPSMap.end();
         shaderMapIt++)
    {
        SafeRelease(shaderMapIt->second);
    }

    mSamplePositionPSMap.clear();

    SafeRelease(mSamplePositionVS);
}

gl::Error SamplePosition11::loadResources()
{
    if (mResourcesLoaded)
    {
        return gl::NoError();
    }

    HRESULT result                     = S_OK;
    ID3D11Device *device               = mRenderer->getDevice();
	ID3D11DeviceContext *deviceContext = mRenderer->getDeviceContext();
	// bind constant buffer
   /* D3D11_BUFFER_DESC constantBufferDesc;
	ZeroMemory(&constantBufferDesc, sizeof(constantBufferDesc));
    constantBufferDesc.ByteWidth           = sizeof(GLuint) * 4;
    constantBufferDesc.Usage               = D3D11_USAGE_DYNAMIC;
    constantBufferDesc.BindFlags           = D3D11_BIND_CONSTANT_BUFFER;
    constantBufferDesc.CPUAccessFlags      = D3D11_CPU_ACCESS_WRITE;

    result = device->CreateBuffer(&constantBufferDesc, nullptr, &mParamsConstantBuffer);
    ASSERT(SUCCEEDED(result));
    if (FAILED(result))
    {
        return gl::Error(GL_OUT_OF_MEMORY,
                         "Failed to create internal sample position constant buffer, result: 0x%X.",
                         result);
    }
    d3d11::SetDebugName(mParamsConstantBuffer, "SamplePosition11 constant buffer");
	*/
	// Set input layout
	// Define the input layout
	D3D11_INPUT_ELEMENT_DESC layout[] = {
		{ "POSITION", 0, DXGI_FORMAT_R32G32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0 },
		{ "TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 0, 8, D3D11_INPUT_PER_VERTEX_DATA, 0 },
	};
	UINT numElements = ARRAYSIZE(layout);

	// Create the input layout
	result = device->CreateInputLayout(layout, numElements, g_VS_SamplePosition,
		ArraySize(g_VS_SamplePosition), &mVertexLayout);

	ASSERT(SUCCEEDED(result));
	if (FAILED(result))
	{
		return gl::OutOfMemory() << "Failed to create sample position input layout, " << result;
	}
	// Set the input layout
	deviceContext->IASetInputLayout(mVertexLayout);

	// bind vertex buffer
	D3D11_BUFFER_DESC vertexBufferDesc;
	ZeroMemory(&vertexBufferDesc, sizeof(vertexBufferDesc));
	vertexBufferDesc.Usage = D3D11_USAGE_DEFAULT;
	vertexBufferDesc.ByteWidth = sizeof(d3d11::PositionTexCoordVertex) * 4;
	vertexBufferDesc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
	vertexBufferDesc.CPUAccessFlags = 0;
	D3D11_SUBRESOURCE_DATA InitData;
	ZeroMemory(&InitData, sizeof(InitData));

	// Set vertices
	d3d11::PositionTexCoordVertex vertices[4];
	d3d11::SetPositionTexCoordVertex(&vertices[0], -1.0f, 1.0f, 0.0f, 1.0f);
	d3d11::SetPositionTexCoordVertex(&vertices[1], -1.0f, -1.0f, 0.0f, 0.0f);
	d3d11::SetPositionTexCoordVertex(&vertices[2], 1.0f, 1.0f, 1.0f, 1.0f);
	d3d11::SetPositionTexCoordVertex(&vertices[3], 1.0f, -1.0f, 1.0f, 0.0f);
	InitData.pSysMem = vertices;
	result = device->CreateBuffer(&vertexBufferDesc, &InitData, &mVertexBuffer);

	ASSERT(SUCCEEDED(result));
	if (FAILED(result))
	{
		return gl::OutOfMemory() << "Failed to create sample position vertex buffer, " << result;
	}
	d3d11::SetDebugName(mVertexBuffer, "SamplePosition11 vertex buffer");
	UINT stride = sizeof(d3d11::PositionTexCoordVertex);
	UINT startIdx = 0;
	deviceContext->IASetVertexBuffers(0, 1, &mVertexBuffer, &stride, &startIdx);

	WORD indices[] =
	{
		1,2,3,
		2,3,4
	};
	
	vertexBufferDesc.Usage = D3D11_USAGE_DEFAULT;
	vertexBufferDesc.ByteWidth = sizeof(WORD) * 6;
	vertexBufferDesc.BindFlags = D3D11_BIND_INDEX_BUFFER;
	vertexBufferDesc.CPUAccessFlags = 0;
	InitData.pSysMem = indices;
	result = device->CreateBuffer(&vertexBufferDesc, &InitData, &mIndexBuffer);
	if (FAILED(result))
	{
		return gl::OutOfMemory() << "Failed to create sample position index buffer, " << result;
	}

	// Set index buffer
	deviceContext->IASetIndexBuffer(mIndexBuffer, DXGI_FORMAT_R16_UINT, 0);

	deviceContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

	// read data from gpu
	ZeroMemory(&readBufferDesc, sizeof(readBufferDesc));
	readBufferDesc.BindFlags = D3D11_BIND_SHADER_RESOURCE | D3D11_BIND_UNORDERED_ACCESS;
	readBufferDesc.ByteWidth = sizeof(BufType);
	readBufferDesc.MiscFlags = D3D11_RESOURCE_MISC_BUFFER_STRUCTURED;
	readBufferDesc.StructureByteStride = sizeof(BufType);
	result = device->CreateBuffer(&readBufferDesc, nullptr, &mReadBuffer);

	D3D11_UNORDERED_ACCESS_VIEW_DESC uav_desc;
	ZeroMemory(&uav_desc, sizeof(uav_desc));
	uav_desc.ViewDimension = D3D11_UAV_DIMENSION_BUFFER;
	uav_desc.Buffer.FirstElement = 0;
	uav_desc.Format = DXGI_FORMAT_UNKNOWN;
	uav_desc.Buffer.NumElements = 1;
	result = device->CreateUnorderedAccessView(mReadBuffer, &uav_desc, &mBufResultUAV);
	if (FAILED(result))
	{
		return gl::OutOfMemory() << "Failed to init UAV for output buffer for SamplePosition, "
			<< result;
	}
	// Set read buffer UAV
	deviceContext->OMSetRenderTargetsAndUnorderedAccessViews(0, nullptr, nullptr, 1, 1,
		&mBufResultUAV, nullptr);

    mResourcesLoaded = true;

    return gl::NoError();
}

gl::Error SamplePosition11::getSample(RenderTargetD3D *attachmentRenderTarget,
                                      size_t index,
                                      GLfloat *xy)
{
    ANGLE_TRY(loadResources());
    buildShaderMap();

    ID3D11DeviceContext *deviceContext = mRenderer->getDeviceContext();
	ID3D11Device *device = mRenderer->getDevice();
    HRESULT result;
	//D3D11_MAPPED_SUBRESOURCE mappedResource;

	// set constant buffer passing index of sample to shader
	/*result = deviceContext->Map(mParamsConstantBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedResource);
	if (FAILED(result))
	{
		return gl::OutOfMemory() << "Failed to map internal constant buffer for sample position, "
			<< result;
	}

	unsigned int *samplePositionIndex = reinterpret_cast<unsigned int *>(mappedResource.pData);
	samplePositionIndex[0] = static_cast<unsigned int>(index);

	deviceContext->Unmap(mParamsConstantBuffer, 0);
	deviceContext->PSSetConstantBuffers(0, 1, &mParamsConstantBuffer);
	*/
	// create shader resource view
    /*ID3D11ShaderResourceView *textureSRV =
        GetAs<RenderTarget11>(attachmentRenderTarget)->getShaderResourceView();

    ASSERT(textureSRV != nullptr);

    D3D11_SHADER_RESOURCE_VIEW_DESC texSRVDesc;
    textureSRV->GetDesc(&texSRVDesc);

	const auto &extents = attachmentRenderTarget->getExtents();
	
    //GLenum componentType = d3d11::GetComponentType(texSRVDesc.Format);
   if (componentType == GL_NONE)
    {
        // We're swizzling the depth component of a depth-stencil texture.
        switch (texSRVDesc.Format)
        {
            case DXGI_FORMAT_R24_UNORM_X8_TYPELESS:
                componentType = GL_UNSIGNED_NORMALIZED;
                break;
            case DXGI_FORMAT_R32_FLOAT_X8X24_TYPELESS:
                componentType = GL_FLOAT;
                break;
            default:
                UNREACHABLE();
                break;
        }
    }
	*/
	GLenum componentType = d3d11::GetComponentType(DXGI_FORMAT_R8G8B8A8_UNORM);
    ID3D11PixelShader *mSamplePositionPS = findSamplePostionPS(componentType);
    deviceContext->VSSetShader(mSamplePositionVS, nullptr, 0);
    deviceContext->PSSetShader(mSamplePositionPS, nullptr, 0);

    //auto stateManager = mRenderer->getStateManager();
    //stateManager->setShaderResource(gl::SAMPLER_PIXEL, 0, textureSRV);
	
	// Set the viewport
	D3D11_VIEWPORT viewport;
	viewport.TopLeftX = 0;
	viewport.TopLeftY = 0;
	//viewport.Width = static_cast<float>(extents.width);
	//viewport.Height = static_cast<float>(extents.height);
	viewport.Width = 1;
	viewport.Height = 1;
	viewport.MinDepth = 0.0f;
	viewport.MaxDepth = 1.0f;
	deviceContext->RSSetViewports(1, &viewport);

	// create texture
	ID3D11Texture2D *tex;
	D3D11_TEXTURE2D_DESC desc;
	desc.Width = 1;
	desc.Height = 1;
	desc.MipLevels = 1;
	desc.ArraySize = 1;
	desc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
	desc.SampleDesc.Count = 4;
	desc.SampleDesc.Quality = 0;
	desc.Usage = D3D11_USAGE_DEFAULT;
	desc.BindFlags = D3D11_BIND_SHADER_RESOURCE;
	desc.CPUAccessFlags = 0;
	desc.MiscFlags = 0;

	result = device->CreateTexture2D(&desc, nullptr, &tex);
	if (FAILED(result))
	{
		return gl::OutOfMemory() << "Failed to create texture for SamplePosition, "
			<< result;
	}
	ID3D11ShaderResourceView*           g_pTextureRV = nullptr;
	D3D11_SHADER_RESOURCE_VIEW_DESC srvDesc;
	srvDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
	srvDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2DMS;
	srvDesc.Texture2D.MostDetailedMip = 0;
	srvDesc.Texture2D.MipLevels = 0;
	result = device->CreateShaderResourceView(tex, &srvDesc, &g_pTextureRV);
	deviceContext->PSSetShaderResources(0, 1, &g_pTextureRV);
	if (FAILED(result))
	{
		return gl::OutOfMemory() << "Failed to create result buffer for SamplePosition, "
			<< result;
	}

    //deviceContext->Draw(4, 0);
	deviceContext->DrawIndexed(6,0,0);

    D3D11_MAPPED_SUBRESOURCE MappedResource;
	ID3D11Buffer *mResultBuffer = nullptr;
	D3D11_BUFFER_DESC result_desc;
	ZeroMemory(&result_desc, sizeof(result_desc));
	mReadBuffer->GetDesc(&result_desc);
	result_desc.CPUAccessFlags = D3D11_CPU_ACCESS_READ;
	result_desc.Usage = D3D11_USAGE_STAGING;
	result_desc.BindFlags = 0;
	result_desc.MiscFlags = 0;
	result_desc.ByteWidth = sizeof(BufType);
	result_desc.StructureByteStride = sizeof(BufType);
	result = device->CreateBuffer(&result_desc, nullptr, &mResultBuffer);
	if (FAILED(result))
	{
		return gl::OutOfMemory() << "Failed to create result buffer for SamplePosition, "
			<< result;
	}
	deviceContext->CopyResource(mResultBuffer, mReadBuffer);
    result = deviceContext->Map(mResultBuffer, 0, D3D11_MAP_READ, 0, &MappedResource);
    if (FAILED(result))
    {
        deviceContext->Unmap(mResultBuffer, 0);
        return gl::OutOfMemory() << "Failed to map internal staging buffer for SamplePosition, "
                                 << result;
    }

    BufType *pData = (BufType *)(MappedResource.pData);
    xy[0]          = pData[0].x;
    xy[1]          = pData[0].y;

    deviceContext->Unmap(mResultBuffer, 0);

    // Unbind textures and render targets and vertex buffer
    //stateManager->setShaderResource(gl::SAMPLER_PIXEL, 0, NULL);

    UINT zero                      = 0;
    ID3D11Buffer *const nullBuffer = nullptr;
    deviceContext->PSSetConstantBuffers(0, 1, &nullBuffer);
    deviceContext->IASetVertexBuffers(0, 1, &nullBuffer, &zero, &zero);
    deviceContext->OMSetRenderTargetsAndUnorderedAccessViews(0, nullptr, nullptr, 0, 0, nullptr, 0);

    mRenderer->markAllStateDirty();

    return gl::NoError();
}

gl::Error SamplePosition11::buildShaderMap()
{
    ID3D11Device *device = mRenderer->getDevice();

    mSamplePositionPSMap[GL_FLOAT] =
        d3d11::CompilePS(device, g_PS_SamplePosition_2F, "SamplePosition F ps");
    mSamplePositionPSMap[GL_FLOAT] =
        d3d11::CompilePS(device, g_PS_SamplePosition_2UI, "SamplePosition UI ps");
    mSamplePositionPSMap[GL_FLOAT] =
        d3d11::CompilePS(device, g_PS_SamplePosition_2I, "SamplePosition I ps");
    mSamplePositionVS = d3d11::CompileVS(device, g_VS_SamplePosition, "SamplePosition vs");

    // Check that all the shaders were created successfully
    for (auto shaderMapIt = mSamplePositionPSMap.begin(); shaderMapIt != mSamplePositionPSMap.end();
         shaderMapIt++)
    {
        if (shaderMapIt->second == NULL)
        {
            return gl::Error(GL_OUT_OF_MEMORY,
                             "Failed to create internal buffer to texture pixel shader.");
        }
    }

    return gl::NoError();
}

ID3D11PixelShader *SamplePosition11::findSamplePostionPS(GLenum componentType) const
{
    if (componentType == GL_SIGNED_NORMALIZED || componentType == GL_UNSIGNED_NORMALIZED)
    {
        componentType = GL_FLOAT;
    }

    auto shaderMapIt = mSamplePositionPSMap.find(componentType);
    return (shaderMapIt == mSamplePositionPSMap.end() ? NULL : shaderMapIt->second);
}

}  // namespace rx
