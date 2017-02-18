//
// Copyright (c) 2013 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//

// Clear11.cpp: Framebuffer clear utility class.

#include "libANGLE/renderer/d3d/d3d11/Clear11.h"

#include <algorithm>

#include "libANGLE/FramebufferAttachment.h"
#include "libANGLE/formatutils.h"
#include "libANGLE/renderer/d3d/FramebufferD3D.h"
#include "libANGLE/renderer/d3d/d3d11/Renderer11.h"
#include "libANGLE/renderer/d3d/d3d11/renderer11_utils.h"
#include "libANGLE/renderer/d3d/d3d11/RenderTarget11.h"
#include "libANGLE/renderer/d3d/d3d11/formatutils11.h"
#include "third_party/trace_event/trace_event.h"

// Precompiled shaders
#include "libANGLE/renderer/d3d/d3d11/shaders/compiled/clearfloat11vs.h"
#include "libANGLE/renderer/d3d/d3d11/shaders/compiled/clearfloat11ps.h"
#include "libANGLE/renderer/d3d/d3d11/shaders/compiled/clearfloat11_fl9ps.h"

#include "libANGLE/renderer/d3d/d3d11/shaders/compiled/clearuint11vs.h"
#include "libANGLE/renderer/d3d/d3d11/shaders/compiled/clearuint11ps.h"

#include "libANGLE/renderer/d3d/d3d11/shaders/compiled/clearsint11vs.h"
#include "libANGLE/renderer/d3d/d3d11/shaders/compiled/clearsint11ps.h"

namespace rx
{

// Updates color and depth components of cached quad vertices if necessary.
// Returns true if vertices are updated, false otherwise.
template <typename T>
static bool UpdateVertices(const gl::Color<T> &color,
                           const float depth,
                           d3d11::Position3DColorVertex<T> *vertices)
{
    // Only update if the input values don't match existing contents
    if ((memcmp(&vertices->r, &color.red, sizeof(color)) != 0) || depth != vertices->z)
    {
        for (uint32_t i = 0; i < 4; i++)
        {
            vertices[i].z = depth;
            vertices[i].r = color.red;
            vertices[i].g = color.green;
            vertices[i].b = color.blue;
            vertices[i].a = color.alpha;
        }

        return true;
    }

    return false;
}

Clear11::ClearShader::ClearShader(DXGI_FORMAT colorType,
                                  const char *inputLayoutName,
                                  const BYTE *vsByteCode,
                                  size_t vsSize,
                                  const char *vsDebugName,
                                  const BYTE *psByteCode,
                                  size_t psSize,
                                  const char *psDebugName)
    : inputLayout(nullptr),
      vertexShader(vsByteCode, vsSize, vsDebugName),
      pixelShader(psByteCode, psSize, psDebugName)
{
    D3D11_INPUT_ELEMENT_DESC quadLayout[] = {
        {"POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0},
        {"COLOR", 0, colorType, 0, 12, D3D11_INPUT_PER_VERTEX_DATA, 0},
    };

    inputLayout = new d3d11::LazyInputLayout(quadLayout, 2, vsByteCode, vsSize, inputLayoutName);
}

Clear11::ClearShader::~ClearShader()
{
    SafeDelete(inputLayout);
    vertexShader.release();
    pixelShader.release();
}

Clear11::Clear11(Renderer11 *renderer)
    : mRenderer(renderer),
      mFloatClearShader(nullptr),
      mUintClearShader(nullptr),
      mIntClearShader(nullptr),
      mClearDepthStencilStates(StructLessThan<ClearDepthStencilInfo>),
      mVertexBuffer(nullptr),
      mScissorEnabledRasterizerState(nullptr),
      mScissorDisabledRasterizerState(nullptr)
{
    TRACE_EVENT0("gpu.angle", "Clear11::Clear11");

    HRESULT result;
    ID3D11Device *device = renderer->getDevice();

    // Initialize VertexCache
    const gl::ColorF initialColor = {0.0f, 0.0f, 0.0f, 0.0f};
    const float left              = -1.0f;
    const float right             = 1.0f;
    const float top               = -1.0f;
    const float bot               = 1.0f;

    static_assert(sizeof(d3d11::Position3DColorVertex<float>) ==
                      sizeof(d3d11::Position3DColorVertex<uint32_t>),
                  "Size mismatch between d3d11::Position3DVertex<float> and "
                  "d3d11::Position3DColorVertex<uint32_t>");
    static_assert(sizeof(d3d11::Position3DColorVertex<float>) ==
                      sizeof(d3d11::Position3DColorVertex<int32_t>),
                  "Size mismatch between d3d11::Position3DVertex<FLOAT> and "
                  "d3d11::Position3DColorVertex<int32_t>");

    mVertexSize      = sizeof(d3d11::Position3DColorVertex<float>);
    mVertexCacheSize = sizeof(mCachedVertices);

    d3d11::SetPosition3DColorVertex<float>(&mCachedVertices[0].f32, left, bot, 0.0f, initialColor);
    d3d11::SetPosition3DColorVertex<float>(&mCachedVertices[1].f32, left, top, 0.0f, initialColor);
    d3d11::SetPosition3DColorVertex<float>(&mCachedVertices[2].f32, right, bot, 0.0f, initialColor);
    d3d11::SetPosition3DColorVertex<float>(&mCachedVertices[3].f32, right, top, 0.0f, initialColor);

    // Create and initialize VB to values in vertex cache
    D3D11_SUBRESOURCE_DATA initialData;
    initialData.pSysMem          = mCachedVertices;
    initialData.SysMemPitch      = mVertexCacheSize;
    initialData.SysMemSlicePitch = mVertexCacheSize;

    D3D11_BUFFER_DESC vbDesc;
    vbDesc.ByteWidth           = mVertexCacheSize;
    vbDesc.Usage               = D3D11_USAGE_DYNAMIC;
    vbDesc.BindFlags           = D3D11_BIND_VERTEX_BUFFER;
    vbDesc.CPUAccessFlags      = D3D11_CPU_ACCESS_WRITE;
    vbDesc.MiscFlags           = 0;
    vbDesc.StructureByteStride = 0;

    result = device->CreateBuffer(&vbDesc, &initialData, mVertexBuffer.GetAddressOf());
    ASSERT(SUCCEEDED(result));
    d3d11::SetDebugName(mVertexBuffer, "Clear11 masked clear vertex buffer");

    // Create Rasterizer States
    D3D11_RASTERIZER_DESC rsDesc;
    rsDesc.FillMode              = D3D11_FILL_SOLID;
    rsDesc.CullMode              = D3D11_CULL_NONE;
    rsDesc.FrontCounterClockwise = FALSE;
    rsDesc.DepthBias             = 0;
    rsDesc.DepthBiasClamp        = 0.0f;
    rsDesc.SlopeScaledDepthBias  = 0.0f;
    rsDesc.DepthClipEnable       = TRUE;
    rsDesc.ScissorEnable         = FALSE;
    rsDesc.MultisampleEnable     = FALSE;
    rsDesc.AntialiasedLineEnable = FALSE;

    result = device->CreateRasterizerState(&rsDesc, mScissorDisabledRasterizerState.GetAddressOf());
    ASSERT(SUCCEEDED(result));
    d3d11::SetDebugName(mScissorDisabledRasterizerState,
                        "Clear11 Rasterizer State with scissor disabled");

    rsDesc.ScissorEnable = TRUE;
    result = device->CreateRasterizerState(&rsDesc, mScissorEnabledRasterizerState.GetAddressOf());
    ASSERT(SUCCEEDED(result));
    d3d11::SetDebugName(mScissorEnabledRasterizerState,
                        "Clear11 Rasterizer State with scissor enabled");

    // Initialize BlendState with defaults
    mBlendStateKey.blendState.blend                 = false;
    mBlendStateKey.blendState.sourceBlendRGB        = GL_ONE;
    mBlendStateKey.blendState.destBlendRGB          = GL_ZERO;
    mBlendStateKey.blendState.sourceBlendAlpha      = GL_ONE;
    mBlendStateKey.blendState.destBlendAlpha        = GL_ZERO;
    mBlendStateKey.blendState.blendEquationRGB      = GL_FUNC_ADD;
    mBlendStateKey.blendState.blendEquationAlpha    = GL_FUNC_ADD;
    mBlendStateKey.blendState.colorMaskRed          = true;
    mBlendStateKey.blendState.colorMaskBlue         = true;
    mBlendStateKey.blendState.colorMaskGreen        = true;
    mBlendStateKey.blendState.colorMaskAlpha        = true;
    mBlendStateKey.blendState.sampleAlphaToCoverage = false;
    mBlendStateKey.blendState.dither                = false;

    // Create Shaders

    if (mRenderer->getRenderer11DeviceCaps().featureLevel <= D3D_FEATURE_LEVEL_9_3)
    {
        mFloatClearShader =
            new ClearShader(DXGI_FORMAT_R32G32B32A32_FLOAT, "Clear11 Float IL", g_VS_ClearFloat,
                            ArraySize(g_VS_ClearFloat), "Clear11 Float VS", g_PS_ClearFloat_FL9,
                            ArraySize(g_PS_ClearFloat_FL9), "Clear11 Float PS");
    }
    else
    {
        mFloatClearShader =
            new ClearShader(DXGI_FORMAT_R32G32B32A32_FLOAT, "Clear11 Float IL", g_VS_ClearFloat,
                            ArraySize(g_VS_ClearFloat), "Clear11 Float VS", g_PS_ClearFloat,
                            ArraySize(g_PS_ClearFloat), "Clear11 Float PS");
    }

    if (renderer->isES3Capable())
    {
        mUintClearShader =
            new ClearShader(DXGI_FORMAT_R32G32B32A32_UINT, "Clear11 UINT IL", g_VS_ClearUint,
                            ArraySize(g_VS_ClearUint), "Clear11 UINT VS", g_PS_ClearUint,
                            ArraySize(g_PS_ClearUint), "Clear11 UINT PS");
        mIntClearShader =
            new ClearShader(DXGI_FORMAT_R32G32B32A32_UINT, "Clear11 SINT IL", g_VS_ClearSint,
                            ArraySize(g_VS_ClearSint), "Clear11 SINT VS", g_PS_ClearSint,
                            ArraySize(g_PS_ClearSint), "Clear11 SINT PS");
    }
}

Clear11::~Clear11()
{
    SafeDelete(mFloatClearShader);
    SafeDelete(mUintClearShader);
    SafeDelete(mIntClearShader);

    for (ClearDepthStencilStateMap::iterator i = mClearDepthStencilStates.begin();
         i != mClearDepthStencilStates.end(); i++)
    {
        SafeRelease(i->second);
    }
    mClearDepthStencilStates.clear();
}

gl::Error Clear11::clearFramebuffer(const ClearParameters &clearParams,
                                    const gl::FramebufferState &fboData)
{
    const auto &colorAttachments  = fboData.getColorAttachments();
    const auto &drawBufferStates  = fboData.getDrawBufferStates();
    const gl::FramebufferAttachment *depthAttachment   = fboData.getDepthAttachment();
    const gl::FramebufferAttachment *stencilAttachment = fboData.getStencilAttachment();
    const gl::FramebufferAttachment *depthStencilAttachment =
        (depthAttachment != nullptr) ? depthAttachment : stencilAttachment;
    RenderTarget11 *depthStencilRenderTarget = nullptr;

    ASSERT(colorAttachments.size() == drawBufferStates.size());

    if (clearParams.clearDepth || clearParams.clearStencil)
    {
        ASSERT(depthStencilAttachment != nullptr);
        ANGLE_TRY(depthStencilAttachment->getRenderTarget(&depthStencilRenderTarget));
    }

    // Iterate over the color buffers which require clearing and determine if they can be
    // cleared with ID3D11DeviceContext::ClearRenderTargetView or ID3D11DeviceContext1::ClearView.
    // This requires:
    // 1) The render target is being cleared to a float value (will be cast to integer when clearing
    // integer render targets as expected but does not work the other way around)
    // 2) The format of the render target has no color channels that are currently masked out.
    // Clear the easy-to-clear buffers on the spot and accumulate the ones that require special
    // work.
    //
    // If these conditions are met, and:
    // - No scissored clear is needed, then clear using ID3D11DeviceContext::ClearRenderTargetView.
    // - A scissored clear is needed then clear using ID3D11DeviceContext1::ClearView if available.
    // Otherwise draw a quad.
    //
    // Also determine if the DSV can be cleared withID3D11DeviceContext::ClearDepthStencilView by
    // checking if the stencil write mask covers the entire stencil.
    //
    // To clear the remaining buffers, quads must be drawn with vertices containing a float, uint or
    // int vertex color attribute.

    gl::Extents framebufferSize;

    if (depthStencilRenderTarget != nullptr)
    {
        framebufferSize = depthStencilRenderTarget->getExtents();
    }
    else
    {
        const auto colorAttachment = fboData.getFirstColorAttachment();

        if (!colorAttachment)
        {
            UNREACHABLE();
            return gl::Error(GL_INVALID_OPERATION);
        }

        framebufferSize = colorAttachment->getSize();
    }

    bool needScissoredClear = false;

    if (clearParams.scissorEnabled)
    {
        if (clearParams.scissor.x >= framebufferSize.width ||
            clearParams.scissor.y >= framebufferSize.height ||
            clearParams.scissor.x + clearParams.scissor.width <= 0 ||
            clearParams.scissor.y + clearParams.scissor.height <= 0)
        {
            // Scissor is enabled and the scissor rectangle is outside the renderbuffer
            return gl::NoError();
        }

        needScissoredClear =
            clearParams.scissor.x > 0 || clearParams.scissor.y > 0 ||
            clearParams.scissor.x + clearParams.scissor.width < framebufferSize.width ||
            clearParams.scissor.y + clearParams.scissor.height < framebufferSize.height;
    }

    std::array<ID3D11RenderTargetView *, D3D11_SIMULTANEOUS_RENDER_TARGET_COUNT> rtvs;
    std::array<uint8_t, D3D11_SIMULTANEOUS_RENDER_TARGET_COUNT> rtvMasks = {0};
    ID3D11DepthStencilView *dsv = nullptr;
    uint32_t numRtvs            = 0;

    ID3D11DeviceContext *deviceContext   = mRenderer->getDeviceContext();
    ID3D11DeviceContext1 *deviceContext1 = mRenderer->getDeviceContext1IfSupported();
    ID3D11Device *device                 = mRenderer->getDevice();
    const uint8_t inputMask =
        gl_d3d11::ConvertColorMask(clearParams.colorMaskRed, clearParams.colorMaskGreen,
                                   clearParams.colorMaskBlue, clearParams.colorMaskAlpha);

    // DEBUG(Shahmeer): Copy to stack var so it can be modified
    gl::ColorF clearColorF = clearParams.colorFClearValue;

    for (size_t colorAttachmentIndex = 0; colorAttachmentIndex < colorAttachments.size();
         colorAttachmentIndex++)
    {
        const gl::FramebufferAttachment &attachment = colorAttachments[colorAttachmentIndex];

        if (clearParams.clearColor[colorAttachmentIndex] && attachment.isAttached() &&
            drawBufferStates[colorAttachmentIndex] != GL_NONE)
        {
            RenderTarget11 *renderTarget = nullptr;
            ANGLE_TRY(attachment.getRenderTarget(&renderTarget));

            const gl::InternalFormat &formatInfo = *attachment.getFormat().info;

            if (clearParams.colorClearType == GL_FLOAT &&
                !(formatInfo.componentType == GL_FLOAT ||
                  formatInfo.componentType == GL_UNSIGNED_NORMALIZED ||
                  formatInfo.componentType == GL_SIGNED_NORMALIZED))
            {
                ERR() << "It is undefined behaviour to clear a render buffer which is not "
                         "normalized fixed point or floating-point to floating point values (color "
                         "attachment "
                      << colorAttachmentIndex << " has internal format " << attachment.getFormat()
                      << ").";
            }

            if ((formatInfo.redBits == 0 || !clearParams.colorMaskRed) &&
                (formatInfo.greenBits == 0 || !clearParams.colorMaskGreen) &&
                (formatInfo.blueBits == 0 || !clearParams.colorMaskBlue) &&
                (formatInfo.alphaBits == 0 || !clearParams.colorMaskAlpha))
            {
                // Every channel either does not exist in the render target or is masked out
                continue;
            }

            ID3D11RenderTargetView *framebufferRTV = renderTarget->getRenderTargetView();
            if (!framebufferRTV)
            {
                return gl::Error(GL_OUT_OF_MEMORY,
                                 "Internal render target view pointer unexpectedly null.");
            }

            const auto &nativeFormat = renderTarget->getFormatSet().format();

            if ((!(mRenderer->getRenderer11DeviceCaps().supportsClearView) && needScissoredClear) ||
                clearParams.colorClearType != GL_FLOAT ||
                (formatInfo.redBits > 0 && !clearParams.colorMaskRed) ||
                (formatInfo.greenBits > 0 && !clearParams.colorMaskGreen) ||
                (formatInfo.blueBits > 0 && !clearParams.colorMaskBlue) ||
                (formatInfo.alphaBits > 0 && !clearParams.colorMaskAlpha)
                // DEBUG(Shahmeer): Force all clears to use masked clear path to repro issue
                //|| 1
                )
            {
                // A masked clear is required, or a scissored clear is required and
                // ID3D11DeviceContext1::ClearView is unavailable
                rtvs[numRtvs]     = framebufferRTV;
                rtvMasks[numRtvs] = gl_d3d11::GetColorMask(&formatInfo) & inputMask;
                numRtvs++;

                // DEBUG(Shahmeer): Explicitly round alpha value. Validates cause of B5G5R5A1 issue.
                // Note: May cause issues for MRT clear scenarios
                /*
                if (formatInfo.alphaBits == 1 && nativeFormat.alphaBits > 1)
                {
                    clearColorF.alpha = (clearColorF.alpha >= 0.5f ? 1.0f : 0.0f);
                }
                */
            }
            else
            {
                // ID3D11DeviceContext::ClearRenderTargetView or ID3D11DeviceContext1::ClearView is
                // possible

                // Check if the actual format has a channel that the internal format does not and
                // set them to the default values

                float clearValues[4] = {
                    ((formatInfo.redBits == 0 && nativeFormat.redBits > 0)
                         ? 0.0f
                         : clearParams.colorFClearValue.red),
                    ((formatInfo.greenBits == 0 && nativeFormat.greenBits > 0)
                         ? 0.0f
                         : clearParams.colorFClearValue.green),
                    ((formatInfo.blueBits == 0 && nativeFormat.blueBits > 0)
                         ? 0.0f
                         : clearParams.colorFClearValue.blue),
                    ((formatInfo.alphaBits == 0 && nativeFormat.alphaBits > 0)
                         ? 1.0f
                         : clearParams.colorFClearValue.alpha),
                };

                if (formatInfo.alphaBits == 1)
                {
                    // Some drivers do not correctly handle calling Clear() on a format with 1-bit
                    // alpha. They can incorrectly round all non-zero values up to 1.0f. Note that
                    // WARP does not do this. We should handle the rounding for them instead.
                    clearValues[3] = (clearParams.colorFClearValue.alpha >= 0.5f) ? 1.0f : 0.0f;
                }

                if (needScissoredClear)
                {
                    // We shouldn't reach here if deviceContext1 is unavailable.
                    ASSERT(deviceContext1);

                    D3D11_RECT rect;
                    rect.left   = clearParams.scissor.x;
                    rect.right  = clearParams.scissor.x + clearParams.scissor.width;
                    rect.top    = clearParams.scissor.y;
                    rect.bottom = clearParams.scissor.y + clearParams.scissor.height;

                    deviceContext1->ClearView(framebufferRTV, clearValues, &rect, 1);
                    if (mRenderer->getWorkarounds().callClearTwiceOnSmallTarget)
                    {
                        if (clearParams.scissor.width <= 16 || clearParams.scissor.height <= 16)
                        {
                            deviceContext1->ClearView(framebufferRTV, clearValues, &rect, 1);
                        }
                    }
                }
                else
                {
                    deviceContext->ClearRenderTargetView(framebufferRTV, clearValues);

                    if (mRenderer->getWorkarounds().callClearTwiceOnSmallTarget)
                    {
                        if (framebufferSize.width <= 16 || framebufferSize.height <= 16)
                        {
                            deviceContext->ClearRenderTargetView(framebufferRTV, clearValues);
                        }
                    }
                }
            }
        }
    }

    if (depthStencilRenderTarget)
    {
        const auto &nativeFormat = depthStencilRenderTarget->getFormatSet().format();

        uint32_t stencilUnmasked =
            (stencilAttachment != nullptr) ? (1 << nativeFormat.stencilBits) - 1 : 0;
        bool needMaskedStencilClear =
            clearParams.clearStencil &&
            (clearParams.stencilWriteMask & stencilUnmasked) != stencilUnmasked;

        dsv = depthStencilRenderTarget->getDepthStencilView();

        if (!dsv)
        {
            return gl::Error(GL_OUT_OF_MEMORY,
                             "Internal depth stencil view pointer unexpectedly null.");
        }

        if (!needScissoredClear && !needMaskedStencilClear)
        {
            uint32_t clearFlags = (clearParams.clearDepth ? D3D11_CLEAR_DEPTH : 0) |
                                  (clearParams.clearStencil ? D3D11_CLEAR_STENCIL : 0);
            FLOAT depthClear   = gl::clamp01(clearParams.depthClearValue);
            uint8_t stencilClear = clearParams.stencilClearValue & 0xFF;

            deviceContext->ClearDepthStencilView(dsv, clearFlags, depthClear, stencilClear);

            dsv = nullptr;
        }
    }

    if (numRtvs == 0 && dsv == nullptr)
    {
        return gl::NoError();
    }

    // Clear the remaining render targets and depth stencil in one pass by rendering a quad:
    //
    // IA/VS: Vertices containing position and color members are passed through to the next stage.
    // The vertex position has XY coordinates equal to clip extents and a Z component equal to the
    // Z clear value. The vertex color contains the clear color.
    //
    // Rasterizer: Viewport scales the VS output over the entire surface and depending on whether
    // or not scissoring is enabled the appropriate scissor rect and rasterizerState with or without
    // the scissor test enabled is set as well.
    //
    // DepthStencilTest: DepthTesting, DepthWrites, StencilMask and StencilWrites will be enabled or
    // disabled or set depending on what the input depthStencil clear parameters are. Since the PS
    // is not writing out depth or rejecting pixels, this should happen prior to the PS stage.
    //
    // PS: Will write out the color values passed through from the previous stage to all outputs.
    //
    // OM: BlendState will perform the required color masking and output to RTV(s).

    //
    // ======================================================================================
    //
    // Luckily, the gl spec (ES 3.0.2 pg 183) states that the results of clearing a render-
    // buffer that is not normalized fixed point or floating point with floating point values
    // are undefined so we can just write floats to them and D3D11 will bit cast them to
    // integers.
    //
    // Also, we don't have to worry about attempting to clear a normalized fixed/floating point
    // buffer with integer values because there is no gl API call which would allow it,
    // glClearBuffer* calls only clear a single renderbuffer at a time which is verified to
    // be a compatible clear type.

    ASSERT(numRtvs <= mRenderer->getNativeCaps().maxDrawBuffers);

    const uint32_t sampleMask    = 0xFFFFFFFF;
    ID3D11BlendState *blendState = nullptr;

    if (numRtvs)
    {
        // Setup BlendStateKey parameters
        mBlendStateKey.blendState.colorMaskRed   = clearParams.colorMaskRed;
        mBlendStateKey.blendState.colorMaskGreen = clearParams.colorMaskGreen;
        mBlendStateKey.blendState.colorMaskBlue  = clearParams.colorMaskBlue;
        mBlendStateKey.blendState.colorMaskAlpha = clearParams.colorMaskAlpha;
        mBlendStateKey.mrt                       = numRtvs > 1;
        memcpy(mBlendStateKey.rtvMasks, &rtvMasks[0], sizeof(rtvMasks));

        ANGLE_TRY(mRenderer->getStateCache().getBlendState(mBlendStateKey, &blendState));
    }

    const uint32_t stencilClearValue = clearParams.stencilClearValue & 0xFF;
    ID3D11DepthStencilState *dsState = nullptr;
    float depthClearValue            = 0.0f;

    if (dsv)
    {
        dsState         = getDepthStencilState(clearParams);
        depthClearValue = clearParams.clearDepth ? gl::clamp01(clearParams.depthClearValue) : 0.0f;
    }

    // Get the appropriate shader and update the vertices if necessary
    const uint32_t startIdx = 0;
    ClearShader *shader     = nullptr;
    bool dirtyVertices      = false;

    switch (clearParams.colorClearType)
    {
        case GL_FLOAT:
            dirtyVertices = UpdateVertices(clearColorF, depthClearValue, &mCachedVertices[0].f32);
            shader       = mFloatClearShader;
            break;
        case GL_UNSIGNED_INT:
            dirtyVertices = UpdateVertices(clearParams.colorUIClearValue, depthClearValue,
                                           &mCachedVertices[0].u32);
            shader       = mUintClearShader;
            break;
        case GL_INT:
            dirtyVertices = UpdateVertices(clearParams.colorIClearValue, depthClearValue,
                                           &mCachedVertices[0].i32);
            shader       = mIntClearShader;
            break;
        default:
            UNREACHABLE();
            break;
    }

    if (dirtyVertices)
    {
        // TODO(Shahmeer): Consider using UpdateSubresource1 D3D11_COPY_DISCARD where possible.
        D3D11_MAPPED_SUBRESOURCE mappedResource;
        HRESULT result =
            deviceContext->Map(mVertexBuffer.Get(), 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedResource);
        if (FAILED(result))
        {
            return gl::OutOfMemory() << "Failed to map vertex buffer for shader clear: "
                                     << result;
        }

        memcpy(mappedResource.pData, mCachedVertices, mVertexCacheSize);
        deviceContext->Unmap(mVertexBuffer.Get(), 0);
    }

    // Set the viewport to be the same size as the framebuffer
    D3D11_VIEWPORT viewport;
    viewport.TopLeftX = 0;
    viewport.TopLeftY = 0;
    viewport.Width    = static_cast<FLOAT>(framebufferSize.width);
    viewport.Height   = static_cast<FLOAT>(framebufferSize.height);
    viewport.MinDepth = 0;
    viewport.MaxDepth = 1;
    deviceContext->RSSetViewports(1, &viewport);

    // Apply state
    deviceContext->OMSetBlendState(blendState, nullptr, sampleMask);
    deviceContext->OMSetDepthStencilState(dsState, stencilClearValue);

    if (needScissoredClear)
    {
        const D3D11_RECT scissorRect = {clearParams.scissor.x, clearParams.scissor.y,
                                        clearParams.scissor.x1(), clearParams.scissor.y1()};
        deviceContext->RSSetScissorRects(1, &scissorRect);
        deviceContext->RSSetState(mScissorEnabledRasterizerState.Get());
    }
    else
    {
        deviceContext->RSSetState(mScissorDisabledRasterizerState.Get());
    }

    // Apply shaders
    deviceContext->IASetInputLayout(shader->inputLayout->resolve(device));
    deviceContext->VSSetShader(shader->vertexShader.resolve(device), nullptr, 0);
    deviceContext->PSSetShader(shader->pixelShader.resolve(device), nullptr, 0);
    deviceContext->GSSetShader(nullptr, nullptr, 0);

    // Apply vertex buffer
    deviceContext->IASetVertexBuffers(0, 1, mVertexBuffer.GetAddressOf(), &mVertexSize, &startIdx);
    deviceContext->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLESTRIP);

    // Apply render targets
    deviceContext->OMSetRenderTargets(numRtvs, numRtvs ? &rtvs[0] : nullptr, dsv);

    // Draw the quad
    deviceContext->Draw(4, 0);

    // Clean up
    mRenderer->markAllStateDirty();

    return gl::NoError();
}

ID3D11DepthStencilState *Clear11::getDepthStencilState(const ClearParameters &clearParams)
{
    ClearDepthStencilInfo dsKey = {0};
    dsKey.clearDepth            = clearParams.clearDepth;
    dsKey.clearStencil          = clearParams.clearStencil;
    dsKey.stencilWriteMask      = clearParams.stencilWriteMask & 0xFF;

    ClearDepthStencilStateMap::const_iterator i = mClearDepthStencilStates.find(dsKey);
    if (i != mClearDepthStencilStates.end())
    {
        return i->second;
    }
    else
    {
        D3D11_DEPTH_STENCIL_DESC dsDesc = {0};
        dsDesc.DepthEnable              = dsKey.clearDepth ? TRUE : FALSE;
        dsDesc.DepthWriteMask =
            dsKey.clearDepth ? D3D11_DEPTH_WRITE_MASK_ALL : D3D11_DEPTH_WRITE_MASK_ZERO;
        dsDesc.DepthFunc                    = D3D11_COMPARISON_ALWAYS;
        dsDesc.StencilEnable                = dsKey.clearStencil ? TRUE : FALSE;
        dsDesc.StencilReadMask              = 0;
        dsDesc.StencilWriteMask             = dsKey.stencilWriteMask;
        dsDesc.FrontFace.StencilFailOp      = D3D11_STENCIL_OP_REPLACE;
        dsDesc.FrontFace.StencilDepthFailOp = D3D11_STENCIL_OP_REPLACE;
        dsDesc.FrontFace.StencilPassOp      = D3D11_STENCIL_OP_REPLACE;
        dsDesc.FrontFace.StencilFunc        = D3D11_COMPARISON_ALWAYS;
        dsDesc.BackFace.StencilFailOp       = D3D11_STENCIL_OP_REPLACE;
        dsDesc.BackFace.StencilDepthFailOp  = D3D11_STENCIL_OP_REPLACE;
        dsDesc.BackFace.StencilPassOp       = D3D11_STENCIL_OP_REPLACE;
        dsDesc.BackFace.StencilFunc         = D3D11_COMPARISON_ALWAYS;

        ID3D11Device *device             = mRenderer->getDevice();
        ID3D11DepthStencilState *dsState = nullptr;
        HRESULT result                   = device->CreateDepthStencilState(&dsDesc, &dsState);
        if (FAILED(result) || !dsState)
        {
            ERR() << "Unable to create a ID3D11DepthStencilState, " << gl::FmtHR(result) << ".";
            return nullptr;
        }

        mClearDepthStencilStates[dsKey] = dsState;

        return dsState;
    }
}
}
