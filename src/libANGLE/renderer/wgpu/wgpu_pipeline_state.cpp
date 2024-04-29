//
// Copyright 2024 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//

#include "libANGLE/renderer/wgpu/wgpu_pipeline_state.h"

#include "common/aligned_memory.h"
#include "common/hash_utils.h"
#include "libANGLE/Error.h"
#include "libANGLE/renderer/wgpu/ContextWgpu.h"
#include "libANGLE/renderer/wgpu/wgpu_utils.h"

namespace rx
{
namespace webgpu
{
// GraphicsPipelineDesc implementation.
RenderPipelineDesc::RenderPipelineDesc()
{
    (void)mPadding;
    memset(this, 0, sizeof(RenderPipelineDesc));
}

RenderPipelineDesc::~RenderPipelineDesc() = default;

RenderPipelineDesc::RenderPipelineDesc(const RenderPipelineDesc &other)
{
    *this = other;
}

RenderPipelineDesc &RenderPipelineDesc::operator=(const RenderPipelineDesc &other)
{
    memcpy(this, &other, sizeof(*this));
    return *this;
}

bool RenderPipelineDesc::setPrimitiveMode(gl::PrimitiveMode primitiveMode,
                                          gl::DrawElementsType indexTypeOrInvalid)
{
    bool changed = false;

    wgpu::PrimitiveTopology topology = gl_wgpu::GetPrimitiveTopology(primitiveMode);
    if (mPrimitiveState.topology != static_cast<uint8_t>(topology))
    {
        mPrimitiveState.topology = static_cast<uint8_t>(topology);
        changed                  = true;
    }

    uint8_t indexFormat = webgpu::IsStripPrimitiveTopology(topology)
                              ? PackIndexFormat(gl_wgpu::GetIndexFormat(indexTypeOrInvalid))
                              : 0;
    if (mPrimitiveState.stripIndexFormat != indexFormat)
    {
        mPrimitiveState.stripIndexFormat = indexFormat;
        changed                          = true;
    }

    return changed;
}

void RenderPipelineDesc::setFrontFace(GLenum frontFace)
{
    mPrimitiveState.frontFace = PackFrontFace(gl_wgpu::GetFrontFace(frontFace));
}

void RenderPipelineDesc::setCullMode(gl::CullFaceMode cullMode, bool cullFaceEnabled)
{
    mPrimitiveState.cullMode =
        static_cast<uint8_t>(gl_wgpu::GetCullMode(cullMode, cullFaceEnabled));
}

size_t RenderPipelineDesc::hash() const
{
    return angle::ComputeGenericHash(this, sizeof(*this));
}

angle::Result RenderPipelineDesc::createPipeline(ContextWgpu *context,
                                                 const wgpu::PipelineLayout &pipelineLayout,
                                                 const gl::ShaderMap<wgpu::ShaderModule> &shaders,
                                                 wgpu::RenderPipeline *pipelineOut) const
{
    wgpu::RenderPipelineDescriptor pipelineDesc;
    pipelineDesc.layout = pipelineLayout;

    pipelineDesc.vertex.module        = shaders[gl::ShaderType::Vertex];
    pipelineDesc.vertex.entryPoint    = "main";
    pipelineDesc.vertex.constantCount = 0;
    pipelineDesc.vertex.constants     = nullptr;
    pipelineDesc.vertex.bufferCount   = 0;
    pipelineDesc.vertex.buffers       = nullptr;

    pipelineDesc.primitive.topology =
        static_cast<wgpu::PrimitiveTopology>(mPrimitiveState.topology);
    if (webgpu::IsStripPrimitiveTopology(pipelineDesc.primitive.topology))
    {
        pipelineDesc.primitive.stripIndexFormat =
            UnpackIndexFormat(mPrimitiveState.stripIndexFormat);
    }
    else
    {
        pipelineDesc.primitive.stripIndexFormat = wgpu::IndexFormat::Undefined;
    }
    pipelineDesc.primitive.frontFace = UnpackFrontFace(mPrimitiveState.frontFace);
    pipelineDesc.primitive.cullMode  = static_cast<wgpu::CullMode>(mPrimitiveState.cullMode);

    /*
    std::array<wgpu::VertexAttribute, gl::MAX_VERTEX_ATTRIBS> vertexAttributes;
    size_t attribCount = 0;
    for (size_t attribIndex = 0; attribIndex < gl::MAX_VERTEX_ATTRIBS; ++attribIndex)
    {
        const webgpu::PackedVertexAttribute &packedAttrib = mVertexAttributes[attribIndex];
        wgpu::VertexAttribute &outputAttrib = vertexAttributes[attribIndex];

        outputAttrib.format = static_cast<wgpu::VertexFormat>(packedAttrib.format);
        outputAttrib.offset = packedAttrib.offset;
        outputAttrib.shaderLocation = packedAttrib.shaderLocation;

        if (outputAttrib.format != wgpu::VertexFormat::Undefined)
        {
            attribCount = attribIndex + 1;
        }
    }
    pipelineDesc.vertex.bufferCount = attribCount;
    pipelineDesc.vertex.buffers = vertexAttributes.data();
    */

    wgpu::FragmentState fragmentState;
    std::array<wgpu::ColorTargetState, gl::IMPLEMENTATION_MAX_DRAW_BUFFERS> colorTargets;
    std::array<wgpu::BlendState, gl::IMPLEMENTATION_MAX_DRAW_BUFFERS> blendStates;
    if (shaders[gl::ShaderType::Fragment])
    {
        fragmentState.module        = shaders[gl::ShaderType::Fragment];
        fragmentState.entryPoint    = "main";
        fragmentState.constantCount = 0;
        fragmentState.constants     = nullptr;

        size_t colorTargetCount = 0;
        for (size_t colorTargetIndex = 0; colorTargetIndex < gl::IMPLEMENTATION_MAX_DRAW_BUFFERS;
             ++colorTargetIndex)
        {
            const webgpu::PackedColorTargetState &packedColorTarget =
                mColorTargetStates[colorTargetIndex];
            wgpu::ColorTargetState &outputColorTarget = colorTargets[colorTargetIndex];

            outputColorTarget.format = static_cast<wgpu::TextureFormat>(packedColorTarget.format);
            if (packedColorTarget.blendEnabled)
            {
                blendStates[colorTargetIndex].color.srcFactor =
                    static_cast<wgpu::BlendFactor>(packedColorTarget.colorBlendSrcFactor);
                blendStates[colorTargetIndex].color.dstFactor =
                    static_cast<wgpu::BlendFactor>(packedColorTarget.colorBlendDstFactor);
                blendStates[colorTargetIndex].color.operation =
                    static_cast<wgpu::BlendOperation>(packedColorTarget.colorBlendOp);

                blendStates[colorTargetIndex].alpha.srcFactor =
                    static_cast<wgpu::BlendFactor>(packedColorTarget.alphaBlendSrcFactor);
                blendStates[colorTargetIndex].alpha.dstFactor =
                    static_cast<wgpu::BlendFactor>(packedColorTarget.alphaBlendDstFactor);
                blendStates[colorTargetIndex].alpha.operation =
                    static_cast<wgpu::BlendOperation>(packedColorTarget.alphaBlendOp);
            }

            outputColorTarget.writeMask =
                static_cast<wgpu::ColorWriteMask>(packedColorTarget.writeMask);

            if (outputColorTarget.format != wgpu::TextureFormat::Undefined)
            {
                colorTargetCount = colorTargetIndex + 1;
            }
        }
        fragmentState.targetCount = colorTargetCount;
        fragmentState.targets     = colorTargets.data();

        pipelineDesc.fragment = &fragmentState;
    }

    wgpu::Device device = context->getDevice();
    *pipelineOut        = device.CreateRenderPipeline(&pipelineDesc);

    return angle::Result::Continue;
}

bool operator==(const RenderPipelineDesc &lhs, const RenderPipelineDesc &rhs)
{
    return memcmp(&lhs, &rhs, sizeof(RenderPipelineDesc)) == 0;
}

// PipelineCache implementation.
PipelineCache::PipelineCache()  = default;
PipelineCache::~PipelineCache() = default;

angle::Result PipelineCache::getRenderPipeline(ContextWgpu *context,
                                               const RenderPipelineDesc &desc,
                                               const wgpu::PipelineLayout &pipelineLayout,
                                               const gl::ShaderMap<wgpu::ShaderModule> &shaders,
                                               wgpu::RenderPipeline *pipelineOut)
{
    auto iter = mRenderPipelines.find(desc);
    if (iter != mRenderPipelines.end())
    {
        *pipelineOut = iter->second;
        return angle::Result::Continue;
    }

    ANGLE_TRY(desc.createPipeline(context, pipelineLayout, shaders, pipelineOut));
    mRenderPipelines.insert(std::make_pair(desc, *pipelineOut));

    return angle::Result::Continue;
}

}  // namespace webgpu

}  // namespace rx
