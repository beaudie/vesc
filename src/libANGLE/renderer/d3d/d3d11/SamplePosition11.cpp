//
// Copyright (c) 2017 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//

// SamplePosition11.h:
//   Get sample position by index of current render target.

#include "libANGLE/renderer/d3d/d3d11/SamplePosition11.h"

#include "libANGLE/renderer/d3d/d3d11/RenderTarget11.h"
#include "libANGLE/renderer/d3d/d3d11/Renderer11.h"
#include "libANGLE/renderer/d3d/d3d11/ResourceManager11.h"
#include "libANGLE/renderer/d3d/d3d11/formatutils11.h"

namespace rx
{

namespace
{
// Precompiled shaders
#include "libANGLE/renderer/d3d/d3d11/shaders/compiled/sampleposition_2f_ps.h"
#include "libANGLE/renderer/d3d/d3d11/shaders/compiled/sampleposition_2i_ps.h"
#include "libANGLE/renderer/d3d/d3d11/shaders/compiled/sampleposition_2ui_ps.h"
#include "libANGLE/renderer/d3d/d3d11/shaders/compiled/sampleposition_vs.h"
}

// Define the input layout
D3D11_INPUT_ELEMENT_DESC layout[] = {
    {"POSITION", 0, DXGI_FORMAT_R32G32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0},
    {"TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 0, 8, D3D11_INPUT_PER_VERTEX_DATA, 0},
};

SamplePosition11::SamplePosition11(Renderer11 *renderer)
    : mRenderer(renderer),
      mResourcesLoaded(false),
      mSamplePositionVS(),
      mParamsConstantBuffer(),
      mInputLayout(layout,
                   ArraySize(layout),
                   g_VS_SamplePosition,
                   ArraySize(g_VS_SamplePosition),
                   "SamplePosition11 input layout")
{
}

SamplePosition11::~SamplePosition11()
{
}

gl::Error SamplePosition11::loadResources()
{
    if (mResourcesLoaded)
    {
        return gl::NoError();
    }

    // Init vertex buffer
    D3D11_BUFFER_DESC vertexBufferDesc = {0};
    vertexBufferDesc.Usage             = D3D11_USAGE_DYNAMIC;
    vertexBufferDesc.ByteWidth         = sizeof(d3d11::PositionTexCoordVertex) * 4;
    vertexBufferDesc.BindFlags         = D3D11_BIND_VERTEX_BUFFER;
    vertexBufferDesc.CPUAccessFlags    = D3D11_CPU_ACCESS_WRITE;

    ANGLE_TRY(mRenderer->allocateResource(vertexBufferDesc, &mVertexBuffer));
    mVertexBuffer.setDebugName("SamplePosition11 vertex buffer");

    // Init input layout
    ANGLE_TRY(mInputLayout.resolve(mRenderer));

    // Init shaders
    ANGLE_TRY(mRenderer->allocateResource(ShaderData(g_VS_SamplePosition), &mSamplePositionVS));
    mSamplePositionVS.setDebugName("SamplePosition VS");
    ANGLE_TRY(buildShaderMap());

    mResourcesLoaded = true;

    return gl::NoError();
}

gl::Error SamplePosition11::getSample(const gl::Context *context,
                                      RenderTargetD3D *attachmentRenderTarget,
                                      size_t index,
                                      GLfloat *xy)
{
    // pData preserve queried sample positions, translate D3D's sample coordinate to OpenGL
    if (pData != nullptr && mAttachmentRenderTarget == attachmentRenderTarget)
    {
        xy[0] = pData[index].x + 0.5f;
        xy[1] = pData[index].y + 0.5f;
        return gl::NoError();
    }

    ANGLE_TRY(loadResources());

    mAttachmentRenderTarget            = attachmentRenderTarget;
    const auto &extents                = attachmentRenderTarget->getExtents();
    ID3D11DeviceContext *deviceContext = mRenderer->getDeviceContext();
    HRESULT result;

    // Set vertices
    D3D11_MAPPED_SUBRESOURCE mappedResource;
    result =
        deviceContext->Map(mVertexBuffer.get(), 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedResource);
    if (FAILED(result))
    {
        return gl::OutOfMemory() << "Failed to map internal vertex buffer for samplePosition, "
                                 << gl::FmtHR(result);
    }
    d3d11::PositionTexCoordVertex *mappedVertices =
        reinterpret_cast<d3d11::PositionTexCoordVertex *>(mappedResource.pData);
    d3d11::SetPositionTexCoordVertex(&mappedVertices[0], -1.0f, -1.0f, 0.0f, 1.0f);
    d3d11::SetPositionTexCoordVertex(&mappedVertices[1], -1.0f, 1.0f, 0.0f, 0.0f);
    d3d11::SetPositionTexCoordVertex(&mappedVertices[2], 1.0f, -1.0f, 1.0f, 1.0f);
    d3d11::SetPositionTexCoordVertex(&mappedVertices[3], 1.0f, 1.0f, 1.0f, 0.0f);
    deviceContext->Unmap(mVertexBuffer.get(), 0);

    // Get shader resource view
    RenderTarget11 *renderTarget11       = GetAs<RenderTarget11>(attachmentRenderTarget);
    ID3D11ShaderResourceView *textureSRV = renderTarget11->getShaderResourceView().get();
    ASSERT(textureSRV != nullptr);

    D3D11_SHADER_RESOURCE_VIEW_DESC texSRVDesc;
    textureSRV->GetDesc(&texSRVDesc);

    // Apply shaders
    auto stateManager = mRenderer->getStateManager();

    stateManager->setSingleVertexBuffer(&mVertexBuffer, sizeof(d3d11::PositionTexCoordVertex), 0);
    stateManager->setInputLayout(&mInputLayout.getObj());
    stateManager->setPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLESTRIP);
    stateManager->setShaderResource(gl::SAMPLER_PIXEL, 0, textureSRV);

    GLenum componentType                        = d3d11::GetComponentType(texSRVDesc.Format);
    const d3d11::PixelShader *mSamplePositionPS = findSamplePostionPS(componentType);
    ASSERT(mSamplePositionPS);
    stateManager->setDrawShaders(&mSamplePositionVS, nullptr, mSamplePositionPS);

    // Set the viewport
    D3D11_VIEWPORT viewport;
    viewport.TopLeftX = 0;
    viewport.TopLeftY = 0;
    viewport.Width    = static_cast<float>(extents.width);
    viewport.Height   = static_cast<float>(extents.height);
    viewport.MinDepth = 0.0f;
    viewport.MaxDepth = 1.0f;
    deviceContext->RSSetViewports(1, &viewport);

    // Set read write buffer
    int sampleCount = attachmentRenderTarget->getSamples();

    D3D11_BUFFER_DESC readBufferDesc   = {0};
    readBufferDesc.BindFlags           = D3D11_BIND_SHADER_RESOURCE | D3D11_BIND_UNORDERED_ACCESS;
    readBufferDesc.ByteWidth           = sizeof(SamplePosition) * sampleCount;
    readBufferDesc.MiscFlags           = D3D11_RESOURCE_MISC_BUFFER_STRUCTURED;
    readBufferDesc.StructureByteStride = sizeof(SamplePosition);
    ANGLE_TRY(mRenderer->allocateResource(readBufferDesc, &mReadBuffer));
    mReadBuffer.setDebugName("SamplePosition read buffer");

    D3D11_UNORDERED_ACCESS_VIEW_DESC uav_desc;
    ZeroMemory(&uav_desc, sizeof(uav_desc));
    uav_desc.ViewDimension       = D3D11_UAV_DIMENSION_BUFFER;
    uav_desc.Buffer.FirstElement = 0;
    uav_desc.Format              = DXGI_FORMAT_UNKNOWN;
    uav_desc.Buffer.NumElements  = sampleCount;

    mRenderer->allocateResource(uav_desc, mReadBuffer.get(), &mBufResultUAV);
    mBufResultUAV.setDebugName("SamplePosition buffer result uav");
    // Set read buffer UAV
    ID3D11UnorderedAccessView *uav = mBufResultUAV.get();
    deviceContext->OMSetRenderTargetsAndUnorderedAccessViews(0, nullptr, nullptr, 1, 1, &uav,
                                                             nullptr);

    deviceContext->Draw(4, 0);

    // Read sample position
    D3D11_MAPPED_SUBRESOURCE MappedResource;
    d3d11::Buffer mResultBuffer;
    D3D11_BUFFER_DESC result_desc = {0};
    mReadBuffer.get()->GetDesc(&result_desc);
    result_desc.CPUAccessFlags      = D3D11_CPU_ACCESS_READ;
    result_desc.Usage               = D3D11_USAGE_STAGING;
    result_desc.BindFlags           = 0;
    result_desc.MiscFlags           = 0;
    result_desc.ByteWidth           = sizeof(SamplePosition) * sampleCount;
    result_desc.StructureByteStride = sizeof(SamplePosition);
    mRenderer->allocateResource(result_desc, &mResultBuffer);
    mResultBuffer.setDebugName("SamplePosition result buffer");

    deviceContext->CopyResource(mResultBuffer.get(), mReadBuffer.get());
    result = deviceContext->Map(mResultBuffer.get(), 0, D3D11_MAP_READ, 0, &MappedResource);
    if (FAILED(result))
    {
        deviceContext->Unmap(mResultBuffer.get(), 0);
        return gl::OutOfMemory() << "Failed to map internal staging buffer for SamplePosition, "
                                 << result;
    }
    pData = (SamplePosition *)(MappedResource.pData);
    // Translate D3D's sample coordinate to OpenGL
    xy[0] = pData[index].x + 0.5f;
    xy[1] = pData[index].y + 0.5f;

    deviceContext->Unmap(mResultBuffer.get(), 0);

    // Unbind textures, render targets and vertex buffer
    stateManager->setShaderResource(gl::SAMPLER_PIXEL, 0, NULL);

    UINT zero                      = 0;
    ID3D11Buffer *const nullBuffer = nullptr;
    deviceContext->IASetVertexBuffers(0, 1, &nullBuffer, &zero, &zero);
    deviceContext->OMSetRenderTargetsAndUnorderedAccessViews(0, nullptr, nullptr, 0, 0, nullptr, 0);

    mRenderer->markAllStateDirty(context);

    return gl::NoError();
}

gl::Error SamplePosition11::buildShaderMap()
{
    d3d11::PixelShader ps_2F, ps_2UI, ps_2I;
    ANGLE_TRY(mRenderer->allocateResource(ShaderData(g_PS_SamplePosition_2F), &ps_2F));
    ANGLE_TRY(mRenderer->allocateResource(ShaderData(g_PS_SamplePosition_2UI), &ps_2UI));
    ANGLE_TRY(mRenderer->allocateResource(ShaderData(g_PS_SamplePosition_2I), &ps_2I));

    ps_2F.setDebugName("SamplePosition RGBA ps");
    ps_2I.setDebugName("SamplePosition RGBA-I ps");
    ps_2UI.setDebugName("SamplePosition RGBA-UI ps");

    mSamplePositionPSMap[GL_FLOAT]        = std::move(ps_2F);
    mSamplePositionPSMap[GL_UNSIGNED_INT] = std::move(ps_2UI);
    mSamplePositionPSMap[GL_INT]          = std::move(ps_2I);

    return gl::NoError();
}

const d3d11::PixelShader *SamplePosition11::findSamplePostionPS(GLenum componentType) const
{
    if (componentType == GL_SIGNED_NORMALIZED || componentType == GL_UNSIGNED_NORMALIZED)
    {
        componentType = GL_FLOAT;
    }

    auto shaderMapIt = mSamplePositionPSMap.find(componentType);
    return (shaderMapIt == mSamplePositionPSMap.end() ? nullptr : &shaderMapIt->second);
}

}  // namespace rx
